#include "Aeroki.h"

int main(int argc, char *argv[]) {
    if (argc == 1) {
        __Ark_Shell();
        return 0;
    }

    FILE *src = fopen(argv[1], "r");
    if (!src) {
        // Try to create the missing file automatically
        src = fopen(argv[1], "w");
        if (!src) {
            fprintf(stderr, "Cannot create file: %s\n", argv[1]);
            return 1;
        }
        fprintf(src, "// Auto-created by Aeroki runtime\n");
        fclose(src);

        src = fopen(argv[1], "r");
        if (!src) {
            fprintf(stderr, "Cannot reopen file: %s\n", argv[1]);
            return 1;
        }
    }

    __Ark_Interpreted(src);
    fclose(src);

    return 0;
}
