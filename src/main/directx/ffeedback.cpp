#include "ffeedback.hpp"

//-----------------------------------------------------------------------------
// Dummy Functions For Non-Windows Builds
//-----------------------------------------------------------------------------
namespace forcefeedback
{
    bool init(int a, int b, int c) { return false; } // Did not initialize
    void close()                   {}
    int  set(int x, int f)         { return 0; }
    bool is_supported()            { return false; } // Not supported
};
