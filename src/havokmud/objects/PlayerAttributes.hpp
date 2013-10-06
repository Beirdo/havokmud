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
 * @brief Player Attributes object
 */

#ifndef __havokmud_objects_PlayerAttributes__
#define __havokmud_objects_PlayerAttributes__

#include <boost/property_tree/json_parser.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/any.hpp>
#include <boost/foreach.hpp>
#include <sstream>
#include <string>

#include "objects/Aggregate.hpp"

namespace havokmud {
    namespace objects {
        using boost::property_tree::ptree;

        class Player;

        class PlayerAttributes
        {
        public:
            PlayerAttributes(Player *player) : m_player(player)  {};
            ~PlayerAttributes()  {};

            void load();
            void save();

            void remove(const std::string &attrib,
                        const std::string &source)
            {
                std::string attrKey   = attrib + "." + source;
                std::string sourceKey = source + "." + attrib;

                m_attributeTree.erase(attrKey);
                m_attributeSourceTree.erase(attrKey);
            }

            template <class T>
            void set(const std::string &attrib,
                     const std::string &source, T value)
            {
                boost::any  anyValue  = value;
                std::string mapValue;
                try {
                    mapValue = boost::any_cast<std::string>(anyValue);
                }
                catch(const boost::bad_any_cast &) {
                }

                std::string attrKey   = attrib + "." + source;
                std::string sourceKey = source + "." + attrib;

                m_attributeTree.put(attrKey, mapValue);
                m_attributeSourceTree.put(sourceKey, mapValue);
            };

            template <class T>
            T get(const std::string &attrib,
                  const std::string &source)
            {
                if (source.empty()) {
                    return getAggregate<T>(attrib);
                }

                std::string attrKey = attrib + "." + source;
                std::string mapValue =
                      m_attributeTree.get<std::string>(attrKey, std::string());

                try {
                    boost::any anyValue = mapValue;
                    T value = boost::any_cast<T>(anyValue);
                    return value;
                }
                catch(const boost::bad_any_cast &) {
                    return T();
                }
            };

            template <class T>
            T getAggregate(const std::string &attrib)
            {
                std::string aggOpText = get<std::string>(attrib, "aggregate");
                if (aggOpText.empty())
                    aggOpText = "+";

                Aggregate agg(aggOpText);
                ptree attribTree = m_attributeTree.get_child(attrib);

                agg.setCore(attribTree.get<std::string>("core-pc",
                                                        std::string()));

                BOOST_FOREACH(ptree::value_type &v, attribTree) {
                    if (v.first != "aggregate" && v.first != "core-pc") {
                        agg.aggregate(v.second.data());
                    }
                }

                return agg.get<T>();
            };

        private:
            Player         *m_player;
            ptree           m_attributeTree;
            ptree           m_attributeSourceTree;
        };
    }
}

#endif  // __havokmud_objects_PlayerAttributes__
