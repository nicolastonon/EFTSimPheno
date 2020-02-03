# Nicolas TONON (DESY)
# Train fully-connected neural networks with Keras (tf back-end)
# //--------------------------------------------

'''
#TODO#
- Allow to choose which years to train on (change namings, open multiple ntuples)
- argparse
- segment code
- Need class weights !

#NOTES#
- fit() is for training the model with the given inputs (and corresponding training labels).
- evaluate() is for evaluating the already trained model using the validation (or test) data and the corresponding labels. Returns the loss value and metrics values for the model.
- predict() is for the actual prediction. It generates output predictions for the input samples.
'''


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
_signal = "tZq"
_nepochs = 3
_batchSize = 5000
_nof_outputs = 2 #single output not supported (e.g. for validation steps)
_maxEvents = -1 #max total nof events to be used
_splitTrainEventFrac = 0.8 #Fraction of events to be used for training (1 <-> use all requested events for training)
# //--------------------------------------------

# Analysis options
# //--------------------------------------------
bkg_type = ""

cuts = "passedBJets==1" #"1" <-> no cut
# //--------------------------------------------


# Define list of input variables
# //--------------------------------------------
var_list = []

if bkg_type == "":
    var_list.append("maxDijetDelR")
    var_list.append("dEtaFwdJetBJet")
    var_list.append("dEtaFwdJetClosestLep")
    var_list.append("mHT")
    var_list.append("mTW")
    var_list.append("Mass_3l")
    var_list.append("forwardJetAbsEta")
    var_list.append("jPrimeAbsEta")
    var_list.append("maxDeepCSV")
    var_list.append("delRljPrime")
    var_list.append("lAsymmetry")
    var_list.append("maxDijetMass")
    var_list.append("maxDelPhiLL")

else :
    print("ERROR ! Wrong background type !")
    exit(1)








# //--------------------------------------------
#Filtering out manually some unimportant warnings
# import warnings
# warnings.filterwarnings("ignore", message="tensorflow:sample_weight modes were coerced")

# --------------------------------------------
# Standard python import
import os    # mkdir
import sys    # exit
import time   # time accounting
import getopt # command line parser
# //--------------------------------------------
import ROOT
from ROOT import TMVA, TFile, TTree, TCut, gROOT, TH1, TH1F
import numpy as np
import tensorflow
import keras
from sklearn.metrics import roc_curve, auc, roc_auc_score
from tensorflow.keras.models import load_model

from Utils.FreezeSession import freeze_session
from Utils.Helper import batchOutput, Write_Variables_To_TextFile, TimeHistory
from Utils.CreateModel import Create_Model
from Utils.GetCallbacks import Get_Callbacks
from Utils.GetData import Get_Data_Keras
from Utils.GetOptimizer import Get_Loss_Optim_Metrics
from Utils.ColoredPrintout import colors
from Utils.Create_Output_Plots_Histos import Create_TrainTest_ROC_Histos, Create_Control_Plots

# Main paths
weight_dir = "../weights/DNN/"
os.makedirs(weight_dir, exist_ok=True)

ntuples_dir = "../input_ntuples/2016/"

np.set_printoptions(threshold=np.inf) #If activated, will print full numpy arrays

# //--------------------------------------------
# //--------------------------------------------



























# //--------------------------------------------
# //--------------------------------------------
# //--------------------------------------------
######## ########     ###    #### ##    ##
   ##    ##     ##   ## ##    ##  ###   ##
   ##    ##     ##  ##   ##   ##  ####  ##
   ##    ########  ##     ##  ##  ## ## ##
   ##    ##   ##   #########  ##  ##  ####
   ##    ##    ##  ##     ##  ##  ##   ###
   ##    ##     ## ##     ## #### ##    ##

######## ########  ######  ########
   ##    ##       ##    ##    ##
   ##    ##       ##          ##
   ##    ######    ######     ##
   ##    ##             ##    ##
   ##    ##       ##    ##    ##
   ##    ########  ######     ##

######## ##     ##    ###    ##
##       ##     ##   ## ##   ##
##       ##     ##  ##   ##  ##
######   ##     ## ##     ## ##
##        ##   ##  ######### ##
##         ## ##   ##     ## ##
########    ###    ##     ## ########
# //--------------------------------------------
# //--------------------------------------------
# //--------------------------------------------

def Train_Test_Eval_PureKeras(_signal, bkg_type, var_list, cuts, _nepochs, _batchSize, _nof_outputs, _maxEvents, _splitTrainEventFrac):

    print('\n\n')
    print(colors.bg.orange, colors.bold, "=====================================", colors.reset)
    print('\t', colors.fg.orange, colors.bold, "DNN Training", colors.reset)
    print(colors.bg.orange, colors.bold, "=====================================", colors.reset, '\n\n')

    #Get data
    print(colors.fg.lightblue, "--- Read and shape the data...", colors.reset); print('\n')
    x_train, y_train, x_test, y_test, weightPHY_train, weightPHY_test, weightLEARN_train, weightLEARN_test, x, y, weightPHY, weightLEARN = Get_Data_Keras(ntuples_dir, _signal, bkg_type, var_list, cuts, _nof_outputs, _maxEvents, _splitTrainEventFrac)

    #Get model, compile
    print('\n'); print(colors.fg.lightblue, "--- Create the Keras model...", colors.reset); print('\n')
    model = Create_Model(weight_dir, "DNN"+bkg_type, _nof_outputs, var_list)

    #-- Can access weights and biases of any layer (debug, ...) #Print before training
    # weights_layer, biases_layer = model.layers[0].get_weights()
    # print(weights_layer.shape)
    # print(biases_layer.shape)
    # print(weights_layer)
    # print(biases_layer[0:2])

    print('\n'); print(colors.fg.lightblue, "--- Define the loss function & metrics...", colors.reset); print('\n')
    _loss, _optim, _metrics = Get_Loss_Optim_Metrics(_nof_outputs)

    print('\n'); print(colors.fg.lightblue, "--- Compile the Keras model...", colors.reset); print('\n')
    model.compile(loss=_loss, optimizer=_optim, metrics=[_metrics]) #For multiclass classification

    callbacks_list = Get_Callbacks(weight_dir)

    #-- KFOLD TEST
    # define 10-fold cross validation test harness
    # kfold = StratifiedKFold(n_splits=10, shuffle=True, random_state=7)
    # cvscores = []
    # for train, test in kfold.split(x, y):
    #     model = None
    #     model = Create_Model(outdir, "DNN"+bkg_type, _nof_outputs)
    #     model.compile(loss=_loss, optimizer=_optim, metrics=[_metrics])
    #     model.fit(x[train], y[train], validation_data=(x[test], y[test]), epochs=_nepochs, batch_size=_batchSize, callbacks=callbacks_list)
    #     score = model.evaluate(x[test], y[test], batch_size=_batchSize)
    #     # print("%s: %.2f%%" % (model.metrics_names[1], scores[1]*100))
    #     cvscores.append(score[1] * 100)
    # print("%.2f%% (+/- %.2f%%)" % (numpy.mean(cvscores), numpy.std(cvscores)))
    # exit(1)

    #Fit model
    print('\n'); print(colors.fg.lightblue, "--- Fit...", colors.reset, " (may take some time)"); print('\n')
    history = model.fit(x_train, y_train, validation_data=(x_test, y_test), epochs=_nepochs, batch_size=_batchSize, sample_weight=weightLEARN_train, callbacks=callbacks_list, shuffle=True)

    #Print weights after training
    # weights_layer, biases_layer = model.layers[0].get_weights()
    # print(weights_layer)

    # Evaluate the model (metrics)
    print('\n'); print(colors.fg.lightblue, "--- Evaluate...", colors.reset); print('\n')
    score = model.evaluate(x_test, y_test, batch_size=_batchSize, sample_weight=weightPHY_test)
    # print(score)


  ####    ##   #    # ######    #    #  ####  #####  ###### #
 #       #  #  #    # #         ##  ## #    # #    # #      #
  ####  #    # #    # #####     # ## # #    # #    # #####  #
      # ###### #    # #         #    # #    # #    # #      #
 #    # #    #  #  #  #         #    # #    # #    # #      #
  ####  #    #   ##   ######    #    #  ####  #####  ###### ######

    print('\n'); print(colors.fg.lightblue, "--- Save model...", colors.reset);

    outname = weight_dir + 'model_DNN'+bkg_type

    #Serialize model to HDF5
    model.save(outname+'.h5')
    # model.save_weights(outname+'.h5')
    # print("Saved model to disk")

    # Save the model architecture
    with open(weight_dir + 'arch_DNN'+bkg_type+'_all.json', 'w') as json_file:
        json_file.write(model.to_json())

    #Save list of variables
    Write_Variables_To_TextFile(weight_dir, var_list)


 ###### #####  ###### ###### ###### ######     ####  #####    ##   #####  #    #
 #      #    # #      #          #  #         #    # #    #  #  #  #    # #    #
 #####  #    # #####  #####     #   #####     #      #    # #    # #    # ######
 #      #####  #      #        #    #         #  ### #####  ###### #####  #    #
 #      #   #  #      #       #     #         #    # #   #  #    # #      #    #
 #      #    # ###### ###### ###### ######     ####  #    # #    # #      #    #

# --- convert model to estimator and save model as frozen graph for c++

    # tensorflow.compat.v1.keras.backend.clear_session() #Closing the last session avoids that node names get a suffix appened when opening a new session #Does not work?

    # sess = tensorflow.compat.v1.keras.backend.get_session()
    # graph = sess.graph

    print('\n'); print(colors.fg.lightblue, "--- Freeze graph...", colors.reset); print('\n')

    with tensorflow.compat.v1.Session() as sess: #Must first open a new session #Can't manage to run code below without this... (why?)

        tensorflow.keras.backend.set_learning_phase(0) # This line must be executed before loading Keras model (why?)
        model = load_model(weight_dir + "model_DNN"+bkg_type+".h5") # model has to be re-loaded

        inputs_names = [input.op.name for input in model.inputs]
        outputs_names = [output.op.name for output in model.outputs]
        # print('\ninputs: ', model.inputs)
        print('\n')
        print(colors.fg.lightgrey, '--> inputs_names: ', inputs_names, colors.reset, '\n')
        # print('\noutputs: ', model.outputs)
        print(colors.fg.lightgrey, '--> outputs_names: ', outputs_names, colors.reset, '\n')
        # tf_node_list = [n.name for n in  tensorflow.compat.v1.get_default_graph().as_graph_def().node]
        # print('nodes list : ', tf_node_list)

        frozen_graph = freeze_session(sess, output_names=[output.op.name for output in model.outputs])

        tensorflow.io.write_graph(frozen_graph, '../weights/DNN', 'model.pbtxt', as_text=True)
        tensorflow.io.write_graph(frozen_graph, '../weights/DNN', 'model.pb', as_text=False)
        # print("\n===> Successfully froze graph...\n\n")
        print('\n'); print(colors.fg.lightgrey, '===> Successfully froze graph...', colors.reset, '\n')


 #####  ######  ####  #    # #      #####  ####
 #    # #      #      #    # #        #   #
 #    # #####   ####  #    # #        #    ####
 #####  #           # #    # #        #        #
 #   #  #      #    # #    # #        #   #    #
 #    # ######  ####   ####  ######   #    ####

        print('\n\n')
        print(colors.bg.orange, colors.bold, "##############################################", colors.reset)
        print(colors.fg.orange, '\t Results & Control Plots', colors.reset)
        print(colors.bg.orange, colors.bold, "##############################################", colors.reset, '\n')

        loss = score[0]
        accuracy = score[1]
        print(colors.fg.lightgrey, '** Accuracy :', str(accuracy), colors.reset)
        print(colors.fg.lightgrey, '** Loss', str(loss), colors.reset)

        nEvents_train, tmp = y_train.shape
        nEvents_test, tmp = y_test.shape

        # with tensorflow.compat.v1.Session() as sess: #Must first open a new session

        auc_score = roc_auc_score(y_test, model.predict(x_test))
        auc_score_train = roc_auc_score(y_train, model.predict(x_train))
        # print("\n*** AUC scores ***")
        # print("-- TEST SAMPLE  \t(" + str(nEvents_test) + " events) \t\t==> " + str(auc_score) )
        # print("-- TRAIN SAMPLE \t(" + str(nEvents_train) + " events) \t==> " + str(auc_score_train) + "\n\n")
        print('\n'); print(colors.fg.lightgrey, '**** AUC scores ****', colors.reset)
        print(colors.fg.lightgrey, "-- TEST SAMPLE  \t(" + str(nEvents_test) + " events) \t\t==> " + str(auc_score), colors.reset)
        print(colors.fg.lightgrey, "-- TRAIN SAMPLE \t(" + str(nEvents_train) + " events) \t==> " + str(auc_score_train), colors.reset); print('\n')

        predictions_train_sig, predictions_train_bkg, predictions_test_sig, predictions_test_bkg, weightLEARN_sig, weightLEARN_bkg, weight_test_sig, weight_test_bkg = Apply_Model_toTrainTestData(_signal, bkg_type, var_list, cuts, _nepochs, _batchSize, _nof_outputs, x_train, y_train, x_test, y_test, weightPHY_train, weightPHY_test)

        Create_TrainTest_ROC_Histos(predictions_train_sig, predictions_train_bkg, predictions_test_sig, predictions_test_bkg, weightLEARN_sig, weightLEARN_bkg, weight_test_sig, weight_test_bkg, _metrics, bkg_type)

        Create_Control_Plots(predictions_train_sig, predictions_train_bkg, predictions_test_sig, predictions_test_bkg, weightLEARN_sig, weightLEARN_bkg, weight_test_sig, weight_test_bkg, y_test, x_test, x_train, y_train, model, history, _metrics, _nof_outputs, weight_dir, bkg_type)

    #End [with ... as sess]
# //--------------------------------------------
# //--------------------------------------------













# //--------------------------------------------
# //--------------------------------------------
# //--------------------------------------------
   ###    ########  ########  ##       ##    ##
  ## ##   ##     ## ##     ## ##        ##  ##
 ##   ##  ##     ## ##     ## ##         ####
##     ## ########  ########  ##          ##
######### ##        ##        ##          ##
##     ## ##        ##        ##          ##
##     ## ##        ##        ########    ##

##     ##  #######  ########  ######## ##
###   ### ##     ## ##     ## ##       ##
#### #### ##     ## ##     ## ##       ##
## ### ## ##     ## ##     ## ######   ##
##     ## ##     ## ##     ## ##       ##
##     ## ##     ## ##     ## ##       ##
##     ##  #######  ########  ######## ########
# //--------------------------------------------
# //--------------------------------------------
# //--------------------------------------------

def Apply_Model_toTrainTestData(_signal, bkg_type, var_list, cuts, _nepochs, _batchSize, _nof_outputs, x_train, y_train, x_test, y_test, weightLEARN, weight_test):

    print(colors.fg.lightblue, "--- Apply model to train & test data...", colors.reset); print('\n')

    #Get data #CHANGED -- directly taken as arg
    # x_train, y_train, x_test, y_test, weightLEARN, weight_test = Get_Data_Keras(signal, bkg_type, var_list, cuts, _nof_outputs)

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


  ####  ###### #####
 #    # #        #
 #      #####    #
 #  ### #        #
 #    # #        #
  ####  ######   #

 #####  #####  ###### #####  #  ####  ##### #  ####  #    #  ####
 #    # #    # #      #    # # #    #   #   # #    # ##   # #
 #    # #    # #####  #    # # #        #   # #    # # #  #  ####
 #####  #####  #      #    # # #        #   # #    # #  # #      #
 #      #   #  #      #    # # #    #   #   # #    # #   ## #    #
 #      #    # ###### #####  #  ####    #   #  ####  #    #  ####

    #Application (can also use : predict_classes, predict_proba)
    #Example : predictions_test_sig[5,0] represents the value for output node 0, test event number 5
    predictions_test_sig = model.predict(x_test_sig)
    predictions_test_bkg = model.predict(x_test_bkg)
    predictions_train_sig = model.predict(x_train_sig)
    predictions_train_bkg = model.predict(x_train_bkg)

    # print(x_test.shape)
    # print(predictions_test_sig.shape)

    #-- Printout of some results
    for i in range(5):
        if y_test[i][0]==1:
            true_label = "Signal"
        else:
            true_label = "Background"
        print("--------------")
        print("X=%s\n=====> Predicted : %s (True label: %s)" % (x_test[i], predictions_test_sig[i,0], true_label)) #For few events : prints values of all inputs variables (rescaled), predicted first output node value, and corresponding class label
        print("--------------\n")

    #Only keep signal proba (redundant info)
    predictions_test_sig = predictions_test_sig[:, 0]
    predictions_test_bkg = predictions_test_bkg[:, 0]
    predictions_train_sig = predictions_train_sig[:, 0]
    predictions_train_bkg = predictions_train_bkg[:, 0]

    return predictions_train_sig, predictions_train_bkg, predictions_test_sig, predictions_test_bkg, weightLEARN_sig, weightLEARN_bkg, weight_test_sig, weight_test_bkg






















# //--------------------------------------------
# //--------------------------------------------
# //--------------------------------------------
# //--------------------------------------------
######## ##     ## ##    ##  ######  ######## ####  #######  ##    ##
##       ##     ## ###   ## ##    ##    ##     ##  ##     ## ###   ##
##       ##     ## ####  ## ##          ##     ##  ##     ## ####  ##
######   ##     ## ## ## ## ##          ##     ##  ##     ## ## ## ##
##       ##     ## ##  #### ##          ##     ##  ##     ## ##  ####
##       ##     ## ##   ### ##    ##    ##     ##  ##     ## ##   ###
##        #######  ##    ##  ######     ##    ####  #######  ##    ##


 ######     ###    ##       ##        ######
##    ##   ## ##   ##       ##       ##    ##
##        ##   ##  ##       ##       ##
##       ##     ## ##       ##        ######
##       ######### ##       ##             ##
##    ## ##     ## ##       ##       ##    ##
 ######  ##     ## ######## ########  ######
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

    # xxxTrain_Test_Eval_PureKeras(bkg_type, var_list, cuts, _nepochs, _batchSize, _nof_outputs)
    # exit(1)


#----------  Manual call to DNN training function
Train_Test_Eval_PureKeras(_signal, bkg_type, var_list, cuts, _nepochs, _batchSize, _nof_outputs, _maxEvents, _splitTrainEventFrac)
