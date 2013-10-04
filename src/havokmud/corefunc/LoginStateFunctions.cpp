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
                    connection->player->account->email());
        }

        void enter_state_get_new_user_password(Connection *connection)
        {
            connection->send("Give me a password for %s: ",
                    connection->player->account->email());
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
                             connection->player->account->email());
            connection->send("$c00151) $c0012ANSI Colors.\n\r");
            connection->send("$c00152) $c0012Change your password.\n\r");
            connection->send("$c00153) $c0012View the MOTD.\n\r");
            connection->send("$c00154) $c0012View the credits.\n\r");
            if( connection->player->account->confirmed() ) {
                connection->send("$c00155) $c0012List characters.\n\r");
                connection->send("$c00156) $c0012Create a new character.\n\r");
                connection->send("$c00157) $c0012Play an existing character.\n\r");
            }

            if( !connection->player->account->confirmed() ) {
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
            if( connection->player->account->confcode().empty() ) {
                connection->send("Sending your confirmation email...\n\r");
            } else {
                connection->send("Resending your confirmation email...\n\r");
            }
            //CreateSendConfirmEmail(player->account);
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
        }

        const std::string &do_state_choose_ansi(Connection *connection,
                const std::string &line)
        {
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
        }

        const std::string &do_state_show_player_list(Connection *connection,
                const std::string &line)
        {
            return("show account menu");
        }

        const std::string &do_state_get_new_password(Connection *connection,
                const std::string &line)
        {
        }

        const std::string &do_state_confirm_new_password(Connection *connection,
                const std::string &line)
        {
        }

        const std::string &do_state_enter_confirm_code(Connection *connection,
                const std::string &line)
        {
        }

        const std::string &do_state_resend_confirm_email(Connection *connection,
                const std::string &line)
        {
        }

        const std::string &do_state_show_creation_menu(Connection *connection,
                const std::string &line)
        {
        }

        const std::string &do_state_choose_name(Connection *connection,
                const std::string &line)
        {
        }

        const std::string &do_state_choose_sex(Connection *connection,
                const std::string &line)
        {
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
