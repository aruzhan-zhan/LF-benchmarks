#include "include/api/schedule.h"
#include <string.h>
#include "low_level_platform/api/low_level_platform.h"
#include "include/mpc/mpc.h"
#include "_mpc_main.h"
// ***** Start of method declarations.
// ***** End of method declarations.
_mpc_main_main_self_t* new__mpc_main() {
    _mpc_main_main_self_t* self = (_mpc_main_main_self_t*)lf_new_reactor(sizeof(_mpc_main_main_self_t));

    return self;
}
