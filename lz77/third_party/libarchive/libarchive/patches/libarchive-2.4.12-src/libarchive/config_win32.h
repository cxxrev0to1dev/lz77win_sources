/*-
 * Copyright (c) 2003-2006 Tim Kientzle
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer
 *    in this position and unchanged.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR(S) ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR(S) BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * $FreeBSD$
 */
#error CONFIG_WIN32_H_INCLUDED
#ifndef CONFIG_WIN32_H_INCLUDED
#define CONFIG_WIN32_H_INCLUDED

/* Start of configuration for native Win32  */
/* TODO: Fix this. */

/* Basic definitions for system and integer types. */
#undef uid_t
#undef gid_t
#undef id_t
typedef int uid_t;
typedef int gid_t;
typedef int id_t;
typedef long ssize_t;

/* Replacement for major/minor/makedev. */
#define	major(x) ((int)(0x00ff & ((x) >> 8)))
#define	minor(x) ((int)(0xffff00ff & (x)))
#define	makedev(maj,min) ((0xff00 & ((maj)<<8))|(0xffff00ff & (min)))


#define	EFTYPE 7
#define	STDERR_FILENO 2

/* Alias the Windows _function to the POSIX equivalent. */
#include <io.h>
#define	write		_write
#define	read			_read
#define	lseek		_lseek
#define	open			_open
#define	chdir		_chdir
#define	mkdir(d,m)	_mkdir(d)
#define	close		_close
#define	lstat		stat

#ifndef HAVE_GETUID
uid_t
extern inline getuid()
{
	return 0;
}	
#define HAVE_GETUID 1
#endif /* HAVE_GETUID */

#ifndef HAVE_GETEUID
uid_t
extern inline geteuid()
{
	return 0;
}	
#define HAVE_GETEUID 1
#endif /* HAVE_GETEUID */

#include <fcntl.h>
#ifndef O_BINARY
# ifdef _O_BINARY
#  define O_BINARY _O_BINARY
# else
#  define O_BINARY 0
# endif
#endif /* O_BINARY */

#if O_BINARY
# define setmode(f,m)                      _setmode(f,m)
# define SET_BINARY(f)                      do {if (!isatty(f)) setmode(f,O_BINARY);} while (0)
#endif /* not O_BINARY */

/* End of Win32 definitions. */

#endif /* CONFIG_WIN32_H_INCLUDED  */
