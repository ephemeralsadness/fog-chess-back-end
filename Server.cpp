#include "Server.h"

#include <boost/algorithm/string.hpp>

unsigned int MASK_OFF = 0xFFFFFFFE;

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


std::map<char, ColoredFigure> char_to_figure_2 = {
        {'P', {Color::WHITE, Figure::PAWN}},
        {'N', {Color::WHITE, Figure::KNIGHT}},
        {'B', {Color::WHITE, Figure::BISHOP}},
        {'R', {Color::WHITE, Figure::ROOK}},
        {'Q', {Color::WHITE, Figure::QUEEN}},
        {'K', {Color::WHITE, Figure::KING}},
        {'p', {Color::BLACK, Figure::PAWN}},
        {'n', {Color::BLACK, Figure::KNIGHT}},
        {'b', {Color::BLACK, Figure::BISHOP}},
        {'r', {Color::BLACK, Figure::ROOK}},
        {'q', {Color::BLACK, Figure::QUEEN}},
        {'k', {Color::BLACK, Figure::KING}},
};

std::string Server::HandleRequest(const std::string &request) {
    std::cout << request << std::endl;

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
            } else {
                return "-";
            }
            games.emplace(lobby_id, Game(lobby_id, lobby_id + 1));
            std::ostringstream output;
            return std::to_string(lobby_id + 1);
        } else if (boost::iequals(what, "CREATE")) {
            std::lock_guard<std::mutex> guard(lobbies_mutex);
            unsigned int lobby_id = id;
            {
                std::lock_guard guard(id_mutex);
                id += 2;
            }
            std::string nickname; stream >> nickname;
            lobbies.emplace(lobby_id, nickname);
            return std::to_string(lobby_id);
        } else if (boost::iequals(what, "REFRESH")) {
            unsigned int lobby_id; stream >> lobby_id;
            if (lobbies.count(lobby_id)) {
                return "-";
            }

            return std::to_string(lobby_id);
        } else if (boost::iequals(what, "DELETE")) {
            unsigned int lobby_id; stream >> lobby_id;
            if (lobbies.count(lobby_id))
                lobbies.erase(lobby_id);

            return "";
        }

    } else if (boost::iequals(method, "GAME")) {
        std::string what;
        stream >> what;

        if (boost::iequals(what, "BOARD")) {
            unsigned int game_id; stream >> game_id;
            unsigned int lobby_id = game_id & MASK_OFF;
            if (!games.count(lobby_id)) {
                return "-";
            }

            Game& game = games.at(lobby_id);
            Color player_color = (game_id & 1 ? Color::BLACK : Color::WHITE);

            return game.GetChessboard().GetFOWFen(player_color);
        } else if (boost::iequals(what, "MOVE")) {
            unsigned int game_id; stream >> game_id;
            unsigned int lobby_id = game_id & MASK_OFF;
            if (!games.count(lobby_id)) {
                return "-";
            }

            Game& game = games.at(lobby_id);
            Color player_color = (game_id & 1 ? Color::BLACK : Color::WHITE);

            // from to figure
            std::string from, to, figure;
            stream >> from >> to >> figure;

            bool success;
            if (figure == "-") {
                success = game.GetChessboard().MakeMove(
                        Coords(from[1] - '1', from[0] - 'A'),
                        Coords(to[1] - '1', to[0] - 'A'));
            } else {
                if (!char_to_figure_2.count(figure[0])) {
                    return "-";
                }
                Figure fig = char_to_figure_2.at(figure[0]).figure;
                success = game.GetChessboard().MakeMove(
                        Coords(from[1] - '1', from[0] - 'A'),
                        Coords(to[1] - '1', to[0] - 'A'), fig);
            }

            return success ? "+" : "-";
        } else if (boost::iequals(what, "RESULT")) {
            unsigned int lobby_id; stream >> lobby_id;
            lobby_id &= MASK_OFF;
            if (!games.count(lobby_id)) {
                return "-";
            }

            switch (games.at(lobby_id).GetChessboard().result_cache) {
                case Result::IN_PROGRESS:
                    return "0";
                case Result::DRAW:
                    return "1";
                case Result::WHITE_WIN:
                    return "2";
                case Result::BLACK_WIN:
                    return "3";
            }
            return "-";
        }
    }

    return "0";
}


