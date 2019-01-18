//===--- MemoryBuffer.cpp - Memory Buffer implementation ------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
//  This file implements the MemoryBuffer interface.
//
//===----------------------------------------------------------------------===//

#include "llvm/Config/config.h"
#include "llvm/Support/MemoryBuffer.h"
#include "llvm/ADT/OwningPtr.h"
#include "llvm/ADT/SmallString.h"
#include "llvm/Support/MathExtras.h"
#include "llvm/System/Errno.h"
#include "llvm/System/Path.h"
#include "llvm/System/Process.h"
#include "llvm/System/Program.h"
#include <cassert>
#include <cstdio>
#include <cstring>
#if !defined(UNDER_CE)
#include <cerrno>
#endif
#if !defined(_MSC_VER) && !defined(__MINGW32__)
#include <unistd.h>
#include <sys/uio.h>
#else
#if !defined(UNDER_CE)
#include <io.h>
#endif
#endif
#if defined(HAVE_FCNTL_H)
#include <fcntl.h>
#endif
using namespace llvm;

#ifdef UNDER_CE
    #ifndef _TIME_T_DEFINED
        #ifdef _USE_32BIT_TIME_T
            typedef __time32_t time_t;      /* time value */
        #else
            typedef __time64_t time_t;      /* time value */
        #endif
        #define _TIME_T_DEFINED
    #endif

    #if !__STDC__
        /* Non-ANSI names for compatibility */
        struct stat {
            _dev_t     st_dev;
            _ino_t     st_ino;
            unsigned short st_mode;
            short      st_nlink;
            short      st_uid;
            short      st_gid;
            _dev_t     st_rdev;
            _off_t     st_size;
            time_t st_atime;
            time_t st_mtime;
            time_t st_ctime;
        };
    #endif
#endif

//===----------------------------------------------------------------------===//
// MemoryBuffer implementation itself.
//===----------------------------------------------------------------------===//

MemoryBuffer::~MemoryBuffer() { }

/// init - Initialize this MemoryBuffer as a reference to externally allocated
/// memory, memory that we know is already null terminated.
void MemoryBuffer::init(const char *BufStart, const char *BufEnd) {
  assert(BufEnd[0] == 0 && "Buffer is not null terminated!");
  BufferStart = BufStart;
  BufferEnd = BufEnd;
}

//===----------------------------------------------------------------------===//
// MemoryBufferMem implementation.
//===----------------------------------------------------------------------===//

/// CopyStringRef - Copies contents of a StringRef into a block of memory and
/// null-terminates it.
static void CopyStringRef(char *Memory, StringRef Data) {
  memcpy(Memory, Data.data(), Data.size());
  Memory[Data.size()] = 0; // Null terminate string.
}

/// GetNamedBuffer - Allocates a new MemoryBuffer with Name copied after it.
template <typename T>
static T* GetNamedBuffer(StringRef Buffer, StringRef Name) {
  char *Mem = static_cast<char*>(operator new(sizeof(T) + Name.size() + 1));
  CopyStringRef(Mem + sizeof(T), Name);
  return new (Mem) T(Buffer);
}

namespace {
/// MemoryBufferMem - Named MemoryBuffer pointing to a block of memory.
class MemoryBufferMem : public MemoryBuffer {
public:
  MemoryBufferMem(StringRef InputData) {
    init(InputData.begin(), InputData.end());
  }

  virtual const char *getBufferIdentifier() const {
     // The name is stored after the class itself.
    return reinterpret_cast<const char*>(this + 1);
  }
};
}

/// getMemBuffer - Open the specified memory range as a MemoryBuffer.  Note
/// that EndPtr[0] must be a null byte and be accessible!
MemoryBuffer *MemoryBuffer::getMemBuffer(StringRef InputData,
                                         StringRef BufferName) {
  return GetNamedBuffer<MemoryBufferMem>(InputData, BufferName);
}

/// getMemBufferCopy - Open the specified memory range as a MemoryBuffer,
/// copying the contents and taking ownership of it.  This has no requirements
/// on EndPtr[0].
MemoryBuffer *MemoryBuffer::getMemBufferCopy(StringRef InputData,
                                             StringRef BufferName) {
  MemoryBuffer *Buf = getNewUninitMemBuffer(InputData.size(), BufferName);
  if (!Buf) return 0;
  memcpy(const_cast<char*>(Buf->getBufferStart()), InputData.data(),
         InputData.size());
  return Buf;
}

/// getNewUninitMemBuffer - Allocate a new MemoryBuffer of the specified size
/// that is not initialized.  Note that the caller should initialize the
/// memory allocated by this method.  The memory is owned by the MemoryBuffer
/// object.
MemoryBuffer *MemoryBuffer::getNewUninitMemBuffer(size_t Size,
                                                  StringRef BufferName) {
  // Allocate space for the MemoryBuffer, the data and the name. It is important
  // that MemoryBuffer and data are aligned so PointerIntPair works with them.
  size_t AlignedStringLen =
    RoundUpToAlignment(sizeof(MemoryBufferMem) + BufferName.size() + 1,
                       sizeof(void*)); // VIV:TODO: Is sizeof(void*) enough?
  size_t RealLen = AlignedStringLen + Size + 1;
  char *Mem = static_cast<char*>(operator new(RealLen, std::nothrow));
  if (!Mem) return 0;

  // The name is stored after the class itself.
  CopyStringRef(Mem + sizeof(MemoryBufferMem), BufferName);

  // The buffer begins after the name and must be aligned.
  char *Buf = Mem + AlignedStringLen;
  Buf[Size] = 0; // Null terminate buffer.

  return new (Mem) MemoryBufferMem(StringRef(Buf, Size));
}

/// getNewMemBuffer - Allocate a new MemoryBuffer of the specified size that
/// is completely initialized to zeros.  Note that the caller should
/// initialize the memory allocated by this method.  The memory is owned by
/// the MemoryBuffer object.
MemoryBuffer *MemoryBuffer::getNewMemBuffer(size_t Size, StringRef BufferName) {
  MemoryBuffer *SB = getNewUninitMemBuffer(Size, BufferName);
  if (!SB) return 0;
  memset(const_cast<char*>(SB->getBufferStart()), 0, Size);
  return SB;
}


/// getFileOrSTDIN - Open the specified file as a MemoryBuffer, or open stdin
/// if the Filename is "-".  If an error occurs, this returns null and fills
/// in *ErrStr with a reason.  If stdin is empty, this API (unlike getSTDIN)
/// returns an empty buffer.
MemoryBuffer *MemoryBuffer::getFileOrSTDIN(StringRef Filename,
                                           std::string *ErrStr,
                                           int64_t FileSize,
                                           void *FileInfo) {
  if (Filename == "-")
    return getSTDIN(ErrStr);
  return getFile(Filename, ErrStr, FileSize, FileInfo);
}

MemoryBuffer *MemoryBuffer::getFileOrSTDIN(const char *Filename,
                                           std::string *ErrStr,
                                           int64_t FileSize,
                                           void *FileInfo) {
  if (strcmp(Filename, "-") == 0)
    return getSTDIN(ErrStr);
  return getFile(Filename, ErrStr, FileSize, FileInfo);
}

//===----------------------------------------------------------------------===//
// MemoryBuffer::getFile implementation.
//===----------------------------------------------------------------------===//

namespace {
/// MemoryBufferMMapFile - This represents a file that was mapped in with the
/// sys::Path::MapInFilePages method.  When destroyed, it calls the
/// sys::Path::UnMapFilePages method.
class MemoryBufferMMapFile : public MemoryBufferMem {
public:
  MemoryBufferMMapFile(StringRef Buffer)
    : MemoryBufferMem(Buffer) { }

  ~MemoryBufferMMapFile() {
    sys::Path::UnMapFilePages(getBufferStart(), getBufferSize());
  }
};

/// FileCloser - RAII object to make sure an FD gets closed properly.
class FileCloser {
  int FD;
public:
  explicit FileCloser(int FD) : FD(FD) {}
  ~FileCloser() {
#if !defined(UNDER_CE)
      ::close(FD);
#else
      gcoOS_Close(gcvNULL, (gctFILE)FD);
#endif
  }
};
}

MemoryBuffer *MemoryBuffer::getFile(StringRef Filename, std::string *ErrStr,
                                    int64_t FileSize, void *FileInfo) {
  SmallString<256> PathBuf(Filename.begin(), Filename.end());
  return MemoryBuffer::getFile(PathBuf.c_str(), ErrStr, FileSize, FileInfo);
}

MemoryBuffer *MemoryBuffer::getFile(const char *Filename, std::string *ErrStr,
                                    int64_t FileSize, void *FileInfo) {
  int FD;

#if !defined(UNDER_CE)
  int OpenFlags = O_RDONLY;
#ifdef O_BINARY
  OpenFlags |= O_BINARY;  // Open input file in binary mode on win32.
#endif
  FD = ::open(Filename, OpenFlags);
  if (FD == -1) {
    if (ErrStr) *ErrStr = sys::StrError();
    return 0;
  }
  FileCloser FC(FD); // Close FD on return.

  // If we don't know the file size, use fstat to find out.  fstat on an open
  // file descriptor is cheaper than stat on a random path.
  if (FileSize == -1 || FileInfo) {
    struct stat MyFileInfo;
    struct stat *FileInfoPtr = FileInfo? (struct stat*)FileInfo : &MyFileInfo;

    // VIV:TODO: This should use fstat64 when available.
    if (fstat(FD, FileInfoPtr) == -1) {
      if (ErrStr) *ErrStr = sys::StrError();
      return 0;
    }
    FileSize = FileInfoPtr->st_size;
  }


  // If the file is large, try to use mmap to read it in.  We don't use mmap
  // for small files, because this can severely fragment our address space. Also
  // don't try to map files that are exactly a multiple of the system page size,
  // as the file would not have the required null terminator.
  //
  // FIXME: Can we just mmap an extra page in the latter case?
  if (FileSize >= 4096*4 &&
      (FileSize & (sys::Process::GetPageSize()-1)) != 0) {
    if (const char *Pages = sys::Path::MapInFilePages(FD, FileSize)) {
      return GetNamedBuffer<MemoryBufferMMapFile>(StringRef(Pages, FileSize),
                                                  Filename);
    }
  }
#else
  gceSTATUS status;
  status = gcoOS_Open(gcvNULL,
                      Filename,
                      gcvFILE_READ,
                      (gctFILE*)&FD);
  if(gcmIS_ERROR(status)) {
      return 0;
  }
  FileCloser FC(FD); // Close FD on return.
  if (FileSize == -1 || FileInfo) {
    struct stat MyFileInfo;
    struct stat *FileInfoPtr = FileInfo ? (struct stat*)FileInfo : &MyFileInfo;

    status = gcoOS_Stat(gcvNULL, Filename, FileInfoPtr);
    if(gcmIS_ERROR(status)) {
      return 0;
    }
    FileSize = FileInfoPtr->st_size;
  }
#endif

  MemoryBuffer *Buf = MemoryBuffer::getNewUninitMemBuffer(FileSize, Filename);
  if (!Buf) {
    // Failed to create a buffer.
    if (ErrStr) *ErrStr = "could not allocate buffer";
    return 0;
  }

  OwningPtr<MemoryBuffer> SB(Buf);
  char *BufPtr = const_cast<char*>(SB->getBufferStart());

#if !defined(UNDER_CE)
  size_t BytesLeft = FileSize;
  while (BytesLeft) {
    ssize_t NumRead = ::read(FD, BufPtr, BytesLeft);
    if (NumRead == -1) {
      if (errno == EINTR)
        continue;
      // Error while reading.
      if (ErrStr) *ErrStr = sys::StrError();
      return 0;
    } else if (NumRead == 0) {
      // We hit EOF early, truncate and terminate buffer.
      Buf->BufferEnd = BufPtr;
      *BufPtr = 0;
      return SB.take();
    }
    BytesLeft -= NumRead;
    BufPtr += NumRead;
  }
#else
  size_t BytesLeft = FileSize;
  while (BytesLeft) {
      gctSIZE_T NumRead;
      status = gcoOS_Read(gcvNULL,
                          (gctFILE)FD,
                          FileSize,
                          BufPtr,
                          &NumRead);

      if(gcmIS_ERROR(status)) {
          return 0;
      }

      BytesLeft -= NumRead;
      BufPtr += NumRead;
  }
#endif

  return SB.take();
}

//===----------------------------------------------------------------------===//
// MemoryBuffer::getSTDIN implementation.
//===----------------------------------------------------------------------===//

MemoryBuffer *MemoryBuffer::getSTDIN(std::string *ErrStr) {
  // Read in all of the data from stdin, we cannot mmap stdin.
  //
  // FIXME: That isn't necessarily true, we should try to mmap stdin and
  // fallback if it fails.
  sys::Program::ChangeStdinToBinary();

  const ssize_t ChunkSize = 4096*4;
  SmallString<ChunkSize> Buffer;
#if !defined(UNDER_CE)
  ssize_t ReadBytes;
  // Read into Buffer until we hit EOF.
  do {
    Buffer.reserve(Buffer.size() + ChunkSize);
    ReadBytes = read(0, Buffer.end(), ChunkSize);
    if (ReadBytes == -1) {
      if (errno == EINTR) continue;
      if (ErrStr) *ErrStr = sys::StrError();
      return 0;
    }
    Buffer.set_size(Buffer.size() + ReadBytes);
  } while (ReadBytes != 0);
#endif

  return getMemBufferCopy(Buffer, "<stdin>");
}
