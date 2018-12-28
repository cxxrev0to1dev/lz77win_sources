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

#include <string.h>
#include <ckcore/convert.hh>
#include "ckfilesystem/dvdvideo.hh"
#include "ckfilesystem/iforeader.hh"

namespace ckfilesystem
{
    IfoReader::IfoReader(const ckcore::tchar *full_path) :
        ifo_type_(IT_UNKNOWN),in_stream_(full_path)
    {
    }

    IfoReader::~IfoReader()
    {
        close();
    }

    /**
        Open the IFO file and determine it's type.
        @param full_path the full file path to the file to open.
        @return true if the file was successfully opened and identified, false
        otherwise.
     */
    bool IfoReader::open()
    {
        if (!in_stream_.open())
            return false;

        char identifier[IFO_IDENTIFIER_LEN + 1];
        ckcore::tint64 processed = in_stream_.read(identifier,IFO_IDENTIFIER_LEN);
        if (processed == -1)
        {
            close();
            return false;
        }

        identifier[IFO_IDENTIFIER_LEN] = '\0';
        if (!strcmp(identifier,IFO_INDETIFIER_VMG))
            ifo_type_ = IT_VMG;
        else if (!strcmp(identifier,IFO_INDETIFIER_VTS))
            ifo_type_ = IT_VTS;
        else
        {
            close();
            return false;
        }

        return true;
    }

    bool IfoReader::close()
    {
        ifo_type_ = IT_UNKNOWN;
        return in_stream_.close();
    }

    bool IfoReader::read_vmg(IfoVmgData &vmg_data)
    {
        // Read last sector of VMG.
        ckcore::tuint32 sector = 0;
        ckcore::tint64 processed = 0;

        in_stream_.seek(12,ckcore::InStream::ckSTREAM_BEGIN);
        processed = in_stream_.read(&sector,sizeof(sector));
        if (processed == -1)
            return false;

        vmg_data.last_vmg_sec_ = ckcore::convert::be_to_le32(sector);

        // Read last sector of IFO.
        in_stream_.seek(28,ckcore::InStream::ckSTREAM_BEGIN);
        processed = in_stream_.read(&sector,sizeof(sector));
        if (processed == -1)
            return false;

        vmg_data.last_vmg_ifo_sec_ = ckcore::convert::be_to_le32(sector);

        // Read number of VTS  title sets.
        ckcore::tuint16 num_titles;

        in_stream_.seek(62,ckcore::InStream::ckSTREAM_BEGIN);
        processed = in_stream_.read(&num_titles,sizeof(num_titles));
        if (processed == -1)
            return false;

        vmg_data.num_vmg_ts_ = ckcore::convert::be_to_le16(num_titles);

        // Read start sector of VMG Menu VOB.
        in_stream_.seek(192,ckcore::InStream::ckSTREAM_BEGIN);
        processed = in_stream_.read(&sector,sizeof(sector));
        if (processed == -1)
            return false;

        vmg_data.vmg_menu_vob_sec_ = ckcore::convert::be_to_le32(sector);

        // Read sector offset to TT_SRPT.
        in_stream_.seek(196,ckcore::InStream::ckSTREAM_BEGIN);
        processed = in_stream_.read(&sector,sizeof(sector));
        if (processed == -1)
            return false;

        vmg_data.srpt_sec_ = ckcore::convert::be_to_le32(sector);

        // Read the TT_SRPT titles.
        in_stream_.seek(DVDVIDEO_BLOCK_SIZE * vmg_data.srpt_sec_,ckcore::InStream::ckSTREAM_BEGIN);
        processed = in_stream_.read(&num_titles,sizeof(num_titles));
        if (processed == -1)
            return false;

        num_titles = ckcore::convert::be_to_le16(num_titles);
        for (ckcore::tuint16 i = 0; i < num_titles; i++)
        {
            in_stream_.seek((DVDVIDEO_BLOCK_SIZE * vmg_data.srpt_sec_) + 8 + (i * 12) + 8,ckcore::InStream::ckSTREAM_BEGIN);
            processed = in_stream_.read(&sector,sizeof(sector));
            if (processed == -1)
                return false;

            vmg_data.titles_.push_back(ckcore::convert::be_to_le32(sector));
        }

        return true;
    }

    bool IfoReader::read_vts(IfoVtsData &vts_data)
    {
        // Read last sector of VTS.
        ckcore::tuint32 sector = 0;
        ckcore::tint64 processed = 0;

        in_stream_.seek(12,ckcore::InStream::ckSTREAM_BEGIN);
        processed = in_stream_.read(&sector,sizeof(sector));
        if (processed == -1)
            return false;

        vts_data.last_vts_sec_ = ckcore::convert::be_to_le32(sector);

        // Read last sector of IFO.
        in_stream_.seek(28,ckcore::InStream::ckSTREAM_BEGIN);
        processed = in_stream_.read(&sector,sizeof(sector));
        if (processed == -1)
            return false;

        vts_data.last_vts_ifo_sec_ = ckcore::convert::be_to_le32(sector);

        // Read start sector of VTS Menu VOB.
        in_stream_.seek(192,ckcore::InStream::ckSTREAM_BEGIN);
        processed = in_stream_.read(&sector,sizeof(sector));
        if (processed == -1)
            return false;

        vts_data.vts_menu_vob_sec_ = ckcore::convert::be_to_le32(sector);

        // Read start sector of VTS Title VOB.
        in_stream_.seek(196,ckcore::InStream::ckSTREAM_BEGIN);
        processed = in_stream_.read(&sector,sizeof(sector));
        if (processed == -1)
            return false;

        vts_data.vts_vob_sec_ = ckcore::convert::be_to_le32(sector);
        return true;
    }

    IfoReader::IfoType IfoReader::get_type()
    {
        return ifo_type_;
    }
};
