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
 * @file include/ckcore/progresser.hh
 * @brief Defines the class for progressing progress interfaces.
 */

#pragma once
#include "ckcore/types.hh"
#include "ckcore/progress.hh"

namespace ckcore
{
    /**
     * @brief Class for calculating and updating progress.
     *
     * This class calculates the total progress from partially processed data.
     */
    class Progresser
    {
    private:
        Progress &progress_;
        tuint64 total_;
        tuint64 count_;

    public:
        /**
         * Constructs a Progresser object.
         * @param [in] progress The progress interface to report the progress to.
         * @param [in] total The total number of units to progress.
         */
        Progresser(Progress &progress,tuint64 total);

        /**
         * Updates the progress depending on the number of units processed.
         * @param [in] count The number of units processed..
         */
        void update(tuint64 count);

        /**
         * Transmits a message to the progress interface. This message i
         * intended to be displayed to the end user.
         * @param [in] type The type of message.
         * @param [in] format The message format.
         */
        void notify(Progress::MessageType type,const tchar *format,...) __attribute__ ((format (printf, 3, 4)));

        /**
         * Checks wether the operation has been cancelled or not
         * @return If the process has been cancelled true is returned, if no
         *         cancelled false is returned.
         */
        bool cancelled();
    };
}
