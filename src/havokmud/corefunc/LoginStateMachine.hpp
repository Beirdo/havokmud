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
 * @brief Login state machine
 */

#ifndef __havokmud_corefunc_LoginStateMachine__
#define __havokmud_corefunc_LoginStateMachine__

#include <boost/function.hpp>
#include "objects/Connection.hpp"
#include <string>

namespace havokmud {
    namespace corefunc {
        using havokmud::objects::Connection;

        typedef boost::function<void (Connection *connection)>
                LoginEntryFunction;
        typedef boost::function<const std::string (Connection *connection,
                const std::string &)> LoginStateFunction;

        class LoginStateMachine;

        class LoginState
        {
        public:
            LoginState(std::string name,
                       LoginEntryFunction enterState,
                       LoginStateFunction doState);
            ~LoginState()  {};

            int id() const  { return m_id; };
            const std::string &name() const  { return m_name; };
            void enter(Connection *connection)
            {
                if (m_enterState)
                    m_enterState(connection);
            };
            const std::string doState(Connection *connection, std::string line)
            { 
                if (m_doState) 
                    return m_doState(connection, line);
                return m_name;
            };

            void setMachine(std::shared_ptr<LoginStateMachine> machine)
                    { m_machine = machine; };

        private:
            std::shared_ptr<LoginStateMachine>  m_machine;
            int                                 m_id;
            std::string                         m_name;
            LoginEntryFunction m_enterState;
            LoginStateFunction m_doState;
        };

        class LoginStateMap : private std::map<int, std::shared_ptr<LoginState> >
        {
        public:
            void add(int id, std::shared_ptr<LoginState> state)
            {
                insert(std::pair<int, std::shared_ptr<LoginState> >(id, state));
            }

            std::shared_ptr<LoginState> findState(int id)
            {
                iterator it = find(id);
                if (it == end())
                    return NULL;
                return it->second;
            }

            std::shared_ptr<LoginState> findState(const std::string &name)
            {
                iterator it;
                for (it = begin(); it != end(); ++it) {
                    if (it->second->name() == name)
                        return it->second;
                }

                return NULL;
            }
        };

        typedef struct {
            std::string name;
            LoginEntryFunction enterState;
            LoginStateFunction doState;
        } LoginStateItem;

        typedef std::vector<LoginStateItem> LoginStateTable;

        class LoginStateMachine
        {
        public:
            LoginStateMachine(const LoginStateTable &states,
                              const std::string &startState,
                              const std::string &exitState);
            LoginStateMachine(const LoginStateMachine &old);
            ~LoginStateMachine()  {};
            static void initialize();

            bool handleLine(const std::string &line);
            void enterState(const std::string &name);

            void setConnection(havokmud::objects::Connection *connection)
                    { m_connection = connection; };
        private:
            void enterState(std::shared_ptr<LoginState> state);

            std::shared_ptr<LoginStateMap>  m_stateMap;
            std::shared_ptr<LoginState>     m_currentState;
            std::shared_ptr<LoginState>     m_initialState;
            std::shared_ptr<LoginState>     m_exitState;

            havokmud::objects::Connection  *m_connection;
        };
    }
}

extern havokmud::corefunc::LoginStateMachine *g_loginStateMachine;

#endif  // __havokmud_corefunc_LoginStateMachine__
