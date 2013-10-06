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

#include <boost/any.hpp>
#include <string>
#include <map>

namespace havokmud {
    namespace objects {

        typedef std::map<std::string, boost::any> SettingsMap;

        class Settings
        {
        public:
            Settings();
            ~Settings()  {};

            bool load(const std::string &setting);
            void save(const std::string &setting);

            template <class T>
            void set(const std::string &setting, T value)
            {
                boost::any mapValue  = value;
                m_map[setting] = mapValue;

                save(setting);
            };

            template <class T>
            T get(const std::string &setting)
            {
                SettingsMap::iterator it = m_map.find(setting);
                if (it == m_map.end()) {
                    if (!load(setting)) {
                        return T();
                    }
                    it = m_map.find(setting);
                }

                boost::any mapValue = it->second;

                try {
                    T value = boost::any_cast<T>(mapValue);
                    return value;
                }
                catch(const boost::bad_any_cast &) {
                    return T();
                }
            };

        private:
            SettingsMap m_map;
        };
    }
}

extern havokmud::objects::Settings g_settings;

#endif  // __havokmud_objects_Settings__
