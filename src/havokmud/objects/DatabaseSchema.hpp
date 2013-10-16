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
 * @brief Database schemas
 */

#ifndef __havokmud_objects_DatabaseSchema__
#define __havokmud_objects_DatabaseSchema__

#include <vector>
#include <string>

#include "objects/DatabaseSchema.hpp"

#define CURRENT_CORE_SCHEMA 5

namespace havokmud {
    namespace objects {

        typedef std::vector<std::string> DatabaseSchemaItem;

        class DatabaseSchema
        {
        public:
            DatabaseSchema(const std::string &name_, int supportedVersion_,
                    const DatabaseSchemaItem baseSchema_,
                    const std::vector<DatabaseSchemaItem> &upgradeItems_) :
                m_name(name_), m_supportedVersion(supportedVersion_),
                m_baseSchema(baseSchema_), m_upgradeItems(upgradeItems_)  {};
            ~DatabaseSchema()  {};

            std::string &name()  { return m_name; };
            int supportedVersion() const  { return m_supportedVersion; };
            const DatabaseSchemaItem &baseSchema() const
                    { return m_baseSchema; };
            const std::vector<DatabaseSchemaItem> &upgradeItems() const
                    { return m_upgradeItems; };

        private:
            std::string m_name;
            int m_supportedVersion;
            DatabaseSchemaItem m_baseSchema;
            std::vector<DatabaseSchemaItem> m_upgradeItems;
        };
    }
}

#endif  // __havokmud_objects_DatabaseSchema__
