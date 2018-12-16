#include "code_gen.h"

#include "utils.h"

void FunctionGenImpl::functionEnter(
        IdentifierId id, String name, StaticType returnType, Slice<VariableDeclaration> arguments,
        String file, size_t line, size_t col)
{
    auto llvmModule = module->getLLVMModule();

    std::vector<LLVMTypeRef> argumentTypes;
    LLVMTypeRef retType = LLVMFunctionType( LLVMVoidType() /*toLLVMType(returnType)*/, argumentTypes.data(), argumentTypes.size(), false );
    LLVMValueRef function = LLVMAddFunction( llvmModule, toStdString(name).c_str(), retType );
}

void FunctionGenImpl::functionLeave(IdentifierId id)
{
    auto llvmModule = module->getLLVMModule();

    // TODO implement
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
