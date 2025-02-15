#include "../include/Bot.hpp"
#include "../include/Commands.hpp"
#include <iostream>

Bot::Bot(const std::string& token) : bot(token, dpp::i_all_intents) {
    setup_events();
}

void Bot::setup_events()
{
    bot.on_log(dpp::utility::cout_logger());

    bot.on_ready([this](const dpp::ready_t& event){
        on_ready(event);
    });

    bot.on_message_create([this](const dpp::message_create_t& event){
        Commands::handle_message(event);
    });

    bot.on_slashcommand([this](const dpp::slashcommand_t& event){
        Commands::handle_command(event);
    });
}

void Bot::on_ready(const dpp::ready_t& event)
{
    std::cout << "✅ Bot connecté en tant que " << bot.me.username << std::endl;

    if (dpp::run_once<struct register_bot_commands>()) {
        Commands::register_commands(bot);
    }
    (void)event;
}

void Bot::run()
{
    bot.start(dpp::st_wait);
}
