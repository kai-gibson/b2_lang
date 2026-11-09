#ifndef FILE_IO_H
#define FILE_IO_H
#include <expected>
#include <string>

#include "error.h"

/*
   Free wrapper functions around C++ IO
*/
enum class FileErrorCode : uint8_t {
  FileNotFound,
  ReadFailed,
  FileSizeError,
  OutOfMemory,
  Unknown,
};

struct ReadFileError : Error {
  FileErrorCode code;

  ReadFileError(FileErrorCode code, const std::string& message) noexcept
      : Error(message), code(code) {}
};

auto read_entire_file(const std::string& filename) _NOEXCEPT
    -> std::expected<std::string, ReadFileError>;

#endif  // FILE_IO_H
