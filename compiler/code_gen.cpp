#include "code_gen.h"

#include "utils.h"

FunctionGenImpl::~FunctionGenImpl() {
    assert(llvmFunction==nullptr);
    assert(currentBlock==nullptr);
    assert(builder==nullptr);
}

void FunctionGenImpl::functionEnter(
        IdentifierId id, String name, StaticType returnType, Slice<VariableDeclaration> arguments,
        String file, size_t line, size_t col)
{
    auto llvmModule = module->getLLVMModule();

    std::vector<LLVMTypeRef> argumentTypes;
    LLVMTypeRef retType = LLVMFunctionType( LLVMVoidType() /*toLLVMType(returnType)*/, argumentTypes.data(), argumentTypes.size(), false );
    llvmFunction = LLVMAddFunction( llvmModule, toStdString(name).c_str(), retType );
    currentBlock = LLVMAppendBasicBlock( llvmFunction, "" );

    builder = LLVMCreateBuilder();
    LLVMPositionBuilderAtEnd(builder, currentBlock);
}

void FunctionGenImpl::functionLeave(IdentifierId id)
{
    auto llvmModule = module->getLLVMModule();

    LLVMDisposeBuilder(builder);
    builder = nullptr;
    currentBlock = nullptr;
    llvmFunction = nullptr;
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
}

std::shared_ptr<FunctionGen> ModuleGenImpl::handleFunction( IdentifierId id )
{
    return std::shared_ptr<FunctionGen>( new FunctionGenImpl(this) );
}
