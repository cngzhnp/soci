//
// Copyright (C) 2004-2006 Maciej Sobczak, Stephen Hutton, Rafal Bobrowski
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//

#define SOCI_FIREBIRD_SOURCE
#include "soci-firebird.h"
#include "error-firebird.h"

#include <cstdlib>
#include <string>

namespace soci
{

firebird_soci_error::firebird_soci_error(std::string const & msg, ISC_STATUS const * status)
    : soci_error(msg)
{
    if (status != 0)
    {
        std::size_t i = 0;
        while (i < stat_size && status[i] != 0)
        {
            status_.push_back(status[i++]);
        }
    }
}

namespace details
{

namespace firebird
{

void get_iscerror_details(ISC_STATUS * status_vector, std::string &msg)
{
    char msg_buffer[SOCI_FIREBIRD_ERRMSG];
    long *pvector = status_vector;

    try
    {
        // fetching first error message
        isc_interprete(msg_buffer, &pvector);
        msg = msg_buffer;

        // fetching next errors
        while (isc_interprete(msg_buffer, &pvector))
        {
            msg += "\n";
            msg += msg_buffer;
        }
    }
    catch (...)
    {
        throw firebird_soci_error("Exception catched while fetching error information");
    }
}

bool check_iscerror(ISC_STATUS const * status_vector, long errNum)
{
    std::size_t i=0;
    while (status_vector[i] != 0)
    {
        if (status_vector[i] == 1 && status_vector[i+1] == errNum)
        {
            return true;
        }
        ++i;
    }

    return false;
}
void throw_iscerror(ISC_STATUS * status_vector)
{
    std::string msg;

    get_iscerror_details(status_vector, msg);
    throw firebird_soci_error(msg, status_vector);
}

} // namespace firebird

} // namespace details

} // namespace soci