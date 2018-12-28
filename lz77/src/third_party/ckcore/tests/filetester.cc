#include <iostream>
#include <assert.h>

#ifdef _WINDOWS
#include <string>
#include <windows.h>
#elif defined(_UNIX)
#include <string.h>
#else
#error "Unknown platform."
#endif

#include <ckcore/path.hh>
#include <ckcore/file.hh>
#include <ckcore/string.hh>

int main(int argc,const char *argv[])
{
    try
    {
        std::cout << "FileTester" << std::endl;

        // Validate program arguments.
        if (argc != 3)
        {
            std::cerr << "Error: Invalid program arguments." << std::endl;
            return 1;
        }

        enum
        {
            MODE_READ,
            MODE_WRITE,
            MODE_DELETE
        } mode;

        if (!strcmp(argv[1],"-r"))
            mode = MODE_READ;
        else if (!strcmp(argv[1],"-w"))
            mode = MODE_WRITE;
        else if (!strcmp(argv[1],"-d"))
            mode = MODE_DELETE;
        else
        {
            std::cerr << "Error: Invalid mode." << std::endl;
            return 1;
        }

        const ckcore::Path file_path = ckcore::string::ansi_to_auto<1024>(argv[2]).c_str();

        if (!ckcore::File::exist(file_path))
        {
            std::wcerr << ckT("Error: The file ") << file_path.name().c_str()
                       << ckT(" doesn't exist.") << std::endl;
            return 1;
        }

        // Take the appropriate action.
        ckcore::File file(file_path);

        switch (mode)
        {
            case MODE_READ:
                file.open2(ckcore::File::ckOPEN_READ);
                file.close();
                break;

            case MODE_WRITE:
                file.open2(ckcore::File::ckOPEN_WRITE);
                file.close();
                break;

            case MODE_DELETE:
                if (!file.remove())
                    return 1;
                break;

            default:
                assert(false);
        }

        return 0;
    }
    catch (const std::exception & e)
    {
      return 1;
    }
}