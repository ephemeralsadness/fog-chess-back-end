#include "Server.h"

#include <boost/algorithm/string.hpp>


void Server::DoSession(tcp::socket &socket) {
    try {
        // Construct the stream by moving in the socket
        websocket::stream<tcp::socket> ws{std::move(socket)};

        // Set a decorator to change the Server of the handshake
        ws.set_option(websocket::stream_base::decorator(
                [](websocket::response_type &res) {
                    res.set(http::field::server,
                            std::string(BOOST_BEAST_VERSION_STRING) +
                            " websocket-server-sync");
                }));

        // Accept the websocket handshake
        ws.accept();

        for (;;) {
            // This buffer will hold the incoming message
            beast::flat_buffer buffer;

            // Read a message
            ws.read(buffer);
            // Echo the message back
            ws.text(ws.got_text());

            auto response = HandleRequest(beast::buffers_to_string(buffer.data()));
            ws.write(net::buffer(std::move(response)));
        }
    } catch (beast::system_error const &se) {
        // This indicates that the session was closed
        if (se.code() != websocket::error::closed)
            std::cerr << "Error: " << se.code().message() << std::endl;
    } catch (std::exception const &e) {
        std::cerr << "Error: " << e.what() << std::endl;
    }
}

void Server::Run(int argc, char *argv[]) {
    try {
        // Check command line arguments.
        if (argc != 3) {
            std::cerr <<
                      "Usage: websocket-server-sync <address> <port>\n" <<
                      "Example:\n" <<
                      "    websocket-server-sync 0.0.0.0 8080\n";
            return;
        }
        auto const address = net::ip::make_address(argv[1]);
        auto const port = static_cast<unsigned short>(std::atoi(argv[2]));

        // The io_context is required for all I/O
        net::io_context ioc{1};

        // The acceptor receives incoming connections
        tcp::acceptor acceptor{ioc, {address, port}};
        for (;;) {
            // This will receive the new connection
            tcp::socket socket{ioc};

            // Block until we get a connection
            acceptor.accept(socket);

            // Launch the session, transferring ownership of the socket
            std::thread{std::bind(
                    [this](tcp::socket &socket) {
                        this->DoSession(socket);
                    },
                    std::move(socket))}.detach();
        }
    } catch (const std::exception &e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return;
    }
}

std::string Server::HandleRequest(const std::string &request) {
    std::istringstream stream(request);
    std::string method;
    stream >> method;

    if (boost::iequals(method, "GET")) {
        std::string what;
        stream >> what;

        // GET LOBBIES
        if (boost::iequals(what, "LOBBIES")) {
            std::lock_guard<std::mutex> guard(lobbies_mutex);
            std::ostringstream output;

            for (auto&[game_id, player] : lobbies) {
                output << '{' << game_id << ", " << player << "} ";
            }

            return output.str();
        }


        return "Hold this - " + what;
    } else if (boost::iequals(method, "LOBBY")) {
        std::string what;
        stream >> what;

        if (boost::iequals(what, "ENTER")) {
            std::lock_guard<std::mutex> guard(lobbies_mutex);
            unsigned int lobby_id; stream >> lobby_id;
            auto it = lobbies.find(lobby_id);
            if (it != lobbies.end()) {
                lobbies.erase(lobby_id);
            }

            std::ostringstream output;

            return output.str();
        }

    } else if (boost::iequals(method, "GAME")) {

    }

    return "0";
}


