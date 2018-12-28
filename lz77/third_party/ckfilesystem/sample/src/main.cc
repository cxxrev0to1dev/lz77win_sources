#include <iostream>
#include <ckcore/filestream.hh>
#include <ckcore/directory.hh>
#include <ckfilesystem/const.hh>
#include <ckfilesystem/fileset.hh>
#include <ckfilesystem/sectorstream.hh>
#include <ckfilesystem/filesystemwriter.hh>
#include "progress.hh"
#include "log.hh"

bool AddFiles(ckfilesystem::FileSet &file_set,std::vector<ckcore::Directory *> &dir_stack,
              ckcore::Directory &dir,const ckcore::tchar *base_path)
{
    ckcore::Path path(dir.name().c_str());

    // Calculate the pointer offset for caluclating the internal file or
    // directory path.
    size_t base_len = ckcore::string::astrlen(base_path);
    if (base_path[base_len - 1] == '\\' ||
        base_path[base_len - 1] == '/')
        base_len--;

    ckcore::Directory::Iterator it;
    for (it = dir.begin(); it != dir.end(); it++)
    {
        ckcore::Path new_path = path + (*it).c_str();

        const ckcore::tchar *ext_path = new_path.name().c_str();
        const ckcore::tchar *int_path = ext_path + base_len;

        // Check if we're dealing with a file or directory.
        if (ckcore::Directory::exist(new_path))
        {
            file_set.insert(new ckfilesystem::FileDescriptor(int_path,ext_path,
                                                             ckfilesystem::FileDescriptor::FLAG_DIRECTORY));
            dir_stack.push_back(new ckcore::Directory(new_path));
        }
        else
        {
            file_set.insert(new ckfilesystem::FileDescriptor(int_path,ext_path));
        }
    }

    return true;
}

int main(int argc,const char *argv[])
{
    // Validate the arguments.
    if (argc != 2)
    {
        std::cerr << "Error: Invalid number of arguments." << std::endl;
        return 1;
    }

    const ckcore::tchar *base_path = argv[1];
    ckcore::Directory dir(base_path);
    if (!dir.exist())
    {
        std::cerr << "Error: The specified folder does not exist" << std::endl;
        return 1;
    }

    // Create a non-DVD-Video file system.
    ckfilesystem::FileComparator filecomp;
    ckfilesystem::FileSet file_set(filecomp);

    std::cout << "Building file tree." << std::endl;

    // Add all file decsiptors.
    std::vector<ckcore::Directory *> dir_stack;
    if (!AddFiles(file_set,dir_stack,dir,base_path))
    {
        std::cerr << "Error: Failed to list directory contents." << std::endl;
        return 1;
    }

    while (dir_stack.size() > 0)
    { 
        ckcore::Directory *cur_dir = dir_stack[dir_stack.size() - 1];
        dir_stack.pop_back();

        bool res = AddFiles(file_set,dir_stack,*cur_dir,base_path);

        delete cur_dir;

        if (!res)
        {
            std::cerr << "Error: Failed to list directory contents." << std::endl;
            return 1;
        }
    }

    std::cout << "Writing disc image." << std::endl;

    // Create output stream.
    ckcore::FileOutStream filestream(ckT("out.iso"));
    if (!filestream.open())
    {
        std::cerr << "Error: Could not open output file for writing" << std::endl;
        return 1;
    }

    // Create disc image.
    Progress progress;
    Log log;

    ckfilesystem::FileSystem file_sys(ckfilesystem::FileSystem::TYPE_UDF,file_set);
    file_sys.set_volume_label(ckT("My Volume"));

    ckfilesystem::FileSystemWriter writer(log,file_sys,true);
    if (writer.write(filestream,progress) != RESULT_OK)
    {
        std::cerr << "Error: Failed to create disc image." << std::endl;
        return 1;
    }

    return 0;
}

