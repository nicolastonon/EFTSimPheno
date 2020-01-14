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

LFLAGS = $(DEBUG) -lTMVA -lPyMVA

INCFLAGS = -I.
# INCFLAGS = -I./pocket-tensor/lib/include -I./pocket-tensor/3rd_party/libsimdpp #Add-on for reading NN in C++

SRCS = $(wildcard *.cxx) #Source files are all files with .cxx extension
HDRS = $(wildcard *.h) #Header files are all files with .h extension
OBJS = $(SRCS:.cxx=.o) #Object files are all files with .o extension, which have same names as source files

MY_ANALYSIS = analysis_main.exe #Name of executable file

# .PHONY : $(wildcard *.o)  #Force to always recompile object

#Instructions
all: $(MY_ANALYSIS)

#Obtain executables from object files
$(MY_ANALYSIS): analysis_main.o Helper.o TopEFT_analysis.o
	@echo "###################################""#"
	@echo "-- Creating executable ./$(MY_ANALYSIS) --"
	@$(CC) analysis_main.o Helper.o TopEFT_analysis.o -o $@ $(ROOTFLAGS) $(LFLAGS) $(INCFLAGS)
	@echo "-- Done --"
	@echo "###################################""#"
	@echo ""

#Obtain objects from source and header files
%.o: %.cxx $(HDRS)
	@echo "-- Compiling : "$@"--"
	@$(CC) $(ROOTFLAGS) $(CFLAGS) $(LFLAGS) $(INCFLAGS) $< -o $@
	@#Example :  g++ `root-config --glibs --cflags` -fPIC -c -g Helper.cxx -o Helper.o

#Erase all objects and executable
clean:
	@-rm -f *.o *.exe ROCS/*.exe MEM/*.o
