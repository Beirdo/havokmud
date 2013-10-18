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
 * @brief Settings object
 */

#ifndef __havokmud_objects_Settings__
#define __havokmud_objects_Settings__

#include <boost/lexical_cast.hpp>
#include <string>
#include <map>

#include "corefunc/Logging.hpp"

namespace havokmud {
    namespace objects {

        typedef std::map<std::string, std::string> SettingsMap;

        class Settings
        {
        public:
            Settings();
            ~Settings()  {};

            bool load(const std::string &setting);
            void save(const std::string &setting);

            void refresh(void)
            {
                m_map.clear();
            }

            template <class T>
            void set(const std::string &setting, T value, bool doSave = true)
            {
                std::string mapValue;
                try {
                    mapValue = boost::lexical_cast<std::string>(value);
                }
                catch(const boost::bad_lexical_cast &) {
                    return;
                }

                //LogPrint(LG_INFO, "Setting %s to %s", setting.c_str(),
                //         mapValue.c_str());
                m_map[setting] = mapValue;

                if (doSave)
                    save(setting);
            };

            template <class T>
            T get(const std::string &setting, T defaultValue = T())
            {
                SettingsMap::iterator it = m_map.find(setting);
                if (it == m_map.end()) {
                    if (!load(setting)) {
                        return defaultValue;
                    }
                    it = m_map.find(setting);
                }

                std::string mapValue = it->second;
                //LogPrint(LG_INFO, "Getting setting %s = %s", setting.c_str(),
                //         mapValue.c_str());

                try {
                    T value = boost::lexical_cast<T>(mapValue);
                    return value;
                }
                catch(const boost::bad_lexical_cast &) {
                    return defaultValue;
                }
            };

        private:
            SettingsMap m_map;
        };
    }
}

extern havokmud::objects::Settings g_settings;

#endif  // __havokmud_objects_Settings__
