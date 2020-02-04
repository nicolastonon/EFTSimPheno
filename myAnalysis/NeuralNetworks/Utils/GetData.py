# Read ROOT files, shape/transform the data (x : input features, y : labels), compute event reweights, ...

# //--------------------------------------------
#Filtering out manually some unimportant warnings
import warnings
warnings.filterwarnings("ignore", message="numpy.dtype size changed")
warnings.filterwarnings("ignore", message="numpy.ufunc size changed")
warnings.filterwarnings("ignore", message="Conversion of the second argument of issubdtype from `float` to `np.floating` is deprecated")
# --------------------------------------------
import ROOT
from ROOT import TMVA, TFile, TTree, TCut, gROOT, TH1, TH1F
import numpy as np
from root_numpy import root2array, tree2array, array2root
import pandas as pd
import tensorflow
import keras
from pathlib import Path
from sklearn.utils import class_weight
from sklearn.feature_selection import RFE, SelectKBest, chi2
from sklearn.decomposition import PCA
from sklearn.preprocessing import MinMaxScaler, Normalizer, StandardScaler
from sklearn.model_selection import train_test_split, StratifiedKFold
from tensorflow.keras import utils

from Utils.ColoredPrintout import colors

np.set_printoptions(threshold=np.inf) #If activated, will print full numpy arrays
# //--------------------------------------------
# //--------------------------------------------





# //--------------------------------------------
# //--------------------------------------------
# //--------------------------------------------
 ######   ######## ########    ########     ###    ########    ###
##    ##  ##          ##       ##     ##   ## ##      ##      ## ##
##        ##          ##       ##     ##  ##   ##     ##     ##   ##
##   #### ######      ##       ##     ## ##     ##    ##    ##     ##
##    ##  ##          ##       ##     ## #########    ##    #########
##    ##  ##          ##       ##     ## ##     ##    ##    ##     ##
 ######   ########    ##       ########  ##     ##    ##    ##     ##

########  #######  ########     ######## ########     ###    #### ##    ## #### ##    ##  ######
##       ##     ## ##     ##       ##    ##     ##   ## ##    ##  ###   ##  ##  ###   ## ##    ##
##       ##     ## ##     ##       ##    ##     ##  ##   ##   ##  ####  ##  ##  ####  ## ##
######   ##     ## ########        ##    ########  ##     ##  ##  ## ## ##  ##  ## ## ## ##   ####
##       ##     ## ##   ##         ##    ##   ##   #########  ##  ##  ####  ##  ##  #### ##    ##
##       ##     ## ##    ##        ##    ##    ##  ##     ##  ##  ##   ###  ##  ##   ### ##    ##
##        #######  ##     ##       ##    ##     ## ##     ## #### ##    ## #### ##    ##  ######
# //--------------------------------------------
# //--------------------------------------------
# //--------------------------------------------

#Define the model and train it, using Keras only
def Get_Data_For_DNN_Training(lumi_years, ntuples_dir, signal, bkg_list, var_list, cuts, nof_outputs, maxEvents, splitTrainEventFrac):

    #Get data from TFiles
    x_sig, sig_weight, list_x_bkgs, list_weights_bkgs = Read_Store_Data(lumi_years, ntuples_dir, signal, bkg_list, var_list, cuts)

    #Shape the data arrays properly #Also read arguments which are modified by the function
    x, y, rows_sig, cols_sig, sig_weight, list_nrows_bkg, list_ncols_bkg, list_weights_bkgs = Shape_Data(x_sig, sig_weight, list_x_bkgs, list_weights_bkgs, maxEvents, nof_outputs)

    #Transform the input features
    x = Transform_Inputs(x)

    #Get event-per-event reweights
    yield_sig, list_yields_bkg, yield_total, sf_sig, weightPHY, weightLEARN = Get_Events_Weights(signal, bkg_list, rows_sig, sig_weight, list_nrows_bkg, list_weights_bkgs)

    # Define training & testing subsamples (takes care of splitting + shuffling)
    # http://scikit-learn.org/stable/modules/generated/sklearn.model_selection.train_test_split.html
    x_train, x_test, y_train, y_test, weightPHY_train, weightPHY_test, weightLEARN_train, weightLEARN_test = train_test_split(x, y, weightPHY, weightLEARN, test_size=1-splitTrainEventFrac, random_state=0) #80% train, 20% test

    # x_train, x_test, y_train, y_test = train_test_split(x, y, train_size=50000, random_state=0) #50K train
    # print(x_train[0:5], "\n")
    # print(y_train[0:5], "\n")

    print(colors.fg.lightblue, "===========", colors.reset)
    print(colors.fg.lightblue, "-- Will use " + str(x_train.shape[0]) + " training events !", colors.reset)
    print(colors.fg.lightblue, "-- Will use " + str(x_test.shape[0]) + " testing events !", colors.reset)
    print(colors.fg.lightblue, "===========\n", colors.reset)

    return x_train, y_train, x_test, y_test, weightPHY_train, weightPHY_test, weightLEARN_train, weightLEARN_test, x, y, weightPHY, weightLEARN
# //--------------------------------------------
# //--------------------------------------------













# //--------------------------------------------
# //--------------------------------------------
# //--------------------------------------------
########  ########    ###    ########     ########     ###    ########    ###
##     ## ##         ## ##   ##     ##    ##     ##   ## ##      ##      ## ##
##     ## ##        ##   ##  ##     ##    ##     ##  ##   ##     ##     ##   ##
########  ######   ##     ## ##     ##    ##     ## ##     ##    ##    ##     ##
##   ##   ##       ######### ##     ##    ##     ## #########    ##    #########
##    ##  ##       ##     ## ##     ##    ##     ## ##     ##    ##    ##     ##
##     ## ######## ##     ## ########     ########  ##     ##    ##    ##     ##
# //--------------------------------------------
# //--------------------------------------------
# //--------------------------------------------

def Read_Store_Data(lumi_years, ntuples_dir, signal, bkg_list, var_list, cuts):

    # Sanity checks
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

# //--------------------------------------------
#SIGNAL

    list_files_sig_allYears = [] #List of signal files for all considered years
    list_trees_sig_allYears = [] #List of signal TTrees for all considered years
    list_x_sig_allYears = [] #List of x-arrays (holding input variables values), for all considered years
    list_weights_sig_allYears = [] #xxx

    print(colors.fg.purple, '* Signal process:', colors.reset, signal)

    for lumiYear in lumi_years:

        filepath = ntuples_dir + lumiYear + '/' + signal + '.root'

        testfilepath = Path(filepath)
        if not testfilepath.is_file():
            print('File '+filepath+' not found ! Abort !')
            exit(1)

        print(colors.fg.lightgrey, '* Opening file:', colors.reset, ' ', filepath)
        file_sig = TFile.Open(filepath)
        tree_sig = file_sig.Get('result')
        list_files_sig_allYears.append(file_sig)
        list_trees_sig_allYears.append(tree_sig)
        list_x_sig_allYears.append(tree2array(tree_sig, branches=var_list, selection=cuts))
        list_weights_sig_allYears.append( np.absolute(tree2array(tree_sig, branches="eventWeight", selection=cuts)) ) #-- using absolute weights

# //--------------------------------------------
#BACKGROUNDS
#Loop on processes first, then lumiYears

    list_files_allYears_allBkgs = [] #List of bkg files for all considered years, for all considered background processes (<-> list of list)
    list_trees_allYears_allBkgs = [] #List of bkg TTrees for all considered years, for all considered background processes (<-> list of list)
    list_x_allYears_allBkgs = [] #List of x-arrays (holding input variables values), for all considered years
    list_weights_allYears_allBkgs = [] #xxx

    for bkg in bkg_list:

        print('\n', colors.fg.purple, '* Background process:', colors.reset, bkg)

        list_files_allYears_bkg = [] #List of files for all bkg processes, for given lumiYear
        list_trees_allYears_bkg = [] #List of bkg ROOT TTrees
        list_x_allYears_bkg = [] #xxx
        list_weights_allYears_bkg = [] #xxx
        for lumiYear in lumi_years:

            filepath = ntuples_dir + lumiYear + '/' + bkg + '.root'
            testfilepath = Path(filepath)
            if not testfilepath.is_file():
                print('File '+filepath+' not found ! Abort !')
                exit(1)

            print(colors.fg.lightgrey, '* Opening file:', colors.reset, ' ', filepath)
            file_year = TFile.Open(filepath)
            tree_year = file_year.Get('result')
            list_files_allYears_bkg.append(file_year)
            list_trees_allYears_bkg.append(tree_year)
            list_x_allYears_bkg.append(tree2array(tree_year, branches=var_list, selection=cuts))
            list_weights_allYears_bkg.append( np.absolute(tree2array(tree_year, branches="eventWeight", selection=cuts)) ) #-- using absolute weights

        list_files_allYears_allBkgs.append(list_files_allYears_bkg)
        list_trees_allYears_allBkgs.append(list_trees_allYears_bkg)
        list_x_allYears_allBkgs.append(list_x_allYears_bkg)
        list_weights_allYears_allBkgs.append(list_weights_allYears_bkg)

    print('\n\n')

  ####  #    # #    #      ##   #      #         #   # ######   ##   #####   ####
 #      #    # ##  ##     #  #  #      #          # #  #       #  #  #    # #
  ####  #    # # ## #    #    # #      #           #   #####  #    # #    #  ####
      # #    # #    #    ###### #      #           #   #      ###### #####       #
 #    # #    # #    #    #    # #      #           #   #      #    # #   #  #    #
  ####   ####  #    #    #    # ###### ######      #   ###### #    # #    #  ####

    #Get 1 single x-array for signal (sum all years)
    x_sig = np.concatenate(list_x_sig_allYears)
    sig_weight = np.concatenate(list_weights_sig_allYears)

    #Get 1 single x-array for each bkg process (sum all years)
    list_x_bkgs = []
    list_weights_bkgs = []
    for i in range(len(list_x_allYears_allBkgs)):
        list_x_bkgs.append(np.concatenate(list_x_allYears_allBkgs[i]))
        list_weights_bkgs.append(np.concatenate(list_weights_allYears_allBkgs[i]))


    return x_sig, sig_weight, list_x_bkgs, list_weights_bkgs










# //--------------------------------------------
# //--------------------------------------------
# //--------------------------------------------
 ######  ##     ##    ###    ########  ########    ########     ###    ########    ###
##    ## ##     ##   ## ##   ##     ## ##          ##     ##   ## ##      ##      ## ##
##       ##     ##  ##   ##  ##     ## ##          ##     ##  ##   ##     ##     ##   ##
 ######  ######### ##     ## ########  ######      ##     ## ##     ##    ##    ##     ##
      ## ##     ## ######### ##        ##          ##     ## #########    ##    #########
##    ## ##     ## ##     ## ##        ##          ##     ## ##     ##    ##    ##     ##
 ######  ##     ## ##     ## ##        ########    ########  ##     ##    ##    ##     ##
# //--------------------------------------------
# //--------------------------------------------
# //--------------------------------------------

def Shape_Data(x_sig, sig_weight, list_x_bkgs, list_weights_bkgs, maxEvents, nof_outputs):

    #Reshape as normal arrays (root_numpy uses different format) : 1 column per variable, 1 line per event
    x_sig_array = x_sig.view(np.float32).reshape(x_sig.shape + (-1,))

    list_x_arrays_bkgs = []
    for x_bkg in list_x_bkgs:
        list_x_arrays_bkgs.append(x_bkg.view(np.float32).reshape(x_bkg.shape + (-1,)) )

    #--- Get nof entries for each sample
    rows_sig, cols_sig = x_sig_array.shape #Get number of rows/columns for signal => nof sig entries
    # print(x_sig_array.shape); print(rows_sig); print(cols_sig)

    list_nrows_bkg = []
    list_ncols_bkg = []
    for i in range(len(list_x_arrays_bkgs)):
        # list_nrows_bkg[i], list_ncols_bkg[i] = list_x_arrays_bkgs[i].shape
        list_nrows_bkg.append(list_x_arrays_bkgs[i].shape[0])
        list_ncols_bkg.append(list_x_arrays_bkgs[i].shape[1])

    total_rows_bkg = 0
    total_cols_bkg = 0
    for i in range(len(list_nrows_bkg)):
        total_rows_bkg+= list_nrows_bkg[i]
        total_cols_bkg+= list_ncols_bkg[i]
    # print(total_rows_bkg)
    # print(total_cols_bkg)

#--- Max nof events for train.test phases
    use_same_nof_events_sig_bkg = False
    nmax = maxEvents

    # if use_same_nof_events_sig_bkg == True: #Use same nof events for both sig and bkg => min(nev_sig, nev_bkg)
    #     if rows_sig > rows_bkg:
    #         nmax = rows_bkg
    #         print("Background sample has lowest stat. --> Chose nmax = ", nmax)
    #     else:
    #         nmax = rows_sig
    #         print("Signal sample has lowest stat. --> Chose nmax = ", nmax)

        #If limiting statistics, want equal proportions of bkg1 and bkg2 !
        # if rows_bkg1 < (nmax / 2.) or rows_bkg2 < (nmax / 2.):
        #     print("Error : 1 of the backgrounds statistics is too low, it can not constitute half of the background sample with nmax = " + str(nmax) + " ! Abort")
        #     exit(1)

    if nmax is not -1:

        #Skim signal events, keep only nmax events
        if rows_sig > nmax:
            rows_sig = nmax
            x_sig_array = x_sig_array[0:nmax]
            sig_weight = sig_weight[0:nmax]
            # print(sig_weight.shape)

        #Skim bkg events, keep only nmax events for each bkg process
        for i in range(len(list_x_arrays_bkgs)):
            if list_nrows_bkg[i] > nmax:
                list_nrows_bkg[i] = nmax
                list_x_arrays_bkgs[i] = list_x_arrays_bkgs[i][0:nmax]
                list_weights_bkgs[i] = list_weights_bkgs[i][0:nmax]
                # print(list_weights_bkgs[i].shape)

    x_array_bkg_total = np.concatenate(list_x_arrays_bkgs, 0)

    #Concatenate all 3 processes into 1 single array containing both signal+bkgs
    x = np.concatenate((x_sig_array, x_array_bkg_total), 0)
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

    if nof_outputs == 2:
        # y = np_utils.to_categorical(y_integer, 2) #One-hot encode the integers => Use 2 classes (sig, bkg) #Convert class vector to binary class matrix, for use with categorical_crossentropy.
        y = utils.to_categorical(y_integer, 2) #One-hot encode the integers => Use 2 classes (sig, bkg) #Convert class vector to binary class matrix, for use with categorical_crossentropy.
    elif nof_outputs == 1:
        y = y_integer #Use simple column instead (*not* one-hot encoded)
    else:
        print("Wrong value of nof_outputs")

    return x, y, rows_sig, cols_sig, sig_weight, list_nrows_bkg, list_ncols_bkg, list_weights_bkgs











# //--------------------------------------------
# //--------------------------------------------
# //--------------------------------------------
########  ######## ##      ## ######## ####  ######   ##     ## ########  ######
##     ## ##       ##  ##  ## ##        ##  ##    ##  ##     ##    ##    ##    ##
##     ## ##       ##  ##  ## ##        ##  ##        ##     ##    ##    ##
########  ######   ##  ##  ## ######    ##  ##   #### #########    ##     ######
##   ##   ##       ##  ##  ## ##        ##  ##    ##  ##     ##    ##          ##
##    ##  ##       ##  ##  ## ##        ##  ##    ##  ##     ##    ##    ##    ##
##     ## ########  ###  ###  ######## ####  ######   ##     ##    ##     ######
# //--------------------------------------------
# //--------------------------------------------
# //--------------------------------------------

def Get_Events_Weights(signal, bkg_list, rows_sig, sig_weight, list_nrows_bkg, list_weights_bkgs):

    #Compute yields
    yield_sig = rows_sig * sig_weight[0]
    yield_total = yield_sig
    # print(yield_sig.shape)

    list_yields_bkg = []
    for i in range(len(bkg_list)):
        list_yields_bkg.append(list_nrows_bkg[i] * list_weights_bkgs[i][0])
        yield_total+= list_yields_bkg[i]
    # print(sig_weight[0], bkg1_weight[0], bkg2_weight[0])
    # print(yield_sig, yield_bkg1, yield_bkg2)

    #Compute scale factors (so all samples have same yield)
    sf_sig = yield_total / yield_sig
    list_SFs_bkg = []
    for i in range(len(bkg_list)):
        list_SFs_bkg.append(yield_total / list_yields_bkg[i])

    #Compute scale factor so that final signal weights are equal to 1 (arbitrary), other samples scaled accordingly
    norm_factor = 1./(sig_weight[0]*sf_sig)
    # norm_factor = 1. #Don't use arbitrary rescaling, as it can make later plots wrong (because we need the real weights for plotting, etc.)
    # print(norm_factor)

    sf_sig = sf_sig * norm_factor
    print('Process', signal, ' / process SF = ', sf_sig, ' / original yield = ', yield_sig, ' ===> ', sf_sig*yield_sig)
    for i in range(len(bkg_list)):
        list_SFs_bkg[i] = (list_SFs_bkg[i] * norm_factor) / len(bkg_list)
        print('Process', bkg_list[i], ' / process SF = ', list_SFs_bkg[i], ' / original yield = ', list_yields_bkg[i], ' ===> ', list_SFs_bkg[i]*list_yields_bkg[i])

    print('\n')

    list_weightsLEARN_bkg = []
    for i in range(len(bkg_list)):
        list_weightsLEARN_bkg.append(list_weights_bkgs[i]*list_SFs_bkg[i])

    # print(sf_sig, sf_bkg1, sf_bkg2)

    #Create set of "physical weights" (real event weights), and "learning" weights (to ensure equal learning)
    bkg_total_weightPHY = np.concatenate(list_weights_bkgs, 0)
    weightPHY = np.concatenate((sig_weight, bkg_total_weightPHY), 0)
    # weightPHY = weightPHY[:, 0] #necessary trick to get 1D vector instead of 2D array (different 'view' of same data)  #not needed anymore ?

    bkg_total_weightLEARN = np.concatenate(list_weightsLEARN_bkg, 0)
    # bkg_total_weightLEARN = np.concatenate((bkg1_weight*sf_bkg1, bkg2_weight*sf_bkg2), 0)
    weightLEARN = np.concatenate((sig_weight*sf_sig, bkg_total_weightLEARN), 0)

    #-- Class reweighting
    # https://scikit-learn.org/stable/modules/generated/sklearn.utils.class_weight.compute_class_weight.html
    # np.unique(y_org) --> array of classes included in dataset
    # Return class_weight_vect --> Array with class_weight_vect[i] the weight for i-th class

    # y_weight = y_train #NEED TO INCLUDE Y_TRAIN...
    # my_classweights = class_weight.compute_class_weight('balanced', np.unique(y_weight), y_weight) #Rescale according to nof entries
    # print("\n\n Using following class weights :")
    # print(my_classweights, "\n\n")

    #Included in sample_weights, as can not used both at once
    # if use_same_nof_events_sig_bkg == True:
    #     my_classweights = {0: 1., 1: 1.} #Same proportions, don't need class reweighting

    return yield_sig, list_yields_bkg, yield_total, sf_sig, weightPHY, weightLEARN













# //--------------------------------------------
# //--------------------------------------------
# //--------------------------------------------
######## ########     ###    ##    ##  ######  ########  #######  ########  ##     ##    #### ##    ## ########  ##     ## ########  ######
   ##    ##     ##   ## ##   ###   ## ##    ## ##       ##     ## ##     ## ###   ###     ##  ###   ## ##     ## ##     ##    ##    ##    ##
   ##    ##     ##  ##   ##  ####  ## ##       ##       ##     ## ##     ## #### ####     ##  ####  ## ##     ## ##     ##    ##    ##
   ##    ########  ##     ## ## ## ##  ######  ######   ##     ## ########  ## ### ##     ##  ## ## ## ########  ##     ##    ##     ######
   ##    ##   ##   ######### ##  ####       ## ##       ##     ## ##   ##   ##     ##     ##  ##  #### ##        ##     ##    ##          ##
   ##    ##    ##  ##     ## ##   ### ##    ## ##       ##     ## ##    ##  ##     ##     ##  ##   ### ##        ##     ##    ##    ##    ##
   ##    ##     ## ##     ## ##    ##  ######  ##        #######  ##     ## ##     ##    #### ##    ## ##         #######     ##     ######
# //--------------------------------------------
# //--------------------------------------------
# //--------------------------------------------

def Transform_Inputs(x):


    #-- INPUT VARIABLES transformations
    np.set_printoptions(precision=4)

    #--- RANGE SCALING
    # print('Before transformation :' x[0:5,:])
    scaler = MinMaxScaler(feature_range=(-1, 1))
    x = scaler.fit_transform(x)
    # print('After transformation :' x[0:5,:])

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

    #PCA -- could reduce number of components -- problem : can't reduce in TMVA...
    # pca = PCA()
    # x = pca.fit_transform(x_train)
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


    return x
