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

#include "Common.h"
#include "Log.h"
#include "GitRevision.h"
#include "StopWatch.h"
#include "Resolver.h"
#include "IoContext.h"
#include "IpAddress.h"
#include <functional>
#include <iostream>

#include <boost/beast.hpp>
#include <boost/beast/http.hpp>

namespace net = boost::asio;
namespace beast = boost::beast;
namespace http = beast::http;
using net::ip::tcp;

int main()
{
    LOG_INFO("> {}", GitRevision::GetFullVersion());
    LOG_INFO("--");

    LOG_INFO("> Start download..."); // https://launcher.wowka.su/patch-ruRU-5.mpq

    try
    {
        net::io_context io;
        tcp::socket s(io);
        s.connect(boost::asio::ip::tcp::endpoint(Warhead::Net::make_address("launcher.wowka.su"), 80));

        {
            http::request<http::empty_body> req;
            req.method(http::verb::get);
            req.target("/patch-ruRU-5.mpq");
            req.version(10);
            http::write(s, req);
        }

        {
            http::request<http::string_body> response;
            beast::flat_buffer buf;
            http::read(s, buf, response);
        }
    }
    catch (const std::exception& e)
    {
        LOG_ERROR("{}", e.what());
    }    

    return 0;
}
