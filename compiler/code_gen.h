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
#include <practical-sa.h>

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
            IdentifierId id, String name, StaticType::Ptr returnType, Slice<const ArgumentDeclaration> arguments,
            String file, size_t line, size_t col) override;

    virtual void functionLeave(IdentifierId id) override;

    virtual void returnValue(ExpressionId id) override;

    virtual void branch(
            ExpressionId id, ExpressionId conditionExpression, JumpPointId elsePoint, JumpPointId continuationPoint
        ) override;
    virtual void setJumpPoint(JumpPointId id, String name) override;
    virtual void jump(JumpPointId destination) override;

    virtual void setLiteral(ExpressionId id, LongEnoughInt value, StaticType::Ptr type) override;
    virtual void setLiteral(ExpressionId id, bool value) override;

    virtual void allocateStackVar(ExpressionId id, StaticType::Ptr type, String name) override;
    virtual void assign( ExpressionId lvalue, ExpressionId rvalue ) override;
    virtual void dereferencePointer( ExpressionId id, StaticType::Ptr type, ExpressionId addr ) override;

    virtual void truncateInteger(
            ExpressionId id, ExpressionId source, StaticType::Ptr sourceType, StaticType::Ptr destType ) override;
    virtual void expandIntegerSigned(
            ExpressionId id, ExpressionId source, StaticType::Ptr sourceType, StaticType::Ptr destType ) override;
    virtual void expandIntegerUnsigned(
            ExpressionId id, ExpressionId source, StaticType::Ptr sourceType, StaticType::Ptr destType ) override;
    virtual void callFunctionDirect(
            ExpressionId id, String name, Slice<const ExpressionId> arguments, StaticType::Ptr returnType ) override;    

    virtual void binaryOperatorPlus(
            ExpressionId id, ExpressionId left, ExpressionId right, StaticType::Ptr resultType ) override;
    virtual void binaryOperatorMinus(
            ExpressionId id, ExpressionId left, ExpressionId right, StaticType::Ptr resultType ) override;
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

    virtual std::shared_ptr<FunctionGen> handleFunction( IdentifierId id ) override;

    void dump() {
        assert(llvmModule != nullptr);
        LLVMDumpModule(llvmModule);
    }
};

#endif // CODE_GEN_H
