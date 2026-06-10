#include "include/api/schedule.h"
#include <string.h>
#include "low_level_platform/api/low_level_platform.h"
#include "include/federate__ref/federate__ref.h"
#include "_federate__ref_main.h"
// ***** Start of method declarations.
// ***** End of method declarations.
_federate__ref_main_main_self_t* new__federate__ref_main() {
    _federate__ref_main_main_self_t* self = (_federate__ref_main_main_self_t*)lf_new_reactor(sizeof(_federate__ref_main_main_self_t));

    return self;
}
