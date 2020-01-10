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

#include <sstream>

JumpPointData::JumpPointData( Type type ) : type(type) {
}

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
    case NamedType::Type::Type:
        abort(); // "Asked to code-gen compile-time only type Type";
        break;
    case NamedType::Type::Boolean:
        ret = LLVMInt1Type();
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

    builder = LLVMCreateBuilder();
    addBlock();

    // Allocate stack location for the arguments, so that they behave like lvalues
    for( size_t i = 0; i<arguments.size(); ++i ) {
        allocateStackVar( arguments[i].lvalueId, arguments[i].type, arguments[i].name );
        LLVMBuildStore(builder, LLVMGetParam( llvmFunction, i ), lookupExpression(arguments[i].lvalueId));
    }
}

void FunctionGenImpl::functionLeave(IdentifierId id)
{
    // XXX Need to support "return" statement from middle of function definition
    LLVMDisposeBuilder(builder);
    builder = nullptr;
    currentBlock = nullptr;
    llvmFunction = nullptr;
}

void FunctionGenImpl::returnValue(ExpressionId id) {
    LLVMBuildRet( builder, lookupExpression(id) );
}

void FunctionGenImpl::returnValue() {
    LLVMBuildRetVoid( builder );
}

void FunctionGenImpl::conditionalBranch(
        ExpressionId id, StaticType::Ptr type, ExpressionId conditionExpression, JumpPointId elsePoint,
        JumpPointId continuationPoint)
{
    LLVMBasicBlockRef previousCurrent = currentBlock;

    auto ifBlock = addBlock();
    BranchPointData &branchData = branchStack.emplace_back();
    branchData.conditionValue = id;
    branchData.type = type;
    branchData.phiBlocks[0] = currentBlock;

    LLVMBasicBlockRef nextBlockInFlow = nullptr;
    if( elsePoint!=JumpPointId() ) {
        branchData.elsePointId = elsePoint;
        auto jumpData = jumpPointsTable.emplace(
                std::piecewise_construct,
                std::forward_as_tuple( elsePoint ),
                std::forward_as_tuple( JumpPointData::Type::Branch )
        );
        assert( jumpData.second );
        nextBlockInFlow = branchData.phiBlocks[1] = branchData.elsePointBlock = addBlock( jumpData.first->second.getLabel() );
        jumpData.first->second.definePoint();
    } else
        assert( id==ExpressionId() );

    branchData.continuationPointId = continuationPoint;
    auto jumpData = jumpPointsTable.emplace(
            std::piecewise_construct,
            std::forward_as_tuple( continuationPoint ),
            std::forward_as_tuple( JumpPointData::Type::Branch )
    );
    assert( jumpData.second );
    branchData.continuationPointBlock = addBlock( jumpData.first->second.getLabel() );
    jumpData.first->second.definePoint();

    if( !nextBlockInFlow )
        nextBlockInFlow = branchData.continuationPointBlock;

    setCurrentBlock( previousCurrent );
    LLVMBuildCondBr( builder, lookupExpression(conditionExpression), ifBlock, nextBlockInFlow );

    setCurrentBlock( ifBlock );
}

void FunctionGenImpl::setConditionClauseResult( ExpressionId id ) {
    assert( ! branchStack.empty() );

    BranchPointData &stackTop = branchStack.back();
    assert( stackTop.conditionValue!=ExpressionId() );

    if( stackTop.elsePointId!=JumpPointId() ) {
        // Still in the "if" clause
        assert( stackTop.ifBlockValue==ExpressionId() );
        stackTop.ifBlockValue=id;
    } else {
        assert( stackTop.ifBlockValue!=ExpressionId() );
        assert( stackTop.elseBlockValue==ExpressionId() );
        stackTop.elseBlockValue=id;
    }
}

void FunctionGenImpl::setJumpPoint(JumpPointId id, String name) {
    if( ! branchStack.empty() ) {
        // We are inside a nested context. We need to see whether id is trying to tell us this context is done
        auto &stackTop = branchStack.back();

        if( stackTop.elsePointId!=JumpPointId() ) {
            if( id==stackTop.elsePointId ) {
                // We just finished the "if" clause, need to start the "else" clause
                LLVMBuildBr( builder, stackTop.continuationPointBlock );
                setCurrentBlock( stackTop.elsePointBlock );
                stackTop.elsePointId = JumpPointId();

                return;
            }
        } else {
            if( id==stackTop.continuationPointId ) {
                // We just finished the "else" clause (or an elseless "if" clause")
                LLVMBuildBr( builder, stackTop.continuationPointBlock );
                setCurrentBlock( stackTop.continuationPointBlock );

                if( stackTop.conditionValue!=ExpressionId() ) {
                    // We need to set a value to the condition
                    assert( stackTop.ifBlockValue!=ExpressionId() );
                    assert( stackTop.elseBlockValue!=ExpressionId() );

                    LLVMValueRef phiValue = LLVMBuildPhi(builder, toLLVMType(stackTop.type), "");
                    addExpression( stackTop.conditionValue, phiValue );
                    LLVMValueRef values[2] = {
                        lookupExpression(stackTop.ifBlockValue),
                        lookupExpression(stackTop.elseBlockValue)
                    };
                    LLVMAddIncoming( phiValue, values, stackTop.phiBlocks, 2 );
                }

                branchStack.pop_back();

                return;
            }
        }
    }

    abort(); // XXX setJumpPoint not as part of a branch not yet implemented
    // LLVMBuildBr( builder, dest );
}

void FunctionGenImpl::jump(JumpPointId destination) {
    // TODO implement
    abort();
}

void FunctionGenImpl::setLiteral(ExpressionId id, LongEnoughInt value, StaticType::Ptr type) {
    addExpression( id, LLVMConstInt(toLLVMType(type), value, true) );
}

void FunctionGenImpl::setLiteral(ExpressionId id, bool value) {
    addExpression( id, LLVMConstInt(LLVMInt1Type(), value, true) );
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

LLVMBasicBlockRef FunctionGenImpl::addBlock( const std::string &label ) {
    LLVMBasicBlockRef ret = nullptr;
    if( nextBlock==nullptr )
        ret = LLVMAppendBasicBlock( llvmFunction, label.c_str() );
    else
        ret = LLVMInsertBasicBlock( nextBlock, label.c_str() );

    setCurrentBlock( ret );

    return ret;
}

void FunctionGenImpl::setCurrentBlock( LLVMBasicBlockRef newCurrentBlock ) {
    currentBlock = newCurrentBlock;
    nextBlock = LLVMGetNextBasicBlock( currentBlock );
    LLVMPositionBuilderAtEnd(builder, currentBlock);
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
