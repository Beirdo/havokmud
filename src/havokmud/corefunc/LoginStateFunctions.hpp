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
 * @brief Login state functions
 */

#ifndef __havokmud_corefunc_LoginStateFunctions__
#define __havokmud_corefunc_LoginStateFunctions__

namespace havokmud {
    namespace corefunc {
        // Entry functions
        // typedef boost::function<void ()> LoginEntryFunction;
        void enter_state_initial(Connection::pointer connection);
        void enter_state_get_email(Connection::pointer connection);
        void enter_state_confirm_email(Connection::pointer connection);
        void enter_state_get_new_user_password(Connection::pointer connection);
        void enter_state_confirm_password(Connection::pointer connection);
        void enter_state_get_password(Connection::pointer connection);
        void enter_state_choose_ansi(Connection::pointer connection);
        void enter_state_show_motd(Connection::pointer connection);
        void enter_state_show_wmotd(Connection::pointer connection);
        void enter_state_show_credits(Connection::pointer connection);
        void enter_state_disconnect(Connection::pointer connection);
        void enter_state_press_enter(Connection::pointer connection);
        void enter_state_show_account_menu(Connection::pointer connection);
        void enter_state_show_player_list(Connection::pointer connection);
        void enter_state_get_new_password(Connection::pointer connection);
        void enter_state_confirm_new_password(Connection::pointer connection);
        void enter_state_enter_confirm_code(Connection::pointer connection);
        void enter_state_resend_confirm_email(Connection::pointer connection);
        void enter_state_show_creation_menu(Connection::pointer connection);
        void enter_state_choose_name(Connection::pointer connection);
        void enter_state_choose_sex(Connection::pointer connection);
        void enter_state_choose_race(Connection::pointer connection);
        void enter_state_choose_class(Connection::pointer connection);
        void enter_state_choose_stats(Connection::pointer connection);
        void enter_state_choose_alignment(Connection::pointer connection);
        void enter_state_show_login_menu(Connection::pointer connection);
        void enter_state_wait_for_auth(Connection::pointer connection);
        void enter_state_edit_extra_descr(Connection::pointer connection);
        void enter_state_delete_user(Connection::pointer connection);

        // State action functions
        // typedef boost::function<const std::string &(const std::string &)>
        //         LoginStateFunction;
        const std::string do_state_initial(Connection::pointer connection,
                const std::string &line);
        const std::string do_state_get_email(Connection::pointer connection,
                const std::string &line);
        const std::string do_state_confirm_email(Connection::pointer connection,
                const std::string &line);
        const std::string do_state_get_new_user_password(Connection::pointer connection,
                const std::string &line);
        const std::string do_state_confirm_password(Connection::pointer connection,
                const std::string &line);
        const std::string do_state_get_password(Connection::pointer connection,
                const std::string &line);
        const std::string do_state_choose_ansi(Connection::pointer connection,
                const std::string &line);
        const std::string do_state_show_motd(Connection::pointer connection,
                const std::string &line);
        const std::string do_state_show_wmotd(Connection::pointer connection,
                const std::string &line);
        const std::string do_state_show_credits(Connection::pointer connection,
                const std::string &line);
        const std::string do_state_press_enter(Connection::pointer connection,
                const std::string &line);
        const std::string do_state_show_account_menu(Connection::pointer connection,
                const std::string &line);
        const std::string do_state_show_player_list(Connection::pointer connection,
                const std::string &line);
        const std::string do_state_get_new_password(Connection::pointer connection,
                const std::string &line);
        const std::string do_state_confirm_new_password(Connection::pointer connection,
                const std::string &line);
        const std::string do_state_enter_confirm_code(Connection::pointer connection,
                const std::string &line);
        const std::string do_state_show_creation_menu(Connection::pointer connection,
                const std::string &line);
        const std::string do_state_choose_name(Connection::pointer connection,
                const std::string &line);
        const std::string do_state_choose_sex(Connection::pointer connection,
                const std::string &line);
        const std::string do_state_choose_race(Connection::pointer connection,
                const std::string &line);
        const std::string do_state_choose_class(Connection::pointer connection,
                const std::string &line);
        const std::string do_state_choose_stats(Connection::pointer connection,
                const std::string &line);
        const std::string do_state_choose_alignment(Connection::pointer connection,
                const std::string &line);
        const std::string do_state_show_login_menu(Connection::pointer connection,
                const std::string &line);
        const std::string do_state_wait_for_auth(Connection::pointer connection,
                const std::string &line);
        const std::string do_state_edit_extra_descr(Connection::pointer connection,
                const std::string &line);
        const std::string do_state_delete_user(Connection::pointer connection,
                const std::string &line);

    }
}

#endif  // __havokmud_corefunc_LoginStateFunctions__
