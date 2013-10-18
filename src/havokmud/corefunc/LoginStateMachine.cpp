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
 * @brief Login State Machine
 */

#include <string>
#include <map>
#include <vector>

#include "corefunc/Logging.hpp"
#include "corefunc/LoginStateMachine.hpp"
#include "corefunc/LoginStateFunctions.hpp"

std::string toHex(const std::string &line);

namespace havokmud {
    namespace corefunc {
        int LoginState::s_nextId = 0;

        static LoginStateItem loginStates[] = {
            { "initial", enter_state_initial, do_state_initial },
            { "get email", enter_state_get_email, do_state_get_email },
            { "confirm email", enter_state_confirm_email,
              do_state_confirm_email },
            { "get new user password", enter_state_get_new_user_password,
              do_state_get_new_user_password },
            { "confirm password", enter_state_confirm_password,
              do_state_confirm_password },
            { "get password", enter_state_get_password, do_state_get_password },
            { "choose ansi", enter_state_choose_ansi, do_state_choose_ansi },
            { "show motd", enter_state_show_motd, do_state_show_motd },
            { "show wmotd", enter_state_show_wmotd, do_state_show_wmotd },
            { "show credits", enter_state_show_credits, do_state_show_credits },
            { "disconnect", enter_state_disconnect, 0 },
            { "press enter", enter_state_press_enter, do_state_press_enter },
            { "show account menu", enter_state_show_account_menu,
              do_state_show_account_menu },
            { "show player list", enter_state_show_player_list,
              do_state_show_player_list },
            { "get new password", enter_state_get_new_password,
              do_state_get_new_password },
            { "confirm new password", enter_state_confirm_new_password,
              do_state_confirm_new_password },
            { "enter confirm code", enter_state_enter_confirm_code,
              do_state_enter_confirm_code },
            { "resend confirm email", enter_state_resend_confirm_email, 0 },
            { "show creation menu", enter_state_show_creation_menu,
              do_state_show_creation_menu },
            { "choose name", enter_state_choose_name, do_state_choose_name },
            { "choose sex", enter_state_choose_sex, do_state_choose_sex },
            { "choose race", enter_state_choose_race, do_state_choose_race },
            { "choose class", enter_state_choose_class, do_state_choose_class },
            { "choose stats", enter_state_choose_stats, do_state_choose_stats },
            { "choose alignment", enter_state_choose_alignment,
              do_state_choose_alignment },
            { "show login menu", enter_state_show_login_menu,
              do_state_show_login_menu },
            { "wait for auth", enter_state_wait_for_auth,
              do_state_wait_for_auth },
            { "edit extra descr", enter_state_edit_extra_descr,
              do_state_edit_extra_descr },
            { "delete user", enter_state_delete_user, do_state_delete_user },
            { "playing", 0, 0 },
            { "no change", 0, 0 }
        };

        void LoginStateMachine::initialize()
        {
            LoginStateTable table(loginStates,
                                  loginStates + NELEMS(loginStates));

            g_loginStateMachine = new LoginStateMachine(table, "initial",
                                                        "playing");
        }

        LoginStateMachine::LoginStateMachine(const LoginStateTable &states,
                                             const std::string &startState,
                                             const std::string &exitState)
        {
            m_stateMap = std::shared_ptr<LoginStateMap>(new LoginStateMap);
            for(LoginStateTable::const_iterator it = states.begin();
                it != states.end(); ++it ) {
                std::shared_ptr<LoginState>
                        state(new LoginState(it->name, it->enterState,
                                             it->doState));
                m_stateMap->add(state);
            }
            
            m_initialState = m_stateMap->findState(startState);
            m_exitState = m_stateMap->findState(exitState);
        }

        LoginStateMachine::LoginStateMachine(LoginStateMachine *base,
                                             Connection::pointer connection) :
                m_stateMap(base->m_stateMap),
                m_initialState(base->m_initialState),
                m_exitState(base->m_exitState),
                m_currentState(m_initialState),
                m_connection(connection)
        {
            connection->setLoginStateMachine(this);
            enterState(m_initialState);
        }

        void LoginStateMachine::enterState(const std::string &name)
        {
            std::shared_ptr<LoginState> state = m_stateMap->findState(name);
            if (!state)
                state = m_initialState;

            enterState(state);
        }

        void LoginStateMachine::enterState(std::shared_ptr<LoginState> state)
        {
            LogPrint(LG_INFO, "Connection %d: state: %s -> %s",
                     m_connection->id(), m_currentState->name().c_str(),
                     state->name().c_str());
            m_currentState = state;
            state->enter(m_connection);
        }

        bool LoginStateMachine::handleLine(std::string line)
        {
            //LogPrint(LG_INFO, "Line: %s", toHex(line).c_str());
            std::string name = m_currentState->doState(m_connection, line);
            if (name != "no change") {
                enterState(name);
            }

            return (m_currentState == m_exitState);
        }
    }
}

std::string toHex(const std::string &line)
{
    std::string output;
    char hex[3];

    for (std::string::const_iterator it = line.begin(); it != line.end(); ++it)
    {
        sprintf(hex, "%02X ", (unsigned char)*it);
        output += std::string(hex);
    }

    return output;
}

havokmud::corefunc::LoginStateMachine *g_loginStateMachine = NULL;

