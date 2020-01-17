# Makefile for COMBINE codes

# $< = name of first dependancy
# $@ = name of target ("main")
# -c ask the compiler not to link
# -o specify target
#

#Define variables
CC=g++ #Compiler used
DEBUG = -g
CFLAGS = -c  $(DEBUG) #Compilation options
LFLAGS = $(DEBUG) -lTMVA
ROOTFLAGS = `root-config --glibs --cflags`

SCRIPT_PRODUCER = ScriptProducer_GenAllDatacards.exe
GENERATE_DATACARDS = Generate_Datacard_Template.exe

#Instructions
all: $(GENERATE_DATACARDS) $(SCRIPT_PRODUCER)

$(GENERATE_DATACARDS):	Generate_Datacard_Template.cxx
	@g++ $< -o $@ $(ROOTFLAGS) $(LFLAGS)
	@echo "-- Created executable ./$(GENERATE_DATACARDS) --"

$(SCRIPT_PRODUCER):	ScriptProducer_GenAllDatacards.cxx
	@g++ $< -o $@ $(ROOTFLAGS) $(LFLAGS)
	@echo "-- Created executable ./$(SCRIPT_PRODUCER) --"

#Erase all objects and executable
clean:
	rm $(GENERATE_DATACARDS) $(SCRIPT_PRODUCER) *.sh
