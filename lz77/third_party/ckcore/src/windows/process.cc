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

#include "stdafx.hh"
#include "ckcore/assert.hh"
#include "ckcore/stream.hh"
#include "ckcore/process.hh"

namespace ckcore
{
    Process::Process() : invalid_inheritor_(false),
        pipe_stdin_(NULL),pipe_output_(NULL),
        process_handle_(NULL),thread_handle_(NULL),
        start_event_(NULL),stop_event_(NULL),
        thread_id_(0),state_(STATE_STOPPED),exit_code_(0),
        mutex_(NULL),mutex_exec_(NULL)
    {
        mutex_ = CreateMutex(NULL,FALSE,NULL);
        mutex_exec_ = CreateMutex(NULL,FALSE,NULL);

        // Create the start event that will notify the create function when the
        // listening thread has been started.
        start_event_ = CreateEvent(NULL,true,false,NULL);

        // Create the stop event that will be used to kill the listening thread.
        stop_event_ = CreateEvent(NULL,true,false,NULL);

        // Insert default delimiters.
        block_delims_.insert('\n');
        block_delims_.insert('\r');
    }

    Process::~Process()
    {
        // Make sure that the execution is completed before destroying this object.
        bool locked = WaitForSingleObject(mutex_exec_,INFINITE) == WAIT_OBJECT_0;
        if (locked)
            ReleaseMutex(mutex_exec_);

        close();

        // Destroy start event.
        if (start_event_ != NULL)
        {
            ckVERIFY(0 != CloseHandle(start_event_));
            start_event_ = NULL;
        }

        // Destroy stop event.
        if (stop_event_ != NULL)
        {
            ckVERIFY(0 != CloseHandle(stop_event_));
            stop_event_ = NULL;
        }

        // Closes mutexes.
        if (mutex_exec_ != NULL)
        {
            ckVERIFY(0 != CloseHandle(mutex_exec_));
            mutex_exec_ = NULL;
        }

        if (mutex_ != NULL)
        {
            ckVERIFY(0 != CloseHandle(mutex_));
            mutex_ = NULL;
        }
    }

    void Process::close()
    {
        bool locked = WaitForSingleObject(mutex_,INFINITE) == WAIT_OBJECT_0;

        // Close handles.
        if (thread_handle_ != NULL)
        {
            if (GetCurrentThreadId() != thread_id_)
            {
                // If the thread does not respond within 5 seconds it will be
                // forced to terminate.
                ckVERIFY(0 != SetEvent(stop_event_));

                /*if (WaitForSingleObject(thread_handle_,5000) == WAIT_TIMEOUT)
                    TerminateThread(thread_handle_,-2);*/
                // UPDATE: Rather let the program hang to make the problem present to the
                //         user.
                if (WaitForSingleObject(thread_handle_,INFINITE) != WAIT_OBJECT_0)
                {
                    ckASSERT(false);
                    ckVERIFY(0 != TerminateThread(thread_handle_,-2));
                }
            }

            ckVERIFY(0 != CloseHandle(thread_handle_));
            thread_handle_ = NULL;
        }


        // Close the pipes before closing the child process.
        // That way, if the child process is still alive,
        // it will get an error when writing to stdout or reading
        // from stdin. If the child process properly checks for errors
        // (which is normally the case at least for stdin),
        // it will terminate early. If not, it will carry on
        // and terminate at some point time.
        // If we don't close the pipes, there is a risk
        // that the process will forever hang waiting for us
        // to read its output or to write its input. If we then
        // wait for the child to terminate, we'll get a nice deadlock.
        if (pipe_stdin_ != NULL)
        {
            ckVERIFY(0 != CloseHandle(pipe_stdin_));
            pipe_stdin_ = NULL;
        }

        if (pipe_output_ != NULL)
        {
            ckVERIFY(0 != CloseHandle(pipe_output_));
            pipe_output_ = NULL;
        }

        if (process_handle_ != NULL)
        {
            // We expect all our child processes to properly terminate
            // and don't freeze on exit. However, there is a risk
            // that they don't behave properly. Therefore,
            // instead of forever waiting at this point, we could wait for some time,
            // and then forcibly terminate the child process. But we don't really
            // want to kill children that way, as we may end up with garbage on
            // the hard disk or get into some other kind of trouble. The best thing
            // to do would be to warn the user and let him decide. This
            // is not implemented yet.
            ckVERIFY(WAIT_OBJECT_0 == WaitForSingleObject(process_handle_,INFINITE));

            if (GetExitCodeProcess(process_handle_,&exit_code_) == FALSE)
            {
                ckASSERT(false);   // Actually, this should never fail.
                exit_code_ = -1;
            }

            ckASSERT(exit_code_ != STILL_ACTIVE);

            ckVERIFY(0 != CloseHandle(process_handle_));
            process_handle_ = NULL;
        }

        // Reset state.
        ckVERIFY(0 != ResetEvent(start_event_));
        ckVERIFY(0 != ResetEvent(stop_event_));

        thread_id_ = 0;
        state_ = STATE_STOPPED;

        if (locked)
            ckVERIFY(0 != ReleaseMutex(mutex_));
    }

    bool Process::read_output(HANDLE handle)
    {
        char buffer[READ_BUFFER_SIZE];

        while (true)
        {
            unsigned long bytes_avail = 0;
            if (!PeekNamedPipe(handle,NULL,0,NULL,&bytes_avail,NULL))
                break;

            if (bytes_avail == 0)
                return true;

            unsigned long read = 0;
            if (!ReadFile(handle,buffer,min(bytes_avail,READ_BUFFER_SIZE - 1),&read,NULL) || read == 0)
                break;

            buffer[read] = '\0';

            // Split the buffer into blocks.
            for (unsigned long i = 0; i < read; i++)
            {
                // Check if we have found a block delimiter.
                bool is_delim = false;

                std::set<char>::const_iterator it;
                for (it = block_delims_.begin(); it != block_delims_.end(); it++)
                {
                    if (buffer[i] == *it)
                    {
                        is_delim = true;
                        break;
                    }
                }

                if (is_delim)
                {
                    // Avoid flushing an empty buffer.
                    if (block_buffer_.size() >  0)
                    {
                        if (!invalid_inheritor_)
                            event_output(block_buffer_);

                        block_buffer_.resize(0);
                    }
                }
                else
                {
                    block_buffer_.push_back(buffer[i]);
                }
            }
        }

        /*unsigned long last_err = GetLastError();
        if (last_err == ERROR_BROKEN_PIPE || last_err == ERROR_NO_DATA)
            return false;*/

        return false;
    }

    unsigned long WINAPI Process::listen(void *param)
    {
        Process *process = static_cast<Process *>(param);

        // Prevent the object from being destroyed while running the separate thread.
        WaitForSingleObject(process->mutex_exec_,INFINITE);

        // Now when we have the mutex we can notify the thread creator that we have
        // started.
        SetEvent(process->start_event_);

        HANDLE handles[2];
        handles[0] = process->process_handle_;
        handles[1] = process->stop_event_;

        process->block_buffer_.resize(0);

        while (true)
        {
            if (!process->read_output(process->pipe_output_))
                break;

            unsigned long wait_res = WaitForMultipleObjects(2,handles,false,100);
            
            if (wait_res == WAIT_OBJECT_0)
            {
                process->read_output(process->pipe_output_);
                break;
            }
            else if (wait_res == WAIT_OBJECT_0 + 1)       // Check if the stop event has been signaled.
            {
                break;
            }
        }

        process->close();

        ReleaseMutex(process->mutex_exec_);

        // Notify that the process has finished.
        if (!process->invalid_inheritor_)
            process->event_finished();
        return 0;
    }

    bool Process::create(const tchar *cmd_line)
    {
        // Check if a process is already running.
        if (running())
            return false;

        // Close prevous data.
        close();

        HANDLE output_read_temp = NULL,output_write = NULL;
        HANDLE input_write_temp = NULL,input_read = NULL;
        HANDLE error_write = NULL;

        SECURITY_ATTRIBUTES sa;
        sa.nLength = sizeof(SECURITY_ATTRIBUTES);
        sa.lpSecurityDescriptor = NULL;
        sa.bInheritHandle = true;

        // Create the child output pipe.
        if (!CreatePipe(&output_read_temp,&output_write,&sa,0))
            return false;

        // Duplicate the output write handle since the application might close
        // one of its handles.
        if (!DuplicateHandle(GetCurrentProcess(),output_write,GetCurrentProcess(),
            &error_write,0,true,DUPLICATE_SAME_ACCESS))
        {
            ckVERIFY(0 != CloseHandle(output_read_temp));
            ckVERIFY(0 != CloseHandle(output_write));

            return false;
        }

        // Create the child input pipe.
        if (!CreatePipe(&input_read,&input_write_temp,&sa,0))
        {
            ckVERIFY(0 != CloseHandle(output_read_temp));
            ckVERIFY(0 != CloseHandle(output_write));
            ckVERIFY(0 != CloseHandle(error_write));

            return false;
        }

        // Create new output read handle and the input write handles. The properties
        // must be set to false, otherwise we can't close the handles since they will
        // be inherited.
        if (!DuplicateHandle(GetCurrentProcess(),output_read_temp,GetCurrentProcess(),
            &pipe_output_,0,false,DUPLICATE_SAME_ACCESS))
        {
            ckVERIFY(0 != CloseHandle(output_read_temp));
            ckVERIFY(0 != CloseHandle(output_write));
            ckVERIFY(0 != CloseHandle(error_write));
            ckVERIFY(0 != CloseHandle(input_write_temp));
            ckVERIFY(0 != CloseHandle(input_read));

            return false;
        }

        if (!DuplicateHandle(GetCurrentProcess(),input_write_temp,GetCurrentProcess(),
            &pipe_stdin_,0,false,DUPLICATE_SAME_ACCESS))
        {
            ckVERIFY(0 != CloseHandle(output_read_temp));
            ckVERIFY(0 != CloseHandle(output_write));
            ckVERIFY(0 != CloseHandle(error_write));
            ckVERIFY(0 != CloseHandle(input_write_temp));
            ckVERIFY(0 != CloseHandle(input_read));

            ckVERIFY(0 != CloseHandle(pipe_output_));

            return false;
        }

        // Now we can close the inherited pipes.
        ckVERIFY(0 != CloseHandle(output_read_temp));
        ckVERIFY(0 != CloseHandle(input_write_temp));

        // Create the process.
        PROCESS_INFORMATION pi;

        STARTUPINFO si;
        memset(&si,0,sizeof(STARTUPINFO));

        si.cb = sizeof(STARTUPINFO);
        si.dwFlags = STARTF_USESTDHANDLES | STARTF_USESHOWWINDOW;
        si.hStdInput = input_read;
        si.hStdOutput = output_write;
        si.hStdError = error_write;
        si.wShowWindow = SW_HIDE;

        bool result = CreateProcess(NULL,const_cast<tchar *>(cmd_line),NULL,
                                    NULL,true,CREATE_NEW_CONSOLE,NULL,NULL,
                                    &si,&pi) != 0;

        process_handle_ = pi.hProcess;

        // Close any unnecessary handles.
        if (pi.hThread)
            ckVERIFY(0 != CloseHandle(pi.hThread));

        ckVERIFY(0 != CloseHandle(input_read));
        ckVERIFY(0 != CloseHandle(output_write));
        ckVERIFY(0 != CloseHandle(error_write));

        if (!result)
        {
            ckVERIFY(0 != CloseHandle(pipe_stdin_));
            ckVERIFY(0 != CloseHandle(pipe_output_));
            pipe_stdin_ = NULL;
            pipe_output_ = NULL;

            return false;
        }

        // Create the listener thread.
        thread_handle_ = CreateThread(NULL,0,listen,this,0,const_cast<unsigned long *>(&thread_id_));
        if (thread_handle_ == NULL)
        {
            close();
            return false;
        }

        state_ = STATE_RUNNING;

        // Wait for the thread to start before returning.
        if (WaitForSingleObject(start_event_,INFINITE) == WAIT_FAILED)
            Sleep(200);

        return true;
    }

    bool Process::running() const
    {
        bool locked = WaitForSingleObject(mutex_,INFINITE) == WAIT_OBJECT_0;
        bool running = state_ == STATE_RUNNING;
        if (locked)
            ReleaseMutex(mutex_);

        return running;
    }

    bool Process::wait() const
    {
        bool locked = WaitForSingleObject(mutex_,INFINITE) == WAIT_OBJECT_0;
        HANDLE thread_handle = thread_handle_;
        if (locked)
            ReleaseMutex(mutex_);

        if (thread_handle == NULL || !running())
            return false;

        if (WaitForSingleObject(thread_handle,INFINITE) == WAIT_FAILED)
            return false;

        // Make sure that the state is valid.
        while (state_ != STATE_STOPPED)
            Sleep(100);

        return true;
    }

    bool Process::kill() const
    {
        bool locked = WaitForSingleObject(mutex_,INFINITE) == WAIT_OBJECT_0;
        HANDLE process_handle = process_handle_;
        if (locked)
            ReleaseMutex(mutex_);

        if (process_handle == NULL || !running())
            return false;

        return TerminateProcess(process_handle,0) == TRUE;
    }

    void Process::add_block_delim(char delim)
    {
        block_delims_.insert(delim);
    }

    void Process::remove_block_delim(char delim)
    {
        std::set<char>::iterator it = block_delims_.find(delim);
        if (it != block_delims_.end())
            block_delims_.erase(it);
    }

    tint64 Process::write(const void *buffer,tuint32 count)
    {
        // Wait if the process has not been started.
        while (!running())
            Sleep(100);

        unsigned long written = 0;
        WriteFile(pipe_stdin_,buffer,count,&written,NULL);

        return written;
    }

    bool Process::exit_code(ckcore::tuint32 &exit_code) const
    {
        if (running())
            return false;

        exit_code = exit_code_;
        return true;
    }
};
