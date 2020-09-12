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

static LLVMTypeRef toLLVMType(StaticType::CPtr practiType) {
    struct Visitor {
        LLVMTypeRef operator()( const PracticalSemanticAnalyzer::StaticType::Scalar *scalar ) {
            return static_cast<LLVMTypeRef>( scalar->getTypeId().p );
        }
        LLVMTypeRef operator()( const PracticalSemanticAnalyzer::StaticType::Function *function ) {
            abort();
        }
    };

    return std::visit( Visitor(), practiType->getType() );
}

void FunctionGenImpl::functionEnter(
        String name, StaticType::CPtr returnType, Slice<const ArgumentDeclaration> arguments,
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

void FunctionGenImpl::functionLeave()
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
        ExpressionId id, StaticType::CPtr type, ExpressionId conditionExpression, JumpPointId elsePoint,
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

void FunctionGenImpl::setLiteral(ExpressionId id, LongEnoughInt value, StaticType::CPtr type) {
    addExpression( id, LLVMConstInt(toLLVMType(type), value, true) );
}

void FunctionGenImpl::setLiteral(ExpressionId id, bool value) {
    addExpression( id, LLVMConstInt(LLVMInt1Type(), value, true) );
}

void FunctionGenImpl::allocateStackVar(ExpressionId id, StaticType::CPtr type, String name) {
    addExpression( id, LLVMBuildAlloca(builder, toLLVMType(type), toCStr(name)) );
}

void FunctionGenImpl::assign( ExpressionId lvalue, ExpressionId rvalue ) {
    LLVMBuildStore(builder, lookupExpression(rvalue), lookupExpression(lvalue));
}

void FunctionGenImpl::dereferencePointer( ExpressionId id, StaticType::CPtr type, ExpressionId addr ) {
    addExpression( id, LLVMBuildLoad(builder, lookupExpression(addr), "") );
}

void FunctionGenImpl::truncateInteger(
        ExpressionId id, ExpressionId source, StaticType::CPtr sourceType, StaticType::CPtr destType )
{
    addExpression( id, LLVMBuildTrunc(builder, lookupExpression(source), toLLVMType(destType), "") );
}

void FunctionGenImpl::changeIntegerSign(
        ExpressionId id, ExpressionId source, StaticType::CPtr sourceType, StaticType::CPtr destType )
{
    addExpression( id, lookupExpression(source) );
}

void FunctionGenImpl::expandIntegerSigned(
        ExpressionId id, ExpressionId source, StaticType::CPtr sourceType, StaticType::CPtr destType )
{
    addExpression( id, LLVMBuildSExt(builder, lookupExpression(source), toLLVMType(destType), "") );
}

void FunctionGenImpl::expandIntegerUnsigned(
        ExpressionId id, ExpressionId source, StaticType::CPtr sourceType, StaticType::CPtr destType )
{
    addExpression( id, LLVMBuildZExt(builder, lookupExpression(source), toLLVMType(destType), "") );
}

void FunctionGenImpl::callFunctionDirect(
        ExpressionId id, String name, Slice<const ExpressionId> arguments, StaticType::CPtr returnType )
{
    LLVMValueRef functionRef = LLVMGetNamedFunction(module->getLLVMModule(), toCStr(name));
    std::vector<LLVMValueRef> llvmArguments;
    llvmArguments.reserve(arguments.size());
    for( const auto &argument: arguments ) {
        llvmArguments.emplace_back( lookupExpression(argument) );
    }
    addExpression( id, LLVMBuildCall(builder, functionRef, llvmArguments.data(), llvmArguments.size(), "") );
}

void FunctionGenImpl::binaryOperatorPlusUnsigned(
            ExpressionId id, ExpressionId left, ExpressionId right, StaticType::CPtr resultType )
{
    addExpression( id, LLVMBuildAdd(builder, lookupExpression(left), lookupExpression(right), "") );
}

void FunctionGenImpl::binaryOperatorPlusSigned(
            ExpressionId id, ExpressionId left, ExpressionId right, StaticType::CPtr resultType )
{
    addExpression( id, LLVMBuildNSWAdd(builder, lookupExpression(left), lookupExpression(right), "") );
}

void FunctionGenImpl::binaryOperatorMinusUnsigned(
            ExpressionId id, ExpressionId left, ExpressionId right, StaticType::CPtr resultType )
{
    addExpression( id, LLVMBuildSub(builder, lookupExpression(left), lookupExpression(right), "") );
}

void FunctionGenImpl::binaryOperatorMinusSigned(
            ExpressionId id, ExpressionId left, ExpressionId right, StaticType::CPtr resultType )
{
    addExpression( id, LLVMBuildNSWSub(builder, lookupExpression(left), lookupExpression(right), "") );
}

void FunctionGenImpl::binaryOperatorMultiplyUnsigned(
            ExpressionId id, ExpressionId left, ExpressionId right, StaticType::CPtr resultType )
{
    addExpression( id, LLVMBuildMul(builder, lookupExpression(left), lookupExpression(right), "") );
}

void FunctionGenImpl::binaryOperatorMultiplySigned(
            ExpressionId id, ExpressionId left, ExpressionId right, StaticType::CPtr resultType )
{
    addExpression( id, LLVMBuildNSWMul(builder, lookupExpression(left), lookupExpression(right), "") );
}

void FunctionGenImpl::binaryOperatorDivideUnsigned(
            ExpressionId id, ExpressionId left, ExpressionId right, StaticType::CPtr resultType )
{
    addExpression( id, LLVMBuildUDiv(builder, lookupExpression(left), lookupExpression(right), "") );
}

void FunctionGenImpl::operatorEquals(
            ExpressionId id, ExpressionId left, ExpressionId right, StaticType::CPtr resultType )
{
    addExpression( id, LLVMBuildICmp(builder, LLVMIntEQ, lookupExpression(left), lookupExpression(right), "") );
}

void FunctionGenImpl::operatorNotEquals(
            ExpressionId id, ExpressionId left, ExpressionId right, StaticType::CPtr resultType )
{
    addExpression( id, LLVMBuildICmp(builder, LLVMIntNE, lookupExpression(left), lookupExpression(right), "") );
}

void FunctionGenImpl::operatorLessThanUnsigned(
            ExpressionId id, ExpressionId left, ExpressionId right, StaticType::CPtr resultType )
{
    addExpression( id, LLVMBuildICmp(builder, LLVMIntULT, lookupExpression(left), lookupExpression(right), "") );
}

void FunctionGenImpl::operatorLessThanSigned(
            ExpressionId id, ExpressionId left, ExpressionId right, StaticType::CPtr resultType )
{
    addExpression( id, LLVMBuildICmp(builder, LLVMIntSLT, lookupExpression(left), lookupExpression(right), "") );
}

void FunctionGenImpl::operatorLessThanOrEqualsUnsigned(
            ExpressionId id, ExpressionId left, ExpressionId right, StaticType::CPtr resultType )
{
    addExpression( id, LLVMBuildICmp(builder, LLVMIntULE, lookupExpression(left), lookupExpression(right), "") );
}

void FunctionGenImpl::operatorLessThanOrEqualsSigned(
            ExpressionId id, ExpressionId left, ExpressionId right, StaticType::CPtr resultType )
{
    addExpression( id, LLVMBuildICmp(builder, LLVMIntSLE, lookupExpression(left), lookupExpression(right), "") );
}

void FunctionGenImpl::operatorGreaterThanUnsigned(
            ExpressionId id, ExpressionId left, ExpressionId right, StaticType::CPtr resultType )
{
    addExpression( id, LLVMBuildICmp(builder, LLVMIntUGT, lookupExpression(left), lookupExpression(right), "") );
}

void FunctionGenImpl::operatorGreaterThanSigned(
            ExpressionId id, ExpressionId left, ExpressionId right, StaticType::CPtr resultType )
{
    addExpression( id, LLVMBuildICmp(builder, LLVMIntSGT, lookupExpression(left), lookupExpression(right), "") );
}

void FunctionGenImpl::operatorGreaterThanOrEqualsUnsigned(
            ExpressionId id, ExpressionId left, ExpressionId right, StaticType::CPtr resultType )
{
    addExpression( id, LLVMBuildICmp(builder, LLVMIntUGE, lookupExpression(left), lookupExpression(right), "") );
}

void FunctionGenImpl::operatorGreaterThanOrEqualsSigned(
            ExpressionId id, ExpressionId left, ExpressionId right, StaticType::CPtr resultType )
{
    addExpression( id, LLVMBuildICmp(builder, LLVMIntSGE, lookupExpression(left), lookupExpression(right), "") );
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

std::shared_ptr<FunctionGen> ModuleGenImpl::handleFunction()
{
    return std::shared_ptr<FunctionGen>( new FunctionGenImpl(this) );
}

void ModuleGenImpl::dump() {
    assert(llvmModule != nullptr);
    LLVMDumpModule(llvmModule);
}
