/***************************************************************************
    Copyright Chris White.
    See license.txt for more details.
***************************************************************************/

#pragma once

//-----------------------------------------------------------------------------
// Function prototypes 
//-----------------------------------------------------------------------------
namespace forcefeedback
{
    extern bool init(int max_force, int min_force, int force_duration);
    extern void close();
    extern int  set(int xdirection, int force);
    extern bool is_supported();
};
