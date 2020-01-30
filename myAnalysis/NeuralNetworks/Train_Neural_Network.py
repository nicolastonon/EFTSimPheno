# Nicolas TONON (DESY)
# Train fully-connected neural networks with Keras (tf back-end)
# //--------------------------------------------

# //--------------------------------------------
# //--------------------------------------------
# //--------------------------------------------
 #######  ########  ######## ####  #######  ##    ##  ######
##     ## ##     ##    ##     ##  ##     ## ###   ## ##    ##
##     ## ##     ##    ##     ##  ##     ## ####  ## ##
##     ## ########     ##     ##  ##     ## ## ## ##  ######
##     ## ##           ##     ##  ##     ## ##  ####       ##
##     ## ##           ##     ##  ##     ## ##   ### ##    ##
 #######  ##           ##    ####  #######  ##    ##  ######
# //--------------------------------------------
# //--------------------------------------------
# //--------------------------------------------

#--- Set here the main training options
# //--------------------------------------------
_nepochs = 3
_batchSize = 200000
nof_outputs = 2 #can choose single output, or 2 (1 per class) #FIXME 1
# //--------------------------------------------

# Analysis options
# //--------------------------------------------
bkg_type = ""

cuts = "1" #"1" <-> no cut
# //--------------------------------------------


# Define list of input variables
# //--------------------------------------------
var_list = []

if bkg_type == "":
    var_list.append("mTW")
    var_list.append("forwardJetAbsEta")

else :
    print("ERROR ! Wrong background type !")
    exit(1)



# --------------------------------------------
# --------------------------------------------
# --------------------------------------------



# --------------------------------------------
# Standard python import
import os    # mkdir
import sys    # exit
import time   # time accounting
import getopt # command line parser
# //--------------------------------------------
#Filtering out manually some unimportant warnings
import warnings
warnings.filterwarnings("ignore", message="numpy.dtype size changed")
warnings.filterwarnings("ignore", message="numpy.ufunc size changed")
warnings.filterwarnings("ignore", message="Conversion of the second argument of issubdtype from `float` to `np.floating` is deprecated")
# //--------------------------------------------
import math
import ROOT
from ROOT import TMVA, TFile, TTree, TCut, gROOT, TH1, TH1F
import numpy as np
from root_numpy import root2array, tree2array, array2root, fill_hist
import keras
from tensorflow.keras.models import Sequential
from tensorflow.keras.models import load_model
from tensorflow.keras.layers import Dense, Dropout, AlphaDropout, Activation, BatchNormalization
from tensorflow.keras.models import model_from_json
from tensorflow.keras.optimizers import SGD, Adam, RMSprop
from tensorflow.keras import utils
# from tensorflow.keras.utils import np_utils
from tensorflow.keras.regularizers import l2
from tensorflow.keras.callbacks import TensorBoard, EarlyStopping, LambdaCallback, LearningRateScheduler, ReduceLROnPlateau
# from tensorflow.keras.utils.vis_utils import plot_model
from sklearn.model_selection import train_test_split, StratifiedKFold
from sklearn.preprocessing import MinMaxScaler, Normalizer, StandardScaler
# from tensorflow.keras.layers.advanced_activations import PReLU
from tensorflow.keras import regularizers
from tensorflow.keras.layers import LeakyReLU
from matplotlib import pyplot as plt
# import matplotlib.pyplot as plt
from ann_visualizer.visualize import ann_viz;
from sklearn.metrics import roc_curve, auc, roc_auc_score
from sklearn.utils import class_weight
from scipy import interp
from itertools import cycle
from sklearn.feature_selection import RFE, SelectKBest
from sklearn.feature_selection import chi2
from sklearn.decomposition import PCA

from pathlib import Path

from Helper import close_event, batchOutput, Write_Variables_To_TextFile, TimeHistory

weight_dir = "../weights/DNN/Keras/"
os.makedirs(weight_dir, exist_ok=True)

ntuples_dir = "../input_ntuples/2016/"

np.set_printoptions(threshold=np.inf) #If activated, will print full numpy arrays


# //--------------------------------------------
# //--------------------------------------------








# //--------------------------------------------
 ######   ######## ########     #######  ########  ######## #### ##     ## #### ######## ######## ########
##    ##  ##          ##       ##     ## ##     ##    ##     ##  ###   ###  ##       ##  ##       ##     ##
##        ##          ##       ##     ## ##     ##    ##     ##  #### ####  ##      ##   ##       ##     ##
##   #### ######      ##       ##     ## ########     ##     ##  ## ### ##  ##     ##    ######   ########
##    ##  ##          ##       ##     ## ##           ##     ##  ##     ##  ##    ##     ##       ##   ##
##    ##  ##          ##       ##     ## ##           ##     ##  ##     ##  ##   ##      ##       ##    ##
 ######   ########    ##        #######  ##           ##    #### ##     ## #### ######## ######## ##     ##
# //--------------------------------------------

#LINKS :
# - https://keras.io/optimizers/

#Choose here the Loss / Optimizer / Metrics to be used to train the model
def Get_Loss_Optim_Metrics():

    #The bigger the LR, the bigger the changes of weights in-between epochs. Too low -> weights don't update. Too large -> Instability
    _lr = 0.001

    _momentum = 0.9 #helps preventing oscillations. Usually 0.5 - 0.9
    _decay = 0.0 #Decreases the _lr by specified amount after each epoch. Used in similar way as LearningRateScheduler
    _nesterov = True #improved momentum, stronger theoretical converge guarantees for convex functions


    #Some possible choices of optimizers
    # optim = RMSprop(lr=_lr)

    # optim = SGD(lr=_lr)
    # optim = SGD(lr=_lr, decay=_decay, momentum=_momentum, nesterov=_nesterov)

    # optim = tensorflow.keras.optimizers.Adam(lr=_lr, decay=_decay) #default lr=0.001
    optim = Adam(lr=_lr, decay=_decay) #default lr=0.001

    #LOSS -- function used to optimize the model (minimized). Must be differentiable (for gradient method)
    # Examples : binary_crossentropy, categorical_crossentropy, mean_squared_error, , ...

    #METRICS -- used to judge performance of model. Not related to training. Can be used (e.g. with Keras' callbacks) to assess model's performance at given stages, or to stop training at some point (EarlyStopping, etc.)
    # metrics = 'accuracy'
    # metrics = 'binary_accuracy' #Calculates the mean accuracy rate across all predictions for binary classification problems.
    # metrics = 'categorical_accuracy'#Calculates the mean accuracy rate across all predictions for multiclass classification problems.
    # metrics = 'mean_squared_error' #Calculates the mean squared error (mse) rate between predicted and target values.
    # metrics = 'mean_absolute_error' #Calculates the mean absolute error (mae) rate between predicted and target values.
    # metrics = 'hinge' #Calculates the hinge loss, which is defined as max(1 - y_true * y_pred, 0).
    # metrics = 'binary_crossentropy' #Calculates the cross-entropy value for binary classification problems.


    # if nof_outputs > 1:
    #     loss = 'categorical_crossentropy'
    #     metrics = 'categorical_accuracy'
    # elif nof_outputs == 1:
    #     loss = 'binary_crossentropy'
    #     metrics = 'binary_accuracy'
    # else:
    #     print("Wrong value for nof_outputs!")
    #     exit(1)

    #Automatically set within Keras
    # loss = 'mean_squared_error'
    loss = 'categorical_crossentropy'
    metrics = 'accuracy'

    #Return the one you want to use
    return loss, optim, metrics


# Define learning rate schedule
def step_decay(epoch):
    initial_lrate = 0.05
    drop = 0.5
    epochs_drop = 20
    lrate = initial_lrate * math.pow(drop, math.floor((1+epoch)/epochs_drop))
    print("== Epoch "+str(epoch+1)+" : learning rate = "+str(lrate) )
    return lrate














# //--------------------------------------------
 ######  ########  ########    ###    ######## ########    ##     ##  #######  ########  ######## ##
##    ## ##     ## ##         ## ##      ##    ##          ###   ### ##     ## ##     ## ##       ##
##       ##     ## ##        ##   ##     ##    ##          #### #### ##     ## ##     ## ##       ##
##       ########  ######   ##     ##    ##    ######      ## ### ## ##     ## ##     ## ######   ##
##       ##   ##   ##       #########    ##    ##          ##     ## ##     ## ##     ## ##       ##
##    ## ##    ##  ##       ##     ##    ##    ##          ##     ## ##     ## ##     ## ##       ##
 ######  ##     ## ######## ##     ##    ##    ########    ##     ##  #######  ########  ######## ########
# //--------------------------------------------

#Advice :
# If your targets are [0,1], use a sigmoid output layer and binary_crossentropy loss.
# If your targets are [-1,1], use a linear or tanh output layer and hinge or squared_hinge loss.
# If your targets are labels for k categories, use to_categorical to convert to one-hot, use a softmax output layer with k outputs, and use categorical_crossentropy loss.

# * Has you model's performance plateaued? If not train for more epochs.
# * Compare the performance on training versus test sample. Are you over training?

#Sigmoid/tanh/softmax work fine for classifiers, but can have problems of vanishing gradients
#ReLu activations should only be used for hidden layers, avoids vanishing gradient issue
#Should use sigmoid (binary) of softmax (multiclass) for output layer, to get class probabilities ? NB : if nof_outputs=1, softmax doesn't seem to work

#Define here the Keras DNN model
def Create_Model(outdir, DNN_name, nof_outputs):

    use_batchNorm = True
    use_dropout = True #Necessary to avoid overtraining (for deep or wide NN)

    #Set some hyperparameters
    # decay_rate = learning_rate / _nepochs
    droprate = 0.5

    # Define model
    model = Sequential()
    num_input_variables = len(var_list) #Nof input variables to be read by the DNN

    # my_init = 'Zeros' #-- Can check that with this init and few training epochs, ROC is ~0.5 (no time to learn)
    # my_init = 'Ones' #-- Can check that with this init and few training epochs, ROC is ~0.5 (no time to learn)

    my_init = 'he_normal'
    # my_init = 'glorot_normal'
    # my_init = 'lecun_normal'

    #Regularizers
    #NB : "In practice, if you are not concerned with explicit feature selection, L2 regularization can be expected to give superior performance over L1."
    # my_regul = regularizers.l1(0.001)
    my_regul = regularizers.l2(0.001) #Default 0.001
    # my_regul = regularizers.l1_l2(l1=0.01, l2=0.01)

    #Examples of advanced activations (should be added as layers, after dense layers)
    # model.add(LeakyReLU(alpha=0.1))
    # model.add(PReLU(alpha_initializer=my_init))
    # model.add(Activation('selu'))

    model_choice = 1

    #-- List different models, by order of complexity

    #Model 1 -- simple
    if model_choice == 1:
       # //--------------------------------------------
        model.add(Dense(64, kernel_initializer=my_init, activation='tanh', input_dim=num_input_variables))
        model.add(Dropout(droprate))
        model.add(Dense(64, kernel_initializer=my_init, activation='relu'))
        model.add(Dropout(droprate))
        model.add(Dense(64, kernel_initializer=my_init, activation='relu'))
        model.add(Dropout(droprate))

        if nof_outputs == 1 :
            model.add(Dense(nof_outputs, kernel_initializer=my_init, activation='sigmoid'))
        elif nof_outputs == 2:
            model.add(Dense(nof_outputs, kernel_initializer=my_init, activation='softmax'))
       # //--------------------------------------------

    #Model 2 -- more elaborate, overtrained
    elif model_choice == 2:
       # //--------------------------------------------
       model.add(Dense(100, input_dim=num_input_variables, activation='tanh', kernel_initializer=my_init) ) #Input layer
       # model.add(BatchNormalization())
       model.add(Dropout(droprate))

       model.add(Dense(100, activation='relu', kernel_initializer=my_init) ) #hidden layer
       # model.add(BatchNormalization())
       # model.add(LeakyReLU(alpha=0.1))
       model.add(Dropout(droprate))

       model.add(Dense(100, activation='relu', kernel_initializer=my_init) ) #hidden layer
       # model.add(BatchNormalization())
       # model.add(LeakyReLU(alpha=0.1))
       model.add(Dropout(droprate))

       model.add(Dense(100, activation='relu', kernel_initializer=my_init) ) #hidden layer
       # model.add(BatchNormalization())
       # model.add(LeakyReLU(alpha=0.1))
       model.add(Dropout(droprate))

       model.add(Dense(nof_outputs, activation='softmax', kernel_initializer=my_init) ) #output layer
       # //--------------------------------------------

    #Model 2 -- more elaborate, overtrained
    elif model_choice == 3:
        # //--------------------------------------------
        model.add(Dense(150, input_dim=num_input_variables, activation='tanh', kernel_initializer=my_init) ) #Input layer
        if use_batchNorm==True:
            model.add(BatchNormalization())
        if use_dropout==True:
            model.add(Dropout(droprate))

        model.add(Dense(150, activation='relu', kernel_initializer=my_init) ) #hidden layer
        model.add(LeakyReLU(alpha=0.1))
        if use_batchNorm==True:
            model.add(BatchNormalization())
        if use_dropout==True:
            model.add(Dropout(droprate))

        model.add(Dense(150, activation='relu', kernel_initializer=my_init) ) #hidden layer
        # model.add(LeakyReLU(alpha=0.1))
        if use_batchNorm==True:
            model.add(BatchNormalization())
        if use_dropout==True:
            model.add(Dropout(droprate))

        model.add(Dense(150, activation='relu', kernel_initializer=my_init) ) #hidden layer
        # model.add(LeakyReLU(alpha=0.1))
        if use_batchNorm==True:
            model.add(BatchNormalization())
        if use_dropout==True:
            model.add(Dropout(droprate))

        model.add(Dense(150, activation='relu', kernel_initializer=my_init) ) #hidden layer
        if use_batchNorm==True:
            model.add(BatchNormalization())
        if use_dropout==True:
            model.add(Dropout(droprate))

        # model.add(Dense(150, activation='relu', kernel_initializer=my_init) ) #hidden layer
        # if use_batchNorm==True:
        #     model.add(BatchNormalization())
        # if use_dropout==True:
        #     model.add(Dropout(droprate))

        model.add(Dense(nof_outputs, activation='softmax', kernel_initializer=my_init) ) #output layer
# //--------------------------------------------


    #Model 3 -- even more elaborate : advanced activation, etc.
    elif model_choice == 4:
        # //--------------------------------------------
        model.add(Dense(150, input_dim=num_input_variables, activation='tanh', kernel_initializer=my_init) ) #Input layer
        # model.add(Dense(100, input_dim=num_input_variables, kernel_regularizer=my_regul, kernel_initializer=my_init) ) #Input layer
        # model.add(PReLU(alpha_initializer=my_init))
        # model.add(Activation('selu'))
        # if use_batchNorm==True:
        #     model.add(BatchNormalization())
        # if use_dropout==True:
        #     model.add(Dropout(droprate))
        # model.add(AlphaDropout(droprate))

        model.add(Dense(150, kernel_initializer=my_init) ) #hidden layer
        # model.add(PReLU(alpha_initializer=my_init))
        model.add(LeakyReLU(alpha=0.1))
        # model.add(Activation('selu'))
        if use_batchNorm==True:
            model.add(BatchNormalization())
        if use_dropout==True:
            model.add(Dropout(droprate))

        model.add(Dense(150, kernel_initializer=my_init) ) #hidden layer
        # model.add(PReLU(alpha_initializer=my_init))
        model.add(LeakyReLU(alpha=0.1))
        # model.add(Activation('selu'))
        if use_batchNorm==True:
            model.add(BatchNormalization())
        if use_dropout==True:
            model.add(Dropout(droprate))

        model.add(Dense(150, kernel_initializer=my_init) ) #hidden layer
        # model.add(PReLU(alpha_initializer=my_init))
        model.add(LeakyReLU(alpha=0.1))
        # model.add(Activation('selu'))
        if use_batchNorm==True:
            model.add(BatchNormalization())
        if use_dropout==True:
            model.add(Dropout(droprate))


        model.add(Dense(100, kernel_initializer=my_init) ) #hidden layer
        # model.add(PReLU(alpha_initializer=my_init))
        model.add(LeakyReLU(alpha=0.1))
        if use_batchNorm==True:
            model.add(BatchNormalization())
        if use_dropout==True:
            model.add(Dropout(droprate))

        model.add(Dense(nof_outputs, activation='softmax', kernel_initializer=my_init) ) #output layer
       # //--------------------------------------------

    else:
        print("\n-- ERROR : wrong model_choice value !\n")



    #Model visualization
    print(model.summary())

    # outname = outdir+'graphviz_'+DNN_name+'.png'
    # plot_model(model, to_file=outname, show_shapes=True, show_layer_names=True)
    # print("\n-- Created DNN arch plot with graphviz : " + outname)

    # outname = outdir+'annviz_'+DNN_name+'.gv'
    # ann_viz(model, title="Neural network architecture", filename=outname, view=True) #cant handle batchnorm?
    # print("\n-- Created DNN arch plot with annviz : " + outname)

    return model
# //--------------------------------------------
# //--------------------------------------------














# //--------------------------------------------
 ######     ###    ##       ##       ########     ###     ######  ##    ##  ######
##    ##   ## ##   ##       ##       ##     ##   ## ##   ##    ## ##   ##  ##    ##
##        ##   ##  ##       ##       ##     ##  ##   ##  ##       ##  ##   ##
##       ##     ## ##       ##       ########  ##     ## ##       #####     ######
##       ######### ##       ##       ##     ## ######### ##       ##  ##         ##
##    ## ##     ## ##       ##       ##     ## ##     ## ##    ## ##   ##  ##    ##
 ######  ##     ## ######## ######## ########  ##     ##  ######  ##    ##  ######
# //--------------------------------------------


def Get_Callbacks():

    # batchLogCallback = LambdaCallback(on_batch_end=batchOutput) #Could be used to perform action at end of each batch

    #Create logfile for Tensorboard, allowing to get visualization of training/test metrics
    #Usage : tensorboard --logdir=/full_path_to_your_logs --port 0
    dirlog = weight_dir + 'logs'
    tensorboard = TensorBoard(log_dir=dirlog, histogram_freq=0, write_graph=True, write_images=True)

    #Reduce learning rate when reach metrics plateau
    lrate_plateau = ReduceLROnPlateau(monitor='val_loss', factor=0.5, patience=10, verbose=1, mode='auto', min_delta=1e-4, cooldown=0, min_lr=1e-6)
    # lrate_plateau = keras.callbacks.ReduceLROnPlateau(monitor='val_loss', factor=0.5, patience=10, verbose=1, mode='auto', min_delta=1e-4, cooldown=0, min_lr=1e-6)

# keras.callbacks.callbacks.ReduceLROnPlateau(monitor='val_loss', factor=0.1, patience=10, verbose=0, mode='auto', min_delta=0.0001, cooldown=0, min_lr=0)

    #NB : ES takes place when monitored quantity has not improved **WRT BEST VALUE YET** for a number 'patience' of epochs
    # ES = keras.callbacks.EarlyStopping(monitor='val_loss', min_delta=1e-4, patience=100, verbose=1, restore_best_weights=True, mode='auto') #Try early stopping after N epochs without metrics update # monitor='val_loss'
    ES = EarlyStopping(monitor='val_loss', min_delta=1e-4, patience=100, verbose=1, restore_best_weights=True, mode='auto') #Try early stopping after N epochs without metrics update # monitor='val_loss'

    #Reduce learning rate when a metric has stopped improving
    #NB : Do not manually set learning rate (ex: model.optimizer.lr = 3e-4) when using ReduceLROnPlateau().
    lrate_sched = LearningRateScheduler(step_decay)

    #Get training time at each epoch
    time_callback = TimeHistory()

    # callbacks_list = [tensorboard, ES]
    list = [tensorboard, lrate_plateau, time_callback]

    return list




















# //--------------------------------------------
 ######   ######## ########    ########     ###    ########    ###
##    ##  ##          ##       ##     ##   ## ##      ##      ## ##
##        ##          ##       ##     ##  ##   ##     ##     ##   ##
##   #### ######      ##       ##     ## ##     ##    ##    ##     ##
##    ##  ##          ##       ##     ## #########    ##    #########
##    ##  ##          ##       ##     ## ##     ##    ##    ##     ##
 ######   ########    ##       ########  ##     ##    ##    ##     ##
# //--------------------------------------------


#Define the model and train it, using Keras only
def Get_Data_Keras(signal, bkg_type, var_list, cuts, nof_outputs):

    # Sanity checks
    if bkg_type != "":
        print("Wrong bkg_type value ! Exit !")
        return None

    if signal != "tZq" and signal != "ttZ":
        print("Wrong signal value ! Exit !")
        return None


 #       ####    ##   #####     #####    ##   #####   ##
 #      #    #  #  #  #    #    #    #  #  #    #    #  #
 #      #    # #    # #    #    #    # #    #   #   #    #
 #      #    # ###### #    #    #    # ######   #   ######
 #      #    # #    # #    #    #    # #    #   #   #    #
 ######  ####  #    # #####     #####  #    #   #   #    #

    testdirpath = Path(ntuples_dir)
    if not testdirpath.is_dir():
        print('Ntuple dir. '+ntuples_dir+' not found ! Abort !')
        exit(1)

    #Get the TFiles
    filepath = ntuples_dir + signal + '.root'

    testfilepath = Path(filepath)
    if not testfilepath.is_file():
        print('File '+filepath+' not found ! Abort !')
        exit(1)

    print("\n* Opening file : " + filepath)
    file_signal = TFile.Open(filepath)

    if bkg_type == "": #FIXME
        filepath = ntuples_dir + 'ttZ.root'
        testfilepath = Path(filepath)
        if not testfilepath.is_file():
            print('File '+filepath+' not found ! Abort !')
            exit(1)
        print("\n* Opening file : " + filepath)
        file_bkg1 = TFile.Open(filepath)

        filepath = ntuples_dir + 'ttW.root'
        testfilepath = Path(filepath)
        if not testfilepath.is_file():
            print('File '+filepath+' not found ! Abort !')
            exit(1)
        print("\n* Opening file : " + filepath)
        file_bkg2 = TFile.Open(filepath)

    print("\n===========")
    print("--- Signal = " + signal)
    if bkg_type == "":
        print("--- Backgrounds = ttZ, ttW")
    else:
        print("--- Backgrounds = TTbar_DiLep, TTbar_SemiLep")
    print("===========\n\n")

    #Get trees
    tree_sig = file_signal.Get('result')
    tree_bkg1 = file_bkg1.Get('result')
    tree_bkg2 = file_bkg2.Get('result')

    #Define cuts to apply on samples
    sel = cuts

    #Use 'root_numpy' to get numpy arrays from root files
    x_sig = tree2array(tree_sig, branches=var_list, selection=sel)
    x_bkg1 = tree2array(tree_bkg1, branches=var_list, selection=sel)
    x_bkg2 = tree2array(tree_bkg2, branches=var_list, selection=sel)
    # print(x_sig.shape)


    sig_weight = tree2array(tree_sig, branches="eventWeight", selection=sel)
    bkg1_weight = tree2array(tree_bkg1, branches="eventWeight", selection=sel)
    bkg2_weight = tree2array(tree_bkg2, branches="eventWeight", selection=sel)


    #Use only positive weights
    sig_weight = np.absolute(sig_weight)
    bkg1_weight = np.absolute(bkg1_weight)
    bkg2_weight = np.absolute(bkg2_weight)

    # sig_weight_SF = sig_weight
    # bkg1_weight_SF = bkg1_weight
    # bkg2_weight_SF = bkg2_weight

    # bkg_total_weight = np.concatenate(bkg1_weight, bkg2_weight)
    # print(sig_weight.shape)
    # print(sig_weight[0:5])
    # print(bkg1_weight[0:5])
    # print(bkg2_weight[0:5])

  ####  #    #   ##   #####  ######    #####    ##   #####   ##
 #      #    #  #  #  #    # #         #    #  #  #    #    #  #
  ####  ###### #    # #    # #####     #    # #    #   #   #    #
      # #    # ###### #####  #         #    # ######   #   ######
 #    # #    # #    # #      #         #    # #    #   #   #    #
  ####  #    # #    # #      ######    #####  #    #   #   #    #


    #Reshape as normal arrays (root_numpy uses different format) : 1 column per variable, 1 line per event
    x_sig_array = x_sig.view(np.float32).reshape(x_sig.shape + (-1,))
    x_bkg1_array = x_bkg1.view(np.float32).reshape(x_bkg1.shape + (-1,))
    x_bkg2_array = x_bkg2.view(np.float32).reshape(x_bkg2.shape + (-1,))
    x_bkg_total = np.concatenate((x_bkg1_array, x_bkg2_array), 0)

    #CHOOSE SIZES OF SIGNAL / BACKGROUND SAMPLES
    #If different sizes for signal/bkg samples, can get artificially good metrics (NN always predicts most populated class => good accuracy)

    #--- Get nof entries for each sample
    rows_sig, col_sig = x_sig_array.shape #Get number of rows/columns for signal => nof sig entries
    # print(x_sig_array.shape)
    # print(rows_sig)
    # print(col_sig)

    rows_bkg1, col_bkg1 = x_bkg1_array.shape #Get number of bkg entries
    rows_bkg2, col_bkg2 = x_bkg2_array.shape
    rows_bkg = rows_bkg1 + rows_bkg2
    col_bkg = col_bkg1 + col_bkg2
    # print(rows_bkg)
    # print(col_bkg)

    use_same_nof_events_sig_bkg = False #FIXME

    if use_same_nof_events_sig_bkg == True: #Wrong ?

        # nmax = 30000
        if rows_sig > rows_bkg:
            nmax = rows_bkg
            print("Background sample has lowest stat. --> Chose nmax = ", nmax)
        else:
            nmax = rows_sig
            print("Signal sample has lowest stat. --> Chose nmax = ", nmax)

        #If limiting statistics, want equal proportions of bkg1 and bkg2 !
        if rows_bkg1 < (nmax / 2.) or rows_bkg2 < (nmax / 2.):
            print("Error : 1 of the backgrounds statistics is too low, it can not constitute half of the background sample with nmax = " + str(nmax) + " ! Abort")
            exit(1)

        x_sig_array = x_sig_array[0:nmax]
        x_bkg_total = np.concatenate((x_bkg1_array[0:int(nmax/2.)], x_bkg2_array[0:int(nmax/2.)]), 0)

        #If equal amounts of signal and bkg, should use weights of 1 for all events ?
        weight = np.ones((x_sig_array.shape[0]+x_bkg_total.shape[0]), dtype = int)
        # print(weight.shape)
        # print(weight)
        # exit(1)

        # bkg_total_weight = np.concatenate((bkg1_weight[0:int(nmax/2.)], bkg2_weight[0:int(nmax/2.)]), 0)
        # weight = np.concatenate((sig_weight[0:nmax], bkg_total_weight[0:nmax]), 0)
        # weight = weight[:, 0] #trick to get 1D vector instead of 2D array (different 'view' of same data)
        # print((abs(weight-0.0031702)<0.0001).sum()) #Count nof events classed as "Signal"
        # print((abs(weight-0.00353645)<0.0001).sum()) #Count nof events classed as "ttZ"
        # print((abs(weight-0.00501802)<0.0001).sum()) #Count nof events classed as "ttW"

        # print(x_sig_array.shape)
        # print(x_bkg_total.shape)


    else: #Reweight each class such that they all have same total yield

        #Compute yields
        yield_sig = rows_sig * sig_weight[0]
        yield_bkg1 = rows_bkg1 * bkg1_weight[0]
        yield_bkg2 = rows_bkg2 * bkg2_weight[0]
        yield_total = yield_sig + yield_bkg1 + yield_bkg2
        # print(sig_weight[0], bkg1_weight[0], bkg2_weight[0])
        # print(yield_sig, yield_bkg1, yield_bkg2)

        #Compute scale factors (so all samples have same yield)
        sf_sig = yield_total / yield_sig
        if bkg_type == "": #Rescale such that ttW=ttZ  #Correct ?
            sf_bkg1 = yield_total / yield_bkg1
            sf_bkg2 = yield_total / yield_bkg2
            # print(sf_sig, sf_bkg1, sf_bkg2)
        else: #Merge ttbar_Dilep/SemiLep as one single process #Correct ?
            sf_bkg = yield_total / (yield_bkg1+yield_bkg2)
            # print(sf_sig, sf_bkg)

        #Compute scale factor so that final signal weights are equal to 1 (arbitrary), other samples scaled accordingly
        norm_factor = 1./(sig_weight[0]*sf_sig)
        # norm_factor = 1. #Don't use arbitrary rescaling, as it can make later plots wrong (because we need the real weights for plotting, etc.)
        # print(norm_factor)
        sf_sig = sf_sig * norm_factor

        if bkg_type == "": #Also divide bkg weight so that bkg1 and bkg2 have equal yields
            sf_bkg1 = (sf_bkg1 * norm_factor) / 2.
            sf_bkg2 = (sf_bkg2 * norm_factor) / 2.
        else:
            sf_bkg = (sf_bkg * norm_factor)
            # print(sf_sig, sf_bkg)
            sf_bkg1 = sf_bkg
            sf_bkg2 = sf_bkg


        # print(sf_sig, sf_bkg1, sf_bkg2)

        #Create set of "physical weights" (real event weights), and "learning" weights (to ensure equal learning)
        bkg_total_weightPHY = np.concatenate((bkg1_weight, bkg2_weight), 0)
        weightPHY = np.concatenate((sig_weight, bkg_total_weightPHY), 0)
        # weightPHY = weightPHY[:, 0] #necessary trick to get 1D vector instead of 2D array (different 'view' of same data)  #not needed anymore ?

        bkg_total_weightLEARN = np.concatenate((bkg1_weight*sf_bkg1, bkg2_weight*sf_bkg2), 0)
        weightLEARN = np.concatenate((sig_weight*sf_sig, bkg_total_weightLEARN), 0)
        # weightLEARN = weightLEARN[:, 0] #not needed anymore ?
        # print(weightPHY[0:10])
        # print(weightLEARN[0:10])


    #Recompute yields, make sure they are equal for sig and bkg_total
    # yield_sig = rows_sig * sig_weight[0] * sf_sig
    # yield_bkg1 = rows_bkg1 * bkg1_weight[0] * sf_bkg1
    # yield_bkg2 = rows_bkg2 * bkg2_weight[0] * sf_bkg2
    # print(yield_sig, yield_bkg1, yield_bkg2)


    #Concatenate all 3 processes into 1 single array containing both signal+bkgs
    x = np.concatenate((x_sig_array, x_bkg_total), 0)
    # print(x.shape)

    #Create array of labels, 1 column, 1 line per event
    #'1' = signal, '0' = bdf.
    #NB : for multiclass classification, should add more labels !
    y_integer = np.ones((x.shape[0]), dtype = int)
    y_integer[0:x_sig_array.shape[0]] = 0 #Signal events <-> 0
    y_integer[x_sig.shape[0]:x.shape[0]] = 1 #Bkg events <-> 1
    # print(y_integer.shape)
    # print(y_integer)
    # exit(1)

    #FIXME -- limit here the nof train/test events ?

    if nof_outputs == 2:
        # y = np_utils.to_categorical(y_integer, 2) #One-hot encode the integers => Use 2 classes (sig, bkg) #Convert class vector to binary class matrix, for use with categorical_crossentropy.
        y = utils.to_categorical(y_integer, 2) #One-hot encode the integers => Use 2 classes (sig, bkg) #Convert class vector to binary class matrix, for use with categorical_crossentropy.
    elif nof_outputs == 1:
        y = y_integer #Use simple column instead (*not* one-hot encoded)
    else:
        print("Wrong value of nof_outputs")



    #-- INPUT VARIABLES transformations
    np.set_printoptions(precision=4)

    #--- RANGE SCALING
    # print(x[0:5,:])
    scaler = MinMaxScaler(feature_range=(-1, 1))
    x = scaler.fit_transform(x)
    # print(x[0:5,:])

    #--- RESCALE TO UNIT GAUSSIAN -- BUGGY in c++ code... (NB : does not consider weights !)
    # scaler = StandardScaler().fit(x)
    # x = scaler.transform(x)
    # print(x[0:5,:])
    # exit(1)

    # print(y.shape)
    # print(y)
    # print((y == 0).sum()) #Count nof events classed as "Signal"
    # print((y == 1).sum()) #Count nof events classed as "Bkg"
    # exit(1)

    # Print first and last 5 entries of y. Make sure the first labels are of 1 type, and last labels are of other type !
    # print(y[0:5])
    # print("...")
    # print(y[ y.shape[0]-5-1:y.shape[0]-1 ])
    # print("y shape : ", y.shape)

    # Print first and last 5 entries of x
    # print(x[0:5])
    # print("...")
    # print(x[x.shape[0]-5-1:x.shape[0]-1])
    # print("x shape : ", x.shape)



    #-- Class reweighting #Included in sample_weights, as can not used both at once
    # if use_same_nof_events_sig_bkg == True:
    #     my_classweights = {0: 1., 1: 1.} #Same proportions, don't need class reweighting

    # else: #Reweight according to nof entries
        # if nof_outputs == 1:
        #     y_weight = y_train
        # elif nof_outputs > 1:
        #     y_weight = [y.argmax() for y in y_train]

        # my_classweights = class_weight.compute_class_weight('balanced', np.unique(y_weight), y_weight) #Rescale according to nof entries
        # my_classweights = {0: 5.67, 1: 1.} #Manual reweighting : according to nof entries * weight !

    # print("\n\n Using following class weights :")
    # print(my_classweights, "\n\n")


    #Define training & testing subsamples -- Takes care of splitting & shuffling
    # http://scikit-learn.org/stable/modules/generated/sklearn.model_selection.train_test_split.html

    x_train, x_test, y_train, y_test, weightPHY_train, weightPHY_test, weightLEARN_train, weightLEARN_test = train_test_split(x, y, weightPHY, weightLEARN, test_size=0.20, random_state=0) #80% train, 20% test
    # x_train, x_test, y_train, y_test = train_test_split(x, y, train_size=50000, random_state=0) #50K train
    # print(x_train[0:5], "\n")
    # print(y_train[0:5], "\n")

    #PCA -- could reduce number of components -- problem : can't reduce in TMVA...
    # pca = PCA()
    # x_train = pca.fit_transform(x_train)
    # x_test = pca.transform(x_test)
    # explained_variance = pca.explained_variance_ratio_
    # print(x_train[0:5], "\n")
    # print(explained_variance)

    # pca = PCA()
    # pca.fit(x_train)
    # plt.plot(np.cumsum(pca.explained_variance_ratio_))
    # plt.xlabel('Number of components')
    # plt.ylabel('Cumulative explained variance')
    # plt.show()

    # pca_std = np.std(x_train)
    # print(x_train.shape)
    # print(pca_std)
    # NCOMPONENTS = len(var_list)
    # pca = PCA(n_components=NCOMPONENTS)
    # x_train = pca.fit_transform(x_train)
    # x_test = pca.transform(x_test)
    # pca_std = np.std(x_train)
    # print(x_train.shape)
    # print(pca_std)
    # exit(1)


    print("\n===========")
    print("-- Will use " + str(x_train.shape[0]) + " training events !")
    print("-- Will use " + str(x_test.shape[0]) + " testing events !")
    print("===========\n")

    # print(y_train.shape)
    # print(y_train)
    # exit(1)

    return x_train, y_train, x_test, y_test, weightPHY_train, weightPHY_test, weightLEARN_train, weightLEARN_test, x, y, weightPHY, weightLEARN
# //--------------------------------------------
# //--------------------------------------------

















# //--------------------------------------------
######## ########     ###    #### ##    ##          ##    ######## ########  ######  ########
   ##    ##     ##   ## ##    ##  ###   ##         ##        ##    ##       ##    ##    ##
   ##    ##     ##  ##   ##   ##  ####  ##        ##         ##    ##       ##          ##
   ##    ########  ##     ##  ##  ## ## ##       ##          ##    ######    ######     ##
   ##    ##   ##   #########  ##  ##  ####      ##           ##    ##             ##    ##
   ##    ##    ##  ##     ##  ##  ##   ###     ##            ##    ##       ##    ##    ##
   ##    ##     ## ##     ## #### ##    ##    ##             ##    ########  ######     ##
# //--------------------------------------------

def Train_Test_Eval_PureKeras(bkg_type, var_list, cuts, _nepochs, _batchSize, nof_outputs):

    signal = "tZq"

    #Get data
    x_train, y_train, x_test, y_test, weightPHY_train, weightPHY_test, weightLEARN_train, weightLEARN_test, x, y, weightPHY, weightLEARN = Get_Data_Keras(signal, bkg_type, var_list, cuts, nof_outputs)

    #Get model, compile
    model = Create_Model(weight_dir, "DNN"+bkg_type, nof_outputs)

    #-- Can access weights and biases of any layer (debug, ...) #Print before training
    # weights_layer, biases_layer = model.layers[0].get_weights()
    # print(weights_layer.shape)
    # print(biases_layer.shape)
    # print(weights_layer)
    # print(biases_layer[0:2])

    _loss, _optim, _metrics = Get_Loss_Optim_Metrics()
    model.compile(loss=_loss, optimizer=_optim, metrics=[_metrics]) #For multiclass classification

    callbacks_list = Get_Callbacks()

    #-- KFOLD TEST
    # define 10-fold cross validation test harness
    # kfold = StratifiedKFold(n_splits=10, shuffle=True, random_state=7)
    # cvscores = []
    # for train, test in kfold.split(x, y):
    #     model = None
    #     model = Create_Model(outdir, "DNN"+bkg_type, nof_outputs)
    #     model.compile(loss=_loss, optimizer=_optim, metrics=[_metrics])
    #     model.fit(x[train], y[train], validation_data=(x[test], y[test]), epochs=_nepochs, batch_size=_batchSize, callbacks=callbacks_list)
    #     score = model.evaluate(x[test], y[test], batch_size=_batchSize)
    #     # print("%s: %.2f%%" % (model.metrics_names[1], scores[1]*100))
    #     cvscores.append(score[1] * 100)
    # print("%.2f%% (+/- %.2f%%)" % (numpy.mean(cvscores), numpy.std(cvscores)))
    # exit(1)

    history = model.fit(x_train, y_train, validation_data=(x_test, y_test), epochs=_nepochs, batch_size=_batchSize, sample_weight=weightLEARN_train, callbacks=callbacks_list, shuffle=True)

    #Print weights after training
    # weights_layer, biases_layer = model.layers[0].get_weights()
    # print(weights_layer)

    # Evaluate the model (metrics)
    score = model.evaluate(x_test, y_test, batch_size=_batchSize, sample_weight=weightPHY_test)
    # print(score)


####    ##   #    # ######    #    #  ####  #####  ###### #
#       #  #  #    # #         ##  ## #    # #    # #      #
####  #    # #    # #####     # ## # #    # #    # #####  #
  # ###### #    # #         #    # #    # #    # #      #
#    # #    #  #  #  #         #    # #    # #    # #      #
####  #    #   ##   ######    #    #  ####  #####  ###### ######

    outname = weight_dir + 'model_DNN'+bkg_type

    #Serialize model to HDF5
    model.save(outname+'.h5')
    # model.save_weights(outname+'.h5')
    # print("Saved model to disk")

    # Save the model architecture
    with open(weight_dir + 'arch_DNN'+bkg_type+'_all.json', 'w') as json_file:
        json_file.write(model.to_json())


    # export_model(model, outname+'.model') # Use 'kerasify' add-on to export model in specific format #obsolete

    #Save list of variables
    # Write_Variables_To_TextFile(var_list)


 #####  ######  ####  #    # #      #####  ####
 #    # #      #      #    # #        #   #
 #    # #####   ####  #    # #        #    ####
 #####  #           # #    # #        #        #
 #   #  #      #    # #    # #        #   #    #
 #    # ######  ####   ####  ######   #    ####

    nEvents_train, tmp = y_train.shape
    nEvents_test, tmp = y_test.shape

    #Create some more control plots for quick checking
    print("\n\n\n\n########################")
    print("########################")
    print("## Results & Control Plots ##")
    print("########################")
    print("########################")

    accuracy = score[1]
    print("\n\n*** Accuracy ***")
    print(str(accuracy) + "\n\n")

    auc_score = roc_auc_score(y_test, model.predict(x_test))
    auc_score_train = roc_auc_score(y_train, model.predict(x_train))
    print("\n*** AUC scores ***")
    print("-- TEST SAMPLE  \t(" + str(nEvents_test) + " events) \t\t==> " + str(auc_score) )
    print("-- Train sample \t(" + str(nEvents_train) + " events) \t\t==> " + str(auc_score_train) + "\n\n")

    predictions_train_sig, predictions_train_bkg, predictions_test_sig, predictions_test_bkg, weightLEARN_sig, weightLEARN_bkg, weight_test_sig, weight_test_bkg = Apply_Model(bkg_type, var_list, cuts, _nepochs, _batchSize, nof_outputs, x_train, y_train, x_test, y_test, weightPHY_train, weightPHY_test)

    # Fill a ROOT histogram from a NumPy array
    rootfile_outname = "../outputs/PredictKeras_DNN"+bkg_type+".root"
    fout = ROOT.TFile(rootfile_outname, "RECREATE")

    # print("last : ")
    # print(weightLEARN_sig[0:5])

    #Write histograms with reweighting
    hist_train_sig = TH1F('hist_train_sig', '', 100, -1, 1)
    fill_hist(hist_train_sig, predictions_train_sig, weights=weightLEARN_sig)
    # hist_test_sig.Draw('hist')
    hist_train_sig.Write()

    hist_train_bkg = TH1F('hist_train_bkg', '', 100, -1, 1)
    fill_hist(hist_train_bkg, predictions_train_bkg, weights=weightLEARN_bkg)
    # hist_train_bkg.Draw('hist')
    hist_train_bkg.Write()

    hist_test_sig = TH1F('hist_test_sig', '', 100, -1, 1)
    fill_hist(hist_test_sig, predictions_test_sig, weights=weight_test_sig)
    # hist_test_sig.Draw('hist')
    hist_test_sig.Write()

    hist_test_bkg = TH1F('hist_test_bkg', '', 100, -1, 1)
    fill_hist(hist_test_bkg, predictions_test_bkg, weights=weight_test_bkg)
    # hist_test_bkg.Draw('hist')
    hist_test_bkg.Write()

    #Also write histos witought reweighting
    hist_train_sig_noReweight = TH1F('hist_train_sig_noReweight', '', 100, -1, 1)
    fill_hist(hist_train_sig_noReweight, predictions_train_sig)
    # hist_train_sig_noReweight.Draw('hist')
    hist_train_sig_noReweight.Write()

    hist_train_bkg_noReweight = TH1F('hist_train_bkg_noReweight', '', 100, -1, 1)
    fill_hist(hist_train_bkg_noReweight, predictions_train_bkg)
    # hist_train_bkg_noReweight.Draw('hist')
    hist_train_bkg_noReweight.Write()

    hist_test_sig_noReweight = TH1F('hist_test_sig_noReweight', '', 100, -1, 1)
    fill_hist(hist_test_sig_noReweight, predictions_test_sig)
    # hist_test_sig_noReweight.Draw('hist')
    hist_test_sig_noReweight.Write()

    hist_test_bkg_noReweight = TH1F('hist_test_bkg_noReweight', '', 100, -1, 1)
    fill_hist(hist_test_bkg_noReweight, predictions_test_bkg)
    # hist_test_bkg_noReweight.Draw('hist')
    hist_test_bkg_noReweight.Write()

    fout.Close()
    print("Saved output ROOT file containing Keras Predictions as histograms : " + rootfile_outname)


    #Get ROC curve using test data -- different for nof_outputs>1, should fix it
    #Uses predict() function, which generates (output) given (input + model)
    if nof_outputs == 1:
        fpr, tpr, _ = roc_curve(y_test, model.predict(x_test))
        roc_auc = auc(fpr, tpr)
        plt.plot(fpr, tpr,color='darkorange',label='ROC curve (area = %0.2f)' % roc_auc)
        # plt.plot(tpr, 1-fpr,color='darkorange',label='ROC curve (area = %0.2f)' % roc_auc)
        plt.legend(loc="lower right")
        plt.xlabel('True Positive Rate')
        plt.ylabel('False Positive Rate')
        plt.show()
    else: #different for multiclass
        # Plot linewidth.
        lw = 2
        n_classes = 2

        # Compute ROC curve and ROC area for each class
        fpr = dict()
        tpr = dict()
        roc_auc = dict()
        fpr_train = dict()
        tpr_train = dict()
        roc_auc_train = dict()
        for i in range(n_classes):
            fpr[i], tpr[i], _ = roc_curve(y_test[:, i], model.predict(x_test)[:, i])
            roc_auc[i] = auc(fpr[i], tpr[i])

            #NEW -- try to also show ROC on training sample for comparison
            fpr_train[i], tpr_train[i], _ = roc_curve(y_train[:, i], model.predict(x_train)[:, i])
            roc_auc_train[i] = auc(fpr_train[i], tpr_train[i])


        # Compute micro-average ROC curve and ROC area
        # fpr["micro"], tpr["micro"], _ = roc_curve(y_test.ravel(), model.predict(x_test).ravel())
        # roc_auc["micro"] = auc(fpr["micro"], tpr["micro"])

        # Compute macro-average ROC curve and ROC area

        # First aggregate all false positive rates
        # all_fpr = np.unique(np.concatenate([fpr[i] for i in range(n_classes)]))

        # Then interpolate all ROC curves at this points
        # mean_tpr = np.zeros_like(all_fpr)
        # for i in range(n_classes):
        #     mean_tpr += interp(all_fpr, fpr[i], tpr[i])

        # Finally average it and compute AUC
        # mean_tpr /= n_classes

        # fpr["macro"] = all_fpr
        # tpr["macro"] = mean_tpr
        # roc_auc["macro"] = auc(fpr["macro"], tpr["macro"])

        # Plot all ROC curves
        fig1 = plt.figure(1)
        timer = fig1.canvas.new_timer(interval = 3000) #creating a timer object and setting an interval of N milliseconds
        timer.add_callback(close_event)

        ax = fig1.gca()
        ax.set_xticks(np.arange(0, 1, 0.1))
        ax.set_yticks(np.arange(0, 1., 0.1))
        plt.grid()

        # plt.plot(fpr["micro"], tpr["micro"],
        #  label='micro-average ROC curve (area = {0:0.2f})'
        #        ''.format(roc_auc["micro"]),
        #  color='deeppink', linestyle=':', linewidth=4)

        # plt.plot(fpr["macro"], tpr["macro"],
        #  label='macro-average ROC curve (area = {0:0.2f})'
        #        ''.format(roc_auc["macro"]),
        #  color='navy', linestyle=':', linewidth=4)

        #--- To plot several classes, micro/macro, etc -- commented out !
        # colors = cycle(['darkorange', 'aqua', 'cornflowerblue'])
        # for i, color in zip(range(1), colors): #replaced 'n_classes' by 1 => only plot signal ROC
            # plt.plot(fpr[i], tpr[i], color=color, lw=lw,
            #      label='ROC curve of class {0} (area = {1:0.2f})'
            #      ''.format(i, roc_auc[i]))
            # plt.plot(tpr[0], 1-fpr[0], color='darkorange', lw=lw,
            #      label='ROC DNN (test) (AUC = {1:0.2f})'
            #      ''.format(i, roc_auc[i]))

        plt.plot(tpr[0], 1-fpr[0], color='darkorange', lw=lw,
             label='ROC DNN (test) (AUC = {1:0.2f})'
             ''.format(i, roc_auc[i]))
        plt.plot(tpr_train[0], 1-fpr_train[0], color='cornflowerblue', lw=lw,
             label='ROC DNN (train) (AUC = {1:0.2f})'
             ''.format(i, roc_auc_train[i]))

        plt.plot([1, 0], [0, 1], 'k--', lw=lw)
        plt.xlim([0.0, 1.0])
        plt.ylim([0.0, 1.0])
        plt.xlabel('Signal efficiency')
        plt.ylabel('Background rejection')
        plt.title('')
        plt.legend(loc="lower left")
        timer.start()
        plt.show()
        plotname = weight_dir + 'ROC_DNN'+bkg_type+'.png'
        fig1.savefig(plotname)
        print("Saved ROC plot as : " + plotname)



    # Plotting the loss with the number of iterations
    fig2 = plt.figure(2)
    timer = fig2.canvas.new_timer(interval = 3000) #creating a timer object and setting an interval of N milliseconds
    timer.add_callback(close_event)
    plt.plot(history.history['loss'])
    plt.plot(history.history['val_loss'])
    plt.title('Loss VS Epoch')
    plt.ylabel('Loss')
    plt.xlabel('Epoch')
    plt.legend(['Train', 'Test'], loc='upper right')
    timer.start()
    plt.show()
    plotname = weight_dir + 'Loss_DNN'+bkg_type+'.png'
    fig2.savefig(plotname)
    print("Saved Loss plot as : " + plotname)


    # Plotting the error with the number of iterations
    fig3 = plt.figure(3)
    timer = fig3.canvas.new_timer(interval = 3000) #creating a timer object and setting an interval of N milliseconds
    timer.add_callback(close_event)
    # plt.plot(history.history[_metrics]) #metrics name
    plt.plot(history.history['acc'])
    plt.plot(history.history['val_acc'])
    plt.title('Accuracy VS Epoch')
    plt.ylabel('Accuracy')
    plt.xlabel('Epoch')
    plt.legend(['Train', 'Test'], loc='lower right')
    timer.start()
    plt.show()
    plotname = weight_dir + 'Accuracy_DNN'+bkg_type+'.png'
    fig3.savefig(plotname)
    print("Saved Accuracy plot as : " + plotname)

# //--------------------------------------------
# //--------------------------------------------
























# //--------------------------------------------
   ###    ########  ########  ##       ####  ######     ###    ######## ####  #######  ##    ##
  ## ##   ##     ## ##     ## ##        ##  ##    ##   ## ##      ##     ##  ##     ## ###   ##
 ##   ##  ##     ## ##     ## ##        ##  ##        ##   ##     ##     ##  ##     ## ####  ##
##     ## ########  ########  ##        ##  ##       ##     ##    ##     ##  ##     ## ## ## ##
######### ##        ##        ##        ##  ##       #########    ##     ##  ##     ## ##  ####
##     ## ##        ##        ##        ##  ##    ## ##     ##    ##     ##  ##     ## ##   ###
##     ## ##        ##        ######## ####  ######  ##     ##    ##    ####  #######  ##    ##
# //--------------------------------------------

def Apply_Model(bkg_type, var_list, cuts, _nepochs, _batchSize, nof_outputs, x_train, y_train, x_test, y_test, weightLEARN, weight_test):

    signal = "tZq"

    #Get data #CHANGED -- directly taken as arg
    # x_train, y_train, x_test, y_test, weightLEARN, weight_test = Get_Data_Keras(signal, bkg_type, var_list, cuts, nof_outputs)

    #Split test & train sample between "true signal" & "true background" (must be separated for plotting)
    x_test_sig = x_test[y_test[:, 0]==1]
    y_test_sig = y_test[y_test[:, 0]==1]
    weight_test_sig = weight_test[y_test[:, 0]==1]
    # print(weight_test_sig[0:5])

    x_test_bkg = x_test[y_test[:, 1]==1]
    y_test_bkg = y_test[y_test[:, 1]==1]
    weight_test_bkg = weight_test[y_test[:, 1]==1]

    x_train_sig = x_train[y_train[:, 0]==1]
    y_train_sig = y_train[y_train[:, 0]==1]
    weightLEARN_sig = weightLEARN[y_train[:, 0]==1]
    # print(weightLEARN_sig[0:5])

    x_train_bkg = x_train[y_train[:, 1]==1]
    y_train_bkg = y_train[y_train[:, 1]==1]
    weightLEARN_bkg = weightLEARN[y_train[:, 1]==1]

    #Cross-checks
    # print(x_test.shape)
    # print(y_test.shape)
    # print(x_test_sig.shape)
    # print(y_test_sig.shape)
    # print(x_test_bkg.shape)
    # print(y_test_bkg.shape)
    # print(weight_test_sig.shape)
    # print(weight_test_bkg.shape)

    #Load model
    model = load_model(weight_dir + "model_DNN"+bkg_type+".h5")


 #####  #####  ###### #####  #  ####  #####
 #    # #    # #      #    # # #    #   #
 #    # #    # #####  #    # # #        #
 #####  #####  #      #    # # #        #
 #      #   #  #      #    # # #    #   #
 #      #    # ###### #####  #  ####    #

    #Application (can also use : predict_classes, predict_proba)
    predictions_test_sig = model.predict(x_test_sig)
    predictions_test_bkg = model.predict(x_test_bkg)
    predictions_train_sig = model.predict(x_train_sig)
    predictions_train_bkg = model.predict(x_train_bkg)

    #Only keep signal proba (redundant info)
    predictions_test_sig = predictions_test_sig[:, 0]
    predictions_test_bkg = predictions_test_bkg[:, 0]
    predictions_train_sig = predictions_train_sig[:, 0]
    predictions_train_bkg = predictions_train_bkg[:, 0]

    #-- Printout of some results
    # for i in range(5):
    #     if y_test[i][0]==1:
    #         true_label = "Signal"
    #     else:
    #         true_label = "Background"
    #     print("\n--------------")
    #     print("X=%s\n\n=====> Predicted : %s (True label : %s)" % (x_test[i], predictions[i][0], true_label))
    #     print("--------------\n")






  ####   ####  #    # ##### #####   ####  #         #####  #       ####  #####  ####
 #    # #    # ##   #   #   #    # #    # #         #    # #      #    #   #   #
 #      #    # # #  #   #   #    # #    # #         #    # #      #    #   #    ####
 #      #    # #  # #   #   #####  #    # #         #####  #      #    #   #        #
 #    # #    # #   ##   #   #   #  #    # #         #      #      #    #   #   #    #
  ####   ####  #    #   #   #    #  ####  ######    #      ######  ####    #    ####


    #--- PLOTTING OUTPUT
    nbins = 25
    rmin = 0.
    rmax = 1.

    #--- COSMETICS
    #grey=#E6E6E6 #white=#FFFFFF

    #grey=#E6E6E6 #white=#FFFFFF
    fig4 = plt.figure(4)
    ax = plt.axes()
    # fig4, ax = plt.subplots(facecolor='white') #outer bkg color

    timer = fig4.canvas.new_timer(interval = 3000) #creating a timer object and setting an interval of N milliseconds
    timer.add_callback(close_event)

    ax.set_xlim([0.,1.])

    # use a gray background
    # ax.patch.set_facecolor('white')
    ax.patch.set_edgecolor('black')
    ax.patch.set_facecolor('#E6E6E6') #inner bkg color

    # draw solid white grid lines
    plt.grid(color='w', linestyle='solid')

    # hide axis spines
    for spine in ax.spines.values():
        spine.set_visible(False)

    # hide top and right ticks
    ax.xaxis.tick_bottom()
    # ax.yaxis.tick_left()

    # lighten ticks and labels
    ax.tick_params(colors='gray', direction='out')
    for tick in ax.get_xticklabels():
        tick.set_color('gray')
        # tick.set_weight('bold')
    for tick in ax.get_yticklabels():
        tick.set_color('gray')

    #Colors : darkorange, cornflowerblue

    plt.hist(predictions_test_sig, bins=nbins, range=(rmin,rmax), color= 'cornflowerblue', alpha=0.50, weights=weight_test_sig, density=True, histtype='step', log=False, label="Signal (Test)", edgecolor='cornflowerblue',fill=True)

    plt.hist(predictions_test_bkg, bins=nbins, range=(rmin,rmax), color='orangered', alpha=0.50, weights=weight_test_bkg, density=True, histtype='step', log=False, label="Background (Test)", hatch='/', edgecolor='orangered',fill=False)

    #Get bin content of histograms -- First not normalized
    counts_sig_noNorm, bin_edges_sig = np.histogram(predictions_train_sig, bins=nbins, range=(rmin,rmax), weights=weightLEARN_sig, density=False)
    counts_bkg_noNorm, bin_edges_bkg = np.histogram(predictions_train_bkg, bins=nbins, range=(rmin,rmax), weights=weightLEARN_bkg, density=False)
    bin_centres = (bin_edges_sig[:-1] + bin_edges_sig[1:])/2.

    #Now get normalized histos
    counts_sig, bin_edges_sig = np.histogram(predictions_train_sig, bins=nbins, range=(rmin,rmax), weights=weightLEARN_sig, density=True)
    counts_bkg, bin_edges_bkg = np.histogram(predictions_train_bkg, bins=nbins, range=(rmin,rmax), weights=weightLEARN_bkg, density=True)
    # print(bin_edges_sig[:-1])
    # print(bin_edges_sig[1:])

    #Compute rescaling ratio (for norm.)
    # print(counts_sig)
    if counts_sig[5]==0 or counts_sig_noNorm[5] == 0: #If could not compute ratio, set errors to 0
        normRatio_sig = 0
        normRatio_bkg = 0
    else:
        normRatio_sig = counts_sig[5] / counts_sig_noNorm[5]
        normRatio_bkg = counts_bkg[5] / counts_bkg_noNorm[5]

    # err_sig = np.sqrt(counts_sig) * normRatio_sig
    # err_bkg = np.sqrt(counts_bkg) * normRatio_bkg

    #FIXME -- set errors to 0 for now, not checked
    err_sig = np.zeros(shape=counts_sig.shape)
    err_bkg = np.zeros(shape=counts_bkg.shape)

    plt.errorbar(bin_centres, counts_sig, marker='o', yerr=err_sig, linestyle='None', markersize=6, color='blue', alpha=0.90, label='Signal (Train)')
    plt.errorbar(bin_centres, counts_bkg, marker='o', yerr=err_bkg, linestyle='None', markersize=6, color='red', alpha=0.90, label='Background (Train)')

    #Plot markers, no error
    # plt.plot(bin_centres, counts_sig, marker='o', linestyle='None', markersize=8, color='blue', alpha=0.90, label='Signal (Train)')
    # plt.plot(bin_centres, counts_bkg, marker='o', linestyle='None', markersize=8, color='orangered', alpha=0.90, label='Background (Train)')

    plt.legend(loc='upper center', numpoints=1)

    plt.title("Output distributions for Signal & Background")
    plt.grid(axis='y', alpha=0.75)
    plt.grid(axis='x', alpha=0.75)
    plt.xlabel('DNN output')
    plt.ylabel('PDF')
    timer.start()
    plt.show()

    plotname = weight_dir + 'Overtraining_DNN'+bkg_type+'.png'
    fig4.savefig(plotname)
    print("Saved Overtraining plot as : " + plotname)

    return predictions_train_sig, predictions_train_bkg, predictions_test_sig, predictions_test_bkg, weightLEARN_sig, weightLEARN_bkg, weight_test_sig, weight_test_bkg































# //--------------------------------------------
# //--------------------------------------------
# //--------------------------------------------
# //--------------------------------------------
######## ##     ## ##    ##  ######      ######     ###    ##       ##        ######
##       ##     ## ###   ## ##    ##    ##    ##   ## ##   ##       ##       ##    ##
##       ##     ## ####  ## ##          ##        ##   ##  ##       ##       ##
######   ##     ## ## ## ## ##          ##       ##     ## ##       ##        ######
##       ##     ## ##  #### ##          ##       ######### ##       ##             ##
##       ##     ## ##   ### ##    ##    ##    ## ##     ## ##       ##       ##    ##
##        #######  ##    ##  ######      ######  ##     ## ######## ########  ######
# //--------------------------------------------
# //--------------------------------------------
# //--------------------------------------------
# //--------------------------------------------


#-------- Call from external script (e.g. main C++ analysis code) --> Read command line args
# if len(sys.argv) == 2:
#     if sys.argv[1] == True:
#         nLep = "3l"
#     else:
#         nLep = "2l"

    Train_Test_Eval_PureKeras(bkg_type, var_list, cuts, _nepochs, _batchSize, nof_outputs)
    exit(1)


#----------  Manual call to DNN training function
Train_Test_Eval_PureKeras(bkg_type, var_list, cuts, _nepochs, _batchSize, nof_outputs)
