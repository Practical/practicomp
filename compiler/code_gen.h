/* This file is part of the Practical programming langauge. https://github.com/Practical/practical-sa
 *
 * To the extent header files enjoy copyright protection, this file is file is copyright (C) 2018-2019 by its authors
 * You can see the file's authors in the AUTHORS file in the project's home repository.
 *
 * This is available under the Boost license. The license's text is available under the LICENSE file in the project's
 * home directory.
 */
#ifndef CODE_GEN_H
#define CODE_GEN_H

#include <nocopy.h>
#include <practical/practical.h>

#include <llvm-c/Core.h>

#include <deque>
#include <unordered_map>

using namespace PracticalSemanticAnalyzer;

class ModuleGenImpl;

class JumpPointData : NoCopy {
public:
    enum class Type { Label, Branch } type;

private:
    std::string label;
    bool defined = false;

public:
    explicit JumpPointData( Type type );
    explicit JumpPointData( const std::string &label ) : type(Type::Label), label( std::move(label) ) {}

    void definePoint() {
        assert( !defined );
        defined = true;
    }

    const std::string &getLabel() const {
        return label;
    }
};

struct BranchPointData {
    JumpPointId elsePointId, continuationPointId;
    LLVMBasicBlockRef elsePointBlock = nullptr, continuationPointBlock = nullptr;
    LLVMBasicBlockRef phiBlocks[2];
    ExpressionId conditionValue, ifBlockValue, elseBlockValue;
    StaticType::CPtr type;
};

class FunctionGenImpl : public FunctionGen, private NoCopy {
    ModuleGenImpl *module = nullptr;
    LLVMValueRef llvmFunction = nullptr;
    LLVMBasicBlockRef currentBlock = nullptr, nextBlock = nullptr;
    LLVMBuilderRef builder = nullptr;

    std::unordered_map< ExpressionId, LLVMValueRef > expressionValuesTable;
    std::unordered_map< JumpPointId, JumpPointData > jumpPointsTable;
    std::deque< BranchPointData > branchStack;

public:
    FunctionGenImpl(ModuleGenImpl *module) : module(module) {}
    virtual ~FunctionGenImpl();

    virtual void functionEnter(
            String name, StaticType::CPtr returnType, Slice<const ArgumentDeclaration> arguments,
            String file, const SourceLocation &location) override;

    virtual void functionLeave() override;

    virtual void returnValue(ExpressionId id) override;
    virtual void returnValue() override;

    virtual void conditionalBranch(
            ExpressionId id, StaticType::CPtr type, ExpressionId conditionExpression, JumpPointId elsePoint,
            JumpPointId continuationPoint
        ) override;
    virtual void setConditionClauseResult( ExpressionId id ) override;
    virtual void setJumpPoint(JumpPointId id, String name) override;
    virtual void jump(JumpPointId destination) override;

    virtual void setLiteral(ExpressionId id, LongEnoughInt value, StaticType::CPtr type) override;
    virtual void setLiteral(ExpressionId id, bool value) override;
    virtual void setLiteral(ExpressionId id, String value) override;
    virtual void setLiteralNull(ExpressionId id, StaticType::CPtr type) override;

    virtual void allocateStackVar(ExpressionId id, StaticType::CPtr type, String name) override;
    virtual void assign( ExpressionId lvalue, ExpressionId rvalue ) override;
    virtual void dereferencePointer( ExpressionId id, StaticType::CPtr type, ExpressionId addr ) override;

    virtual void truncateInteger(
            ExpressionId id, ExpressionId source, StaticType::CPtr sourceType, StaticType::CPtr destType ) override;
    virtual void changeIntegerSign(
            ExpressionId id, ExpressionId source, StaticType::CPtr sourceType, StaticType::CPtr destType ) override;
    virtual void expandIntegerSigned(
            ExpressionId id, ExpressionId source, StaticType::CPtr sourceType, StaticType::CPtr destType ) override;
    virtual void expandIntegerUnsigned(
            ExpressionId id, ExpressionId source, StaticType::CPtr sourceType, StaticType::CPtr destType ) override;
    virtual void callFunctionDirect(
            ExpressionId id, String name, Slice<const ExpressionId> arguments, StaticType::CPtr returnType ) override;

    virtual void binaryOperatorPlusUnsigned(
            ExpressionId id, ExpressionId left, ExpressionId right, StaticType::CPtr resultType ) override;
    virtual void binaryOperatorPlusSigned(
            ExpressionId id, ExpressionId left, ExpressionId right, StaticType::CPtr resultType ) override;
    virtual void binaryOperatorMinusUnsigned(
            ExpressionId id, ExpressionId left, ExpressionId right, StaticType::CPtr resultType ) override;
    virtual void binaryOperatorMinusSigned(
            ExpressionId id, ExpressionId left, ExpressionId right, StaticType::CPtr resultType ) override;
    virtual void binaryOperatorMultiplyUnsigned(
            ExpressionId id, ExpressionId left, ExpressionId right, StaticType::CPtr resultType ) override;
    virtual void binaryOperatorMultiplySigned(
            ExpressionId id, ExpressionId left, ExpressionId right, StaticType::CPtr resultType ) override;
    virtual void binaryOperatorDivideUnsigned(
            ExpressionId id, ExpressionId left, ExpressionId right, StaticType::CPtr resultType ) override;

    virtual void operatorEquals(
            ExpressionId id, ExpressionId left, ExpressionId right, StaticType::CPtr resultType ) override;
    virtual void operatorNotEquals(
            ExpressionId id, ExpressionId left, ExpressionId right, StaticType::CPtr resultType ) override;
    virtual void operatorLessThanUnsigned(
            ExpressionId id, ExpressionId left, ExpressionId right, StaticType::CPtr resultType ) override;
    virtual void operatorLessThanSigned(
            ExpressionId id, ExpressionId left, ExpressionId right, StaticType::CPtr resultType ) override;
    virtual void operatorLessThanOrEqualsUnsigned(
            ExpressionId id, ExpressionId left, ExpressionId right, StaticType::CPtr resultType ) override;
    virtual void operatorLessThanOrEqualsSigned(
            ExpressionId id, ExpressionId left, ExpressionId right, StaticType::CPtr resultType ) override;
    virtual void operatorGreaterThanUnsigned(
            ExpressionId id, ExpressionId left, ExpressionId right, StaticType::CPtr resultType ) override;
    virtual void operatorGreaterThanSigned(
            ExpressionId id, ExpressionId left, ExpressionId right, StaticType::CPtr resultType ) override;
    virtual void operatorGreaterThanOrEqualsUnsigned(
            ExpressionId id, ExpressionId left, ExpressionId right, StaticType::CPtr resultType ) override;
    virtual void operatorGreaterThanOrEqualsSigned(
            ExpressionId id, ExpressionId left, ExpressionId right, StaticType::CPtr resultType ) override;


    virtual void operatorLogicalNot( ExpressionId id, ExpressionId argument ) override;
private:
    LLVMValueRef lookupExpression( ExpressionId id ) const;
    void addExpression( ExpressionId id, LLVMValueRef value );

    LLVMBasicBlockRef addBlock( const std::string &label = "" );
    void setCurrentBlock( LLVMBasicBlockRef newCurrentBlock );
};

class ModuleGenImpl : public ModuleGen, private NoCopy {
    LLVMModuleRef llvmModule = nullptr;
public:

    virtual ~ModuleGenImpl() {
        LLVMDisposeModule(llvmModule);
    }

    LLVMModuleRef getLLVMModule() {
        return llvmModule;
    }

    virtual void moduleEnter(
            ModuleId id,
            String name,
            String file,
            size_t line,
            size_t col) override;

    virtual void moduleLeave(ModuleId id) override;

    virtual void declareIdentifier(String name, String mangledName, StaticType::CPtr type) override;
    virtual void declareStruct(StaticType::CPtr structType) override;
    virtual void defineStruct(StaticType::CPtr strct) override;

    virtual std::shared_ptr<FunctionGen> handleFunction() override;

    void dump();
};

#endif // CODE_GEN_H
