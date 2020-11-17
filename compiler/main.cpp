/* This file is part of the Practical programming langauge. https://github.com/Practical/practical-sa
 *
 * This file is file is copyright (C) 2018-2020 by its authors.
 * You can see the file's authors in the AUTHORS file in the project's home repository.
 *
 * This is available under the Boost license. The license's text is available under the LICENSE file in the project's
 * home directory.
 */
#include "config.h"

#include "code_gen.h"
#include "lookup_context.h"
#include "nocopy.h"
#include "object_output.h"
#include "support.h"

#include <practical/errors.h>

#include <sys/types.h>
#include <execinfo.h>
#include <filesystem>
#include <unistd.h>
#include <signal.h>

const char *signalToStr(int signum) {
#define NAME(sig) case sig: return #sig
    switch(signum) {
        NAME(SIGSEGV);
        NAME(SIGABRT);
        NAME(SIGUSR1);
        NAME(SIGUSR2);
        NAME(SIGINT);
        NAME(SIGILL);
        NAME(SIGFPE);
        NAME(SIGTERM);
        NAME(SIGHUP);
        NAME(SIGQUIT);
        NAME(SIGTRAP);
        NAME(SIGKILL);
        NAME(SIGBUS);
        NAME(SIGSYS);
        NAME(SIGPIPE);
        NAME(SIGALRM);
        NAME(SIGURG);
        NAME(SIGSTOP);
        NAME(SIGCHLD);
    }

    return "Unknown signal";
}

void abortHandler(int signum) {
    std::cerr<<"Compiler ABORTed on signal " << signalToStr(signum) << std::endl;

    void *bt[50];
    int btDepth = backtrace( bt, 50 );
    backtrace_symbols_fd( bt, btDepth, 2 );

    signal(signum, SIG_DFL);
    kill( getpid(), signum );
}

int main(int argc, char *argv[]) {
    if( argc<2 ) {
        emitMsg(MsgLevel::Error, PACKAGE_NAME, "no input files");
        exit(1);
    }

    signal(SIGABRT, abortHandler);
    signal(SIGSEGV, abortHandler);

    auto arguments = allocateArguments();

    ModuleGenImpl codeGen;

    std::filesystem::path inputFilePath(argv[1]);
    if( inputFilePath.extension() != PRACTICAL_SOURCE_FILE_EXTENSION ) {
        emitMsg(MsgLevel::Error, PACKAGE_NAME, "Expected a source file with " PRACTICAL_SOURCE_FILE_EXTENSION " extension");
        exit(1);
    }

    try {
        ::BuiltinContextGen builtinGen;
        PracticalSemanticAnalyzer::prepare( &builtinGen );
        int ret = compile(argv[1], arguments.get(), &codeGen);
        if( ret!=0 )
            return ret;
    } catch(const compile_error &err) {
        std::cerr<<inputFilePath.c_str()<<":"<<err.getLine()<<":"<<err.getCol()<<": error: "<<err.what()<<"\n";

        return 1;
    }

    codeGen.dump();

    auto outputFileName = inputFilePath.filename();
    outputFileName.replace_extension(".o");

    ObjectOutput output(outputFileName, TARGET_TRIPLET, codeGen);
    return 0;
}
