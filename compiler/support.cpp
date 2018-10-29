#include "config.h"

#include "support.h"

#include <iostream>

void emitMsg(MsgLevel level, const char *fileName, const char *msg) {
    std::cerr << fileName << ": ";
    switch(level) {
    case MsgLevel::Info:
        std::cerr << "info: ";
        break;
    case MsgLevel::Warning:
        std::cerr << "warning: ";
        break;
    case MsgLevel::Error:
        std::cerr << "error: ";
        break;
    case MsgLevel::Fatal:
        std::cerr << "fatal error: ";
        break;
    }

    std::cerr << msg << "\n";
}
