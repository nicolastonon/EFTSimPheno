#Usage : 'python Parser_Datacard_Template.py [channel] [variable] [root file] ...'

import fileinput
import os
import sys, getopt
import string
import re
import os

# //--------------------------------------------
total = len(sys.argv)
cmdargs = str(sys.argv)

channel = str(sys.argv[1])
theVar = str(sys.argv[2])
theFiletoRead= sys.argv[3]
systChoice = str(sys.argv[4])
statChoice = str(sys.argv[5])
datacard_dir = str(sys.argv[6])
region = str(sys.argv[7]) #'SR', 'ttZ' (CR), etc.

fileToSearch = "Template_Datacard.txt" #TEMPLATE to parse

# //--------------------------------------------
# if(channel!="" and channel!="all" and channel!="uuu" and channel!="uue" and channel!="eeu" and channel!="eee" and channel!="ee" and channel!="uu" and channel!="ue"):
#     print("wrong channel")
#     print("channel should be '', 'all', 'uuu', 'uue', 'eeu' or 'uuu' or 'ee' or 'uu' or 'ue'")
#     exit()

#Use the channel='all' keyword because parser needs to read some arg ! But don't want to appear in datacards
varchan="var_chan"
if channel=="all":
    channel=""
    varchan = "varchan" #if no subcategorization, want to remove the "_" between 'var' and 'chan' !


#If don't want shape systematics, will comment them out
if systChoice=="withShape":
    shape = ""
elif systChoice == "noShape":
    shape = "#"
else:
    print("Wrong systChoice value ! should be 'withShape' or 'noShape'")
    exit()

#If don't want statistical uncertainties, will comment them out
if statChoice=="withStat":
    stat = ""
elif statChoice == "noStat":
    stat = "#"
else:
    print("Wrong statChoice value ! should be 'withStat' or 'noStat'")
    exit()


#--------------------------------------------
# ele_sys = "" #Ele systematics only in ele channels

# if channel=="uuu" or "mmm" in channel:
#     ele_sys = "#"

#--------------------------------------------
#Can add a rateParam line to control normalization of processes from datacard
ratePar = "#" #empty to activate, or '#' to disactivate
sigPar = "tZq" #e.g. "tZq"
rateVal = "1" #or '2' to double the rate of the process ? (verify)


#--------------------------------------------
print('\n *Creating datacard for channel : '+channel+' / variable : '+theVar)
s = open( fileToSearch).read()

#-- REPLACE KEYWORDS
s = s.replace("[SHAPE]", shape)
s = s.replace("[STAT]", stat)
s = s.replace("filetoread", theFiletoRead)
s = s.replace("var_chan", varchan)
s = s.replace("chan", channel)
s = s.replace("var",theVar)

# s = s.replace("[ele_sys]",ele_sys)
# s = s.replace("sigPar", sigPar)
# s = s.replace("[ratePar]", ratePar)
# s = s.replace("rateVal", rateVal)




# //--------------------------------------------
#Replace some predefined markers with relevant values

#-- QFlip markers
# if nLep == "3l" or (nLep == "2l" and (channel == "uu" or "mm" in channel) ):
#     s = re.sub(r'\[qflip\].*?\[qflip\]', r'', s) #Erase stuff signaled by markers => Remove QFlip
    # print(s)
# s = s.replace("[qflip]", "") #Remove the remaining markers

#--------------------------------------------
print('==> Datacard created...')

outname = datacard_dir+"/datacard_";
if channel != "":
    outname=outname+channel+"_";
outname=outname+theVar+".txt"

f = open(outname, 'w')
f.write(s)
f.close()
