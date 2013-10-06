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

#include "objects/Aggregate.hpp"
#include "corefunc/Logging.hpp"
#include "util/misc.hpp"

namespace havokmud {
    namespace objects {

        typedef struct {
            std::string opText;
            AggregateOperator op;
            std::type_index type;
        } AggregateItem;

        static AggregateItem operators[] = {
            { "+",    AGG_SUM,           typeid(double) },
            { "-",    AGG_SUB_FROM_CORE, typeid(double) },
            { "avg",  AGG_AVERAGE,       typeid(double) },
            { "+1/x", AGG_RECIP_SUM,     typeid(double) },
            { "|",    AGG_BIN_OR,        typeid(unsigned int) },
            { "&",    AGG_BIN_AND,       typeid(unsigned int) },
            { "||",   AGG_LOG_OR,        typeid(bool) },
            { "&&",   AGG_LOG_AND,       typeid(bool) },
            { ".",    AGG_CONCAT,        typeid(std::string) }
        };
        static int operatorCount = NELEMS(operators);

        Aggregate::Aggregate(const std::string &operatorText) :
                m_op(AGG_UNKNOWN), m_count(0), m_type(typeid(void))
        {
            for (int i = 0; i < operatorCount; i++) {
                if (operators[i].opText == operatorText) {
                    m_op   = operators[i].op;
                    m_type = operators[i].type;
                    break;
                }
            }
        }

        double Aggregate::to_double(std::string value)
        {
            return std::stod(value);
        }

        unsigned int Aggregate::to_unsigned_int(std::string value)
        {
            return std::stoul(value);
        }

        bool Aggregate::to_bool(std::string value)
        {
            if (value == "false")
                return false;

            if (value == "true")
                return true;

            int iVal = std::stoi(value);
            return (iVal != 0);
        }

        std::string Aggregate::to_string(std::string value)
        {
            return std::string(value);
        }

        double Aggregate::to_double(boost::any value)
        {
            try {
                double retValue = boost::any_cast<double>(value);
                return retValue;
            }
            catch(const boost::bad_any_cast &) {
                return 0.0;
            }
        }

        unsigned int Aggregate::to_unsigned_int(boost::any value)
        {
            try {
                unsigned int retValue = boost::any_cast<unsigned int>(value);
                return retValue;
            }
            catch(const boost::bad_any_cast &) {
                return 0;
            }
        }

        bool Aggregate::to_bool(boost::any value)
        {
            try {
                bool retValue = boost::any_cast<bool>(value);
                return retValue;
            }
            catch(const boost::bad_any_cast &) {
                return false;
            }
        }

        std::string Aggregate::to_string(boost::any value)
        {
            try {
                std::string retValue = boost::any_cast<std::string>(value);
                return retValue;
            }
            catch(const boost::bad_any_cast &) {
                return std::string();
            }
        }

        void Aggregate::setCore(std::string value)
        {
            switch (m_op) {
            case AGG_SUM:
            case AGG_AVERAGE:
            case AGG_SUB_FROM_CORE:
                m_value = to_double(value);
                m_count++;
                break;
            case AGG_RECIP_SUM:
                {
                    double fVal = to_double(value);
                    if (fVal != 0.0) {
                        m_value = (1.0 / fVal);
                        m_count++;
                    }
                }
                break;
            case AGG_BIN_OR:
            case AGG_BIN_AND:
                m_value = to_unsigned_int(value);
                m_count++;
                break;
            case AGG_LOG_OR:
            case AGG_LOG_AND:
                m_value = to_bool(value);
                m_count++;
                break;
            case AGG_CONCAT:
                m_value = to_string(value);
                m_count++;
                break;
            default:
                break;
            }
        };

        void Aggregate::aggregate(std::string value)
        {
            switch (m_op) {
            case AGG_SUM:
            case AGG_AVERAGE:
                {
                    double fVal = to_double(m_value);
                    fVal += to_double(value);
                    m_value = fVal;
                }
                m_count++;
                break;
            case AGG_SUB_FROM_CORE:
                {
                    double fVal = to_double(m_value);
                    fVal -= to_double(value);
                    m_value = fVal;
                }
                m_count++;
                break;
            case AGG_RECIP_SUM:
                {
                    double fVal = to_double(value);
                    if (fVal != 0.0) {
                        fVal = 1.0 / fVal;
                        fVal += to_double(m_value);
                        m_value = fVal;
                        m_count++;
                    }
                }
                break;
            case AGG_BIN_OR:
                {
                    unsigned int iVal = to_unsigned_int(m_value);
                    iVal |= to_unsigned_int(value);
                    m_value = iVal;
                }
                m_count++;
                break;
            case AGG_BIN_AND:
                {
                    unsigned int iVal = to_unsigned_int(m_value);
                    iVal &= to_unsigned_int(value);
                    m_value = iVal;
                }
                m_count++;
                break;
            case AGG_LOG_OR:
                {
                    bool bVal = to_bool(m_value);
                    bVal = bVal || to_bool(value);
                    m_value = bVal;
                }
                m_count++;
                break;
            case AGG_LOG_AND:
                {
                    bool bVal = to_bool(m_value);
                    bVal = bVal && to_bool(value);
                    m_value = bVal;
                }
                m_count++;
                break;
            case AGG_CONCAT:
                {
                    std::string sVal = to_string(m_value);
                    sVal += to_string(value);
                    m_value = sVal;
                }
                m_count++;
                break;
            default:
                break;
            }
        }

        boost::any Aggregate::get()
        {
            boost::any value;

            switch (m_op) {
            case AGG_SUM:
            case AGG_SUB_FROM_CORE:
            case AGG_BIN_OR:
            case AGG_BIN_AND:
            case AGG_LOG_OR:
            case AGG_LOG_AND:
            case AGG_CONCAT:
                value = m_value;
                break;

            case AGG_AVERAGE:
                if (m_count) {
                    value = to_double(value) / (double)m_count;
                } else {
                    value = 0.0;
                }
                break;

            case AGG_RECIP_SUM:
                {
                    double fVal = to_double(m_value);
                    if (fVal != 0.0) {
                        value = 1.0 / fVal;
                    } else {
                        value = 0.0;
                    }
                }
                break;
            default:
                value = boost::any();
                break;
            }

            return value;
        }

    }
}

