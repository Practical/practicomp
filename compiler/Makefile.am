bin_PROGRAMS = practicomp
noinst_PROGRAMS = practinop

CPPFLAGS += -I$(top_srcdir)/external/practical-sa/include $(LLVM_CPPFLAGS)
CXXFLAGS += $(LLVM_CXXFLAGS) -fexceptions
LDFLAGS += -L$(top_builddir)/external/practical-sa/lib/ $(LLVM_LDFLAGS)
LIBS += -lpractical-sa $(LLVM_LIBS) -lstdc++fs

practicomp_SOURCES = main.cpp support.cpp code_gen.cpp object_output.cpp lookup_context.cpp

practinop_SOURCES = main.cpp support.cpp dummy_code_gen.cpp lookup_context.cpp
