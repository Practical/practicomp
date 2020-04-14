/* This file is part of the Practical programming langauge. https://github.com/Practical/practical-sa
 *
 * This file is file is copyright (C) 2018-2020 by its authors.
 * You can see the file's authors in the AUTHORS file in the project's home repository.
 *
 * This is available under the Boost license. The license's text is available under the LICENSE file in the project's
 * home directory.
 */
#ifndef LOOKUP_CONTEXT_H
#define LOOKUP_CONTEXT_H

#include <practical-sa.h>

class BuiltinContextGen : public PracticalSemanticAnalyzer::BuiltinContextGen {
public:
    virtual PracticalSemanticAnalyzer::TypeId registerVoidType() override final;
    virtual PracticalSemanticAnalyzer::TypeId registerBoolType() override final;
    virtual PracticalSemanticAnalyzer::TypeId registerIntegerType( size_t bitSize, size_t alignment, bool _signed ) override final;
    virtual PracticalSemanticAnalyzer::TypeId registerCharType( size_t bitSize, size_t alignment, bool _signed ) override final;
};

#endif // LOOKUP_CONTEXT_H
