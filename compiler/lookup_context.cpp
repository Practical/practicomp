/* This file is part of the Practical programming langauge. https://github.com/Practical/practical-sa
 *
 * This file is file is copyright (C) 2018-2020 by its authors.
 * You can see the file's authors in the AUTHORS file in the project's home repository.
 *
 * This is available under the Boost license. The license's text is available under the LICENSE file in the project's
 * home directory.
 */
#include "lookup_context.h"

#include <llvm-c/Core.h>

static_assert( sizeof(LLVMTypeRef)==sizeof(PracticalSemanticAnalyzer::TypeId::p),
        "PracticalSemanticAnalyzer::TypeId is not the right size for storing LLVMTypeRef" );
PracticalSemanticAnalyzer::TypeId BuiltinContextGen::registerVoidType() {
    PracticalSemanticAnalyzer::TypeId ret;
    ret.p = LLVMVoidType();
    return ret;
}

PracticalSemanticAnalyzer::TypeId BuiltinContextGen::registerBoolType() {
    PracticalSemanticAnalyzer::TypeId ret;
    ret.p = LLVMInt1Type();
    return ret;
}

PracticalSemanticAnalyzer::TypeId BuiltinContextGen::registerIntegerType( size_t bitSize, size_t alignment, bool _signed )
{
    PracticalSemanticAnalyzer::TypeId ret;
    ret.p = LLVMIntType(bitSize);
    return ret;
}

PracticalSemanticAnalyzer::TypeId BuiltinContextGen::registerCharType( size_t bitSize, size_t alignment, bool _signed )
{
    return registerIntegerType( bitSize, alignment, _signed );
}
