/* This file is part of the Practical programming langauge. https://github.com/Practical/practical-sa
 *
 * To the extent header files enjoy copyright protection, this file is file is copyright (C) 2018-2019 by its authors
 * You can see the file's authors in the AUTHORS file in the project's home repository.
 *
 * This is available under the Boost license. The license's text is available under the LICENSE file in the project's
 * home directory.
 */
#ifndef UTILS_H
#define UTILS_H

#include <practical/slice.h>

#include <string>

static inline std::string toStdString(String str) {
    return std::string(str.get(), str.size());
}

class toCStr : private NoCopy {
    char *buffer;

public:
    explicit toCStr(String str) : buffer(new char[str.size()+1]) {
        memcpy(buffer, str.get(), str.size());
        buffer[str.size()] = '\0';
    }

    ~toCStr() {
        delete [] buffer;
    }

    operator const char *() const {
        return buffer;
    }
};

#endif // UTILS_H
