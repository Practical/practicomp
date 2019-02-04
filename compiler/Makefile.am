bin_PROGRAMS = practicomp

practicomp_SOURCES = main.cpp support.cpp code_gen.cpp object_output.cpp
practicomp_CPPFLAGS = -I$(top_srcdir)/external/practical-sa/include
practicomp_CXXFLAGS = $(LLVM_CXXFLAGS) -fexceptions
practicomp_LDFLAGS = -L$(top_builddir)/external/practical-sa/lib/ $(LLVM_LDFLAGS)
practicomp_LDADD = -lpractical-sa $(LLVM_LIBS) -lstdc++fs
