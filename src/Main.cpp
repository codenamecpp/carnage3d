#include "stdafx.h"

int main(int argc, char *argv[])
{
#if OS_NAME == OS_WINDOWS
    _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif

    gSystem.Initialize(argc - 1, argv + 1);
    gSystem.Execute();
    gSystem.Deinit();

    return 0;
}