bin_PROGRAMS = practicomp

practicomp_SOURCES = main.cpp support.cpp
practicomp_CPPFLAGS = -I$(top_srcdir)/external/practical-sa/include
practicomp_CXXFLAGS = $(LLVM_CXXFLAGS)
practicomp_LDFLAGS = -L$(top_builddir)/external/practical-sa/lib/ $(LLVM_LDFLAGS)
practicomp_LDADD = -lpractical-sa $(LLVM_LIBS)
