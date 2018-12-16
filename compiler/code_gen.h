#ifndef CODE_GEN_H
#define CODE_GEN_H

#include <nocopy.h>
#include <practical-sa.h>

#include <llvm-c/Core.h>

using namespace PracticalSemanticAnalyzer;

class ModuleGenImpl;

class FunctionGenImpl : public FunctionGen, private NoCopy {
    ModuleGenImpl *module;

public:
    FunctionGenImpl(ModuleGenImpl *module) : module(module) {}

    virtual void functionEnter(
            IdentifierId id, String name, StaticType returnType, Slice<VariableDeclaration> arguments,
            String file, size_t line, size_t col) override;

    virtual void functionLeave(IdentifierId id) override;
};

class ModuleGenImpl : public ModuleGen, private NoCopy {
    LLVMModuleRef llvmModule = nullptr;
public:

    ~ModuleGenImpl() {
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
        LLVMDumpModule(llvmModule);
    }
};

#endif // CODE_GEN_H
