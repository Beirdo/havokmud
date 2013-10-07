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
#include "corefunc/Logging.hpp"
#include "objects/Connection.hpp"
#include <string>

namespace havokmud {
    namespace corefunc {
        using havokmud::objects::Connection;

        typedef boost::function<void (Connection::pointer connection)>
                LoginEntryFunction;
        typedef boost::function<const std::string (Connection::pointer connection,
                const std::string &)> LoginStateFunction;

        class LoginStateMachine;

        class LoginState
        {
        public:
            LoginState(std::string name,
                       LoginEntryFunction enterState_,
                       LoginStateFunction doState_) : m_name(name),
                    m_enterState(enterState_), m_doState(doState_),
                    m_id(s_nextId++)  {};
            ~LoginState()  {};

            int id() const  { return m_id; };
            const std::string &name() const  { return m_name; };
            void enter(Connection::pointer connection)
            {
                if (m_enterState)
                    m_enterState(connection);
            };
            const std::string doState(Connection::pointer connection,
                                      const std::string &line)
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

            static int s_nextId;
        };

        class LoginStateMap : private std::map<int, std::shared_ptr<LoginState> >
        {
        public:
            typedef std::map<int, std::shared_ptr<LoginState> > StateIdMap;
            typedef std::map<std::string, std::shared_ptr<LoginState> > StateNameMap;

            void add(std::shared_ptr<LoginState> state)
            {
                m_idMap.insert(std::pair<int, std::shared_ptr<LoginState> >(state->id(), state));
                m_nameMap.insert(std::pair<std::string, std::shared_ptr<LoginState> >(state->name(), state));
            }

            std::shared_ptr<LoginState> findState(int id)
            {
                StateIdMap::iterator it = m_idMap.find(id);
                if (it == m_idMap.end())
                    return std::shared_ptr<LoginState>();
                return it->second;
            }

            std::shared_ptr<LoginState> findState(const std::string &name)
            {
                if (name.empty())
                    return std::shared_ptr<LoginState>();

                StateNameMap::iterator it = m_nameMap.find(name);
                if (it == m_nameMap.end())
                    return std::shared_ptr<LoginState>();
                return it->second;
            }

        private:
            StateIdMap m_idMap;
            StateNameMap m_nameMap;
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
            LoginStateMachine(LoginStateMachine *base,
                              Connection::pointer connection);
            ~LoginStateMachine()  
            {
                LogPrint(LG_INFO, "Destroying login state machine");
            };
            static void initialize();

            bool handleLine(std::string line);
            void enterState(const std::string &name);

        private:
            void enterState(std::shared_ptr<LoginState> state);

            std::shared_ptr<LoginStateMap>  m_stateMap;
            std::shared_ptr<LoginState>     m_currentState;
            std::shared_ptr<LoginState>     m_initialState;
            std::shared_ptr<LoginState>     m_exitState;

            havokmud::objects::Connection::pointer  m_connection;
        };
    }
}

extern havokmud::corefunc::LoginStateMachine *g_loginStateMachine;

#endif  // __havokmud_corefunc_LoginStateMachine__
