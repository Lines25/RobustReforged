#include <cstdio>

void reforged_log(const char* message) {
    fprintf(stdout, "[Reforged/Native] %s\n", message);
    fflush(stdout);
}
