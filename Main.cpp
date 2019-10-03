#include "stdafx.h"

int main()
{
#if OS_NAME == OS_WINDOWS
    _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif
    gSystem.Execute();
    return 0;
}