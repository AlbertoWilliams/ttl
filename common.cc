/**
 * common.cc - common utilities
 *
 * Author: Bao Hexing <HexingB@qq.com>
 * Created: 25 November 2017
 *
 * Copyright © 2017, Bao Hexing. All Rights Reserved.
 */

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string>
#include <fcntl.h>

namespace ttl {
    bool FileExists(const std::string& filename) {
        struct stat buf;
        int ret = stat(filename.c_str(), &buf);
        return ret == 0 && (buf.st_mode & S_IFMT) == S_IFREG && (buf.st_mode & S_IRUSR);
    }

    const char * ReadFile(const std::string& filename) {
        if (FileExists(filename) == false) {
            return NULL;
        }

        struct stat buf;
        int ret = stat(filename.c_str(), &buf);

        // TODO: use mmap
        char * buffer = new char[buf.st_size + 1];
        size_t size = 0;

        int fd = open(filename.c_str(), O_RDONLY);
        while (true) {
            ssize_t bytes = read(fd, buffer + size, buf.st_size - size);
            if (bytes > 0) {
                size += bytes;
            }

            if (bytes == 0) {
                break;
            }
        }
        buffer[buf.st_size] = '\0';

        return buffer;
    }
}
