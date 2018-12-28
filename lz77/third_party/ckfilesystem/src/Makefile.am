EXTRA_DIST = ../include/ckfilesystem/const.hh \
			 ../include/ckfilesystem/dvdvideo.hh \
			 ../include/ckfilesystem/eltorito.hh \
			 ../include/ckfilesystem/exception.hh \
			 ../include/ckfilesystem/fileset.hh \
			 ../include/ckfilesystem/filesystem.hh \
			 ../include/ckfilesystem/filesystemhelper.hh \
			 ../include/ckfilesystem/filesystemwriter.hh \
			 ../include/ckfilesystem/filetree.hh \
			 ../include/ckfilesystem/iforeader.hh \
			 ../include/ckfilesystem/iso9660.hh \
			 ../include/ckfilesystem/iso9660reader.hh \
			 ../include/ckfilesystem/iso9660writer.hh \
			 ../include/ckfilesystem/joliet.hh \
			 ../include/ckfilesystem/sectormanager.hh \
			 ../include/ckfilesystem/sectorstream.hh \
			 ../include/ckfilesystem/stringtable.hh \
			 ../include/ckfilesystem/udf.hh \
			 ../include/ckfilesystem/udfwriter.hh \
			 ../include/ckfilesystem/util.hh \
			 ../include/ckfilesystem/iso9660pathtable.hh

AM_CPPFLAGS = -I$(srcdir)/../include
lib_LTLIBRARIES = libckfilesystem.la
bin_PROGRAMS = verifier

libckfilesystem_la_SOURCES = dvdvideo.cc eltorito.cc filesystem.cc \
							 filesystemwriter.cc filesystemhelper.cc \
							 filetree.cc iforeader.cc iso9660.cc \
							 iso9660reader.cc iso9660writer.cc joliet.cc \
							 sectormanager.cc sectorstream.cc stringtable.cc \
							 udf.cc udfwriter.cc util.cc \
							 iso9660pathtable.cc

libckfilesystem_la_LDFLAGS = -version-info $(CKFILESYSTEM_VERSION)

verifier_SOURCES = iso9660verifier.cc verifier.cc
verifier_LDADD = libckfilesystem.la
verifier_LDFLAGS = -ldl -lpthread

library_includedir = $(includedir)/ckfilesystem
library_include_HEADERS = ../include/ckfilesystem/const.hh \
						  ../include/ckfilesystem/dvdvideo.hh \
						  ../include/ckfilesystem/eltorito.hh \
						  ../include/ckfilesystem/exception.hh \
						  ../include/ckfilesystem/fileset.hh \
						  ../include/ckfilesystem/filesystem.hh \
						  ../include/ckfilesystem/filesystemhelper.hh \
						  ../include/ckfilesystem/filesystemwriter.hh \
						  ../include/ckfilesystem/filetree.hh \
						  ../include/ckfilesystem/iforeader.hh \
						  ../include/ckfilesystem/iso9660.hh \
						  ../include/ckfilesystem/iso9660reader.hh \
						  ../include/ckfilesystem/iso9660writer.hh \
						  ../include/ckfilesystem/joliet.hh \
						  ../include/ckfilesystem/sectormanager.hh \
						  ../include/ckfilesystem/sectorstream.hh \
						  ../include/ckfilesystem/stringtable.hh \
						  ../include/ckfilesystem/udf.hh \
						  ../include/ckfilesystem/udfwriter.hh \
						  ../include/ckfilesystem/util.hh \
						  ../include/ckfilesystem/iso9660pathtable.hh