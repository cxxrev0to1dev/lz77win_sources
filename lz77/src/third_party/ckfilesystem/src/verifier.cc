#include <iostream>
#include <ckcore/path.hh>
#include <ckcore/file.hh>
#include <ckcore/string.hh>
#include <ckcore/filestream.hh>
#include <ckcore/exception.hh>
#include "ckfilesystem/sectorstream.hh"
#include "ckfilesystem/isoverifier.hh"

#define CKFSVFY_VERSION         "0.1"

#if defined(_WINDOWS) && defined(_UNICODE)
std::wostream &out = std::wcout;
std::wostream &err = std::wcerr;
#else
std::ostream &out = std::cout;
std::ostream &err = std::cerr;
#endif

int main(int argc,const char *argv[])
{
    out << "ckFsVerifier " << CKFSVFY_VERSION
        << " Copyright (C) Christian Kindahl 2009" << std::endl << std::endl;

    // Parse the command line.
    if (argc != 2)
    {
        err << "Error: Invalid usage, please specify a disc image file to analyze."
            << std::endl;
        return 1;
    }

    ckcore::Path file_path(ckcore::string::ansi_to_auto<1024>(argv[1]).c_str());
    if (!ckcore::File::exist(file_path))
    {
        err << "Error: The specified file doesn't exist." << std::endl;
        return 1;
    }

    // Prepare file stream.
    ckcore::FileInStream file_stream(file_path);
    if (!file_stream.open())
    {
        err << "Error: Unable to open file for reading." << std::endl;
        return 1;
    }

    // Verify the file system.
    try
    {
        out << ckT("Input: ") << std::endl << ckT("  File: ")
            << file_path.name() << std::endl << ckT("  Size: ")
            << file_stream.size() << ckT(" bytes") << std::endl
            << std::endl;

        ckfilesystem::IsoVerifier verifier;

        ckfilesystem::SectorInStream in_stream(file_stream);
        verifier.verify(in_stream);
    }
    catch (ckfilesystem::VerificationException &e)
    {
        err << ckT("=> Error: ") << e.what() << std::endl;
        err << ckT("=>        See ") << e.reference() << std::endl;
        return 1;
    }
    catch (const std::exception &e)
    {
        err << ckT("=> Error: ") << ckcore::get_except_msg(e) << std::endl;
        return 1;
    }

    return 0;
}
