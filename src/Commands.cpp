#include "../include/Commands.hpp"
#include <iostream>

void Commands::register_commands(dpp::cluster& bot)
{
    bot.global_command_create(
        dpp::slashcommand("ping", "Ping Pong!", bot.me.id)
    );

    bot.global_command_create(
        dpp::slashcommand("ban", "Bannit un utilisateur du serveur", bot.me.id)
            .add_option(dpp::command_option(dpp::co_user, "membre", "Utilisateur à bannir", true))
            .add_option(dpp::command_option(dpp::co_string, "raison", "Raison du bannissement", false))
    );

    bot.global_command_create(
        dpp::slashcommand("mute", "Mute un utilisateur pour une durée spécifiée", bot.me.id)
            .add_option(dpp::command_option(dpp::co_user, "membre", "Utilisateur à muter", true))
            .add_option(dpp::command_option(dpp::co_integer, "duree", "Durée du mute en minutes", false))
            .add_option(dpp::command_option(dpp::co_string, "raison", "Raison du mute", false))
    );

    std::cout << "\u2705 Commandes enregistrées !" << std::endl;
}

static void commands_ban_handle(const dpp::slashcommand_t& event)
{
    dpp::snowflake user_id = std::get<dpp::snowflake>(event.get_parameter("membre"));
    auto member_opt = event.command.resolved.members.find(user_id);

    if (member_opt == event.command.resolved.members.end()) {
        event.from->creator->guild_get_member(event.command.guild_id, user_id, 
        [event, user_id](const dpp::confirmation_callback_t& member_callback) {
            if (member_callback.is_error()) {
                event.reply("❌ Impossible de récupérer les informations du membre.");
                return;
            }

            dpp::guild_member member = std::get<dpp::guild_member>(member_callback.value);

            event.from->creator->roles_get(event.command.guild_id, 
            [event, user_id, member](const dpp::confirmation_callback_t& roles_callback) {
                if (roles_callback.is_error()) {
                    event.reply("❌ Impossible de récupérer les rôles du serveur.");
                    return;
                }

                auto roles = std::get<std::unordered_map<dpp::snowflake, dpp::role>>(roles_callback.value);

                bool has_permission = false;
                for (const auto& role_id : member.get_roles()) {
                    if (roles.find(role_id) != roles.end() && roles[role_id].permissions.has(dpp::p_ban_members)) {
                        has_permission = true;
                        break;
                    }
                }
                if (!has_permission) {
                    event.reply("⛔ Tu n'as pas la permission de bannir des membres.");
                    return;
                }
                event.from->creator->guild_ban_add(event.command.guild_id, user_id, 0, 
                [event, user_id](const dpp::confirmation_callback_t& ban_callback) {
                    if (ban_callback.is_error()) {
                        event.reply("❌ Impossible de bannir l'utilisateur !");
                    } else {
                        event.reply("✅ L'utilisateur <@" + std::to_string(user_id) + "> a été banni avec succès.");
                    }
                });
            });
        });
    }
}

static void commands_mute_handle(const dpp::slashcommand_t& event)
{
    dpp::snowflake issuer_id = event.command.get_issuing_user().id;
    dpp::snowflake user_id = std::get<dpp::snowflake>(event.get_parameter("membre"));
    int duration = 10;
    std::string reason = event.get_parameter("raison").index() == 1 ? std::get<std::string>(event.get_parameter("raison")) : "Aucune raison spécifiée.";
    if (event.get_parameter("duree").index() == 2) {
        duration = static_cast<int>(std::get<long>(event.get_parameter("duree")));
    } else if (event.get_parameter("duree").index() == 5) {
        duration = static_cast<int>(std::get<double>(event.get_parameter("duree")));
    }
    event.from->creator->roles_get(event.command.guild_id, [event, issuer_id, user_id, duration, reason](const dpp::confirmation_callback_t& callback) {
        if (callback.is_error()) {
            event.reply("⛔ Impossible de récupérer les rôles.");
            return;
        }
        auto roles = std::get<std::unordered_map<dpp::snowflake, dpp::role>>(callback.value);
        auto member_roles = event.command.get_resolved_member(issuer_id).get_roles();
        bool has_permission = false;
        for (const auto& role_id : member_roles) {
            if (roles.find(role_id) != roles.end() && roles[role_id].permissions.has(dpp::p_manage_roles)) {
                has_permission = true;
                break;
            }
        }
        if (!has_permission) {
            event.reply("⛔ Tu n'as pas la permission de muter des membres.");
            return;
        }
        dpp::snowflake mute_role_id = 0;
        for (const auto& [role_id, role] : roles) {
            if (role.name == "Muted") {
                mute_role_id = role_id;
                break;
            }
        }
        if (mute_role_id == 0) {
            event.reply("⛔ Le rôle 'Muted' n'existe pas. Veuillez le créer manuellement.");
            return;
        }
        dpp::snowflake guild_id = event.command.guild_id;
        event.from->creator->guild_member_add_role(event.command.guild_id, user_id, mute_role_id, 
        [event, user_id, mute_role_id, duration, guild_id](const dpp::confirmation_callback_t& callback) {
            if (callback.is_error()) {
                event.reply("❌ Impossible de muter l'utilisateur.");
                return;
            }
            event.reply("✅ L'utilisateur <@" + std::to_string(user_id) + "> a été muté pour " + std::to_string(duration) + " minutes.");
            std::thread([event, user_id, mute_role_id, guild_id, duration]() {
                std::this_thread::sleep_for(std::chrono::minutes{duration});
                event.from->creator->guild_member_remove_role(guild_id, user_id, mute_role_id);
            }).detach();
        });
    });
}

void Commands::handle_command(const dpp::slashcommand_t& event)
{
    if (event.command.get_command_name() == "ping") {
        event.reply("Pong!");
    } else if (event.command.get_command_name() == "ban") {
        commands_ban_handle(event);
    } else if (event.command.get_command_name() == "mute") {
        commands_mute_handle(event);
    }
}

static void commands_ban_message(const dpp::message_create_t& event)
{
    event.from->creator->guild_get_member(event.msg.guild_id, event.msg.author.id,
    [event](const dpp::confirmation_callback_t& callback) {
        if (callback.is_error()) {
            event.reply("⛔ Impossible de récupérer les permissions.");
            return;
        }

        if (callback.value.valueless_by_exception()) {
            event.reply("⛔ Erreur : Impossible de récupérer le membre.");
            return;
        }

        dpp::guild_member member = std::get<dpp::guild_member>(callback.value);

        if (member.get_roles().empty()) {
            event.reply("⛔ Erreur : Aucun rôle trouvé pour cet utilisateur.");
            return;
        }

        event.from->creator->roles_get(event.msg.guild_id, [event, member](const dpp::confirmation_callback_t& role_callback) {
            if (role_callback.is_error()) {
                event.reply("⛔ Impossible de récupérer les rôles.");
                return;
            }

            auto roles = std::get<std::unordered_map<dpp::snowflake, dpp::role>>(role_callback.value);
            bool has_permission = false;
            
            for (const auto& role_id : member.get_roles()) {
                if (roles.find(role_id) != roles.end() && roles[role_id].permissions.has(dpp::p_ban_members)) {
                    has_permission = true;
                    break;
                }
            }

            if (!has_permission) {
                event.reply("⛔ Tu n'as pas la permission de bannir des membres.");
                return;
            }

            std::vector<std::string> args;
            std::istringstream iss(event.msg.content);
            std::string word;
            while (iss >> word) {
                args.push_back(word);
            }

            if (args.size() < 2) {
                event.reply("❌ Usage: `!ban @membre [raison]`");
                return;
            }

            if (event.msg.mentions.empty()) {
                event.reply("❌ Erreur : Aucun utilisateur mentionné.");
                return;
            }

            dpp::snowflake user_id = event.msg.mentions[0].first.id;
            std::string reason = (args.size() > 2) ? args[2] : "Aucune raison spécifiée.";

            event.from->creator->guild_ban_add(event.msg.guild_id, user_id, 0, [event, user_id](const dpp::confirmation_callback_t& callback) {
                if (callback.is_error()) {
                    event.reply("❌ Impossible de bannir l'utilisateur !");
                } else {
                    event.reply("✅ L'utilisateur <@" + std::to_string(user_id) + "> a été banni avec succès.");
                }
            });
        });
    });
}

static void commands_mute_message(const dpp::message_create_t& event)
{
    std::istringstream iss(event.msg.content);
    std::vector<std::string> args;
    std::string word;

    while (iss >> word) {
        args.push_back(word);
    }
    if (args.size() < 2 || event.msg.mentions.empty()) {
        event.reply("❌ Usage: `!mute @membre [durée] [raison]`");
        return;
    }

    dpp::snowflake issuer_id = event.msg.author.id;
    dpp::snowflake user_id = event.msg.mentions[0].first.id;
    int duration = args.size() > 2 ? std::stoi(args[2]) : 10;
    std::string reason = args.size() > 3 ? args[3] : "Aucune raison spécifiée.";

    event.from->creator->roles_get(event.msg.guild_id, [event, issuer_id, user_id, duration, reason](const dpp::confirmation_callback_t& callback) {
        if (callback.is_error()) {
            event.reply("⛔ Impossible de récupérer les rôles.");
            return;
        }

        auto roles = std::get<std::unordered_map<dpp::snowflake, dpp::role>>(callback.value);
        auto member_roles = event.msg.member.get_roles();
        bool has_permission = false;

        for (const auto& role_id : member_roles) {
            if (roles.find(role_id) != roles.end() && roles[role_id].permissions.has(dpp::p_manage_roles)) {
                has_permission = true;
                break;
            }
        }
        if (!has_permission) {
            event.reply("⛔ Tu n'as pas la permission de muter des membres.");
            return;
        }

        dpp::snowflake mute_role_id = 0;

        for (const auto& [role_id, role] : roles) {
            if (role.name == "Muted") {
                mute_role_id = role_id;
                break;
            }
        }
        if (mute_role_id == 0) {
            event.reply("⛔ Le rôle 'Muted' n'existe pas. Veuillez le créer manuellement.");
            return;
        }
        event.from->creator->guild_member_add_role(event.msg.guild_id, user_id, mute_role_id, [event, user_id, mute_role_id, duration](const dpp::confirmation_callback_t& callback) {
            if (callback.is_error()) {
                event.reply("❌ Impossible de muter l'utilisateur.");
                return;
            }
            dpp::snowflake guild_id = event.msg.guild_id;
            event.reply("✅ L'utilisateur <@" + std::to_string(user_id) + "> a été muté pour " + std::to_string(duration) + " minutes.");
            std::thread([event, user_id, mute_role_id, guild_id, duration]() {
                std::this_thread::sleep_for(std::chrono::minutes{duration});
                event.from->creator->guild_member_remove_role(guild_id, user_id, mute_role_id);
            }).detach();
        });
    });
}

void Commands::handle_message(const dpp::message_create_t& event)
{
    if (event.msg.content.front() == '!'){
        if (event.msg.content.rfind("!ban", 0) == 0)
            commands_ban_message(event);
        else if (event.msg.content.rfind("!mute", 0) == 0)
            commands_mute_message(event);
        else     
            event.reply("Commands not find");
    }
}
