#include <zmq.hpp>
#include <iostream>
#include <string>

int main() {
    zmq::context_t ctx(1);
    zmq::socket_t sock(ctx, zmq::socket_type::req);
    sock.connect("tcp://localhost:5555");

    std::string player;
    std::cout << "Введите имя игрока: ";
    std::cin >> player;

    while (true) {
        std::cout <<
            "\n1. Создать игру\n"
            "2. Присоединиться\n"
            "3. Найти игры\n"
            "4. Ход\n"
            "5. Выйти из игры\n"
            "0. Выход\n> ";

        int cmd;
        std::cin >> cmd;

        std::string req;

        if (cmd == 0) break;

        if (cmd == 1) {
            std::string game;
            int maxp;
            std::cout << "Имя игры: ";
            std::cin >> game;
            std::cout << "Количество игроков: ";
            std::cin >> maxp;
            req = "CREATE " + player + " " + game + " " + std::to_string(maxp);
        }
        else if (cmd == 2) {
            std::string game;
            std::cout << "Имя игры: ";
            std::cin >> game;
            req = "JOIN " + player + " " + game;
        }
        else if (cmd == 3) {
            req = "FIND " + player;
        }
        else if (cmd == 4) {
            std::string guess;
            std::cout << "Ваше число: ";
            std::cin >> guess;
            req = "MOVE " + player + " " + guess;
        }
        else if (cmd == 5) {
            req = "LEAVE " + player;
        }
        else continue;

        zmq::message_t msg(req.size());
        memcpy(msg.data(), req.data(), req.size());
        sock.send(msg, zmq::send_flags::none);

        zmq::message_t reply;
        sock.recv(reply, zmq::recv_flags::none);
        std::cout << "Ответ сервера: "
                  << std::string((char*)reply.data(), reply.size()) << "\n";
    }
}
