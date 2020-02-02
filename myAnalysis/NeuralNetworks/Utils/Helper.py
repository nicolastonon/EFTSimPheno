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
import tensorflow
import keras
from tensorflow.keras.callbacks import LambdaCallback
from matplotlib import pyplot as plt

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


def batchOutput(batch, logs):

    print("Finished batch: " + str(batch))
    print(logs)
# //--------------------------------------------
# //--------------------------------------------

def Write_Variables_To_TextFile(weight_dir, var_list):
    text_file = open(weight_dir + "ListVariables.txt", "w")
    for var in var_list:
        text_file.write(var)
        text_file.write("\n")
    text_file.close()
    print("Saved list of variables in : " + weight_dir + "ListVariables.txt")

# //--------------------------------------------
# //--------------------------------------------

class TimeHistory(tensorflow.keras.callbacks.Callback):
    def on_train_begin(self, logs={}):
        self.times = []

    def on_epoch_begin(self, batch, logs={}):
        self.epoch_time_start = time.time()

    def on_epoch_end(self, batch, logs={}):
        self.times.append(time.time() - self.epoch_time_start)

# //--------------------------------------------
# //--------------------------------------------
