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
        void enter_state_initial();
        void enter_state_get_email();
        void enter_state_confirm_email();
        void enter_state_get_new_user_password();
        void enter_state_confirm_password();
        void enter_state_get_password();
        void enter_state_choose_ansi();
        void enter_state_show_motd();
        void enter_state_show_wmotd();
        void enter_state_show_credits();
        void enter_state_disconnect();
        void enter_state_press_enter();
        void enter_state_show_account_menu();
        void enter_state_show_player_list();
        void enter_state_get_new_password();
        void enter_state_confirm_new_password();
        void enter_state_enter_confirm_code();
        void enter_state_resend_confirm_email();
        void enter_state_show_creation_menu();
        void enter_state_choose_name();
        void enter_state_choose_sex();
        void enter_state_choose_race();
        void enter_state_choose_class();
        void enter_state_choose_stats();
        void enter_state_choose_alignment();
        void enter_state_reroll_abilities();
        void enter_state_show_login_menu();
        void enter_state_wait_for_auth();
        void enter_state_edit_extra_descr();
        void enter_state_delete_user();

        // State action functions
        // typedef boost::function<const std::string &(const std::string &)>
        //         LoginStateFunction;
        const std::string &do_state_initial(const std::string &line);
        const std::string &do_state_get_email(const std::string &line);
        const std::string &do_state_confirm_email(const std::string &line);
        const std::string &do_state_get_new_user_password(const std::string &line);
        const std::string &do_state_confirm_password(const std::string &line);
        const std::string &do_state_get_password(const std::string &line);
        const std::string &do_state_choose_ansi(const std::string &line);
        const std::string &do_state_show_motd(const std::string &line);
        const std::string &do_state_show_wmotd(const std::string &line);
        const std::string &do_state_show_credits(const std::string &line);
        const std::string &do_state_press_enter(const std::string &line);
        const std::string &do_state_show_account_menu(const std::string &line);
        const std::string &do_state_show_player_list(const std::string &line);
        const std::string &do_state_get_new_password(const std::string &line);
        const std::string &do_state_confirm_new_password(const std::string &line);
        const std::string &do_state_enter_confirm_code(const std::string &line);
        const std::string &do_state_show_creation_menu(const std::string &line);
        const std::string &do_state_choose_name(const std::string &line);
        const std::string &do_state_choose_sex(const std::string &line);
        const std::string &do_state_choose_race(const std::string &line);
        const std::string &do_state_choose_class(const std::string &line);
        const std::string &do_state_choose_stats(const std::string &line);
        const std::string &do_state_choose_alignment(const std::string &line);
        const std::string &do_state_reroll_abilities(const std::string &line);
        const std::string &do_state_show_login_menu(const std::string &line);
        const std::string &do_state_wait_for_auth(const std::string &line);
        const std::string &do_state_edit_extra_descr(const std::string &line);
        const std::string &do_state_delete_user(const std::string &line);

    }
}

#endif  // __havokmud_corefunc_LoginStateFunctions__
