#include "ch_string.h"

usize ch::strlen(const tchar* c_str) {
    for (usize i = 0; i < U64_MAX; i++) {
        if (c_str[i] == 0) {
            return i;
        }
    }

    return 0;
}
