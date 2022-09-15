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

#ifndef _DOWNLOAD_MGR_H
#define _DOWNLOAD_MGR_H

#include "Define.h"
#include "Optional.h"
#include <filesystem>
#include <memory>
#include <string>
#include <boost/asio/ip/tcp.hpp>
#include <boost/beast/core/flat_buffer.hpp>
#include <boost/beast/http/message.hpp>
#include <boost/beast/http/parser.hpp>
#include <boost/beast/http/buffer_body.hpp>

namespace boost::asio::ssl
{
    template <typename Stream>
    class stream;
}

namespace Warhead::Asio
{
    class IoContext;
}

using HTTPResponceParser = boost::beast::http::response_parser<boost::beast::http::buffer_body>;
using FlatBuffer = boost::beast::flat_buffer;

class DownloadMgr
{
public:
    static DownloadMgr* instance();

    bool Connect(std::string const& host, std::string const& port);
    void Shutdown();
    bool SendDefaultRequest(std::string const& host, std::string const& path);
    bool SendRequestWithRange(std::string const& host, std::string const& path, std::size_t downloadedSize);
    void GetRequest();
    bool WriteDataToFile(std::filesystem::path& pathToSave, std::size_t downloadedSize = 0);
    bool FinishDownloadFile(std::string const& host, std::string const& port, std::string const& path, std::filesystem::path& pathToSave, std::size_t downloadedSize);
    bool DownloadFile(std::string const& host, std::string const& port, std::string const& path, std::filesystem::path& pathToSave);

private:
    std::unique_ptr<Warhead::Asio::IoContext> _ioContext;
    std::unique_ptr<boost::asio::ssl::stream<boost::asio::ip::tcp::socket>> _socket;
    std::unique_ptr<HTTPResponceParser> _responce;
    std::unique_ptr<FlatBuffer> _flatBuffer;

    DownloadMgr(DownloadMgr const&) = delete;
    DownloadMgr(DownloadMgr&&) = delete;
    DownloadMgr& operator=(DownloadMgr const&) = delete;
    DownloadMgr& operator=(DownloadMgr&&) = delete;

    DownloadMgr();
    ~DownloadMgr();
};

#define sDownloadMgr DownloadMgr::instance()

#endif
