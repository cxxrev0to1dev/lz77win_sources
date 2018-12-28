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
 * @file src/windows/util.hh
 * @brief Defines Windows utility functions.
 */
#pragma once
#include "ckcore/file.hh"
#include "ckcore/types.hh"
#include "ckcore/path.hh"

namespace ckcore
{
    /**
     * Converts a SYSTEMTIME structure to a struct tm structure.
     * @param [in] stime The input time.
     * @param [out] time The output time.
     */
    void SysTimeToTm(SYSTEMTIME &stime,struct tm &time);
};
