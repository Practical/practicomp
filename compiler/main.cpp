#include "config.h"

#include <practical-sa.h>

#include "support.h"

int main(int argc, char *argv[]) {
    if( argc<2 ) {
        emitMsg(MsgLevel::Error, PACKAGE_NAME, "no input files");
        exit(1);
    }

    auto arguments = PracticalSemanticAnalyzer::allocateArguments();

    return compile(argv[1], arguments.get());
}
