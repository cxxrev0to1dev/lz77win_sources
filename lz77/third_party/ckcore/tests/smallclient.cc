#include <iostream>

#ifdef _WINDOWS
#include <string>
#include <windows.h>
#define sleep(x) Sleep(x*1000)
#elif defined(_UNIX)
#include <string.h>
#else
#error "Unknown platform"
#endif

int main(int argc,const char *argv[])
{
    int mode = 1;

    if (argc >= 2)
    {
        if (!strcmp(argv[1],"-m2"))
            mode = 2;
        else if (!strcmp(argv[1],"-m3"))
            mode = 3;
        else if (!strcmp(argv[1],"-m4"))
            mode = 4;
        else if (!strcmp(argv[1],"-m5"))
            mode = 5;
    }

    std::cout << "SmallClient" << std::endl;

    switch (mode)
    {
        // Test fragmented message.
        case 1:
            std::cout << "MESSA";
            std::cout.flush();
            sleep(1);
            std::cout << "GE 1" << std::endl;
            break;

        // Test interleaved writing to standard output and standard error.
        case 2:
            std::cout << "MESSAGE ";
            std::cout.flush();
            std::cerr << "MESSAGE ";
            std::cerr.flush();
            std::cout << "1" << std::endl;
            std::cerr << "2" << std::endl;
            break;

        // Test killing a process.
        case 3:
            std::cout << "Sleeping for 30 seconds" << std::endl;
            sleep(30);
            break;

        // Test standard input.
        case 4:
            {
                std::cout << "MESSAGE 1" << std::endl;

                std::string str;
                int i = 0;
                std::cin >> str >> i;

                if (str == "TEST" && i == 1)
                    std::cout << "RESPONSE 1" << std::endl;
                else
                    std::cout << "UNKNOWN" << std::endl;
            }
            break;

        // Test exit code.
        case 5:
            return 42;
    }

    return 0;
}
