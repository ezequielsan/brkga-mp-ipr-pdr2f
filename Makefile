###############################################################################
# Makefile: build script for the PDR2F using BRKGA-MP-IPR.
#
# Adaptado de examples/tsp/src/single_obj/Makefile.
###############################################################################

# Set OPT = opt for optimized builds, anything else for debug builds.
OPT = opt

# Mating: MATING_SEED_ONLY torna o resultado dependente apenas da semente
# (e nao do numero de threads), o que e' o desejavel nos experimentos.
USER_FLAGS += -DMATING_SEED_ONLY

# Caminhos dos cabecalhos. Se a API for um submodulo git, troque por:
INCLUDES = \
	-I. \
	-I./brkga_mp_ipr

OBJS = \
	./pdr2f/pdr2f_instance.o \
	./decoders/pdr2f_decoder.o

MAIN_MINIMAL_OBJ = main_minimal.o
MAIN_MINIMAL_EXE = main_minimal

CXX = g++

USER_FLAGS += -std=c++20

ifneq ($(OPT), opt)
	USER_FLAGS += -ggdb3 -fexceptions -fno-omit-frame-pointer \
		-fno-optimize-sibling-calls -fno-inline
else
	USER_FLAGS += -O3 -fomit-frame-pointer -funroll-loops
	ifeq ($(CXX), g++)
		USER_FLAGS += -ftracer -fpeel-loops -fprefetch-loop-arrays -flto=auto
	endif
endif

USER_FLAGS += -pthread -fopenmp

USER_FLAGS += -Wall -Wextra -Wcast-align -Wcast-qual -Wdisabled-optimization \
	-Wformat=2 -Winit-self -Wmissing-format-attribute -Wshadow \
	-Wpointer-arith -Wredundant-decls -Wstrict-aliasing=2 \
	-Wfloat-equal -Weffc++

CXXFLAGS = $(USER_FLAGS)

.PHONY: all clean
.SUFFIXES: .cpp .o

all: main_minimal

main_minimal: $(OBJS) $(MAIN_MINIMAL_OBJ)
	@echo "--> Linking objects... "
	$(CXX) $(CXXFLAGS) $(OBJS) $(MAIN_MINIMAL_OBJ) -o $(MAIN_MINIMAL_EXE)
	@echo

.cpp.o:
	@echo "--> Compiling $<..."
	$(CXX) $(CXXFLAGS) $(INCLUDES) $(USER_DEFINES) -c $< -o $@
	@echo

clean:
	@echo "--> Cleaning compiled..."
	rm -rf $(OBJS) $(MAIN_MINIMAL_OBJ) $(MAIN_MINIMAL_EXE)
	rm -rf *.o
	rm -rf *.dSYM