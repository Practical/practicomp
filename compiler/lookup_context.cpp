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

PracticalSemanticAnalyzer::TypeId BuiltinContextGen::registerVoidType( String name ) {
    PracticalSemanticAnalyzer::TypeId ret;
    ret.p = LLVMVoidType();
    return ret;
}

PracticalSemanticAnalyzer::TypeId BuiltinContextGen::registerBoolType( String name ) {
    PracticalSemanticAnalyzer::TypeId ret;
    ret.p = LLVMInt1Type();
    return ret;
}

PracticalSemanticAnalyzer::TypeId BuiltinContextGen::registerIntegerType(
        String name, size_t bitSize, size_t alignment, bool _signed )
{
    PracticalSemanticAnalyzer::TypeId ret;
    ret.p = LLVMIntType(bitSize);
    return ret;
}

PracticalSemanticAnalyzer::TypeId BuiltinContextGen::registerCharType(
            String name, size_t bitSize, size_t alignment, bool _signed )
{
    return registerIntegerType( name, bitSize, alignment, _signed );
}
