#ifndef VIRTUAL_SESSION_H_
#define VIRTUAL_SESSION_H_

#include "machine.h"

/*
 * struct session: Represents a user session running a virtual machine
 */
struct session {
    /*
     * machine: Represents a single virtual machine the user is currently
     * running
     */
    struct machine* machine;
};

#endif  // VIRTUAL_SESSION_H_

