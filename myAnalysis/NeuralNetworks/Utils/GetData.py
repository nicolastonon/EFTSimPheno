# xxx


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
# //--------------------------------------------
# //--------------------------------------------
# //--------------------------------------------


#Define the model and train it, using Keras only
def Get_Data_Keras(ntuples_dir, signal, bkg_type, var_list, cuts, nof_outputs, maxEvents, splitTrainEventFrac):

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

    #FIXME
    if bkg_type == "":
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

#--- Max nof events for train.test phases
    use_same_nof_events_sig_bkg = False
    use_fixed_nof_events = True
    nmax = maxEvents

    if use_same_nof_events_sig_bkg == True: #Use same nof events for both sig and bkg => min(nev_sig, nev_bkg)
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

    if (use_same_nof_events_sig_bkg or use_fixed_nof_events) and nmax is not -1:
        x_sig_array = x_sig_array[0:nmax]
        x_bkg_total = np.concatenate((x_bkg1_array[0:int(nmax/2.)], x_bkg2_array[0:int(nmax/2.)]), 0)

        #If equal amounts of signal and bkg, should use weights of 1 for all events ?
        weight = np.ones((x_sig_array.shape[0]+x_bkg_total.shape[0]), dtype = int)

        #Both 'real' and 'learning' weights taken as 1*** ac

        weightPHY=weight
        weightLEARN=weight

        # print(weight.shape)
        # print(weight)
        # exit(1)

        # bkg_total_weight = np.concatenate((bkg1_weight[0:int(nmax/2.)], bkg2_weight[0:int(nmax/2.)]), 0)
        # weight = np.concatenate((sig_weight[0:nmax], bkg_total_weight[0:nmax]), 0)
        # weight = weight[:, 0] #trick to get 1D vector instead of 2D array (different 'view' of same data)

        # print(x_sig_array.shape)
        # print(x_bkg_total.shape)
        # print(x_bkg1_array.shape)
        # print(x_bkg2_array.shape)

    # Reweight each class such that they all have same total yield
    # else:

    #Compute yields
    yield_sig = rows_sig * sig_weight[0]
    yield_bkg1 = rows_bkg1 * bkg1_weight[0]
    yield_bkg2 = rows_bkg2 * bkg2_weight[0]
    yield_total = yield_sig + yield_bkg1 + yield_bkg2
    # print(sig_weight[0], bkg1_weight[0], bkg2_weight[0])
    # print(yield_sig, yield_bkg1, yield_bkg2)

    #Compute scale factors (so all samples have same yield)
    sf_sig = yield_total / yield_sig
    if False: #Rescale such that ttW=ttZ  #Correct ?
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

    if False: #Also divide bkg weight so that bkg1 and bkg2 have equal yields
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

    x_train, x_test, y_train, y_test, weightPHY_train, weightPHY_test, weightLEARN_train, weightLEARN_test = train_test_split(x, y, weightPHY, weightLEARN, test_size=1-splitTrainEventFrac, random_state=0) #80% train, 20% test
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
