/* This file is part of the Practical programming langauge. https://github.com/Practical/practical-sa
 *
 * This file is file is copyright (C) 2018-2019 by its authors.
 * You can see the file's authors in the AUTHORS file in the project's home repository.
 *
 * This is available under the Boost license. The license's text is available under the LICENSE file in the project's
 * home directory.
 */
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
