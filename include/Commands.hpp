#ifndef COMMANDS_HPP
#define COMMANDS_HPP

#include <dpp/dpp.h>

class Commands {
    public:
        static void register_commands(dpp::cluster& bot);
        static void handle_command(const dpp::slashcommand_t& event);
        static void handle_message(const dpp::message_create_t& event);
};

#endif // COMMANDS_HPP
