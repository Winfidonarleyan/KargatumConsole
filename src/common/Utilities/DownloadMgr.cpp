/*
 * This file is part of the WarheadApp Project. See AUTHORS file for Copyright information
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include "DownloadMgr.h"
#include "ProgressBar.h"
#include "Log.h"
#include "StringConvert.h"
#include "IoContext.h"
#include "StopWatch.h"
#include <boost/asio/io_context.hpp>
#include <boost/asio/streambuf.hpp>
#include <boost/asio/read.hpp>
#include <boost/asio/ssl/context.hpp>
#include <boost/asio/ssl/stream.hpp>
#include <boost/asio/connect.hpp>
#include <boost/beast/http/string_body.hpp>
#include <boost/beast/http/read.hpp>
#include <boost/beast/http/write.hpp>
#include <filesystem>
#include <iostream>
#include <fstream>

using namespace boost::beast;
using namespace boost::asio;

using HTTPRequest = http::request<http::string_body>;

DownloadMgr::DownloadMgr()
{
    _ioContext = std::make_unique<Warhead::Asio::IoContext>();
    _ioContext->run();
}

DownloadMgr::~DownloadMgr()
{
    if (_ioContext)
        _ioContext->stop();
}

DownloadMgr* DownloadMgr::instance()
{
    static DownloadMgr instance;
    return &instance;
}

bool DownloadMgr::Connect(std::string const& host, std::string const& port)
{
    boost::system::error_code err;

    ssl::context ctx(ssl::context::sslv23_client);
    _socket = std::make_unique<ssl::stream<ip::tcp::socket>>(_ioContext->get_executor().context(), ctx);

    ip::tcp::resolver resolver(_ioContext->get_executor().context());
    auto it = resolver.resolve(host, port, err);

    if (err)
    {
        LOG_ERROR("Error at resolve: {}", err.message());
        return false;
    }

    connect(_socket->lowest_layer(), it, err);

    if (err)
    {
        LOG_ERROR("Error at connect: {}", err.message());
        return false;
    }

    _socket->handshake(ssl::stream_base::handshake_type::client, err);

    if (err)
    {
        LOG_ERROR("Error at handshake: {}", err.message());
        return false;
    }

    return true;
}

void DownloadMgr::Shutdown()
{
    if (_socket)
    {
        boost::system::error_code error;

        _socket->lowest_layer().close(error);

        if (error)
            LOG_ERROR("Error at shutdown: {}", error.message());

        _socket.reset();
    }

    _responce.reset();
    _flatBuffer.reset();
}

bool DownloadMgr::SendDefaultRequest(std::string const& host, std::string const& path)
{
    if (!_socket)
        return false;

    boost::system::error_code error;
    HTTPRequest req{ http::verb::get, path, 11 };
    req.set(http::field::host, host);
    req.set(http::field::connection, "close");

    // Send request
    http::write(*_socket, req, error);

    if (error)
        LOG_ERROR("Error send default request: {}", error.message());

    return !error;
}

bool DownloadMgr::SendRequestWithRange(std::string const& host, std::string const& path, std::size_t downloadedSize)
{
    if (!_socket)
        return false;

    HTTPRequest req{ http::verb::get, path, 11 };
    req.set(http::field::host, host);
    req.set(http::field::connection, "close");

    std::string bytesNeed{ "bytes=" };
    bytesNeed.append(Warhead::ToString(downloadedSize));
    bytesNeed.append("-");

    req.set(http::field::range, bytesNeed);

    // Send request
    boost::system::error_code error;
    http::write(*_socket, req, error);

    if (error)
        LOG_ERROR("Error send request with range: {}", error.message());

    return !error;
}

void DownloadMgr::GetRequest()
{
    boost::system::error_code error;
    _responce = std::make_unique<HTTPResponceParser>();
    _responce->body_limit(1024LL * 1024LL * 1024LL * 3LL); // 3GB

    _flatBuffer = std::make_unique<FlatBuffer>();

    // Get request
    http::read_header(*_socket, *_flatBuffer, *_responce, error);

    if (error)
        LOG_ERROR("Error at get request: {}", error.message());
}

bool DownloadMgr::WriteDataToFile(std::filesystem::path& pathToSave, std::size_t downloadedSize /*= 0*/)
{
    // Read the response status line. The response streambuf will automatically
    // grow to accommodate the entire line. The growth may be limited by passing
    // a maximum size to the streambuf constructor.
    boost::asio::streambuf response;

    auto openMode = downloadedSize ? std::ofstream::out | std::ofstream::binary | std::ofstream::app :
        std::ofstream::out | std::ofstream::binary;

    std::ofstream outFile(pathToSave.generic_string(), openMode);

    auto AddDataToFile = [&outFile](std::string_view data)
    {
        outFile.write(data.data(), data.size());
    };

    std::size_t totalSize = _responce->content_length().value();

    if (_flatBuffer->size())
    {
        std::string_view str{ reinterpret_cast<const char*>(_flatBuffer->data().data()), _flatBuffer->size() };
        AddDataToFile(str);
        totalSize -= str.size();
    }

    ProgressBar progress("Loading file '" + pathToSave.generic_string() + "'...", totalSize + downloadedSize, downloadedSize);

    boost::system::error_code error;
    while (boost::asio::read(*_socket, response, boost::asio::transfer_at_least(1), error))
    {
        std::string_view readData{ reinterpret_cast<const char*>(response.data().data()), response.size() };

        AddDataToFile(readData);
        response.consume(readData.size());
        progress.Update(readData.size());
        /*progress.UpdateLastMessage(Warhead::StringFormat("Speed {}", readData.size() / sw.Elapsed().count()));
        sw.Reset();*/
    }

    progress.Stop();
    outFile.close();
    Shutdown();

    if (error != boost::asio::error::eof)
    {
        LOG_ERROR("Error: {}", error.message());
        return false;
    }

    return true;
}

bool DownloadMgr::FinishDownloadFile(std::string const& host, std::string const& port, std::string const& path, std::filesystem::path& pathToSave, std::size_t downloadedSize)
{
    if (!Connect(host, port))
        return false;

    SendRequestWithRange(host, path, downloadedSize);
    GetRequest();

    auto contentLength = _responce->content_length();
    if (!contentLength)
    {
        LOG_ERROR("Not found content lenght in responce");
        return false;
    }

    return WriteDataToFile(pathToSave, downloadedSize);
}

bool DownloadMgr::DownloadFile(std::string const& host, std::string const& port, std::string const& path, std::filesystem::path& pathToSave)
{
    if (!Connect(host, port))
        return false;

    if (!SendDefaultRequest(host, path))
        return false;

    GetRequest();

    auto contentLength = _responce->content_length();
    if (!contentLength)
    {
        LOG_ERROR("Not found content lenght in responce");
        return false;
    }

    bool acceptRanges = _responce->get().at(http::field::accept_ranges) == "bytes";

    std::size_t contentSize = contentLength.value();

    LOG_INFO("ContentSize: {} bytes", contentSize);

    auto const& fileName = pathToSave.filename().generic_string();

    if (std::filesystem::exists(pathToSave))
    {
        std::size_t downloadedSize = std::filesystem::file_size(pathToSave);

        LOG_WARN("File '{}' exist. Size {}", fileName, downloadedSize);

        if (downloadedSize == contentSize)
        {
            LOG_INFO("File '{}' is actual", fileName);
            return true;
        }
        else
        {
            if (!downloadedSize)
                LOG_WARN("File '{}' is empty. Need to finish downloading.", fileName);
            else
            {
                float downloadedPercent = static_cast<float>(downloadedSize) / static_cast<float>(contentSize) * 100.0f;
                LOG_WARN("File '{}' is not actual {:0.4}%. Need to finish downloading.", fileName, downloadedPercent);
            }
        }

        if (!acceptRanges)
            LOG_WARN("This file don't support accept ranges. Start download again");
        else
        {
            Shutdown();
            return FinishDownloadFile(host, port, path, pathToSave, downloadedSize);
        }
    }

    return WriteDataToFile(pathToSave);
}
