#ifndef CODE_GEN_H
#define CODE_GEN_H

#include <nocopy.h>
#include <practical-sa.h>

#include <llvm-c/Core.h>

#include <unordered_map>

using namespace PracticalSemanticAnalyzer;

class ModuleGenImpl;

class FunctionGenImpl : public FunctionGen, private NoCopy {
    ModuleGenImpl *module = nullptr;
    LLVMValueRef llvmFunction = nullptr;
    LLVMBasicBlockRef currentBlock = nullptr;
    LLVMBuilderRef builder = nullptr;

    std::unordered_map< ExpressionId, LLVMValueRef > expressionValuesTable;

public:
    FunctionGenImpl(ModuleGenImpl *module) : module(module) {}
    virtual ~FunctionGenImpl();

    virtual void functionEnter(
            IdentifierId id, String name, const StaticType &returnType, Slice<const ArgumentDeclaration> arguments,
            String file, size_t line, size_t col) override;

    virtual void functionLeave(IdentifierId id) override;

    virtual void returnValue(ExpressionId id) override;
    virtual void setLiteral(ExpressionId id, LongEnoughInt value, const StaticType &type) override;

    virtual void allocateStackVar(ExpressionId id, const StaticType &type, String name) override;
    virtual void assign( ExpressionId lvalue, ExpressionId rvalue ) override;
    virtual void dereferencePointer( ExpressionId id, const StaticType &type, ExpressionId addr ) override;

    virtual void truncateInteger(
            ExpressionId id, ExpressionId source, const StaticType &sourceType, const StaticType &destType ) override;
    virtual void expandIntegerSigned(
            ExpressionId id, ExpressionId source, const StaticType &sourceType, const StaticType &destType ) override;
    virtual void expandIntegerUnsigned(
            ExpressionId id, ExpressionId source, const StaticType &sourceType, const StaticType &destType ) override;
    void callFunctionDirect(
            ExpressionId id, String name, Slice<const ExpressionId> arguments, const StaticType &returnType ) override;    

private:
    LLVMValueRef lookupExpression( ExpressionId id ) const;
    void addExpression( ExpressionId id, LLVMValueRef value );
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
