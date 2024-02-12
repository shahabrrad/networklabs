#include "utils.h"
#include <string.h>

void remove_trailing_Z(char *str) {
    int len = strlen(str);
    while (len > 0 && str[len - 1] == 'Z') {
        str[len - 1] = '\0'; // Replace 'Z' with null terminator
        len--; // Move backwards in the string
    }
}
