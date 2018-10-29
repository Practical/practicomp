#ifndef SUPPORT_H
#define SUPPORT_H

enum class MsgLevel {
    Info,
    Warning,
    Error,
    Fatal
};

void emitMsg(MsgLevel level, const char *fileName, const char *msg);

#endif // SUPPORT_H
