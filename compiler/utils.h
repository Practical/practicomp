#ifndef UTILS_H
#define UTILS_H

#include <slice.h>

#include <string>

static inline std::string toStdString(String str) {
    return std::string(str.get(), str.size());
}

#endif // UTILS_H
