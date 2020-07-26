#pragma once

#include "engine/Game.h"

#include <boost/beast/core.hpp>
#include <boost/beast/websocket.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <cstdlib>
#include <functional>
#include <iostream>
#include <string>
#include <thread>
#include <unordered_map>
#include <mutex>

namespace beast = boost::beast;         // from <boost/beast.hpp>
namespace http = beast::http;           // from <boost/beast/http.hpp>
namespace websocket = beast::websocket; // from <boost/beast/websocket.hpp>
namespace net = boost::asio;            // from <boost/asio.hpp>
using tcp = boost::asio::ip::tcp;       // from <boost/asio/ip/tcp.hpp>

using games_map = std::unordered_map<unsigned int, Game>;
using lobbies_map = std::unordered_map<unsigned int, std::string>;

class Server {
public:
    Server() = default;
    void DoSession(tcp::socket &socket);
    void Run(int argc, char* argv[]);
    std::string HandleRequest(const std::string& request);
private:
    unsigned int id = 0;
    std::mutex id_mutex;
    games_map games;
    std::mutex games_mutex;
    lobbies_map lobbies;
    std::mutex lobbies_mutex;
};
