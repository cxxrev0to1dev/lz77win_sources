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
#include "ckcore/system.hh"
#include "ckcore/stream.hh"

namespace ckcore
{
    namespace stream
    {
        bool copy(InStream &from,OutStream &to)
        {
            // UPDATE: Hangs the application on some systems.
            tuint32 buffer_size = 8192;
            /*tuint32 buffer_size = System::Cache(System::ckLEVEL_1);
            if (buffer_size == 0)
                buffer_size = 8192;*/

            unsigned char *buffer = new unsigned char[buffer_size];
            if (buffer == NULL)
                return false;

            tint64 res = 0;
            while (!from.end())
            {
                res = from.read(buffer,buffer_size);
                if (res == -1)
                {
                    delete [] buffer;
                    return false;
                }

                res = to.write(buffer,(tuint32)res);
                if (res == -1)
                {
                    delete [] buffer;
                    return false;
                }
            }

            delete [] buffer;
            return true;
        }

        bool copy(InStream &from,OutStream &to,Progress &progress)
        {
            // UPDATE: Hangs the application on some systems.
            tuint32 buffer_size = 8192;
            /*tuint32 buffer_size = System::Cache(System::ckLEVEL_1);
            if (buffer_size == 0)
                buffer_size = 8192;*/

            unsigned char *buffer = new unsigned char[buffer_size];
            if (buffer == NULL)
                return false;

            // Initialize progress.
            tint64 total = from.size(),written = 0;
            progress.set_marquee(total == -1);

            tint64 res = 0;
            while (!from.end())
            {
                // Check if we should cancel.
                if (progress.cancelled())
                    return false;

                res = from.read(buffer,buffer_size);
                if (res == -1)
                {
                    delete [] buffer;
                    return false;
                }

                res = to.write(buffer,(tuint32)res);
                if (res == -1)
                {
                    delete [] buffer;
                    return false;
                }

                // Update progress.
                if (total != -1)
                {
                    written += res;
                    progress.set_progress((unsigned char)((written * 100)/total));
                }
            }

            // Set to 100 in case of rounding errors.
            if (total != -1)
                progress.set_progress(100);

            delete [] buffer;
            return true;
        }

        bool copy(InStream &from,OutStream &to,Progresser &progresser)
        {
            // UPDATE: Hangs the application on some systems.
            tuint32 buffer_size = 8192;
            /*tuint32 buffer_size = System::Cache(System::ckLEVEL_1);
            if (buffer_size == 0)
                buffer_size = 8192;*/

            unsigned char *buffer = new unsigned char[buffer_size];
            if (buffer == NULL)
                return false;

            tint64 res = 0;
            while (!from.end())
            {
                // Check if we should cancel.
                if (progresser.cancelled())
                    return false;

                res = from.read(buffer,buffer_size);
                if (res == -1)
                {
                    delete [] buffer;
                    return false;
                }

                res = to.write(buffer,(tuint32)res);
                if (res == -1)
                {
                    delete [] buffer;
                    return false;
                }

                // Update progress.
                progresser.update(res);
            }

            delete [] buffer;
            return true;
        }

        bool copy(InStream &from,OutStream &to,Progresser &progresser,
                  tuint64 size)
        {
            // UPDATE: Hangs the application on some systems.
            tuint32 buffer_size = 8192;
            /*tuint32 buffer_size = System::Cache(System::ckLEVEL_1);
            if (buffer_size == 0)
                buffer_size = 8192;*/

            unsigned char *buffer = new unsigned char[buffer_size];
            if (buffer == NULL)
                return false;

            tint64 res = 0;
            while (!from.end() && size > 0)
            {
                // Check if we should cancel.
                if (progresser.cancelled())
                    return false;

                tuint32 to_read = size < buffer_size ?
                                  static_cast<tuint32>(size) : buffer_size;
                res = from.read(buffer,to_read);
                if (res == -1)
                {
                    delete [] buffer;
                    return false;
                }

                res = to.write(buffer,static_cast<tuint32>(res));
                if (res == -1)
                {
                    delete [] buffer;
                    return false;
                }

                size -= res;

                // Update progress.
                progresser.update(res);
            }

            // Pad if necessary. This is not very efficient but it should also not
            // happen.
            while (size > 0)
            {
                tuint32 to_write = size < buffer_size ?
                                   static_cast<tuint32>(size) : buffer_size;
                memset(buffer,0,buffer_size);

                res = to.write(buffer,to_write);
                if (res == -1)
                {
                    delete [] buffer;
                    return false;
                }

                size -= res;

                // Update progress.
                progresser.update(res);
            }

            delete [] buffer;
            return true;
        }
    }
}

