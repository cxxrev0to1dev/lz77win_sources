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
 * @file include/ckcore/task.hh
 * @brief Task interface.
 */

#pragma once
#include "ckcore/types.hh"

namespace ckcore
{
    /**
     * @brief Task interface.
     */
    class Task
    {
    private:
        bool auto_delete_;

    public:
        Task() : auto_delete_(true) {}
        virtual ~Task() {}

        /**
         * Starts the task task.
         */
        virtual void start() = 0;

        /**
         * Check whether the task should be automatically deleted after
         * execution.
         * @return If the task should be automatically deleted true is
         *         returned, if not false is returned.
         */
        bool auto_delete() const
        {
            return auto_delete_;
        }

        /**
         * Sets whether to automatically delete the task after execution or
         * not.
         * @param [in] enable Set to true to enable automatic deletion and
         *                    false to disable it.
         */
        void set_auto_delete(bool enable)
        {
            auto_delete_ = enable;
        }
    };
}

