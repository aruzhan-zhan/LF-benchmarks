#include "include/api/schedule.h"
#include <string.h>
#include "low_level_platform/api/low_level_platform.h"
#include "include/federate__plant/federate__plant.h"
#include "_federate__plant_main.h"
// ***** Start of method declarations.
// ***** End of method declarations.
_federate__plant_main_main_self_t* new__federate__plant_main() {
    _federate__plant_main_main_self_t* self = (_federate__plant_main_main_self_t*)lf_new_reactor(sizeof(_federate__plant_main_main_self_t));

    return self;
}
