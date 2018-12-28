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
 * @file include/ckcore/progress.hh
 * @brief Defines the progress interface.
 */

#pragma once
#include "ckcore/types.hh"

namespace ckcore
{
    /**
     * @brief Interface for receiving and providing progress information.
     *
     * This interface is designed for implementation by user interfaces displaying
     * progress information with the option to cancel the operation.
     */
    class Progress
    {
    public:
        virtual ~Progress() {};

        /**
         * Defines different message types.
         */
        enum MessageType
        {
            ckINFORMATION,
            ckWARNING,
            ckERROR,
            ckEXTERNAL
        };

        /**
         * Notifies change in progress.
         * @param [in] percent The progress in percent.
         */
        virtual void set_progress(unsigned char percent) { ckUNUSED(percent); };

        /**
         * Enables or disables marquee progress reporting
         * @param [in] enable Set to true to enable marquee mode or set to false to
         *             disable the marquee mode.
         */
        virtual void set_marquee(bool enable) { ckUNUSED(enable); };

        /**
         * Sets the status message describing the current operation.
         * @param [in] format The status message format.
         */
        virtual void set_status(const tchar *format,...) __attribute__ ((format (printf, 2, 3))) = 0;

        /**
         * Transmits a message to the interface implementor. This message is
         * intended to be displayed to the end user.
         * @param [in] type The type of message.
         * @param [in] format The message format.
         */
        virtual void notify(MessageType type,const tchar *format,...) __attribute__ ((format (printf, 3, 4))) = 0;

        /**
         * Checks wether the operation has been cancelled or not.
         * @return If the process has been cancelled true is returned, if not
         *         cancelled false is returned.
         */
        virtual bool cancelled() = 0;
    };
}
