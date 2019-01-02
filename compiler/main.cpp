#include "config.h"

#include "code_gen.h"
#include "nocopy.h"
#include "object_output.h"
#include "support.h"

#include <filesystem>

int main(int argc, char *argv[]) {
    if( argc<2 ) {
        emitMsg(MsgLevel::Error, PACKAGE_NAME, "no input files");
        exit(1);
    }

    auto arguments = allocateArguments();

    ModuleGenImpl codeGen;

    std::filesystem::path inputFilePath(argv[1]);
    if( inputFilePath.extension() != PRACTICAL_SOURCE_FILE_EXTENSION ) {
        emitMsg(MsgLevel::Error, PACKAGE_NAME, "Expected a source file with " PRACTICAL_SOURCE_FILE_EXTENSION " extension");
        exit(1);
    }

    int ret = compile(argv[1], arguments.get(), &codeGen);
    if( ret!=0 )
        return ret;

    codeGen.dump();

    auto outputFileName = inputFilePath.filename();
    outputFileName.replace_extension(".o");

    ObjectOutput output(outputFileName, TARGET_TRIPLET, codeGen);
    return ret;
}
