#include "include/api/schedule.h"
#include <string.h>
#include "low_level_platform/api/low_level_platform.h"
#include "include/federate__optimizer/federate__optimizer.h"
#include "_federate__optimizer_main.h"
// ***** Start of method declarations.
// ***** End of method declarations.
_federate__optimizer_main_main_self_t* new__federate__optimizer_main() {
    _federate__optimizer_main_main_self_t* self = (_federate__optimizer_main_main_self_t*)lf_new_reactor(sizeof(_federate__optimizer_main_main_self_t));

    return self;
}
