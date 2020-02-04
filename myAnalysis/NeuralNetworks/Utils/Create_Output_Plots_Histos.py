#Create control plots, ROC histos, etc.

import ROOT
from ROOT import TMVA, TFile, TTree, TCut, gROOT, TH1, TH1F
import numpy as np
import keras
from sklearn.metrics import roc_curve, auc, roc_auc_score
import matplotlib.pyplot as plt
from root_numpy import fill_hist
from Utils.Helper import close_event

from Utils.ColoredPrintout import colors




# //--------------------------------------------
# //--------------------------------------------
# //--------------------------------------------
########   #######   ######     ##     ## ####  ######  ########  #######   ######
##     ## ##     ## ##    ##    ##     ##  ##  ##    ##    ##    ##     ## ##    ##
##     ## ##     ## ##          ##     ##  ##  ##          ##    ##     ## ##
########  ##     ## ##          #########  ##   ######     ##    ##     ##  ######
##   ##   ##     ## ##          ##     ##  ##        ##    ##    ##     ##       ##
##    ##  ##     ## ##    ##    ##     ##  ##  ##    ##    ##    ##     ## ##    ##
##     ##  #######   ######     ##     ## ####  ######     ##     #######   ######
# //--------------------------------------------
# //--------------------------------------------
# //--------------------------------------------

def Create_TrainTest_ROC_Histos(predictions_train_sig, predictions_train_bkg, predictions_test_sig, predictions_test_bkg, weightLEARN_sig, weightLEARN_bkg, weight_test_sig, weight_test_bkg, _metrics):

    print(colors.fg.lightblue, "--- Create & store ROC histos...", colors.reset); print('\n')

    # Fill a ROOT histogram from a NumPy array
    rootfile_outname = "../outputs/PredictKeras_DNN.root"
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











# //--------------------------------------------
# //--------------------------------------------
# //--------------------------------------------
 ######   #######  ##    ## ######## ########   #######  ##          ########  ##        #######  ########  ######
##    ## ##     ## ###   ##    ##    ##     ## ##     ## ##          ##     ## ##       ##     ##    ##    ##    ##
##       ##     ## ####  ##    ##    ##     ## ##     ## ##          ##     ## ##       ##     ##    ##    ##
##       ##     ## ## ## ##    ##    ########  ##     ## ##          ########  ##       ##     ##    ##     ######
##       ##     ## ##  ####    ##    ##   ##   ##     ## ##          ##        ##       ##     ##    ##          ##
##    ## ##     ## ##   ###    ##    ##    ##  ##     ## ##          ##        ##       ##     ##    ##    ##    ##
 ######   #######  ##    ##    ##    ##     ##  #######  ########    ##        ########  #######     ##     ######
# //--------------------------------------------
# //--------------------------------------------
# //--------------------------------------------

def Create_Control_Plots(predictions_train_sig, predictions_train_bkg, predictions_test_sig, predictions_test_bkg, weightLEARN_sig, weightLEARN_bkg, weight_test_sig, weight_test_bkg, y_test, x_test, x_train, y_train, model, history, _metrics, _nof_outputs, weight_dir):

    print('\n'); print(colors.fg.lightblue, "--- Create control plots...", colors.reset); print('\n')

    #Get ROC curve using test data -- different for _nof_outputs>1, should fix it
    #Uses predict() function, which generates (output) given (input + model)
    if _nof_outputs == 1:
        lw = 2 #linewidth
        fpr, tpr, _ = roc_curve(y_test, model.predict(x_test)) #Need '_' to read all the return values
        roc_auc = auc(fpr, tpr)
        fpr_train, tpr_train, _ = roc_curve(y_train, model.predict(x_train)) #Need '_' to read all the return values
        roc_auc_train = auc(fpr_train, tpr_train)

        plt.plot(tpr, 1-fpr, color='darkorange', lw=lw, label='ROC DNN (test) (AUC = {0:0.2f})' ''.format(roc_auc))
        plt.plot(tpr_train, 1-fpr_train, color='cornflowerblue', lw=lw, label='ROC DNN (train) (AUC = {0:0.2f})' ''.format(roc_auc_train))

        # plt.plot(tpr, 1-fpr,color='darkorange',label='ROC curve (area = %0.2f)' % roc_auc)
        # plt.legend(loc="lower right")
        # plt.xlabel('True Positive Rate')
        # plt.ylabel('False Positive Rate')
        # plt.show()

    else: #different for multiclass
        lw = 2 #linewidth
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

            plt.plot(tpr[0], 1-fpr[0], color='darkorange', lw=lw, label='ROC DNN (test) (AUC = {1:0.2f})' ''.format(i, roc_auc[i]))
            plt.plot(tpr_train[0], 1-fpr_train[0], color='cornflowerblue', lw=lw, label='ROC DNN (train) (AUC = {1:0.2f})' ''.format(i, roc_auc_train[i]))

    # Plot ROC curves
    fig1 = plt.figure(1)
    ax = fig1.gca()
    ax.set_xticks(np.arange(0, 1, 0.1))
    ax.set_yticks(np.arange(0, 1., 0.1))
    plt.grid()

    plt.plot([1, 0], [0, 1], 'k--', lw=lw)
    plt.xlim([0.0, 1.0])
    plt.ylim([0.0, 1.0])
    plt.xlabel('Signal efficiency')
    plt.ylabel('Background rejection')
    plt.title('')
    plt.legend(loc="lower left")

    #Display plot in terminal for quick check
    timer = fig1.canvas.new_timer(interval = 3000) #creating a timer object and setting an interval of N milliseconds
    timer.add_callback(close_event)
    timer.start()
    plt.show()
    plotname = weight_dir + 'ROC_DNN.png'
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
    plotname = weight_dir + 'Loss_DNN.png'
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
    plotname = weight_dir + 'Accuracy_DNN.png'
    fig3.savefig(plotname)
    print("Saved Accuracy plot as : " + plotname)

# //--------------------------------------------

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

    plotname = weight_dir + 'Overtraining_DNN.png'
    fig4.savefig(plotname)
    print("Saved Overtraining plot as : " + plotname)
