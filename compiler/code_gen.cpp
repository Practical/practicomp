/* This file is part of the Practical programming langauge. https://github.com/Practical/practical-sa
 *
 * This file is file is copyright (C) 2018-2019 by its authors.
 * You can see the file's authors in the AUTHORS file in the project's home repository.
 *
 * This is available under the Boost license. The license's text is available under the LICENSE file in the project's
 * home directory.
 */
#include "code_gen.h"

#include "utils.h"

#include <llvm-c/Analysis.h>

FunctionGenImpl::~FunctionGenImpl() {
    assert(llvmFunction==nullptr);
    assert(currentBlock==nullptr);
    assert(builder==nullptr);
}

static LLVMTypeRef toLLVMTypeBuiltin(const NamedType *type) {
    LLVMTypeRef ret;

    switch(type->type()) {
    case NamedType::Type::Void:
        ret = LLVMVoidType();
        break;
    case NamedType::Type::Boolean:
        ret = LLVMInt8Type();
        break;
    case NamedType::Type::SignedInteger:
    case NamedType::Type::UnsignedInteger:
    case NamedType::Type::Char:
        ret = LLVMIntType(type->size());
        break;
    }

    return ret;
}

static LLVMTypeRef toLLVMType(StaticType::Ptr practiType) {
    const NamedType *namedType = lookupTypeId( practiType->getId() );

    assert( namedType->isBuiltin() ); // TODO implement support for user defined types

    return toLLVMTypeBuiltin( namedType );
}

void FunctionGenImpl::functionEnter(
        IdentifierId id, String name, StaticType::Ptr returnType, Slice<const ArgumentDeclaration> arguments,
        String file, size_t line, size_t col)
{
    auto llvmModule = module->getLLVMModule();

    std::vector<LLVMTypeRef> argumentTypes;
    argumentTypes.reserve(arguments.size());
    for( auto &argDecl : arguments ) {
        argumentTypes.emplace_back( toLLVMType(argDecl.type) );
    }

    LLVMTypeRef retType = LLVMFunctionType( toLLVMType(returnType), argumentTypes.data(), argumentTypes.size(), false );
    llvmFunction = LLVMAddFunction( llvmModule, toStdString(name).c_str(), retType );
    currentBlock = LLVMAppendBasicBlock( llvmFunction, "" );

    builder = LLVMCreateBuilder();
    LLVMPositionBuilderAtEnd(builder, currentBlock);

    // Allocate stack location for the arguments, so that they behave like lvalues
    for( size_t i = 0; i<arguments.size(); ++i ) {
        allocateStackVar( arguments[i].lvalueId, arguments[i].type, arguments[i].name );
        LLVMBuildStore(builder, LLVMGetParam( llvmFunction, i ), lookupExpression(arguments[i].lvalueId));
    }
}

void FunctionGenImpl::functionLeave(IdentifierId id)
{
    LLVMDisposeBuilder(builder);
    builder = nullptr;
    currentBlock = nullptr;
    llvmFunction = nullptr;
}

void FunctionGenImpl::returnValue(ExpressionId id) {
    LLVMBuildRet( builder, lookupExpression(id) );
}

void FunctionGenImpl::setLiteral(ExpressionId id, LongEnoughInt value, StaticType::Ptr type) {
    addExpression( id, LLVMConstInt(toLLVMType(type), value, true) );
}

void FunctionGenImpl::allocateStackVar(ExpressionId id, StaticType::Ptr type, String name) {
    addExpression( id, LLVMBuildAlloca(builder, toLLVMType(type), toCStr(name)) );
}

void FunctionGenImpl::assign( ExpressionId lvalue, ExpressionId rvalue ) {
    LLVMBuildStore(builder, lookupExpression(rvalue), lookupExpression(lvalue));
}

void FunctionGenImpl::dereferencePointer( ExpressionId id, StaticType::Ptr type, ExpressionId addr ) {
    addExpression( id, LLVMBuildLoad(builder, lookupExpression(addr), "") );
}

void FunctionGenImpl::truncateInteger(
        ExpressionId id, ExpressionId source, StaticType::Ptr sourceType, StaticType::Ptr destType )
{
    addExpression( id, LLVMBuildTrunc(builder, lookupExpression(source), toLLVMType(destType), "") );
}

void FunctionGenImpl::expandIntegerSigned(
        ExpressionId id, ExpressionId source, StaticType::Ptr sourceType, StaticType::Ptr destType )
{
    addExpression( id, LLVMBuildSExt(builder, lookupExpression(source), toLLVMType(destType), "") );
}

void FunctionGenImpl::expandIntegerUnsigned(
        ExpressionId id, ExpressionId source, StaticType::Ptr sourceType, StaticType::Ptr destType )
{
    addExpression( id, LLVMBuildZExt(builder, lookupExpression(source), toLLVMType(destType), "") );
}

void FunctionGenImpl::callFunctionDirect(
        ExpressionId id, String name, Slice<const ExpressionId> arguments, StaticType::Ptr returnType )
{
    LLVMValueRef functionRef = LLVMGetNamedFunction(module->getLLVMModule(), toCStr(name));
    std::vector<LLVMValueRef> llvmArguments;
    llvmArguments.reserve(arguments.size());
    for( const auto &argument: arguments ) {
        llvmArguments.emplace_back( lookupExpression(argument) );
    }
    addExpression( id, LLVMBuildCall(builder, functionRef, llvmArguments.data(), llvmArguments.size(), "") );
}

void FunctionGenImpl::binaryOperatorPlus(
            ExpressionId id, ExpressionId left, ExpressionId right, StaticType::Ptr resultType )
{
    addExpression( id, LLVMBuildAdd(builder, lookupExpression(left), lookupExpression(right), "") );
}

void FunctionGenImpl::binaryOperatorMinus(
            ExpressionId id, ExpressionId left, ExpressionId right, StaticType::Ptr resultType )
{
    addExpression( id, LLVMBuildSub(builder, lookupExpression(left), lookupExpression(right), "") );
}

LLVMValueRef FunctionGenImpl::lookupExpression(ExpressionId id) const {
    auto iter = expressionValuesTable.find(id);
    assert( iter!=expressionValuesTable.end() ); // Looked up an invalid id

    return iter->second;
}

void FunctionGenImpl::addExpression( ExpressionId id, LLVMValueRef value ) {
    assert( expressionValuesTable.find(id)==expressionValuesTable.end() ); // Adding an already existing expression
    expressionValuesTable[id] = value;
}

void ModuleGenImpl::moduleEnter(
        ModuleId id,
        String name,
        String file,
        size_t line,
        size_t col)
{
    llvmModule = LLVMModuleCreateWithName(nullptr);
    LLVMSetModuleIdentifier(llvmModule, name.get(), name.size());
    LLVMSetSourceFileName(llvmModule, file.get(), file.size());
}

void ModuleGenImpl::moduleLeave(ModuleId id) {
    char *error = NULL;
    LLVMVerifyModule(llvmModule, LLVMAbortProcessAction, &error);
    LLVMDisposeMessage(error);
}

std::shared_ptr<FunctionGen> ModuleGenImpl::handleFunction( IdentifierId id )
{
    return std::shared_ptr<FunctionGen>( new FunctionGenImpl(this) );
}
