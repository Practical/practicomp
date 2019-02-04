#include "code_gen.h"

#include "utils.h"

#include <llvm-c/Analysis.h>

FunctionGenImpl::~FunctionGenImpl() {
    assert(llvmFunction==nullptr);
    assert(currentBlock==nullptr);
    assert(builder==nullptr);
}

static LLVMTypeRef toLLVMTypeBuiltin(const NamedType::BuiltIn *builtInType) {
    LLVMTypeRef ret;

    switch(builtInType->type) {
    case NamedType::BuiltIn::Type::Invalid:
        abort();
        break;
    case NamedType::BuiltIn::Type::Void:
        ret = LLVMVoidType();
        break;
    case NamedType::BuiltIn::Type::Boolean:
        ret = LLVMInt8Type();
        break;
    case NamedType::BuiltIn::Type::SignedInt:
    case NamedType::BuiltIn::Type::UnsignedInt:
    case NamedType::BuiltIn::Type::InternalUnsignedInt:
        ret = LLVMIntType(builtInType->sizeInBits);
        break;
    }

    return ret;
}

static LLVMTypeRef toLLVMType(const StaticType &practiType) {
    auto meaning = getTypeMeaning( practiType.getId() );

    LLVMTypeRef ret;

    switch(meaning.index()) {
    case TypeMeanings::BuiltIn:
        ret = toLLVMTypeBuiltin( std::get<const NamedType::BuiltIn *>(meaning) );
        break;
    default:
        abort(); // Unexpected result
    }

    return ret;
}

void FunctionGenImpl::functionEnter(
        IdentifierId id, String name, const StaticType &returnType, Slice<VariableDeclaration> arguments,
        String file, size_t line, size_t col)
{
    auto llvmModule = module->getLLVMModule();

    std::vector<LLVMTypeRef> argumentTypes;
    LLVMTypeRef retType = LLVMFunctionType( toLLVMType(returnType), argumentTypes.data(), argumentTypes.size(), false );
    llvmFunction = LLVMAddFunction( llvmModule, toStdString(name).c_str(), retType );
    currentBlock = LLVMAppendBasicBlock( llvmFunction, "" );

    builder = LLVMCreateBuilder();
    LLVMPositionBuilderAtEnd(builder, currentBlock);
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

void FunctionGenImpl::setLiteral(ExpressionId id, LongEnoughInt value, const StaticType &type) {
    addExpression( id, LLVMConstInt(toLLVMType(type), value, true) );
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
