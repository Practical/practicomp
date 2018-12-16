#include "config.h"

#include "code_gen.h"
#include "nocopy.h"
#include "support.h"

int main(int argc, char *argv[]) {
    if( argc<2 ) {
        emitMsg(MsgLevel::Error, PACKAGE_NAME, "no input files");
        exit(1);
    }

    auto arguments = allocateArguments();

    ModuleGenImpl codeGen;

    int ret = compile(argv[1], arguments.get(), &codeGen);

    if( ret==0 ) {
        codeGen.dump();
    }

    return ret;
}
