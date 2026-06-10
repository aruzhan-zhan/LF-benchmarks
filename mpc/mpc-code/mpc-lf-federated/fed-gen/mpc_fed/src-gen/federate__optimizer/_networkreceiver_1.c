#include "include/api/schedule.h"
#include <string.h>
#include "low_level_platform/api/low_level_platform.h"
#include "include/federate__optimizer/NetworkReceiver_1.h"
#include "_networkreceiver_1.h"
// ***** Start of method declarations.
// ***** End of method declarations.
#include "include/api/reaction_macros.h"
void _networkreceiver_1reaction_function_0(void* instance_args) {
    _networkreceiver_1_self_t* self = (_networkreceiver_1_self_t*)instance_args; SUPPRESS_UNUSED_WARNING(self);
    lf_critical_section_enter(self->base.environment);
    // Expose the action struct as a local variable whose name matches the action name.
    _networkreceiver_1_networkMessage_t* networkMessage = &self->_lf_networkMessage;
    // Set the fields of the action struct to match the current trigger.
    networkMessage->is_present = (bool)self->_lf__networkMessage.status;
    networkMessage->has_value = ((self->_lf__networkMessage.tmplt.token) != NULL && (self->_lf__networkMessage.tmplt.token)->value != NULL);
    _lf_replace_template_token((token_template_t*)networkMessage, (self->_lf__networkMessage.tmplt.token));
    if (networkMessage->has_value) {
        networkMessage->value = *(state_t*)(self->_lf__networkMessage.tmplt.token)->value;
    }
    lf_critical_section_exit(self->base.environment);
    _networkreceiver_1_msg_t* msg = &self->_lf_msg;
    #line 207 "/home/aruzhanmassalina/REPOS/LF-benchmarks/mpc-code/mpc-lf-federated/./fed-gen/mpc_fed/src/federate__optimizer.lf"
    msg->physical_time_of_arrival = self->_lf__networkMessage.physical_time_of_arrival;
    lf_set(msg, networkMessage->value);
#line 27 "/home/aruzhanmassalina/REPOS/LF-benchmarks/mpc-code/mpc-lf-federated/./fed-gen/mpc_fed/src-gen/federate__optimizer/_networkreceiver_1.c"
}
#include "include/api/reaction_macros_undef.h"
_networkreceiver_1_self_t* new__networkreceiver_1() {
    _networkreceiver_1_self_t* self = (_networkreceiver_1_self_t*)lf_new_reactor(sizeof(_networkreceiver_1_self_t));
    self->_lf_networkMessage._base.trigger = &self->_lf__networkMessage;
    self->_lf_networkMessage.parent = (self_base_t*)self;
    self->_lf_networkMessage.source_id = -1;
    self->_lf__reaction_0.number = 0;
    self->_lf__reaction_0.function = _networkreceiver_1reaction_function_0;
    self->_lf__reaction_0.self = self;
    self->_lf__reaction_0.deadline_violation_handler = NULL;
    self->_lf__reaction_0.STP_handler = NULL;
    self->_lf__reaction_0.name = "?";
    self->_lf__reaction_0.mode = NULL;
    self->_lf__reaction_0.is_an_input_reaction = true;
    self->_lf__networkMessage.last_tag = NEVER_TAG;
    #ifdef FEDERATED_DECENTRALIZED
    self->_lf__networkMessage.intended_tag = (tag_t) { .time = NEVER, .microstep = 0u};
    #endif // FEDERATED_DECENTRALIZED
    self->_lf__networkMessage_reactions[0] = &self->_lf__reaction_0;
    self->_lf__networkMessage.reactions = &self->_lf__networkMessage_reactions[0];
    self->_lf__networkMessage.number_of_reactions = 1;
    #ifdef FEDERATED
    self->_lf__networkMessage.physical_time_of_arrival = NEVER;
    #endif // FEDERATED
    self->_lf__networkMessage.is_physical = false;
    
    self->_lf__networkMessage.tmplt.type.element_size = sizeof(state_t);
    self->_lf_networkMessage.type.element_size = sizeof(state_t);
    self->_lf__networkMessage.tmplt.length = 1;
    self->_lf_networkMessage.length = 1;
    return self;
}
