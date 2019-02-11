#ifndef UTILS_H
#define UTILS_H

#include <slice.h>

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
