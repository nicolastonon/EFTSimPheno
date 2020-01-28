# Nicolas Tonon (DESY)
# Python helper function related to DNNs

# --------------------------------------------
# Standard python import
import os    # mkdir
import sys    # exit
import time   # time accounting
import getopt # command line parser
import math
import ROOT
from ROOT import TMVA, TFile, TTree, TCut, gROOT, TH1, TH1F
import numpy as np
from root_numpy import root2array, tree2array, array2root, fill_hist
import keras
from keras.callbacks import LambdaCallback


# //--------------------------------------------
##     ## ######## ##       ########  ######## ########
##     ## ##       ##       ##     ## ##       ##     ##
##     ## ##       ##       ##     ## ##       ##     ##
######### ######   ##       ########  ######   ########
##     ## ##       ##       ##        ##       ##   ##
##     ## ##       ##       ##        ##       ##    ##
##     ## ######## ######## ##        ######## ##     ##
# //--------------------------------------------

#Used to automatically close matplotlib plot after some time
def close_event():
    plt.close() #timer calls this function after 3 seconds and closes the window
# //--------------------------------------------
# //--------------------------------------------


#NB : if want to use "Training" categ. events, modify here !
def Get_Boolean_Categ(nLep, region):
    if nLep == "3l":
        return "is_tHq_3l_Training"
    elif nLep == "2l":
        return "is_tHq_2lSS_Training"

    return ""
# //--------------------------------------------
# //--------------------------------------------


# Helper function, to replace a string in a file
# Here, used to bypass TMVA : the trained model location is hard-coded in the xml weight file...
# I don't want that, so will modify the path the hard way
def Modify_String_WeightFile(filename, old, new):

    with open(filename, 'r') as file :
      filedata = file.read()

    # Replace the target string
    filedata = filedata.replace(old, new)

    # Write the file out again
    with open(filename, 'w') as file:
      file.write(filedata)
# //--------------------------------------------
# //--------------------------------------------


def batchOutput(batch, logs):

    print("Finished batch: " + str(batch))
    print(logs)
# //--------------------------------------------
# //--------------------------------------------

def Write_Variables_To_TextFile(var_list):
    text_file = open(weight_dir + "ListVariables.txt", "w")
    for var in var_list:
        text_file.write(var)
        text_file.write("\n")
    text_file.close()
    print("Saved list of variables in : " + weight_dir + "ListVariables.txt")

# //--------------------------------------------
# //--------------------------------------------

class TimeHistory(keras.callbacks.Callback):
    def on_train_begin(self, logs={}):
        self.times = []

    def on_epoch_begin(self, batch, logs={}):
        self.epoch_time_start = time.time()

    def on_epoch_end(self, batch, logs={}):
        self.times.append(time.time() - self.epoch_time_start)

# //--------------------------------------------
# //--------------------------------------------
