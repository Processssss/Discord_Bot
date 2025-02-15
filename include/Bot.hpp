/*
** EPITECH PROJECT, 2025
** bot
** File description:
** Bot
*/

#ifndef BOT_HPP_
#define BOT_HPP_

#include <dpp/dpp.h>

class Bot {
    public:
        Bot(const std::string &token);
        ~Bot() = default;
        void run();

    private:
        dpp::cluster bot;
        void setup_events();
        void on_ready(const dpp::ready_t& event);
};

#endif /* !BOT_HPP_ */
