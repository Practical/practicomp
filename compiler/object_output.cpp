/* This file is part of the Practical programming langauge. https://github.com/Practical/practical-sa
 *
 * This file is file is copyright (C) 2018-2019 by its authors.
 * You can see the file's authors in the AUTHORS file in the project's home repository.
 *
 * This is available under the Boost license. The license's text is available under the LICENSE file in the project's
 * home directory.
 */
#include "object_output.h"

#include <llvm-c/Target.h>

ObjectOutput::ObjectOutput(std::filesystem::path outputFile, const char *targetTriplet, ModuleGenImpl &module)
{
    LLVMInitializeAllTargetInfos();
    LLVMInitializeAllTargets();
    LLVMInitializeAllTargetMCs();
    LLVMInitializeAllAsmParsers();
    LLVMInitializeAllAsmPrinters();

    LLVMTargetRef target;
    char *errorMessage = nullptr;
    if( LLVMGetTargetFromTriple( targetTriplet, &target, &errorMessage )!=0 ) {
        std::cerr<<"LLVMGetTargetFromTriple failed: "<<errorMessage<<"\n";
        abort();
    }

    auto targetMachine = LLVMCreateTargetMachine(target, targetTriplet, "generic", "", LLVMCodeGenLevelNone, LLVMRelocDefault, LLVMCodeModelDefault);

    LLVMSetDataLayout(module.getLLVMModule(), "e-S64-p:64:64-i8:8-i16:16-i32:32-i64:64"); // TODO value for x86-64
    errorMessage = nullptr;
    if( LLVMTargetMachineEmitToFile( targetMachine, module.getLLVMModule(), const_cast<char *>(outputFile.c_str()), LLVMObjectFile, &errorMessage )!=0 ) {
        std::cerr<<"Output to file failed: "<<errorMessage<<"\n";
        abort();
    }
}
