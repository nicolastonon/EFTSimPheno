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
import warnings
warnings.filterwarnings("ignore", message="tensorflow:sample_weight modes were coerced")

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
from ann_visualizer.visualize import ann_viz;
from sklearn.metrics import roc_curve, auc, roc_auc_score
from tensorflow.keras.models import load_model
import matplotlib.pyplot as plt
from root_numpy import fill_hist

from Utils.FreezeSession import freeze_session
from Utils.Helper import close_event, batchOutput, Write_Variables_To_TextFile, TimeHistory
from Utils.CreateModel import Create_Model
from Utils.GetCallbacks import Get_Callbacks
from Utils.GetData import Get_Data_Keras
from Utils.GetOptimizer import Get_Loss_Optim_Metrics
from Utils.ColoredPrintout import colors

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
######## ########     ###    #### ##    ##          ##    ######## ########  ######  ########
   ##    ##     ##   ## ##    ##  ###   ##         ##        ##    ##       ##    ##    ##
   ##    ##     ##  ##   ##   ##  ####  ##        ##         ##    ##       ##          ##
   ##    ########  ##     ##  ##  ## ## ##       ##          ##    ######    ######     ##
   ##    ##   ##   #########  ##  ##  ####      ##           ##    ##             ##    ##
   ##    ##    ##  ##     ##  ##  ##   ###     ##            ##    ##       ##    ##    ##
   ##    ##     ## ##     ## #### ##    ##    ##             ##    ########  ######     ##
# //--------------------------------------------
# //--------------------------------------------
# //--------------------------------------------

def Train_Test_Eval_PureKeras(bkg_type, var_list, cuts, _nepochs, _batchSize, _nof_outputs, _maxEvents, _splitTrainEventFrac):

    print('\n\n')
    print(colors.bg.orange, colors.bold, "=====================================", colors.reset)
    print('\t', colors.bg.orange, colors.bold, "DNN Training", colors.reset)
    print(colors.bg.orange, colors.bold, "=====================================", colors.reset, '\n\n')

    signal = "tZq"

    #Get data
    print(colors.fg.lightblue, "--- Read and shape the data...", colors.reset); print('\n')
    x_train, y_train, x_test, y_test, weightPHY_train, weightPHY_test, weightLEARN_train, weightLEARN_test, x, y, weightPHY, weightLEARN = Get_Data_Keras(ntuples_dir, signal, bkg_type, var_list, cuts, _nof_outputs, _maxEvents, _splitTrainEventFrac)

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
    print('\n'); print(colors.fg.lightblue, "--- Fit... (may take some time)", colors.reset); print('\n')
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
        print(colors.fg.lightgrey, '--> inputs_names: ', inputs_names, colors.reset, '\n')
        # print('\noutputs: ', model.outputs)
        print(colors.fg.lightgrey, '--> outputs_names: ', outputs_names, colors.reset, '\n')
        # tf_node_list = [n.name for n in  tensorflow.compat.v1.get_default_graph().as_graph_def().node]
        # print('nodes list : ', tf_node_list)

        frozen_graph = freeze_session(sess, output_names=[output.op.name for output in model.outputs])

        tensorflow.io.write_graph(frozen_graph, '../weights/DNN', 'model.pbtxt', as_text=True)
        tensorflow.io.write_graph(frozen_graph, '../weights/DNN', 'model.pb', as_text=False)
        # print("\n===> Successfully froze graph...\n\n")
        print(colors.fg.lightgrey, '===> Successfully froze graph...', colors.reset, '\n')

 #####  ######  ####  #    # #      #####  ####
 #    # #      #      #    # #        #   #
 #    # #####   ####  #    # #        #    ####
 #####  #           # #    # #        #        #
 #   #  #      #    # #    # #        #   #    #
 #    # ######  ####   ####  ######   #    ####

        print('\n\n')
        print(colors.bg.orange, colors.bold, "##############################################", colors.reset)
        print('\t', colors.bg.orange, "Results & Control Plots", colors.reset)
        print(colors.bg.orange, colors.bold, "##############################################", colors.reset, '\n')

        loss = score[0]
        accuracy = score[1]
        print(colors.fg.lightgrey, '** Accuracy :', str(accuracy), colors.reset)
        print(colors.fg.lightgrey, '** Loss', str(loss), colors.reset)

        show_control_plots = True

        if show_control_plots==True:
            #Create some more control plots for quick checking
            # print("\n\n\n\n########################")
            # print("## Results & Control Plots ##")
            # print("########################")

            # nEvents_train = y_train.shape
            # nEvents_test = y_test.shape
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

            predictions_train_sig, predictions_train_bkg, predictions_test_sig, predictions_test_bkg, weightLEARN_sig, weightLEARN_bkg, weight_test_sig, weight_test_bkg = Apply_Model(bkg_type, var_list, cuts, _nepochs, _batchSize, _nof_outputs, x_train, y_train, x_test, y_test, weightPHY_train, weightPHY_test)

            # Fill a ROOT histogram from a NumPy array
            rootfile_outname = "../outputs/PredictKeras_DNN"+bkg_type+".root"
            fout = ROOT.TFile(rootfile_outname, "RECREATE")

            # print("last : ")
            # print(weightLEARN_sig[0:5])

            #Write histograms with reweighting
            # hist_train_sig = TH1F('hist_train_sig', '', 100, -1, 1)
            hist_train_sig = TH1F('hist_train_sig', '', 1000, 0, 1)
            fill_hist(hist_train_sig, predictions_train_sig, weights=weightLEARN_sig)
            # hist_test_sig.Draw('hist')
            hist_train_sig.Write()

            # hist_train_bkg = TH1F('hist_train_bkg', '', 100, -1, 1)
            hist_train_bkg = TH1F('hist_train_bkg', '', 1000, 0, 1)
            fill_hist(hist_train_bkg, predictions_train_bkg, weights=weightLEARN_bkg)
            # hist_train_bkg.Draw('hist')
            hist_train_bkg.Write()

            # hist_test_sig = TH1F('hist_test_sig', '', 100, -1, 1)
            hist_test_sig = TH1F('hist_test_sig', '', 1000, 0, 1)
            fill_hist(hist_test_sig, predictions_test_sig, weights=weight_test_sig)
            # hist_test_sig.Draw('hist')
            hist_test_sig.Write()

            # hist_test_bkg = TH1F('hist_test_bkg', '', 100, -1, 1)
            hist_test_bkg = TH1F('hist_test_bkg', '', 1000, 0, 1)
            fill_hist(hist_test_bkg, predictions_test_bkg, weights=weight_test_bkg)
            # hist_test_bkg.Draw('hist')
            hist_test_bkg.Write()

            #Also write histos witought reweighting
            # hist_train_sig_noReweight = TH1F('hist_train_sig_noReweight', '', 100, -1, 1)
            hist_train_sig_noReweight = TH1F('hist_train_sig_noReweight', '', 1000, 0, 1)
            fill_hist(hist_train_sig_noReweight, predictions_train_sig)
            # hist_train_sig_noReweight.Draw('hist')
            hist_train_sig_noReweight.Write()

            # hist_train_bkg_noReweight = TH1F('hist_train_bkg_noReweight', '', 100, -1, 1)
            hist_train_bkg_noReweight = TH1F('hist_train_bkg_noReweight', '', 1000, 0, 1)
            fill_hist(hist_train_bkg_noReweight, predictions_train_bkg)
            # hist_train_bkg_noReweight.Draw('hist')
            hist_train_bkg_noReweight.Write()

            # hist_test_sig_noReweight = TH1F('hist_test_sig_noReweight', '', 100, -1, 1)
            hist_test_sig_noReweight = TH1F('hist_test_sig_noReweight', '', 1000, 0, 1)
            fill_hist(hist_test_sig_noReweight, predictions_test_sig)
            # hist_test_sig_noReweight.Draw('hist')
            hist_test_sig_noReweight.Write()

            # hist_test_bkg_noReweight = TH1F('hist_test_bkg_noReweight', '', 100, -1, 1)
            hist_test_bkg_noReweight = TH1F('hist_test_bkg_noReweight', '', 1000, 0, 1)
            fill_hist(hist_test_bkg_noReweight, predictions_test_bkg)
            # hist_test_bkg_noReweight.Draw('hist')
            hist_test_bkg_noReweight.Write()

            fout.Close()
            print("Saved output ROOT file containing Keras Predictions as histograms : " + rootfile_outname)

            #Get ROC curve using test data -- different for _nof_outputs>1, should fix it
            #Uses predict() function, which generates (output) given (input + model)
            if _nof_outputs == 1:
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

                plt.plot(tpr[0], 1-fpr[0], color='darkorange', lw=lw, label='ROC DNN (test) (AUC = {1:0.2f})' ''.format(i, roc_auc[i]))
                plt.plot(tpr_train[0], 1-fpr_train[0], color='cornflowerblue', lw=lw, label='ROC DNN (train) (AUC = {1:0.2f})' ''.format(i, roc_auc_train[i]))

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
            plt.plot(history.history[_metrics]) #metrics name
            plt.plot(history.history['val_'+_metrics])
            # plt.plot(history.history['acc'])
            # plt.plot(history.history['val_acc'])
            plt.title('Accuracy VS Epoch')
            plt.ylabel('Accuracy')
            plt.xlabel('Epoch')
            plt.legend(['Train', 'Test'], loc='lower right')
            timer.start()
            plt.show()
            plotname = weight_dir + 'Accuracy_DNN'+bkg_type+'.png'
            fig3.savefig(plotname)
            print("Saved Accuracy plot as : " + plotname)
        #--- End [if show_control_plots==True]
    #End [with ... as sess]
# //--------------------------------------------
# //--------------------------------------------
























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
# //--------------------------------------------
# //--------------------------------------------

def Apply_Model(bkg_type, var_list, cuts, _nepochs, _batchSize, _nof_outputs, x_train, y_train, x_test, y_test, weightLEARN, weight_test):

    signal = "tZq"

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


 #####  #####  ###### #####  #  ####  #####
 #    # #    # #      #    # # #    #   #
 #    # #    # #####  #    # # #        #
 #####  #####  #      #    # # #        #
 #      #   #  #      #    # # #    #   #
 #      #    # ###### #####  #  ####    #

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

    # Train_Test_Eval_PureKeras(bkg_type, var_list, cuts, _nepochs, _batchSize, _nof_outputs)
    # exit(1)


#----------  Manual call to DNN training function
Train_Test_Eval_PureKeras(bkg_type, var_list, cuts, _nepochs, _batchSize, _nof_outputs, _maxEvents, _splitTrainEventFrac)
