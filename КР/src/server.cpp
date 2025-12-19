#include <zmq.hpp>
#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>
#include <sstream>
#include <random>
#include <algorithm>

struct Game {
    std::string name;
    int max_players;
    std::vector<std::string> players;

    bool started = false;
    bool finished = false;

    std::string secret;
    std::string winner;
};

std::unordered_map<std::string, Game> games;
std::unordered_map<std::string, std::string> player_game;

/* ---------- helpers ---------- */

std::string generate_secret() {
    std::string digits = "0123456789";
    std::shuffle(digits.begin(), digits.end(), std::mt19937{std::random_device{}()});
    return digits.substr(0, 4);
}

std::pair<int,int> bulls_and_cows(const std::string& s, const std::string& g) {
    int bulls = 0, cows = 0;
    for (int i = 0; i < 4; ++i) {
        if (g[i] == s[i]) bulls++;
        else if (s.find(g[i]) != std::string::npos) cows++;
    }
    return {bulls, cows};
}

/* ---------- request handler ---------- */

std::string handle(const std::string& req) {
    std::istringstream iss(req);
    std::string cmd, player;
    iss >> cmd >> player;

    /* ===== CREATE ===== */
    if (cmd == "CREATE") {
        std::string game;
        int maxp;
        iss >> game >> maxp;

        if (games.count(game))
            return "Ошибка: игра с таким именем уже существует";

        // игрок выходит из предыдущей игры
        if (player_game.count(player)) {
            std::string old = player_game[player];
            auto& g_old = games[old];
            g_old.players.erase(
                std::remove(g_old.players.begin(), g_old.players.end(), player),
                g_old.players.end()
            );
            player_game.erase(player);

            if (g_old.players.empty()) {
                std::cout << "[SERVER] Игра \"" << old << "\" закрыта (все игроки вышли)\n";
                games.erase(old);
            }
        }

        Game g;
        g.name = game;
        g.max_players = maxp;
        g.players.push_back(player);

        games[game] = g;
        player_game[player] = game;

        std::cout << "[SERVER] Создана игра \"" << game
                  << "\" | макс. игроков: " << maxp
                  << " | создатель: " << player << "\n";

        return "Игра создана";
    }

    /* ===== JOIN ===== */
    if (cmd == "JOIN") {
        std::string game;
        iss >> game;

        if (!games.count(game))
            return "Игра не найдена";

        auto& g = games[game];

        if (g.finished)
            return "Игра окончена. Победил игрок " + g.winner;

        if ((int)g.players.size() >= g.max_players)
            return "Игра уже заполнена";

        if (player_game.count(player))
            return "Вы уже участвуете в игре";

        g.players.push_back(player);
        player_game[player] = game;

        std::cout << "[SERVER] Игрок " << player
                  << " подключился к игре \"" << game << "\"\n";

        if ((int)g.players.size() == g.max_players) {
            g.started = true;
            g.secret = generate_secret();

            std::cout << "[SERVER] Игра \"" << game << "\" началась\n";
            std::cout << "[SERVER] Секретное число: " << g.secret << "\n";
        }

        return "Подключение успешно";
    }

    /* ===== FIND ===== */
    if (cmd == "FIND") {
        std::ostringstream out;
        bool any = false;

        for (auto& [name, g] : games) {
            if (g.finished) continue;

            out << name << " (" << g.players.size()
                << "/" << g.max_players << " игроков)";
            if (player_game.count(player) && player_game[player] == name)
                out << " [вы в игре]";
            out << "\n";
            any = true;
        }

        return any ? out.str() : "Свободных игр нет";
    }

    /* ===== MOVE ===== */
    if (cmd == "MOVE") {
        std::string guess;
        iss >> guess;

        if (!player_game.count(player))
            return "Вы не в игре";

        auto& g = games[player_game[player]];

        if (g.finished)
            return "Игра окончена. Победил игрок " + g.winner;

        if (!g.started)
            return "Ожидание других игроков";

        auto [b, c] = bulls_and_cows(g.secret, guess);

        std::cout << "[SERVER] Ход игрока " << player
                  << " в игре \"" << g.name
                  << "\" | число: " << guess
                  << " | B=" << b << " C=" << c << "\n";

        if (b == 4) {
            g.finished = true;
            g.winner = player;

            std::cout << "[SERVER] Игра \"" << g.name
                      << "\" завершена. Победитель: " << player << "\n";

            return "Победа! Вы выиграли игру";
        }

        std::ostringstream res;
        res << "Быки: " << b << ", Коровы: " << c;
        return res.str();
    }

    /* ===== LEAVE ===== */
    if (cmd == "LEAVE") {
        if (!player_game.count(player))
            return "Вы не в игре";

        std::string game = player_game[player];
        auto& g = games[game];

        g.players.erase(
            std::remove(g.players.begin(), g.players.end(), player),
            g.players.end()
        );
        player_game.erase(player);

        std::cout << "[SERVER] Игрок " << player
                  << " вышел из игры \"" << game << "\"\n";

        if (g.players.empty()) {
            std::cout << "[SERVER] Игра \"" << game
                      << "\" закрыта (все игроки вышли)\n";
            games.erase(game);
        }

        return "Вы вышли из игры";
    }

    return "Неизвестная команда";
}

/* ---------- main ---------- */

int main() {
    zmq::context_t ctx(1);
    zmq::socket_t sock(ctx, zmq::socket_type::rep);

    sock.bind("tcp://*:5555");
    std::cout << "[SERVER] Сервер запущен на порту 5555\n";

    while (true) {
        zmq::message_t msg;
        sock.recv(msg, zmq::recv_flags::none);

        std::string req((char*)msg.data(), msg.size());
        std::string resp = handle(req);

        zmq::message_t reply(resp.size());
        memcpy(reply.data(), resp.data(), resp.size());
        sock.send(reply, zmq::send_flags::none);
    }
}
