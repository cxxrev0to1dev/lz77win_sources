// Copyright (c) 2011 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef BASE_BASE_EXPORT_H_
#define BASE_BASE_EXPORT_H_
#pragma once

#if defined(COMPONENT_BUILD)
#if defined(OS_WIN)
#if defined(BASE_IMPLEMENTATION)
#define BASE_EXPORT __declspec(dllexport)
#else
#if defined(OS_WIN_X86)
#pragma comment(lib,"base.lib")
#else
#pragma comment(lib,"base_x64.lib")
#endif // WIN32
#define BASE_EXPORT __declspec(dllimport)
#endif  // defined(BASE_IMPLEMENTATION)

#else  // defined(WIN32)
#define BASE_EXPORT __attribute__((visibility("default")))
#endif

#else  // defined(COMPONENT_BUILD)
#define BASE_EXPORT
#endif

#endif  // BASE_BASE_EXPORT_H_
