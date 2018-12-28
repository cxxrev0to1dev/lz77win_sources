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

#include <sys/wait.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <string.h>
#include <stdlib.h>
#include <map>
#include "ckcore/file.hh"
#include "ckcore/string.hh"
#include "ckcore/process.hh"

namespace ckcore
{
    /**
     * Singleton class for monitoring child processes.
     */
    class ProcessMonitor
    {
    private:
        void (*old_sigchld_handler_)(int);
        std::map<pid_t,Process *> pid_map_;

        ProcessMonitor() : old_sigchld_handler_(NULL)
        {
            // Assign a action handler for the SIGCHLD signal.
            struct sigaction new_action,old_action;
            memset(&new_action,0,sizeof(new_action));

            new_action.sa_handler = sigchld_handler;
            new_action.sa_flags = SA_NOCLDSTOP;

            if (sigaction(SIGCHLD,&new_action,&old_action) == 0)
            {
                // Remember the old handler so we can restore it when done.
                if (old_action.sa_handler != sigchld_handler)
                    old_sigchld_handler_ = old_action.sa_handler;
            }
        }

        ~ProcessMonitor()
        {
            // Restore the old SIGCHLD signal handler.
            struct sigaction new_action,old_action;
            memset(&new_action,0,sizeof(new_action));

            new_action.sa_handler = old_sigchld_handler_;
            new_action.sa_flags = SA_NOCLDSTOP;

            if (sigaction(SIGCHLD,&new_action,&old_action) == 0)
            {
                // If the old signal handler was not our new one, switch back.
                if (old_action.sa_handler != sigchld_handler) {
                    sigaction(SIGCHLD,&old_action,0);
                }
            }
        }

        ProcessMonitor(const ProcessMonitor &rhs);
        ProcessMonitor &operator=(const ProcessMonitor &rhs);

        static void sigchld_handler(int signum)
        {
            // Obtain process identifier.
            int status;
            pid_t pid = wait(&status);

            // Change process status to stopped.
            if (ProcessMonitor::instance().pid_map_.count(pid) > 0)
            {
                Process *process = ProcessMonitor::instance().pid_map_[pid];
                process->state_ = Process::STATE_STOPPED;

                if (WIFEXITED(status))
                    process->exit_code_ = WEXITSTATUS(status);
            }

            // Call the old SIGCHLD signal handler.
            void (*old_sigchld_handler)(int) = ProcessMonitor::instance().old_sigchld_handler_;
            if (old_sigchld_handler != NULL && old_sigchld_handler != SIG_IGN)
                old_sigchld_handler(signum);
        }

    public:
        /**
         * Returns the process monitor instance.
         * @return The process monitor instance.
         */
        static ProcessMonitor &instance()
        {
            static ProcessMonitor instance;
            return instance;
        }

        /**
         * Registers a new process in the process monitor.
         * @param [in] pid The process identifier of the process to monitor.
         */
        void register_process(pid_t pid,Process *process)
        {
            pid_map_[pid] = process;
        }
    };

    Process::Process() : invalid_inheritor_(false),
        pid_(-1),state_(STATE_STOPPED),exit_code_(0),
        started_event_(false)
    {
        pipe_stdin_[0] = pipe_stdin_[1] = -1;
        pipe_stdout_[0] = pipe_stdout_[1] = -1;
        pipe_stderr_[0] = pipe_stderr_[1] = -1;

        pthread_mutex_init(&mutex_,NULL);
        pthread_mutex_init(&mutex_exec_,NULL);

        pthread_mutex_init(&started_mutex_,NULL);
        pthread_cond_init(&started_cond_,NULL);

        // Insert default delimiters.
        block_delims_.insert('\n');
        block_delims_.insert('\r');
    }

    Process::~Process()
    {
        // Make sure that the execution is completed before destroying this object.
        bool locked = pthread_mutex_lock(&mutex_exec_) == 0;
        if (locked)
            pthread_mutex_unlock(&mutex_exec_);

        close();

        pthread_mutex_destroy(&mutex_exec_);
        pthread_mutex_destroy(&mutex_);

        pthread_mutex_destroy(&started_mutex_);
        pthread_cond_destroy(&started_cond_);
    }

    void Process::close()
    {
        bool locked = pthread_mutex_lock(&mutex_) == 0;

        // Close handles.
        for (int i = 0; i < 2; i++)
        {
            // Close STDIN.
            if (pipe_stdin_[i] != -1)
            {
                ::close(pipe_stdin_[i]);
                pipe_stdin_[i] = -1;
            }

            // Close STDOUT.
            if (pipe_stdout_[i] != -1)
            {
                ::close(pipe_stdout_[i]);
                pipe_stdout_[i] = -1;
            }

            // Close STDERR.
            if (pipe_stderr_[i] != -1)
            {
                ::close(pipe_stderr_[i]);
                pipe_stderr_[i] = -1;
            }
        }

        // Reset stop event.
        pthread_mutex_lock(&started_mutex_);
        started_event_ = false;
        pthread_mutex_unlock(&started_mutex_);

        // Reset state.
        pid_ = -1;
        state_ = STATE_STOPPED;

        if (locked)
            pthread_mutex_unlock(&mutex_);
    }

    bool Process::read_stdout(int fd)
    {
        char buffer[READ_BUFFER_SIZE];
        ssize_t read_bytes = ::read(fd,buffer,sizeof(buffer) - 1);

        // Check for read errors.
        if (read_bytes <= 0)
            return false;

        buffer[read_bytes] = '\0';

        // Split the buffer into blocks.
        for (ssize_t i = 0; i < read_bytes; i++)
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
                if (block_buffer_out_.size() > 0)
                {
                    if (!invalid_inheritor_)
                        event_output(block_buffer_out_);

                    block_buffer_out_.resize(0);
                }
            }
            else
            {
                block_buffer_out_.push_back(buffer[i]);
            }
        }

        return true;
    }

    bool Process::read_stderr(int fd)
    {
        char buffer[READ_BUFFER_SIZE];
        ssize_t read_bytes = ::read(fd,buffer,sizeof(buffer) - 1);

        // Check for read errors.
        if (read_bytes <= 0)
            return false;

        buffer[read_bytes] = '\0';

        // Split the buffer into blocks.
        for (ssize_t i = 0; i < read_bytes; i++)
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
                if (block_buffer_err_.size() > 0)
                {
                    if (!invalid_inheritor_)
                        event_output(block_buffer_err_);

                    block_buffer_err_.resize(0);
                }
            }
            else
            {
                block_buffer_err_.push_back(buffer[i]);
            }
        }

        return true;
    }

    void *Process::listen(void *param)
    {
        Process *process = static_cast<Process *>(param);

        // Prevent the object from being destroyed while running the separate thread.
        pthread_mutex_lock(&process->mutex_exec_);

        process->block_buffer_out_.resize(0);
        process->block_buffer_err_.resize(0);

        // We can now signal that the process has started.
        pthread_mutex_lock(&process->started_mutex_);
        process->started_event_ = true;
        pthread_cond_signal(&process->started_cond_);
        pthread_mutex_unlock(&process->started_mutex_);

        // Loop while the program is running.
        while (process->state_ == STATE_RUNNING)
        {
            fd_set read_set;
            FD_ZERO(&read_set);

            if (process->pipe_stdout_[FD_READ] != -1)
                FD_SET(process->pipe_stdout_[FD_READ],&read_set);
            if (process->pipe_stderr_[FD_READ] != -1)
                FD_SET(process->pipe_stderr_[FD_READ],&read_set);

            int timeout = 100;  // 100 miliseconds.
            struct timeval tv;
            tv.tv_sec = timeout / 1000;
            tv.tv_usec = (timeout % 1000) * 1000;

            int res;
            do
            {
                res = select(FD_SETSIZE,&read_set,NULL,NULL,&tv);
            } while (res < 0 && errno == EINTR);

            // Read from standard output.
            if (FD_ISSET(process->pipe_stdout_[FD_READ],&read_set))
            {
                if (!process->read_stdout(process->pipe_stdout_[FD_READ]))
                {
                    process->close();
                    break;
                }
            }

            // Read from standard error.
            if (FD_ISSET(process->pipe_stderr_[FD_READ],&read_set))
            {
                if (!process->read_stderr(process->pipe_stderr_[FD_READ]))
                {
                    process->close();
                    break;
                }
            }
        }

        // Notify that the process has finished.
        if (!process->invalid_inheritor_)
            process->event_finished();

        process->close();

        pthread_mutex_unlock(&process->mutex_exec_);
    return NULL;
    }

    std::vector<tstring> Process::parse_cmd_line(const tchar *cmd_line) const
    {
        std::vector<tstring> res;

        std::string cur_arg;
        bool in_quote = false;

        size_t len = string::astrlen(cmd_line);
        for (size_t i = 0; i < len; i++)
        {
            if (cmd_line[i] == '\"')
            {
                in_quote = !in_quote;
            }
            else
            {
                if (cmd_line[i] == ' ' && !in_quote)
                {
                    if (cur_arg.size() > 0)
                    {
                        res.push_back(cur_arg);
                        cur_arg.resize(0);
                    }
                }
                else
                {
                    cur_arg.push_back(cmd_line[i]);
                }
            }
        }

        if (cur_arg.size() > 0)
            res.push_back(cur_arg);

        return res;
    }

    bool Process::create(const tchar *cmd_line)
    {
        // Check if a process is already running.
        if (running())
            return false;

        // Close prevous data.
        close();

        std::vector<tstring> arg_vec = parse_cmd_line(cmd_line);

        // Prepare argument list.
        char *arg_list[MAX_ARG_COUNT + 1];

        size_t num_args = arg_vec.size() > MAX_ARG_COUNT ? MAX_ARG_COUNT : arg_vec.size();
        if (num_args == 0)
            return false;

        for (size_t i = 0; i < num_args; i++)
            arg_list[i] = const_cast<tchar *>(arg_vec[i].c_str());

        char *path = arg_list[0];
        arg_list[num_args] = static_cast<char *>(NULL);

        // Check that the executable exist.
        if (!File::exist(path))
            return false;

        // Create pipes.
        if (pipe(pipe_stdin_) == -1 || pipe(pipe_stdout_) == -1 || pipe(pipe_stderr_) == -1)
            return false;

        fcntl(pipe_stdout_[FD_READ],F_SETFL,fcntl(pipe_stdout_[FD_READ],F_GETFL) | O_NONBLOCK);
        fcntl(pipe_stdout_[FD_WRITE],F_SETFL,fcntl(pipe_stdout_[FD_WRITE],F_GETFL) | O_NONBLOCK);
        fcntl(pipe_stderr_[FD_READ],F_SETFL,fcntl(pipe_stderr_[FD_READ],F_GETFL) | O_NONBLOCK);
        fcntl(pipe_stderr_[FD_WRITE],F_SETFL,fcntl(pipe_stderr_[FD_WRITE],F_GETFL) | O_NONBLOCK);

        // Change state to running (this will change on failure).
        state_ = STATE_RUNNING;

        // Fork process.
        pid_ = fork();
        if (pid_ == -1)
            return false;

        // Check if child process.
        if (pid_ == 0)
        {
            // Redirect STDIN, STDOUT and STDERR.
            if (dup2(pipe_stdin_[FD_READ],STDIN_FILENO) == -1 ||
                dup2(pipe_stdout_[FD_WRITE],STDOUT_FILENO) == -1 ||
                dup2(pipe_stderr_[FD_WRITE],STDERR_FILENO) == -1)
            {
                close();
                exit(-1);
            }

            // Close handles that we no longer need.
            for (int i = 0; i < 2; i++)
            {
                ::close(pipe_stdin_[i]);
                ::close(pipe_stdout_[i]);
                ::close(pipe_stderr_[i]);
            }

            execv(path,arg_list);

            // A successful execv replaces this exit call.
            close();
            exit(-1);
        }

        // Register child.
        ProcessMonitor::instance().register_process(pid_,this);

        // Create listen thread.
        pthread_t thread;
        if (pthread_create(&thread,NULL,listen,this) != 0)
        {
            // If we failed, kill the process.
            ::kill(pid_,SIGKILL);
            return false;
        }

        // Wait until the process has started.
        pthread_mutex_lock(&started_mutex_);
        while (!started_event_)
            pthread_cond_wait(&started_cond_,&started_mutex_);
        pthread_mutex_unlock(&started_mutex_);

        exit_code_ = 0;
        return true;
    }

    bool Process::running() const
    {
        bool locked = pthread_mutex_lock(const_cast<pthread_mutex_t *>(&mutex_)) == 0;
        bool running = state_ == STATE_RUNNING;
        if (locked)
            pthread_mutex_unlock(const_cast<pthread_mutex_t *>(&mutex_));

        return running;
    }

    bool Process::wait() const
    {
        // Make sure that the execution is completed before destroying this object.
        bool locked = pthread_mutex_lock(const_cast<pthread_mutex_t *>(&mutex_exec_)) == 0;
        if (locked)
            pthread_mutex_unlock(const_cast<pthread_mutex_t *>(&mutex_exec_));

        return true;
    }

    bool Process::kill() const
    {
        bool locked = pthread_mutex_lock(const_cast<pthread_mutex_t *>(&mutex_)) == 0;
        pid_t pid = pid_;
        if (locked)
            pthread_mutex_unlock(const_cast<pthread_mutex_t *>(&mutex_));

        if (pid == -1 || !running())
            return false;

        return ::kill(pid,SIGTERM) == 0;
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
        if (pid_ == -1 || !running())
            return -1;

        return ::write(pipe_stdin_[FD_WRITE],buffer,count);
    }

    bool Process::exit_code(ckcore::tuint32 &exit_code) const
    {
        if (running())
            return false;

        exit_code = exit_code_;
        return true;
    }
}
