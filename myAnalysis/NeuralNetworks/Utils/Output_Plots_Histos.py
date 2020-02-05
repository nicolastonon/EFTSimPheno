#Create control plots, ROC histos, etc.

import ROOT
from ROOT import TMVA, TFile, TTree, TCut, gROOT, TH1, TH1F
import numpy as np
import keras
import math
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

def Create_TrainTest_ROC_Histos(signal, lumiName, predictions_train_sig, predictions_train_bkg, predictions_test_sig, predictions_test_bkg, weightPHY_train_sig, weightPHY_train_bkg, weightPHY_test_sig, weightPHY_test_bkg, metrics):

    print(colors.fg.lightblue, "--- Create & store ROC histos...", colors.reset); print('\n')

#FIXME

# Fill a ROOT histogram from NumPy arrays
    rootfile_outname = "../outputs/DNN_"+signal+"_"+lumiName+".root"
    fout = ROOT.TFile(rootfile_outname, "RECREATE")

    #Write histograms with reweighting
    hist_train_sig = TH1F('hist_train_sig', '', 1000, 0, 1); hist_train_sig.Sumw2()
    fill_hist(hist_train_sig, predictions_train_sig, weights=weightPHY_train_sig)
    # hist_test_sig.Draw('hist')
    hist_train_sig.Write()

    hist_train_bkg = TH1F('hist_train_bkg', '', 1000, 0, 1); hist_train_bkg.Sumw2()
    fill_hist(hist_train_bkg, predictions_train_bkg, weights=weightPHY_train_bkg)
    # hist_train_bkg.Draw('hist')
    hist_train_bkg.Write()

    hist_test_sig = TH1F('hist_test_sig', '', 1000, 0, 1); hist_test_sig.Sumw2()
    fill_hist(hist_test_sig, predictions_test_sig, weights=weightPHY_test_sig)
    # hist_test_sig.Draw('hist')
    hist_test_sig.Write()

    hist_test_bkg = TH1F('hist_test_bkg', '', 1000, 0, 1); hist_test_bkg.Sumw2()
    fill_hist(hist_test_bkg, predictions_test_bkg, weights=weightPHY_test_bkg)
    # hist_test_bkg.Draw('hist')
    hist_test_bkg.Write()

#Also write histos witought reweighting
    hist_train_sig_noReweight = TH1F('hist_train_sig_noReweight', '', 1000, 0, 1); hist_train_sig_noReweight.Sumw2()
    fill_hist(hist_train_sig_noReweight, predictions_train_sig)
    # hist_train_sig_noReweight.Draw('hist')
    hist_train_sig_noReweight.Write()

    hist_train_bkg_noReweight = TH1F('hist_train_bkg_noReweight', '', 1000, 0, 1); hist_train_bkg_noReweight.Sumw2()
    fill_hist(hist_train_bkg_noReweight, predictions_train_bkg)
    # hist_train_bkg_noReweight.Draw('hist')
    hist_train_bkg_noReweight.Write()

    hist_test_sig_noReweight = TH1F('hist_test_sig_noReweight', '', 1000, 0, 1); hist_test_sig_noReweight.Sumw2()
    fill_hist(hist_test_sig_noReweight, predictions_test_sig)
    # hist_test_sig_noReweight.Draw('hist')
    hist_test_sig_noReweight.Write()

    hist_test_bkg_noReweight = TH1F('hist_test_bkg_noReweight', '', 1000, 0, 1); hist_test_bkg_noReweight.Sumw2()
    fill_hist(hist_test_bkg_noReweight, predictions_test_bkg)
    # hist_test_bkg_noReweight.Draw('hist')
    hist_test_bkg_noReweight.Write()

    fout.Close()
    # print("Saved output ROOT file containing Keras Predictions as histograms : " + rootfile_outname)
    print(colors.fg.lightgrey, "Saved output ROOT file containing Keras Predictions as histograms :", colors.reset, rootfile_outname, '\n')










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

def Create_Control_Plots(predictions_train_sig, predictions_train_bkg, predictions_test_sig, predictions_test_bkg, weightPHY_train_sig, weightPHY_train_bkg, weightPHY_test_sig, weightPHY_test_bkg, y_test, x_test, x_train, y_train, model, history, metrics, _nof_outputs, weight_dir):

    print('\n'); print(colors.fg.lightblue, "--- Create control plots...", colors.reset); print('\n')

 #####   ####   ####
 #    # #    # #    #
 #    # #    # #
 #####  #    # #
 #   #  #    # #    #
 #    #  ####   ####

    #Get ROC curve using test data -- different for _nof_outputs>1, should fix it
    #Uses predict() function, which generates (output) given (input + model)
    lw = 2 #linewidth
    if _nof_outputs == 1:
        fpr, tpr, _ = roc_curve(y_test, model.predict(x_test)) #Need '_' to read all the return values
        roc_auc = auc(fpr, tpr)
        fpr_train, tpr_train, _ = roc_curve(y_train, model.predict(x_train)) #Need '_' to read all the return values
        roc_auc_train = auc(fpr_train, tpr_train)

        plt.plot(tpr, 1-fpr, color='darkorange', lw=lw, label='ROC DNN (test) (AUC = {0:0.2f})' ''.format(roc_auc))
        plt.plot(tpr_train, 1-fpr_train, color='cornflowerblue', lw=lw, label='ROC DNN (train) (AUC = {0:0.2f})' ''.format(roc_auc_train))

    else: #different for multiclass
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

        plt.plot(tpr_train[0], 1-fpr_train[0], color='darkorange', lw=lw, label='ROC DNN (train) (AUC = {1:0.2f})' ''.format(0, roc_auc_train[0]))
        plt.plot(tpr[0], 1-fpr[0], color='cornflowerblue', lw=lw, label='ROC DNN (test) (AUC = {1:0.2f})' ''.format(0, roc_auc[0]))

    # Plot ROC curves
    fig1 = plt.figure(1)
    timer = fig1.canvas.new_timer(interval = 2000) #creating a timer object and setting an interval of N milliseconds
    timer.add_callback(close_event)

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
    timer.start()
    plt.show()
    plotname = weight_dir + 'ROC_DNN.png'
    fig1.savefig(plotname)
    # print("Saved ROC plot as : " + plotname)
    print(colors.fg.lightgrey, "Saved ROC plot as :", colors.reset, plotname, '\n')

 #       ####   ####   ####
 #      #    # #      #
 #      #    #  ####   ####
 #      #    #      #      #
 #      #    # #    # #    #
 ######  ####   ####   ####

    # Plotting the loss with the number of iterations
    fig2 = plt.figure(2)
    ax1 = fig2.gca()
    timer = fig2.canvas.new_timer(interval = 2000) #creating a timer object and setting an interval of N milliseconds
    timer.add_callback(close_event)
    plt.plot(history.history['loss'], color='darkorange')
    plt.plot(history.history['val_loss'], color='cornflowerblue')
    # plt.plot(history.history['lr'], color='dimgrey')
    plt.title('Loss VS Epoch')
    plt.ylabel('Loss')
    plt.xlabel('Epoch')
    plt.legend(['Train', 'Test', 'lr'], loc='upper right')
    ax2 = ax1.twinx()  # instantiate a second axes that shares the same x-axis
    ax2.set_ylabel('lr')  # we already handled the x-label with ax1
    ax2.tick_params(axis='y', color='dimgrey')
    ax2.plot(history.history['lr'], color='dimgrey', linestyle='--')
    timer.start()
    plt.show()
    plotname = weight_dir + 'Loss_DNN.png'
    fig2.savefig(plotname)
    # print("Saved Loss plot as : " + plotname)
    print(colors.fg.lightgrey, "Saved Loss plot as :", colors.reset, plotname, '\n')

   ##    ####   ####  #    # #####    ##    ####  #   #
  #  #  #    # #    # #    # #    #  #  #  #    #  # #
 #    # #      #      #    # #    # #    # #        #
 ###### #      #      #    # #####  ###### #        #
 #    # #    # #    # #    # #   #  #    # #    #   #
 #    #  ####   ####   ####  #    # #    #  ####    #

    # Plotting the error with the number of iterations
    fig3 = plt.figure(3)
    ax1 = fig3.gca()
    timer = fig3.canvas.new_timer(interval = 2000) #creating a timer object and setting an interval of N milliseconds
    timer.add_callback(close_event)
    plt.plot(history.history[metrics], color='darkorange') #metrics name
    plt.plot(history.history['val_'+metrics], color='cornflowerblue')
    # plt.plot(history.history['lr'], color='dimgrey')
    # plt.plot(history.history['acc'])
    # plt.plot(history.history['val_acc'])
    plt.title('Accuracy VS Epoch')
    plt.ylabel('Accuracy')
    plt.xlabel('Epoch')
    plt.legend(['Train', 'Test', 'lr'], loc='lower right')
    ax2 = ax1.twinx()  # instantiate a second axes that shares the same x-axis
    ax2.set_ylabel('lr')  # we already handled the x-label with ax1
    ax2.tick_params(axis='y', color='dimgrey')
    ax2.plot(history.history['lr'], color='dimgrey', linestyle='--')
    timer.start()
    plt.show()
    plotname = weight_dir + 'Accuracy_DNN.png'
    fig3.savefig(plotname)
    # print("Saved Accuracy plot as : " + plotname)
    print(colors.fg.lightgrey, "Saved Accuracy plot as :", colors.reset, plotname, '\n')

# //--------------------------------------------

  ####  #    # ###### #####  ##### #####    ##   # #    #
 #    # #    # #      #    #   #   #    #  #  #  # ##   #
 #    # #    # #####  #    #   #   #    # #    # # # #  #
 #    # #    # #      #####    #   #####  ###### # #  # #
 #    #  #  #  #      #   #    #   #   #  #    # # #   ##
  ####    ##   ###### #    #   #   #    # #    # # #    #

    nbins = 10
    rmin = 0.
    rmax = 1.

    fig4 = plt.figure(4)
    timer = fig4.canvas.new_timer(interval = 2000) #creating a timer object and setting an interval of N milliseconds
    timer.add_callback(close_event)

    ax = plt.axes()
    ax.set_xlim([rmin,rmax])

    #--- COSMETICS
    #grey=#E6E6E6 #white=#FFFFFF
    ax.patch.set_edgecolor('black')
    ax.patch.set_facecolor('#E6E6E6') #inner bkg color
    # ax.patch.set_facecolor('white')
    plt.grid(color='w', linestyle='solid') # draw solid white grid lines
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

    #-- Trick : for training histos, we want to compute the bin errors correctly ; to do this we first fill TH1Fs, then read their bin contents/errors
    hist_overtrain_train_sig = TH1F('hist_overtrain_train_sig', '', nbins, rmin, rmax); hist_overtrain_train_sig.Sumw2()
    hist_overtrain_train_bkg = TH1F('hist_overtrain_train_bkg', '', nbins, rmin, rmax); hist_overtrain_train_bkg.Sumw2()

    #Fill histos
    for x, w in zip(predictions_train_sig, weightPHY_train_sig): #'zip' stops when the shorter of foo or bar stops
        hist_overtrain_train_sig.Fill(x, w)
    for x, w in zip(predictions_train_bkg, weightPHY_train_bkg):
        hist_overtrain_train_bkg.Fill(x, w)

    #Normalize
    sf_integral = abs(rmax - rmin) / nbins #h.Scale(1/integral) makes the sum of contents equal to 1, but does not account for the bin width
    hist_overtrain_train_sig.Scale(1./(hist_overtrain_train_sig.Integral()*sf_integral)); hist_overtrain_train_bkg.Scale(1./(hist_overtrain_train_bkg.Integral()*sf_integral))

    #Read bin contents/errors
    bin_centres = []; counts_sig = []; err_sig = []; counts_bkg = []; err_bkg = []
    for ibin in range(1, hist_overtrain_train_sig.GetNbinsX()+1):
        bin_centres.append(hist_overtrain_train_sig.GetBinCenter(ibin))
        counts_sig.append(hist_overtrain_train_sig.GetBinContent(ibin)); counts_bkg.append(hist_overtrain_train_bkg.GetBinContent(ibin))
        err_sig.append(hist_overtrain_train_sig.GetBinError(ibin)); err_bkg.append(hist_overtrain_train_bkg.GetBinError(ibin))

    #Plot testing sig/bkg histos, normalized (no errors displayed <-> don't need TH1Fs)
    plt.hist(predictions_test_sig, bins=nbins, range=(rmin,rmax), color= 'cornflowerblue', alpha=0.50, weights=weightPHY_test_sig, density=True, histtype='step', log=False, label="Signal (Test)", edgecolor='cornflowerblue',fill=True)
    plt.hist(predictions_test_bkg, bins=nbins, range=(rmin,rmax), color='orangered', alpha=0.50, weights=weightPHY_test_bkg, density=True, histtype='step', log=False, label="Background (Test)", hatch='/', edgecolor='orangered',fill=False)

    #Plot training sig/bkg histos, normalized, with errorbars
    plt.errorbar(bin_centres, counts_sig, marker='o', yerr=err_sig, linestyle='None', markersize=6, color='blue', alpha=0.90, label='Signal (Train)')
    plt.errorbar(bin_centres, counts_bkg, marker='o', yerr=err_bkg, linestyle='None', markersize=6, color='red', alpha=0.90, label='Background (Train)')

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
    # print("Saved Overtraining plot as : " + plotname)
    print(colors.fg.lightgrey, "Saved Overtraining plot as :", colors.reset, plotname, '\n')
