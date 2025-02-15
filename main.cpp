#include "./include/Bot.hpp"
#include <cstdlib>

int main() 
{
    const auto* token = std::getenv("DISCORD_BOT_TOKEN");

    if (!token) {
        std::cerr << "Erreur : La variable d'environnement DISCORD_BOT_TOKEN n'est pas définie !" << std::endl;
        return 1;
    }
    Bot bot(token);
    bot.run();
}
