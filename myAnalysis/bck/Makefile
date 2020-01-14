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

LFLAGS = $(DEBUG) -L/home/ntonon/root/lib -lTMVA -lPyMVA #HARD-CODED : remove local path, can also remove lPyMva if not using NN
#LFLAGS = $(DEBUG) -lTMVA -lPyMVA

INCFLAGS = -I.
# INCFLAGS = -I./pocket-tensor/lib/include -I./pocket-tensor/3rd_party/libsimdpp #Add-on for reading NN in C++

SRCS = $(wildcard *.cxx) #Source files are all files with .cxx extension
HDRS = $(wildcard *.h) #Header files are all files with .h extension
OBJS = $(SRCS:.cxx=.o) #Object files are all files with .o extension, which have same names as source files

THQ_AN = analysis_main.exe #Name of executable file
Produce_Cutflow = Produce_Cutflow.exe
Create_subsample = Create_Subsample_fromSample.exe
Interface_MEM = Interface_ntuples_TMVA.exe
ROCS = ROCS/Compare_ROC_curves.exe

# .PHONY : $(wildcard *.o)  #Force to always recompile object


#Instructions
all: $(THQ_AN) $(Produce_Cutflow) $(Create_subsample) $(Interface_MEM) $(ROCS)

#Obtain executables from object files
$(THQ_AN): analysis_main.o Func_other.o tHq_analysis.o
	@echo "###################################""#"
	@echo "-- Creating executable ./$(THQ_AN) --"
	@$(CC) analysis_main.o Func_other.o tHq_analysis.o -o $@ $(ROOTFLAGS) $(LFLAGS) $(INCFLAGS)
	@echo "-- Done --"
	@echo "###################################""#"
	@echo ""

$(Produce_Cutflow):	Produce_Cutflow.cxx Func_other.o
	@echo "###################################""#"
	@echo "-- Creating executable ./$(Produce_Cutflow) --"
	@$(CC) Produce_Cutflow.cxx Func_other.o -o $@ $(ROOTFLAGS) $(LFLAGS) $(INCFLAGS)
	@echo "-- Done --"
	@echo "###################################""#"
	@echo ""

$(Create_subsample):	Create_Subsample_fromSample.cxx
	@echo "###################################""#"
	@echo "-- Creating executable ./$(Create_subsample) --"
	@$(CC) Create_Subsample_fromSample.cxx -o $@ $(ROOTFLAGS) $(LFLAGS) $(INCFLAGS)
	@echo "-- Done --"
	@echo "###################################""#"
	@echo ""


$(Interface_MEM):	MEM/Interface_ntuples_TMVA.o Func_other.o
	@echo "###################################""#"
	@echo "-- Creating executable ./MEM/$(Interface_MEM) --"
	@$(CC) MEM/Interface_ntuples_TMVA.o Func_other.o -o MEM/$@ $(ROOTFLAGS) $(LFLAGS) $(INCFLAGS)
	@echo "-- Done --"
	@echo "###################################""#"
	@echo ""


$(ROCS):	ROCS/Compare_ROC_curves.cxx
	@echo "###################################""#"
	@echo "-- Creating executable $(ROCS) --"
	@$(CC) ROCS/Compare_ROC_curves.cxx -o $@ $(ROOTFLAGS) $(LFLAGS)
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
	@-rm -f *.o *.exe ROCS/*.exe MEM/*.o
