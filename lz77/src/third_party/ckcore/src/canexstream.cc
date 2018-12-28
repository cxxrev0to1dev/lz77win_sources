/*
 * The ckCore library provides core software functionality.
 * Copyright (C) 2006-2012 Christian Kindahl
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
#include "ckcore/exception.hh"
#include "ckcore/canexstream.hh"

namespace ckcore
{
    CanexInStream::CanexInStream(InStream &stream,const tchar *ident) :
        stream_(stream),ident_(ident)
    {
    }

    CanexInStream::CanexInStream(InStream &stream,const tstring &ident) :
        stream_(stream),ident_(ident)
    {
    }

    const tstring &CanexInStream::identifier() const
    {
        return ident_;
    }

    void CanexInStream::seek(tuint32 distance,InStream::StreamWhence whence)
    {
        if (!stream_.seek(distance,whence))
        {
            throw Exception2(string::formatstr(ckT("stream seek error in %s."),ident_.c_str()));
        }
    }

    tint64 CanexInStream::read(void *buffer,tuint32 count)
    {
        ckcore::tint64 res = stream_.read(buffer,count);
        if (res == -1)
        {
            throw Exception2(string::formatstr(ckT("stream read error in %s."),ident_.c_str()));
        }

        return res;
    }

    bool CanexInStream::end()
    {
        return stream_.end();
    }

    CanexOutStream::CanexOutStream(OutStream &stream,const tchar *ident) :
        stream_(stream),ident_(ident)
    {
    }

    CanexOutStream::CanexOutStream(OutStream &stream,const tstring &ident) :
        stream_(stream),ident_(ident)
    {
    }

    const tstring &CanexOutStream::identifier() const
    {
        return ident_;
    }

    void CanexOutStream::write(void *buffer,tuint32 count)
    {
        ckcore::tint64 res = stream_.write(buffer,count);
        if (res == -1 || res != count)
        {
            throw Exception2(string::formatstr(ckT("stream write error in %s."),ident_.c_str()));
        }
    }

    namespace canexstream
    {
        void copy(CanexInStream &from,CanexOutStream &to,Progresser &progresser)
        {
            unsigned char buffer[8192];

            tint64 res = 0;
            while (!from.end())
            {
                // Check if we should cancel.
                if (progresser.cancelled())
                    return;

                res = from.read(buffer,sizeof(buffer));
                to.write(buffer,(tuint32)res);

                // Update progress.
                progresser.update(res);
            }
        }

        void copy(CanexInStream &from,CanexOutStream &to,Progresser &progresser,
                  tuint64 size)
        {
            unsigned char buffer[8192];

            tint64 res = 0;
            while (!from.end() && size > 0)
            {
                // Check if we should cancel.
                if (progresser.cancelled())
                    return;

                tuint32 to_read = size < sizeof(buffer) ?
                                  static_cast<tuint32>(size) : sizeof(buffer);
                res = from.read(buffer,to_read);
                to.write(buffer,static_cast<tuint32>(res));

                size -= res;

                // Update progress.
                progresser.update(res);
            }

            // Pad if necessary. This is not very efficient but it should also not
            // happen.
            while (size > 0)
            {
                tuint32 to_write = size < sizeof(buffer) ?
                                   static_cast<tuint32>(size) : sizeof(buffer);
                memset(buffer,0,sizeof(buffer));

                to.write(buffer,to_write);
                size -= to_write;

                // Update progress.
                progresser.update(res);
            }
        }
    }
}
