/*
 * The ckFileSystem library provides file system functionality.
 * Copyright (C) 2006-2011 Christian Kindahl
 * 
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#pragma once
#include <vector>
#include <ckcore/types.hh>
#include <ckcore/filestream.hh>

#define IFO_INDETIFIER_VMG              "DVDVIDEO-VMG"
#define IFO_INDETIFIER_VTS              "DVDVIDEO-VTS"
#define IFO_IDENTIFIER_LEN              12

namespace ckfilesystem
{
    class IfoVmgData
    {
    public:
        ckcore::tuint32 last_vmg_sec_;
        ckcore::tuint32 last_vmg_ifo_sec_;
        ckcore::tuint16 num_vmg_ts_;
        ckcore::tuint32 vmg_menu_vob_sec_;
        ckcore::tuint32 srpt_sec_;

        std::vector<ckcore::tuint32> titles_;
    };

    class IfoVtsData
    {
    public:
        ckcore::tuint32 last_vts_sec_;
        ckcore::tuint32 last_vts_ifo_sec_;
        ckcore::tuint32 vts_menu_vob_sec_;
        ckcore::tuint32 vts_vob_sec_;
    };

    class IfoReader
    {
    public:
        enum IfoType
        {
            IT_UNKNOWN,
            IT_VMG,
            IT_VTS
        };

    private:
        IfoType ifo_type_;

        ckcore::FileInStream in_stream_;

    public:

        IfoReader(const ckcore::tchar *full_path);
        ~IfoReader();

        bool open();
        bool close();

        bool read_vmg(IfoVmgData &vmg_data);
        bool read_vts(IfoVtsData &vts_data);

        IfoType get_type();
    };
};
