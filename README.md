# Practical: Compiler
This is the repository for the compiler (LLVM based) for the Practical programming language.

# What is Practical?
Practical is a new programming language designed to give the safety and features one would expect from a modern programming
language without sacrificing speed in any way. Practical aims to be as fast, if not faster, than C.

Practical takes its inspiration for design and syntax from C++, Rust and D, while trying not to fall into any of the pitfalls
that those languages encountered.

Practical is aimed for programmers in the gaming, storage, video and, eventually, embedded markets, where high throughput and low
latency are everything.

# Compiling the Copiler?

To compile Practicomp, you'll need LLVM version 7 (the C interface) installed on your system. You will also need a fairly up to
date C++ compiler.

If checking out from git (as if there is any other option at this point in time), make sure to `git submodule update --init` to get
the semantic analyzer library checked out under `external/practical-sa`. Since commits under the practical-sa projects don't always
update the practicomp project, updating the subproject to the latest master might be advisable.

# What's Planned?
For specifics on the plans for the language, syntax and features, please check out the
[language's wiki](https://github.com/Practical/practical-sa/wiki).

# Community and Tracking Progress
You can watch the [semantics analyzer](https://github.com/Practical/practical-sa) repository to get minute updates on minor
improvements to the language. Announcements on more significant milestones will be posted to the announcement forum at
https://forum.practical-pl.org. This is also the place to ask questions about the language and its details.
