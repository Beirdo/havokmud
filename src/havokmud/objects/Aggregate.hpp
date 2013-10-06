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
 * @brief Attribute aggregation
 */

#ifndef __havokmud_objects_Aggregate__
#define __havokmud_objects_Aggregate__

#include <boost/any.hpp>
#include <string>
#include <typeinfo>
#include <typeindex>

namespace havokmud {
    namespace objects {

        typedef enum {
            AGG_SUM,
            AGG_SUB_FROM_CORE,
            AGG_AVERAGE,
            AGG_RECIP_SUM,
            AGG_BIN_OR,
            AGG_BIN_AND,
            AGG_LOG_OR,
            AGG_LOG_AND,
            AGG_CONCAT,
            AGG_UNKNOWN
        } AggregateOperator;

        class Aggregate
        {
        public:
            Aggregate(const std::string &operatorText);
            ~Aggregate()  {};

            double to_double(std::string value);
            unsigned int to_unsigned_int(std::string value);
            bool to_bool(std::string value);
            std::string to_string(std::string value);

            double to_double(boost::any value);
            unsigned int to_unsigned_int(boost::any value);
            bool to_bool(boost::any value);
            std::string to_string(boost::any value);

            void setCore(std::string value);
            void aggregate(std::string value);

            boost::any get();

            template <class T>
            T get()
            {
                boost::any value = get();

                try {
                    T retValue = boost::any_cast<T>(value);
                    return retValue;
                }
                catch(const boost::bad_any_cast &) {
                    return T();
                }
            };

        private:
            AggregateOperator m_op;
            boost::any m_value;
            int m_count;
            std::type_index m_type;
        };
    }
}

#endif  // __havokmud_objects_Aggregate__
