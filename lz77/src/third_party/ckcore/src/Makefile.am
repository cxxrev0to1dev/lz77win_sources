EXTRA_DIST = ../include/ckcore/assert.hh ../include/ckcore/buffer.hh \
			 ../include/ckcore/bufferedstream.hh ../include/ckcore/canexstream.hh \
			 ../include/ckcore/cast.hh ../include/ckcore/convert.hh \
			 ../include/ckcore/crcstream.hh ../include/ckcore/directory.hh \
			 ../include/ckcore/dynlib.hh ../include/ckcore/exception.hh \
			 ../include/ckcore/file.hh ../include/ckcore/filestream.hh \
			 ../include/ckcore/locker.hh ../include/ckcore/log.hh \
			 ../include/ckcore/memory.hh ../include/ckcore/memorystream.hh \
			 ../include/ckcore/nullstream.hh ../include/ckcore/path.hh \
			 ../include/ckcore/process.hh ../include/ckcore/progress.hh \
			 ../include/ckcore/progresser.hh ../include/ckcore/stream.hh \
			 ../include/ckcore/string.hh ../include/ckcore/system.hh \
			 ../include/ckcore/task.hh ../include/ckcore/thread.hh \
			 ../include/ckcore/threadpool.hh ../include/ckcore/types.hh
AM_CPPFLAGS = -I$(srcdir)/../include
SUBDIRS = unix

lib_LTLIBRARIES = libckcore.la

libckcore_la_SOURCES = unix/directory.cc unix/file.cc unix/process.cc \
					   unix/thread.cc assert.cc bufferedstream.cc \
					   canexstream.cc convert.cc crcstream.cc dynlib.cc \
					   exception.cc filestream.cc log.cc memorystream.cc \
					   nullstream.cc path.cc progresser.cc stream.cc \
					   string.cc system.cc threadpool.cc
libckcore_la_LDFLAGS = -version-info $(CKCORE_VERSION)

library_includedir = $(includedir)/ckcore
library_include_HEADERS = ../include/ckcore/assert.hh \
						  ../include/ckcore/buffer.hh \
						  ../include/ckcore/bufferedstream.hh \
						  ../include/ckcore/canexstream.hh \
						  ../include/ckcore/cast.hh \
						  ../include/ckcore/convert.hh \
						  ../include/ckcore/crcstream.hh \
						  ../include/ckcore/directory.hh \
						  ../include/ckcore/dynlib.hh \
						  ../include/ckcore/exception.hh \
						  ../include/ckcore/file.hh \
						  ../include/ckcore/filestream.hh \
						  ../include/ckcore/linereader.hh \
						  ../include/ckcore/locker.hh \
						  ../include/ckcore/log.hh \
						  ../include/ckcore/memory.hh \
						  ../include/ckcore/memorystream.hh \
						  ../include/ckcore/nullstream.hh \
						  ../include/ckcore/path.hh \
						  ../include/ckcore/process.hh \
						  ../include/ckcore/progress.hh \
						  ../include/ckcore/progresser.hh \
						  ../include/ckcore/stream.hh \
						  ../include/ckcore/string.hh \
						  ../include/ckcore/system.hh \
						  ../include/ckcore/task.hh \
						  ../include/ckcore/thread.hh \
						  ../include/ckcore/threadpool.hh \
						  ../include/ckcore/types.hh

