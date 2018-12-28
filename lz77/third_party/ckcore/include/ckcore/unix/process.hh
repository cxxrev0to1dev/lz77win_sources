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

/**
 * @file include/ckcore/unix/process.hh
 * @brief Defines the Unix process class.
 */
#pragma once
#include <vector>
#include <set>
#include "ckcore/types.hh"
#include "ckcore/stream.hh"

namespace ckcore
{
    /**
     * @brief The class for creating processes on Unix.
     */
    class Process : public OutStream
    {
    public:
        friend class ProcessMonitor;

    protected:
        volatile bool invalid_inheritor_;   // Set to true to indicate that the
                                            // inheritor is no loner valid and
                                            // thuss the events are purely
                                            // virtual.

    private:
        enum
        {
            FD_READ = 0,
            FD_WRITE = 1
        };

        enum
        {
            MAX_ARG_COUNT = 127,
            READ_BUFFER_SIZE = 128
        };

        enum State
        {
            STATE_STOPPED,
            STATE_RUNNING
        };

        int pipe_stdin_[2];
        int pipe_stdout_[2];
        int pipe_stderr_[2];

        volatile pid_t pid_;            // Process identifier.
        volatile State state_;          // Process state.
        volatile ckcore::tuint32 exit_code_;    // Process exit code (if exited).

        std::set<char> block_delims_;
        std::string block_buffer_out_;  // For buffering partial standard output blocks before commiting them.
        std::string block_buffer_err_;  // For buffering partial standard error blocks before commiting them.

        /**
         * Closes all internal pipes and resets the internal state of the object.
         */
        void close();

        /**
         * Reads from the specified file descriptor to the standard output buffer.
         * @return If successful true is returned, if unsuccessful false is
         *         returned.
         */
        bool read_stdout(int fd);

        /**
         * Reads from the specified file descriptor to the standard error buffer.
         * @return If successful true is returned, if unsuccessful false is
         *         returned.
         */
        bool read_stderr(int fd);

        // For multi-threading.
        pthread_mutex_t mutex_;
        pthread_mutex_t mutex_exec_;    // This mutex will be locked through the entire execution.

        /**
         * The thread entry for listening on the process output pipes.
         * @param [in] param A pointer to the Process object being executed.
         */
        static void *listen(void *param);

        // For signaling that the process has been started.
        pthread_mutex_t started_mutex_;
        pthread_cond_t started_cond_;
        bool started_event_;

        /**
         * Parses the specified command line into a vector of command line arguments.
         * @param [in] cmd_line The full command line to parse.
         * @return Vector of discrete command line arguments.
         */
        std::vector<tstring> parse_cmd_line(const tchar *cmd_line) const;

    public:
        /**
         * Constructs a Process object.
         */
        Process();

        /**
         * Destructs the Process object.
         */
        virtual ~Process();

        /**
         * Creates a new process.
         * @param [in] cmd_line The complete command line to execute.
         * @return If successful true is returned, if unsuccessful false is
         *         returned.
         */
        bool create(const tchar *cmd_line);

        /**
         * Checks if a process is running.
         * @return If a process is running true is returned, if not false is
         *         returned.
         */
        bool running() const;

        /**
         * Wait until the running process completes.
         * @return If successful and a process is running true is returned,
         *         otherwise false is returned.
         */
        bool wait() const;

        /**
         * Kills the process.
         * @return If successful and a process is running true is returned,
         *         otherwise false is returned.
         */
        bool kill() const;

        /**
         * Obtains the exit code of the process.
         * @param [out] exit_code The process exit code.
         * @return If successful true is returned, if not false is returned.
         */
        bool exit_code(ckcore::tuint32 &exit_code) const;

        /**
         * Adds a new block delimiter to be used when splitting process output
         * into blocks.
         * @param [in] delim The delimiter to add.
         */
        void add_block_delim(char delim);

        /**
         * Removes a block delimiter from being used when splitting process output
         * into blocks.
         * @param [in] delim The delimiter to remove.
         */
        void remove_block_delim(char delim);

        /**
         * Writes raw data to the process standard input.
         * @param [in] buffer Pointer to the beginning of the bufferi
         *                    containing the data to be written.
         * @param [in] count The number of bytes to write.
         * @return If the operation failed -1 is returned, otherwise the
         *         function returns the number of bytes written (this may be
         *         zero).
         */
        tint64 write(const void *buffer,tuint32 count);

        /**
         * Called when the process has finished.
         */
        virtual void event_finished() = 0;

        /**
         * Called when a block has been read from either standard output or
         * standard error.
         * @param [in] block The block that has been read.
         */
        virtual void event_output(const std::string &block) = 0;
    };
}
