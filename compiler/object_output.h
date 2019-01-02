#ifndef OBJECT_OUTPUT_H
#define OBJECT_OUTPUT_H

#include "code_gen.h"

#include <llvm-c/TargetMachine.h>

#include <filesystem>

class ObjectOutput {
    LLVMTargetRef target;

public:
    ObjectOutput(std::filesystem::path outputFile, const char *targetTriplet, ModuleGenImpl &module);

};

#endif // OBJECT_OUTPUT_H
