#include "config.h"

#include <practical-sa.h>

#include "nocopy.h"
#include "support.h"

#include <llvm-c/Core.h>

using namespace PracticalSemanticAnalyzer;

int main(int argc, char *argv[]) {
    if( argc<2 ) {
        emitMsg(MsgLevel::Error, PACKAGE_NAME, "no input files");
        exit(1);
    }

    auto arguments = allocateArguments();

    class CodeGen : public PracticalSemanticAnalyzer::CodeGen, private NoCopy {
        LLVMModuleRef llvmModule = nullptr;
    public:

        ~CodeGen() {
            LLVMDisposeModule(llvmModule);
        }

        virtual void module(
                VisitMode mode,
                ModuleId id,
                String name,
                String file,
                size_t line,
                size_t col) override
        {
            switch( mode ) {
            case VisitMode::Enter:
                llvmModule = LLVMModuleCreateWithName(nullptr);
                LLVMSetModuleIdentifier(llvmModule, name.get(), name.size());
                LLVMSetSourceFileName(llvmModule, file.get(), file.size());
                break;
            case VisitMode::Exit:
                break;
            }
        }

        void dump() {
            LLVMDumpModule(llvmModule);
        }
    } codeGen;

    int ret = compile(argv[1], arguments.get(), &codeGen);

    if( ret==0 ) {
        codeGen.dump();
    }

    return ret;
}
