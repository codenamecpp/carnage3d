#include "stdafx.h"

int main(int argc, char *argv[])
{
#if OS_NAME == OS_WINDOWS
    _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif

    SysStartupParameters sysStartupParams;

    for (int iarg = 1; iarg < argc; )
    {
        if (cxx_stricmp(argv[iarg], "-mapname") == 0 && (argc > iarg + 1))
        {
            sysStartupParams.mDebugMapName.set_content(argv[iarg + 1]);
            iarg += 2;
            continue;
        }
        if (cxx_stricmp(argv[iarg], "-gtadata") == 0 && (argc > iarg + 1))
        {
            sysStartupParams.mGtaDataLocation.set_content(argv[iarg + 1]);
            iarg += 2;
            continue;
        }
        if (cxx_stricmp(argv[iarg], "-numplayers") == 0 && (argc > iarg + 1))
        {
            ::sscanf(argv[iarg + 1], "%d", &sysStartupParams.mPlayersCount);
            iarg += 2;
            continue;
        }
        ++iarg;
    }

    gSystem.Execute(sysStartupParams);
    return 0;
}