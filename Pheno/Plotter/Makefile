# Makefile for BDT codes

# $< = name of first dependancy
# $@ = name of target ("main")
# -c ask the compiler not to link
# -o specify target
#

#Define variables
CC=g++ -fPIC #Compiler used
DEBUG = -g
CFLAGS = -c $(DEBUG) #Compilation options
CFLAGS += -std=c++14 #Only necessary to use frugally-deep add-on (to read NN trained with Keras in C++) -- Else, can disactivate
ROOTFLAGS = `root-config --glibs --cflags`

# LFLAGS = $(DEBUG) -L/home/ntonon/myroot/lib -lTMVA
LFLAGS = $(DEBUG) -lTMVA
LFLAGS+= -L/home/ntonon/Postdoc/EFTSimulations/Pheno/Plotter/Makefile

INCFLAGS = -I.

SRCS = $(wildcard *.cxx) #Source files are all files with .cxx extension
HDRS = $(wildcard *.h) #Header files are all files with .h extension
OBJS = $(SRCS:.cxx=.o) #Object files are all files with .o extension, which have same names as source files

LIB = /home/ntonon/Postdoc/EFTSimulations/myAnalysis/myLib.so #Re-use same library generated for main analysis code (to import custom classes)

MAIN = GenPlotter.exe #Name of executable file

# //--------------------------------------------
#Instructions
all: $(MAIN)

#Obtain executables from object files
$(MAIN): GenPlotter.cc $(LIB)
	@echo "###################################""#"
	@echo "-- Creating executable ./$(MAIN) --"
	@$(CC) GenPlotter.cc -o $@ $(ROOTFLAGS) $(LFLAGS) $(INCFLAGS) $(LIB)
	@echo "-- Done --"
	@echo "###################################""#"
	@echo ""

#Obtain objects from source and header files
%.o: %.cxx $(HDRS)
	@echo "-- Compiling : "$@"--"
	@$(CC) $(ROOTFLAGS) $(CFLAGS) $(LFLAGS) $(INCFLAGS) $< -o $@
	@#Example :  g++ `root-config --glibs --cflags` -fPIC -c -g Func_other.cxx -o Func_other.o


#Erase all objects and executable
clean:
	@-rm $(MAIN)
