/*
 *  This file is part of the havokmud package
 *  Copyright (C) 2013 Gavin Hurlbut
 *
 *  havokmud is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

/**
 * @file
 * @brief Login State Machine Functions
 */

#include "corefunc/Logging.hpp"
#include "corefunc/LoginStateMachine.hpp"
#include "corefunc/LoginStateFunctions.hpp"

namespace havokmud {
    namespace corefunc {

        using havokmud::objects::Connection;

        // Entry functions
        // typedef boost::function<void (Connection *connection)>
        //         LoginEntryFunction;
        void enter_state_initial(Connection *connection)
        {
        }

        void enter_state_get_email(Connection *connection)
        {
            connection->send("What is your account name (email address)? ");
        }

        void enter_state_confirm_email(Connection *connection)
        {
            connection->send("Did I get that right, %s (Y/N)? ",
                    connection->account()->email());
        }

        void enter_state_get_new_user_password(Connection *connection)
        {
            connection->send("Give me a password for %s: ",
                    connection->account()->email());
            connection->sendRaw(echo_off, 4);
        }

        void enter_state_confirm_password(Connection *connection)
        {
            connection->send("Password: ");
            connection->sendRaw(echo_off, 4);
        }

        void enter_state_get_password(Connection *connection)
        {
            connection->send("Please retype password: ");
            connection->sendRaw(echo_off, 4):
        }

        void enter_state_choose_ansi(Connection *connection)
        {
            connection->send("Would you like ansi colors? (Yes or No)");
        }

        void enter_state_show_motd(Connection *connection)
        { 
            connection->send("MOTD will eventually be read from the "
                             "database.\n\r\n\r");
            connection->send("\n\r\n[PRESS RETURN]");
        }

        void enter_state_show_wmotd(Connection *connection)
        {
            connection->send("WMOTD will eventually be read from the "
                             "database.\n\r\n\r");
            connection->send("\n\r\n[PRESS RETURN]");
        }

        void enter_state_show_credits(Connection *connection)
        {
            connection->send("Credits will eventually be read from the "
                             "database.\n\r\n\r");
            connection->send("\n\r\n[PRESS RETURN]");
        }

        void enter_state_disconnect(Connection *connection)
        {
            connection->send("Goodbye.\n\r");
            connection->stop();
        }

        void enter_state_press_enter(Connection *connection)
        {
            connection->send("\n\r<Press enter to continue>");
        }

        void enter_state_show_account_menu(Connection *connection)
        {
            connection->send("\n\r\n\r$c0009-=$c0015Havok Account Menu [%s]"
                             "$c0009=-\n\r\n\r",
                             connection->account()->email());
            connection->send("$c00151) $c0012ANSI Colors.\n\r");
            connection->send("$c00152) $c0012Change your password.\n\r");
            connection->send("$c00153) $c0012View the MOTD.\n\r");
            connection->send("$c00154) $c0012View the credits.\n\r");
            if( connection->account()->confirmed() ) {
                connection->send("$c00155) $c0012List characters.\n\r");
                connection->send("$c00156) $c0012Create a new character.\n\r");
                connection->send("$c00157) $c0012Play an existing character.\n\r");
            } else {
                connection->send("$c0015E) $c0012Enter email confirmation code.\n\r");
                connection->send("$c0015R) $c0012Resend the confirmation email.\n\r");
            }
            connection->send("$c0015Q) $c0012Quit!\n\r\n\r");
            connection->send("$c0011Please pick an option: \n\r");
        } 

        void enter_state_show_player_list(Connection *connection)
        {
        }

        void enter_state_get_new_password(Connection *connection)
        {
            connection->send("Enter a new password: ");
            connection->sendRaw(echo_off, 4);
        }

        void enter_state_confirm_new_password(Connection *connection)
        {
            connection->send("Please retype password: ");
            connection->send(echo_off, 4);
        }

        void enter_state_enter_confirm_code(Connection *connection)
        {
            connection->send("Please enter the confirmation code you were "
                             "emailed:  ");
        }

        void enter_state_resend_confirm_email(Connection *connection)
        {
            if( connection->account()->confirmCode().empty() ) {
                connection->send("Sending your confirmation email...\n\r");
            } else {
                connection->send("Resending your confirmation email...\n\r");
            }
            connection->account()->createConfirmCode();
        }

        void enter_state_show_creation_menu(Connection *connection)
        {
        }

        void enter_state_choose_name(Connection *connection)
        {
            connection->send("Choose the name of your new PC: ");
        }

        void enter_state_choose_sex(Connection *connection)
        {
            connection->send("What is your sex (M)ale/(F)emale/(N)eutral? ")
        }

        void enter_state_choose_race(Connection *connection)
        {
        }

        void enter_state_choose_class(Connection *connection)
        {
        }

        void enter_state_choose_stats(Connection *connection)
        {
        }

        void enter_state_choose_alignment(Connection *connection)
        {
        }

        void enter_state_reroll_abilities(Connection *connection)
        {
        }

        void enter_state_show_login_menu(Connection *connection)
        {
        }

        void enter_state_wait_for_auth(Connection *connection)
        {
        }

        void enter_state_edit_extra_descr(Connection *connection)
        {
        }

        void enter_state_delete_user(Connection *connection)
        {
        }


        // State action functions
        // typedef boost::function<const std::string &(Connection *connection,
        //         const std::string &)> LoginStateFunction;
        const std::string &do_state_initial(Connection *connection,
                const std::string &line)
        {
            return("get email");
        }

        const std::string &do_state_get_email(Connection *connection,
                const std::string &line)
        {
        }

        const std::string &do_state_confirm_email(Connection *connection,
                const std::string &line)
        {
        }

        const std::string &do_state_get_new_user_password(Connection *connection,
                const std::string &line)
        {
        }

        const std::string &do_state_confirm_password(Connection *connection,
                const std::string &line)
        {
        }

        const std::string &do_state_get_password(Connection *connection,
                const std::string &line)
        {
            if (line.empty())
                return("disconnect");

            if (!checkPassword(line)) {
                connection->send("Wrong password.\n\r");
                LogPrint(LG_INFO, "%s entered a wrong password",
                         connection->account()->email().c_str());
                return("disconnect");
            }

            LogPrint(LG_INFO, "%s[%s] has connected",
                     connection->account()->email().c_str(),
                     connection->hostName().c_str());
            return("show account menu");
        }

        const std::string &do_state_choose_ansi(Connection *connection,
                const std::string &line)
        {
            if (line.empty()) {
                connection->send("Please type Yes or No.\n\r"
                                 "Would you like ANSI colors? :");
                return("no change");
            }

            switch (tolower(line[0])) {
            case 'y':
                connection->account()->setAnsi(true);

                connection->send("$c0012A$c0010N$c0011S$c0014I$c0007 colors "
                                 "enabled.\n\r\n\r");
                break;

            case 'n':
                connection->account()->setAnsi(false);
                break;

            default:
                connection->send("Please type Yes or No.\n\r"
                                 "Would you like ANSI colors? :");
                return("no change");
            }

            connection->account()->save();
            if (connection->account()->confirmCode().empty())
                // Send the confirmation code by email
                connection->loginStateMachine()->enterState("resend confirm email");
            }

            return("show account menu");
        }

        const std::string &do_state_show_motd(Connection *connection,
                const std::string &line)
        {
            return("show account menu");
        }

        const std::string &do_state_show_wmotd(Connection *connection,
                const std::string &line)
        {
            return("show account menu");
        }

        const std::string &do_state_show_credits(Connection *connection,
                const std::string &line)
        {
            return("show account menu");
        }

        const std::string &do_state_press_enter(Connection *connection,
                const std::string &line)
        {
            return("show login menu");
        }

        const std::string &do_state_show_account_menu(Connection *connection,
                const std::string &line)
        {
            if (line.empty()) {
                connection->send("Invalid Choice.. Try again..\n\r");
                return("show account menu");
            }

            bool confirmed = connection->account()->confirmed();

            switch (tolower(line[0])) 
            {
            case '1':
                return("choose ansi");
            case '2':
                return("get new password");
            case '3':
                return("show motd");
            case '4':
                return("show credits");
            case '5':
                if (confirmed)
                    return("show player list");
                break;
            case '6':
                if (confirmed)
                    return("show creation menu");
                break;
            case '7':
                if (confirmed)
                    return("playing");
                break;
            case 'e':
                if (!confirmed)
                    return("enter confirm code");
                break;
            case 'r':
                if (!confirmed) {
                    // Resend the confirmation email
                    connection->loginStateMachine()->
                            enterState("resend_confirm_email");
                }
                break;
            case 'q':
                connection->send("Thanks for dropping by, seeya later!\n\r");
                return("disconnect");
            default:
                connection->send("Invalid Choice.. Try again..\n\r");
                break;
            }

            return("show account menu");
        }

        const std::string &do_state_show_player_list(Connection *connection,
                const std::string &line)
        {
            return("show account menu");
        }

        const std::string &do_state_get_new_password(Connection *connection,
                const std::string &line)
        {
            if (line.length() <= 10) {
                connection->sendRaw(echo_on, 6);
                connection->send("Illegal password.\n\r");
                return("get new password");
            }

            connection->account()->setNewPassword(line);
            connection->sendRaw(echo_on, 6);
            return("confirm new password");
        }

        const std::string &do_state_confirm_new_password(Connection *connection,
                const std::string &line)
        {
            if (!connection->account()->confirmPassword(line)) {
                connection->sendRaw(echo_on, 6);
                connection->send("Passwords don't match.\n\r");
                return("show account menu");
            } 

            connection->sendRaw(echo_on, 6);
            connection->account->setPassword();
            connection->send("Password changed...\n\r");
            connection->account()->save();

            return("show account menu");
        }

        const std::string &do_state_enter_confirm_code(Connection *connection,
                const std::string &line)
        {
            if (line.empty()) {
                connection->send("Entry aborted\n\r");
                return("show account menu");
            }

            if (connection->account()->checkConfirmCode(line)) {
                connection->send("\n\rYour email is now confirmed, you can now "
                                 "play.  Thank you!\n\r");
                connection->account()->setConfirmed(true);
                connection->account()->save();
            } else {
                connection->send("\n\rConfirmation code does not match our "
                                 "records.  Please try again,\n\r"
                                 "or resend the confirmation email.\n\r");
                connection->account()->setConfirmed(false);
                connection->account()->save();
            }

            return("show account menu");
        }

        const std::string &do_state_show_creation_menu(Connection *connection,
                const std::string &line)
        {
            if( line.empty() ) {
                connection->send("Invalid Choice.. Try again..\n\r");
                return("show creation menu");
            }

            int             bitcount;
            ch = player->charData;

            switch (tolower(line[0])) 
            {
            case '1':
                return("choose name");
            case 'r':
                return("reroll abilities");
            case '2':
                return("choose sex");
            case '3':
                return("choose race");
            case '4':
                return("choose class");
            case '5':
                connection->player()->setReroll(20);
                if (connection->player()->classes().size() != 0) {
                    return("choose stats");
                } else {
                    connection->send("\nPlease select a class first.\n\r");
                    return("no change");
                }
                break;
            case '6':
                return("choose alignment");
            case 'd':
#if 0
                bitcount = 0;
                if (bitcount <= 0) {
                    SendOutput(player, "Please enter a valid class.");
                    return( STATE_SHOW_CREATION_MENU );
                }
#endif

                if (connection->player()->sex() == 0) {
                    connection->send("Please enter a proper sex.");
                    return("show creation menu");
                }

                if (connection->player()->alignment() == 0) {
                    connection->send("Please choose an alignment.");
                    return("show creation menu");
                }

                if (connection->player()->constitution() == 0) {
                    connection->send("Please pick your stats.");
                    return("show creation menu");
                }

                LogPrint(LG_INFO, "%s [%s] new player.",
                         connection->player()->name().c_str(),
                         connection->hostName().c_str());

                /*
                 * now that classes are set, initialize
                 */
#if 0
                init_char(ch);
#endif
                connection->player()->save(AUTO_RENT);

                return("show account menu");
            default:
                connection->send("Invalid Choice.. Try again..\n\r");
                break;
            }
            return("show creation menu");
        }

        const std::string &do_state_choose_name(Connection *connection,
                const std::string &line)
        {
            if( line.empty() ) {
                connection.send("Never mind then.\n\r");
                return("show creation menu");
            } 
            
            Player *player = Player::findPlayer(line);
            if (player) {
                if (connection->account()->id() != player->accountId()) {
                    connection->send("Name taken.\n\r");
                    delete player;
                    return("choose name");
                }
                
                connection->setPlayer(player);
                connection->player()->load();

                if( connection->player()->getAttribute<bool>("complete",
                            "core-pc") ) {
                    connection->send("That PC is completed!\n\r");
                    connection->setPlayer(NULL);
                    delete player;

                    return("show account menu");
                }
                return("show creation menu");
            } 

            // New character name!
            /* TODO: check for banned names */
            player = new Player(line, connection->account()->id());
            connection->account()->addPlayer(player);
            connection->account()->setPlayer(player);
            player->setAttribute<bool>("complete", "core-pc", false);

            std::string setting = g_protobuf.get("MaxReroll");
            if (setting.empty()) {
                player->setAttribute<int>("rerolls", "core-pc", 1);
            } else {
                player->setAttribute<int>("rerolls", "core-pc",
                                        std::stoi(setting) + 1 );
            }

            connection->player()->RollAbilities();
            connection->player()->save();
            return("show creation menu");
        }

        const std::string &do_state_choose_sex(Connection *connection,
                const std::string &line)
        {
            /*
             * query sex of new user
             */
            if (line.empty()) {
                connection->send("That's not a sex..\n\r");
                return("choose sex");
            }

            switch (tolower(line[0])) {
            case 'm':
                connection->player()->setAttribute<int>("sex", "core-pc",
                        SEX_MALE);
                break;

            case 'f':
                connection->player()->setAttribute<int>("sex", "core-pc",
                        SEX_FEMALE);
                break;

            case 'n':
                connection->player()->setAttribute<int>("sex", "core-pc",
                        SEX_NEUTRAL);
                break;

            default:
                connection->send("That's not a sex..\n\r");
                return("choose sex");
            }

            connection->player()->save();
            return("show creation menu");
        }

        const std::string &do_state_choose_race(Connection *connection,
                const std::string &line)
        {
            return("show creation menu");
        }

        const std::string &do_state_choose_class(Connection *connection,
                const std::string &line)
        {
            return("show creation menu");
        }

        const std::string &do_state_choose_stats(Connection *connection,
                const std::string &line)
        {
            return("show creation menu");
        }

        const std::string &do_state_choose_alignment(Connection *connection,
                const std::string &line)
        {
            return("show creation menu");
        }

        const std::string &do_state_reroll_abilities(Connection *connection,
                const std::string &line)
        {
            return("show creation menu");
        }

        const std::string &do_state_show_login_menu(Connection *connection,
                const std::string &line)
        {
            return("show login menu");
        }

        const std::string &do_state_wait_for_auth(Connection *connection,
                const std::string &line)
        {
            return("show creation menu");
        }

        const std::string &do_state_edit_extra_descr(Connection *connection,
                const std::string &line)
        {
            return("show login menu");
        }

        const std::string &do_state_delete_user(Connection *connection,
                const std::string &line)
        {
            return("show creation menu");
        }

    }
}
