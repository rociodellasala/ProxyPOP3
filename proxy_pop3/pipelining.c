#include <string.h>
#include "include/utils.h"
#include <stdbool.h>
#include "include/buffer.h"
#include <stdio.h>

bool is_pipelining_available(const char * capa_response) {
    char * pipelining = "PIPELINING";

    if (strstr(capa_response, pipelining) != NULL) {
        return true;
    } else {
        return false;
    }

}