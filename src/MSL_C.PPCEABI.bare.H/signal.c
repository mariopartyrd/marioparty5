#include <stddef.h>
#include "PowerPC_EABI_Support/Msl/MSL_C/MSL_Common/signal.h"

__signal_func_ptr signal_funcs[6];

int raise(int sig) {
    __signal_func_ptr temp_r31;

    if (sig < 1 || sig > 6) {
        return -1;
    }
    temp_r31 = signal_funcs[sig - 1];
    if ((unsigned long) temp_r31 != 1) {
        signal_funcs[sig - 1] = NULL;
    }
    if ((unsigned long) temp_r31 == 1 || (temp_r31 == NULL && sig == 1)) {
        return 0;
    }
    if (temp_r31 == NULL) {
        exit(0);
    }
    temp_r31(sig);
    return 0;
}
