// XZip.h  Version 1.3

#pragma once

namespace Plugin {
namespace XZip {
#ifndef XZIP_H
#define XZIP_H

// ZIP functions -- for creating zip files
// This file is a repackaged form of the Info-Zip source code available
// at www.info-zip.org. The original copyright notice may be found in
// zip.cpp. The repackaging was done by Lucian Wischik to simplify its
// use in Windows/C++.

#ifndef XUNZIP_H
DECLARE_HANDLE(HZIP);		// An HZIP identifies a zip file that is being created
#endif

typedef DWORD ZRESULT;		// result codes from any of the zip functions. Listed later.

// flag values passed to some functions
#define ZIP_HANDLE   1
#define ZIP_FILENAME 2
#define ZIP_MEMORY   3
#define ZIP_FOLDER   4


///////////////////////////////////////////////////////////////////////////////
//
// CreateZip()
//
// Purpose:     Create a zip archive file
//
// Parameters:  z      - archive file name if flags is ZIP_FILENAME;  for other
//                       uses see below
//              len    - for memory (ZIP_MEMORY) should be the buffer size;
//                       for other uses, should be 0
//              flags  - indicates usage, see below;  for files, this will be
//                       ZIP_FILENAME
//
// Returns:     HZIP   - non-zero if zip archive created ok, otherwise 0
//
HZIP CreateZip(void *z, unsigned int len, DWORD flags);
// CreateZip - call this to start the creation of a zip file.
// As the zip is being created, it will be stored somewhere:
// to a pipe:              CreateZip(hpipe_write, 0,ZIP_HANDLE);
// in a file (by handle):  CreateZip(hfile, 0,ZIP_HANDLE);
// in a file (by name):    CreateZip("c:\\test.zip", 0,ZIP_FILENAME);
// in memory:              CreateZip(buf, len,ZIP_MEMORY);
// or in pagefile memory:  CreateZip(0, len,ZIP_MEMORY);


///////////////////////////////////////////////////////////////////////////////
//
// ZipAdd()
//
// Purpose:     Add a file to a zip archive
//
// Parameters:  hz      - handle to an open zip archive
//              dstzn   - name used inside the zip archive to identify the file
//              src     - for a file (ZIP_FILENAME) this specifies the filename
//                        to be added to the archive;  for other uses, see below
//              len     - for memory (ZIP_MEMORY) this specifies the buffer 
//                        length;  for other uses, this should be 0
//              flags   - indicates usage, see below;  for files, this will be
//                        ZIP_FILENAME
//
// Returns:     ZRESULT - ZR_OK if success, otherwise some other value
//
ZRESULT ZipAdd(HZIP hz, const CHAR *dstzn, void *src, unsigned int len, DWORD flags);

///////////////////////////////////////////////////////////////////////////////
//
// CloseZip()
//
// Purpose:     Close an open zip archive
//
// Parameters:  hz      - handle to an open zip archive
//
// Returns:     ZRESULT - ZR_OK if success, otherwise some other value
//
ZRESULT CloseZip(HZIP hz);
// CloseZip - the zip handle must be closed with this function.


ZRESULT ZipGetMemory(HZIP hz, void **buf, unsigned long *len);
// ZipGetMemory - If the zip was created in memory, via ZipCreate(0,ZIP_MEMORY),
// then this function will return information about that memory block.
// buf will receive a pointer to its start, and len its length.
// Note: you can't add any more after calling this.


unsigned int FormatZipMessage(ZRESULT code, char *buf, unsigned int len);
// FormatZipMessage - given an error code, formats it as a string.
// It returns the length of the error message. If buf/len points
// to a real buffer, then it also writes as much as possible into there.


// These are the result codes:
#define ZR_OK         0x00000000     // nb. the pseudo-code zr-recent is never returned,
#define ZR_RECENT     0x00000001     // but can be passed to FormatZipMessage.
// The following come from general system stuff (e.g. files not openable)
#define ZR_GENMASK    0x0000FF00
#define ZR_NODUPH     0x00000100     // couldn't duplicate the handle
#define ZR_NOFILE     0x00000200     // couldn't create/open the file
#define ZR_NOALLOC    0x00000300     // failed to allocate some resource
#define ZR_WRITE      0x00000400     // a general error writing to the file
#define ZR_NOTFOUND   0x00000500     // couldn't find that file in the zip
#define ZR_MORE       0x00000600     // there's still more data to be unzipped
#define ZR_CORRUPT    0x00000700     // the zipfile is corrupt or not a zipfile
#define ZR_READ       0x00000800     // a general error reading the file
// The following come from mistakes on the part of the caller
#define ZR_CALLERMASK 0x00FF0000
#define ZR_ARGS       0x00010000     // general mistake with the arguments
#define ZR_NOTMMAP    0x00020000     // tried to ZipGetMemory, but that only works on mmap zipfiles, which yours wasn't
#define ZR_MEMSIZE    0x00030000     // the memory size is too small
#define ZR_FAILED     0x00040000     // the thing was already failed when you called this function
#define ZR_ENDED      0x00050000     // the zip creation has already been closed
#define ZR_MISSIZE    0x00060000     // the indicated input file size turned out mistaken
#define ZR_PARTIALUNZ 0x00070000     // the file had already been partially unzipped
#define ZR_ZMODE      0x00080000     // tried to mix creating/opening a zip 
// The following come from bugs within the zip library itself
#define ZR_BUGMASK    0xFF000000
#define ZR_NOTINITED  0x01000000     // initialisation didn't work
#define ZR_SEEK       0x02000000     // trying to seek in an unseekable file
#define ZR_NOCHANGE   0x04000000     // changed its mind on storage, but not allowed
#define ZR_FLATE      0x05000000     // an internal error in the de/inflation code

// Now we indulge in a little skullduggery so that the code works whether
// the user has included just zip or both zip and unzip.
// Idea: if header files for both zip and unzip are present, then presumably
// the cpp files for zip and unzip are both present, so we will call
// one or the other of them based on a dynamic choice. If the header file
// for only one is present, then we will bind to that particular one.
HZIP CreateZipZ(void *z, unsigned int len, DWORD flags);
ZRESULT CloseZipZ(HZIP hz);
unsigned int FormatZipMessageZ(ZRESULT code, char *buf, unsigned int len);
bool IsZipHandleZ(HZIP hz);
BOOL AddFolderContent(HZIP hZip, CHAR* AbsolutePath, CHAR* DirToAdd);

#define CreateZip CreateZipZ

#ifdef XUNZIP_H
#undef CloseZip
#define CloseZip(hz) (IsZipHandleZ(hz)?CloseZipZ(hz):CloseZipU(hz))
#else
#define CloseZip CloseZipZ
#define FormatZipMessage FormatZipMessageZ
#endif

#endif //XZIP_H

}
}

using namespace Plugin::XZip;