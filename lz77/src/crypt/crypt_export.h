// Copyright (c) 2011 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CRYPT_WIN_CRYPT_EXPORT_H_
#define CRYPT_WIN_CRYPT_EXPORT_H_
#pragma once

#if defined(COMPONENT_BUILD)
#if defined(OS_WIN)

#if defined(CRYPT_IMPLEMENTATION)
#define CRYPT_EXPORT __declspec(dllexport)
#else
#if defined(OS_WIN_X86)
#pragma comment(lib,"crypt.lib")
#else
#pragma comment(lib,"crypt_x64.lib")
#endif // WIN32
#define CRYPT_EXPORT __declspec(dllimport)
#endif  // defined(BASE_IMPLEMENTATION)

#else  // defined(WIN32)
#define CRYPT_EXPORT __attribute__((visibility("default")))
#endif

#else  // defined(COMPONENT_BUILD)
#define CRYPT_EXPORT
#endif

#endif  // BASE_BASE_EXPORT_H_
