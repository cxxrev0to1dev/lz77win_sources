#ifdef _WIN32

#include <errno.h>
#include <stddef.h>
#include <unistd.h>
#include <sys/utime.h>
#include <process.h>
#include <stdlib.h>
#include <windows.h>
#include "archive_platform.h"
#include "libarchive-nonposix.h"

char *fullname (const char *name)
{
    char *rpath = NULL;
    char *lpFilePart;
    int len, res;
    len = GetFullPathName(name, 0, rpath, &lpFilePart);
    if (len == 0) {
         errno = EINVAL;
         return NULL;
    }
    rpath = malloc(len);
    res = GetFullPathName(name, len, rpath, &lpFilePart);
    if (res == 0) {
         errno = EINVAL;
         return NULL;
    }
    return rpath;
}

int readlink (const char *path, char *buf, size_t len)
{
	set_errno (ENOSYS);
	return -1;
}

/* Make a link to FROM called TO.  */
int link (from, to)
     const char *from;
     const char *to;
{
	int res;

	if (from == NULL || to == NULL) {
		set_errno (EINVAL);
		return -1;
	}

	if (!_access (from, F_OK))
		res = CopyFile (from, to, FALSE);
	else {
		/* from doesn not exist; try to prepend it with the dirname of to */
		char *fullfrompath, *slash, *todir;
		todir = strdup (to);
		if (!todir)
			return -1;
		slash = strrchr(todir, '/');
		if (slash)
			*slash = '\0';
		fullfrompath = malloc (strlen (from) + strlen (todir) + 2);
		if (!fullfrompath)
			return -1;
		strcpy (fullfrompath, todir);
		strcat (fullfrompath, "/");
		strcat (fullfrompath, from);
		if (todir)
			free (todir);
		if (_access (fullfrompath, R_OK))
			return -1;
		res = CopyFile (fullfrompath, to, FALSE);
		if (fullfrompath)
			free (fullfrompath);
	}

	if (res == 0) {
		set_errno (EINVAL);
		return -1;
	}
	return 0;
}

/* Make a symbolic link to FROM called TO.  */
int symlink (from, to)
     const char *from;
     const char *to;
{
	return link (from, to);
}

int utime (const char *filename, struct utimbuf *times)
{
	int res, WasReadOnly = 0;
	DWORD attr;
	UINT OldErrorMode;
	
	OldErrorMode = SetErrorMode (SEM_FAILCRITICALERRORS | SEM_NOOPENFILEERRORBOX);
	if ((attr = GetFileAttributes ((LPTSTR) filename)) == INVALID_FILE_ATTRIBUTES) {
		set_errno (EACCES); /* file does not exist or attributes cannot be read */
		return -1;
	}
	SetErrorMode (OldErrorMode);
	if (attr & FILE_ATTRIBUTE_DIRECTORY)
		return 0;
	if (attr & FILE_ATTRIBUTE_READONLY) {
		attr &= ~FILE_ATTRIBUTE_READONLY;
		WasReadOnly = 1;		
		if (!SetFileAttributes ((LPTSTR) filename, attr)) {
			set_errno (EACCES);
			return -1;
		}
	}
	res = _utime (filename, (struct _utimbuf *) times);
	if (WasReadOnly) {
		attr |= FILE_ATTRIBUTE_READONLY;
		if (!SetFileAttributes ((LPTSTR) filename, attr)) {
			set_errno (EACCES);
			return -1;
		}
	}
	return res;
}

int utimes (const char *path, const struct timeval times[2])
{
	struct utimbuf ut;
	if (path == NULL) {
		set_errno (EINVAL);
		return -1;
	}
	ut.actime = times[0].tv_sec;
	ut.modtime = times[1].tv_sec;
	return utime (path, &ut);
}

int unlink (const char *filename)
{
	if (_access (filename, W_OK) < 0) 
		switch (errno) {
			case EINVAL:
				return -1;
				break;
			case ENOENT:
				return -1;
				break;
			case EACCES:
				_chmod (filename, _S_IREAD | _S_IWRITE);
				break;
			default:
				break;
		}
	return _unlink (filename);
}

static int get_dev_ino (HANDLE hFile, dev_t *dev, ino_t *ino)
{
/* dev_t: short (2 bytes);  ino_t: unsigned int (4 bytes) */
#define LODWORD(l) ((DWORD)((DWORDLONG)(l)))
#define HIDWORD(l) ((DWORD)(((DWORDLONG)(l)>>32)&0xFFFFFFFF))
#define MAKEDWORDLONG(a,b) ((DWORDLONG)(((DWORD)(a))|(((DWORDLONG)((DWORD)(b)))<<32)))

#define INOSIZE (8*sizeof(ino_t)) /* 32 */
#define DEVSIZE (8*sizeof(dev_t)-1) /* 15 */
#define SEQNUMSIZE (16)

	BY_HANDLE_FILE_INFORMATION FileInformation;
	uint64_t ino64, FileReferenceNumber ;
	ino_t resino;
	dev_t resdev;
	DWORD VolumeSerialNumber;
	
	*ino = 0;
	*dev = 0;
	if (hFile == INVALID_HANDLE_VALUE) /* file cannot be opened */
		return 0;
	ZeroMemory (&FileInformation, sizeof(FileInformation));
	if (!GetFileInformationByHandle (hFile, &FileInformation)) /* cannot obtain FileInformation */
		return 0;
	ino64 = (uint64_t) MAKEDWORDLONG (
		FileInformation.nFileIndexLow, FileInformation.nFileIndexHigh);
	FileReferenceNumber = ino64 & ((~(0ULL)) >> SEQNUMSIZE); /* remove sequence number */
	/* transform 64-bits ino into 32-bits by hashing */ 
	resino = (ino_t) (
			( (LODWORD(FileReferenceNumber)) ^ ((LODWORD(FileReferenceNumber)) >> INOSIZE) )
//		^
//			( (HIDWORD(FileReferenceNumber)) ^ ((HIDWORD(FileReferenceNumber)) >> INOSIZE) )
		);
	*ino = resino;
	VolumeSerialNumber = FileInformation.dwVolumeSerialNumber;
	resdev = 	(unsigned short) ( (LOWORD(VolumeSerialNumber)) ^ ((HIWORD(VolumeSerialNumber)) >> DEVSIZE) );
	*dev = resdev;
printf ("get_dev_ino: dev = %d; ino = %u\n", resdev, resino);
	return 0;
} 

int get_dev_ino_fd (int fd, dev_t *dev, ino_t *ino)
{
	HANDLE hFile;
	hFile = (HANDLE) _get_osfhandle (fd); 
	return get_dev_ino (hFile, dev, ino);
}

int get_dev_ino_filename (char *path, dev_t *dev, ino_t *ino)
{
	HANDLE hFile;
	int res;
	if (!path || !*path) /* path = NULL */
		return 0;
	if (_access (path, F_OK)) /* path does not exist */
		return -1;
/* obtain handle to file "name"; FILE_FLAG_BACKUP_SEMANTICS is used to open directories */
	hFile = CreateFile (path, 0, 0, NULL, OPEN_EXISTING, 
		FILE_FLAG_BACKUP_SEMANTICS | FILE_ATTRIBUTE_READONLY, 
		NULL);
	res = get_dev_ino (hFile, dev, ino);
	CloseHandle (hFile);
	return res;
}

int fstati64 (int fd, struct _stati64 *st)
{
	int res;
	res = _fstati64 (fd, st);
	if (res < 0)
		return -1;
	if (st->st_ino == 0) 
		res = get_dev_ino_fd (fd, &st->st_dev, &st->st_ino);
//	printf ("fstat: dev = %u; ino = %u\n", st->st_dev, st->st_ino);
	return res;
}

int fcntl (int fd, int cmd, ...)
{
  set_errno (ENOSYS);
  return -1;
}

int fork ()
{
  set_errno (ENOSYS);
  return -1;
}

pid_t vfork (void)
{
  return fork ();
}

int
setenv (name, value, replace)
     const char *name;
     const char *value;
     int replace;
{
  char *string;
  int res;
  
  if (name == NULL || *name == '\0' || strchr (name, '=') != NULL)
    {
      set_errno (EINVAL);
      return -1;
    }
  if (getenv (name) && !replace)
  	return -1;

  string = (char *) malloc (strlen(name) + strlen(value) + 2);
  if (!string)
  	return -1;
  strcpy (string, name);
  strcat (string, "=");
  strcat (string, value);
  res = _putenv (string);
  free (string);
  return res;
}

int
unsetenv (name)
     const char *name;
{
  char *string;
  int res;
  
  if (name == NULL || *name == '\0' || strchr (name, '=') != NULL)
    {
      set_errno (EINVAL);
      return -1;
    }
  string = (char *) malloc (strlen(name) + 2);
  if (!string)
  	return -1;
  strcpy (string, name);
  strcat (string, "=");
  res = _putenv (string);
  free (string);
  return res;
}

int pipe (int pipedes[2])
{
     return _pipe(pipedes, 0, _O_BINARY | _O_NOINHERIT);
}

unsigned int sleep (seconds)
     unsigned int seconds;
{
  Sleep (seconds * 1000);
  return 0;
}

pid_t waitpid (pid_t pid, int *stat_loc, int options)
{
     return _cwait (stat_loc, pid, 0);
}

int mkstemp(char *template)
{
	char *tmpfilename;
	
//	fprintf (stderr, "mkstemp: template = %s\n", template);
	tmpfilename = mktemp (template);
//	fprintf (stderr, "mkstemp: tmpfilename = %s\n", tmpfilename);
	return open (tmpfilename, O_CREAT | O_EXCL | O_RDWR | O_BINARY);
}

char *nl_langinfo(nl_item item)
{
	int res;
	TCHAR LCData[2];
	res = GetLocaleInfo(LOCALE_USER_DEFAULT, LOCALE_ILDATE, (LPTSTR) &LCData, 
		sizeof(LCData) / sizeof(TCHAR) );
	if (item == D_MD_ORDER) {
		if (LCData[0] != '0')
			return "d";
		else
			return "m";
	} else {
		errno = EINVAL;
		return NULL;
	}
}

#endif /* _WIN32 */
