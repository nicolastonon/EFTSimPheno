# $< = name of first dependancy
# $@ = name of target ("main")
# -c asks the compiler not to link. Allows to get a compiled "object file" which does not contain
# -o specify target

#Define variables
CC=g++ -fPIC #Compiler used #fPIC useful to get debug messages
DEBUG = -g -Wl,--allow-multiple-definition -Wl,--whole-archive -Wl,--no-as-needed -Os #flags for TF linking (see https://github.com/tensorflow/tensorflow/issues/4242)
CFLAGS = -c #Compilation options
ROOTFLAGS = `root-config --glibs --cflags`

LFLAGS = $(DEBUG) -lTMVA -lPyMVA
LFLAGS += -L/home/ntonon/Documents/Programmes/tensorflow/bazel-bin/tensorflow -Wl,--whole-archive -Wl,--no-as-needed -ltensorflow_framework -lprotobuf

# LFLAGS += -L/home/ntonon/Documents/Programmes/tensorflow/bazel-bin/tensorflow -Wl,--whole-archive -Wl,--no-as-needed -ltensorflow_cc -Wl,--whole-archive -Wl,--no-as-needed -ltensorflow_framework
# LFLAGS += -L/home/ntonon/Documents/Programmes/tensorflow/bazel-bin/tensorflow -Wl,--whole-archive -Wl,--no-as-needed -ltensorflow_framework -Wl,--whole-archive -Wl,--no-as-needed -ltensorflow_all
# LFLAGS += -L/home/ntonon/Documents/Programmes/tensorflow/bazel-bin/tensorflow -Wl,--whole-archive -Wl,--no-as-needed -ltensorflow_framework -Wl,--whole-archive -Wl,--no-as-needed -ltensorflow_all -Wl,--whole-archive -Wl,--no-as-needed -ltensorflow_cc

INCFLAGS = -I./include
INCFLAGS += -I/home/ntonon/Documents/Programmes/tensorflow
INCFLAGS += -I/home/ntonon/Documents/Programmes/tensorflow/bazel-genfiles
INCFLAGS += -I/home/ntonon/Documents/Programmes/tensorflow/bazel-tensorflow/external/eigen_archive
INCFLAGS += -I/home/ntonon/Documents/Programmes/tensorflow/bazel-tensorflow/external/com_google_absl
INCFLAGS += -I/usr/local/include/google/protobuf
INCFLAGS += -I/usr/local/include/google/tensorflow

SRCS = $(wildcard *.cxx) #Source files are all files with .cxx extension
HDRS = $(wildcard *.h) #Header files are all files with .h extension
OBJS = $(SRCS:.cxx=.o) #Object files are all files with .o extension, which have same names as source files

MY_ANALYSIS = analysis_main.exe #Name of executable file
ROCS = ROCS/Compare_ROC_curves.exe
PRODUCE_CUTFLOW = Produce_Cutflow.exe

# .PHONY : $(wildcard *.o)  #Force to always recompile object

#Instructions
all: $(MY_ANALYSIS) $(ROCS) $(PRODUCE_CUTFLOW)

#Obtain executables from object files
$(MY_ANALYSIS): Utils/CMSSW_TensorFlow.o Utils/TFModel.o Utils/Helper.o analysis_main.o TopEFT_analysis.o
	@echo "###################################"
	@echo "-- Creating executable ./$(MY_ANALYSIS) --"
	@$(CC) Utils/CMSSW_TensorFlow.o Utils/TFModel.o Utils/Helper.o analysis_main.o TopEFT_analysis.o -o $@ $(ROOTFLAGS) $(LFLAGS) $(INCFLAGS)
	@echo "-- Done --"
	@echo "###################################""#"
	@echo ""

$(ROCS): ROCS/Compare_ROC_curves.o Utils/Helper.o
	@echo "###################################"
	@echo "-- Creating executable $(ROCS) --"
	@$(CC) ROCS/Compare_ROC_curves.o Utils/Helper.o -o $@ $(ROOTFLAGS) $(LFLAGS)
	@echo "-- Done --"
	@echo "###################################"
	@echo ""

$(PRODUCE_CUTFLOW):	Utils/Produce_Cutflow.o Utils/Helper.o
	@echo "###################################""#"
	@echo "-- Creating executable ./$(PRODUCE_CUTFLOW) --"
	@$(CC) Utils/Produce_Cutflow.o Utils/Helper.o -o $@ $(ROOTFLAGS) $(LFLAGS) $(INCFLAGS)
	@echo "-- Done --"
	@echo "###################################""#"
	@echo ""

#Obtain objects from source and header files
#WARNING : only works for .cxx files ! e.g. for .cpp, would need a specific rule
%.o: %.cxx $(HDRS)
	@echo "-- Compiling : "$@"--"
# @echo "-- INCFLAGS : "$(INCFLAGS)" --"
# @echo "-- LFLAGS : "$(LFLAGS)" --"
# @echo "-- ROOTFLAGS : "$(ROOTFLAGS)" --"
# @echo "-- CFLAGS : "$(CFLAGS)" --"
	@$(CC) $(ROOTFLAGS) $(CFLAGS) $(LFLAGS) $(INCFLAGS) $< -o $@
	@echo ""
# @Example :  g++ `root-config --glibs --cflags` -fPIC -c -g Utils/Helper.cxx -o Utils/Helper.o

#Erase all objects and executable
clean:
	@-rm -f *.o *.exe Utils/*.o ROCS/*.o ROCS/*.exe
