/**
 * Compare distributions obtained under the SM or EFT hypotheses
 * Input root file obtained with the GenAnalyzer code
 * Also other functions : plot xsec vs operators, etc.
*/

/* BASH COLORS */
#define RST   "[0m"
#define KRED  "[31m"
#define KGRN  "[32m"
#define KYEL  "[33m"
#define KBLU  "[34m"
#define KMAG  "[35m"
#define KCYN  "[36m"
#define KWHT  "[37m"
#define FRED(x) KRED x RST
#define FGRN(x) KGRN x RST
#define FYEL(x) KYEL x RST
#define FBLU(x) KBLU x RST
#define FMAG(x) KMAG x RST
#define FCYN(x) KCYN x RST
#define FWHT(x) KWHT x RST
#define BOLD(x) "[1m" x RST
#define UNDL(x) "[4m" x RST

#include <sstream>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <vector>
#include <sys/stat.h> // to be able to check file existence
#include <dirent.h> //list dir content

#include "TF1.h"
#include "TF2.h"
#include <TLatex.h>
#include <TChain.h>
#include <TFile.h>
#include <TH1F.h>
#include <TStyle.h>
#include <TCanvas.h>
#include <TLorentzVector.h>
#include "TTree.h"
#include "TString.h"
#include "TColor.h"
#include "TCut.h"
#include "TLegend.h"
#include "TLine.h"
#include "THStack.h"
#include "TString.h"
#include "TH2F.h"
#include "TObject.h"
#include "TGraph.h"

#include "TMVA/Tools.h"
#include "TMVA/Factory.h"
#include "TMVA/DataLoader.h"
#include "TMVA/Reader.h"
#include "TMVA/MethodCuts.h"
#include "TMVA/Timer.h"
#include "TMVA/Config.h"

#include <iostream>
#include <cstdlib>
#include <iostream>
#include <map>
#include <string>
#include <cmath>
#include <sstream>
#include <fstream>
#include <boost/algorithm/string.hpp>

//Custom classes for EFT (see https://github.com/Andrew42/EFTGenReader/blob/maste)
#include "/home/ntonon/Postdoc/TopEFT/Analysis/Utils/TH1EFT.h"
#include "/home/ntonon/Postdoc/TopEFT/Analysis/Utils/WCPoint.h"
#include "/home/ntonon/Postdoc/TopEFT/Analysis/Utils/WCFit.h"

#include "./Helper.h"

#include <cassert>     //Can be used to terminate program if argument is not true.
//Ex : assert(test > 0 && "Error message");
#include <sys/stat.h> // to be able to use mkdir

//Hardcoded values if want to plot real nof events... (corresponding to cuts applied in GenAnalyzer)
#define xsec_tllq 30.5
#define xsec_ttll 281
#define lumi 41.5

using namespace std;



//--------------------------------------------
//  ######   #######  ##     ## ########     ###    ########  ########    ######## ######## ########
// ##    ## ##     ## ###   ### ##     ##   ## ##   ##     ## ##          ##       ##          ##
// ##       ##     ## #### #### ##     ##  ##   ##  ##     ## ##          ##       ##          ##
// ##       ##     ## ## ### ## ########  ##     ## ########  ######      ######   ######      ##
// ##       ##     ## ##     ## ##        ######### ##   ##   ##          ##       ##          ##
// ##    ## ##     ## ##     ## ##        ##     ## ##    ##  ##          ##       ##          ##
//  ######   #######  ##     ## ##        ##     ## ##     ## ########    ######## ##          ##
//--------------------------------------------

void Compare_Distributions(vector<TString> v_process, vector<TString> v_var, vector<TString> v_reweightNames_fromMG, vector<TString> v_reweightNames_extrapol, vector<pair<float, float>> v_min_max)
{
    cout<<endl<<BOLD(UNDL(FYEL("=== Compare Distributions (different processes, EFT points, ...) ===")))<<endl<<endl;

//--------------------------------------------
    bool normalize = true; //true <--> scale all histos to 1
    bool show_overflow = true; //true <--> include under- and over-flow (from bins 0 and nbins+1)
//--------------------------------------------

    //For each var, each process and each reweight, fill/store an histo
    vector<vector<vector<TH1F*>>> v3_histos_var_proc_reweight(v_var.size());
    vector<vector<vector<TH1F*>>> v3_histos_var_proc_reweight_subplot(v_var.size());

    //Idem for TH1EFT (but only need 1 TH1EFT for all reweights)
    vector<vector<TH1EFT*>> v2_TH1EFT_var_proc(v_var.size());

    vector<vector<vector<int>>> v3_nentries_var_proc_bin(v_var.size()); //For each bin of each var of each process, count nof entries -- for debug, understand uncerts.

    double debug_uncert=0;

    //Allocate memory to histos
    const int nbins_histos = 20;
    for(int ivar=0; ivar<v_var.size(); ivar++)
    {
        v3_histos_var_proc_reweight[ivar].resize(v_process.size());
        v3_histos_var_proc_reweight_subplot[ivar].resize(v_process.size());

        v2_TH1EFT_var_proc[ivar].resize(v_process.size());

        v3_nentries_var_proc_bin[ivar].resize(v_process.size());

        for(int iproc=0; iproc<v_process.size(); iproc++)
        {
            v3_histos_var_proc_reweight[ivar][iproc].resize(v_reweightNames_fromMG.size());
            v3_histos_var_proc_reweight_subplot[ivar][iproc].resize(v_reweightNames_fromMG.size());

            v2_TH1EFT_var_proc[ivar][iproc] = new TH1EFT("", "", nbins_histos, v_min_max[ivar].first, v_min_max[ivar].second);

            v3_nentries_var_proc_bin[ivar][iproc].resize(nbins_histos+2); //Also under/over flow bins

            for(int iweight=0; iweight<v_reweightNames_fromMG.size(); iweight++)
            {
                v3_histos_var_proc_reweight[ivar][iproc][iweight] = new TH1F("", "", nbins_histos, v_min_max[ivar].first, v_min_max[ivar].second);
                v3_histos_var_proc_reweight_subplot[ivar][iproc][iweight] = new TH1F("", "", nbins_histos, v_min_max[ivar].first, v_min_max[ivar].second);
            }
        }
    }

    //-- LOOP ON PROCESSES
    for(int iproc=0; iproc<v_process.size(); iproc++)
    {
        TString dir = "./";
        TString filepath = dir + "output_" + v_process[iproc] + ".root";
        if(!Check_File_Existence(filepath)) {cout<<endl<<BOLD(FRED("--- File not found ! Exit !"))<<endl<<endl; return;}

        TFile* f = TFile::Open(filepath);
        if(f == 0) {cout<<endl<<BOLD(FRED("--- File not found ! Exit !"))<<endl<<endl; return;}

        // TString treename = "tree";
        TString treename = "GenAnalyzer/tree"; //new name
        TTree* t = (TTree*) f->Get(treename);
        if(t == 0) {cout<<endl<<BOLD(FRED("--- Tree not found ! Exit !"))<<endl<<endl; return;}

        cout<<FBLU("Process : "<<v_process[iproc]<<" // Reading file : ")<<filepath<<endl<<endl;

        //Try to get proper event normalization ? (but depends on cuts in GenAnalyzer)
        float weight_SF = xsec_tllq * lumi;
        if(v_process[iproc] == "tllq" || v_process[iproc] == "ttz") {weight_SF = xsec_ttll * lumi;}

        //Read branches
        vector<float>* v_reweights_floats = new vector<float>();
        vector<string>* v_reweights_ids = new vector<string>();
        vector<float> v_var_floats(v_var.size());
        t->SetBranchAddress("v_weightIds", &v_reweights_ids);
        t->SetBranchAddress("v_weights", &v_reweights_floats);
        for(int ivar=0; ivar<v_var.size(); ivar++)
        {
            t->SetBranchAddress(v_var[ivar], &v_var_floats[ivar]);
        }

        // int index_Z, index_top, index_antitop;
        // t->SetBranchAddress("index_Z", &index_Z);
        // t->SetBranchAddress("index_top", &index_top);
        // t->SetBranchAddress("index_antitop", &index_antitop);

        float mc_weight_originalValue, originalXWGTUP;
        t->SetBranchAddress("mc_weight_originalValue", &mc_weight_originalValue);
        t->SetBranchAddress("originalXWGTUP", &originalXWGTUP);

        //Check that all requested weights (from MG) are found in sample
        t->GetEntry(0);
        for(int iweight=0; iweight<v_reweightNames_fromMG.size(); iweight++)
        {
            bool weightFound = false;
            for(int iweightid=0; iweightid<v_reweights_ids->size(); iweightid++)
            {
                if(v_reweightNames_fromMG[iweight] == TString(v_reweights_ids->at(iweightid)) ) {weightFound = true;}
            }
            if(!weightFound) {cout<<FRED("ERROR ! Weight "<<v_reweightNames_fromMG[iweight]<<" not found ! Abort !")<<endl; return;}
        }

        //Read and store sums of weights (SWE)
        // TString hSWEname = "h_SWE";
        TString hSWEname = "GenAnalyzer/h_SWE"; //new name
        TH1F* h_SWE = (TH1F*) f->Get(hSWEname);
        vector<float> v_SWE;
        for(int ibin=0; ibin<h_SWE->GetNbinsX(); ibin++)
        {
            v_SWE.push_back(h_SWE->GetBinContent(ibin+1)); //1 SWE stored for each stored weight
            // cout<<"v_SWE[ibin] = "<<v_SWE[ibin]<<endl;
        }

     // ###### #    # ###### #    # #####  ####     #       ####   ####  #####
     // #      #    # #      ##   #   #   #         #      #    # #    # #    #
     // #####  #    # #####  # #  #   #    ####     #      #    # #    # #    #
     // #      #    # #      #  # #   #        #    #      #    # #    # #####
     // #       #  #  #      #   ##   #   #    #    #      #    # #    # #
     // ######   ##   ###### #    #   #    ####     ######  ####   ####  #

        // int nentries = 100;
        int nentries = t->GetEntries();
        cout<<FMAG("Processing "<<nentries<<" entries...")<<endl;
        for(int ientry=0; ientry<nentries; ientry++)
        {
            t->GetEntry(ientry);

            if(ientry%1000==0) {cout<<ientry<<" entries..."<<endl;}

            // cout<<"//--------------------------------------------"<<endl;
            // for(int iweight=0; iweight<v_reweights_ids->size(); iweight++)
            // {
            //     cout<<"v_reweights_ids[iweight] "<<v_reweights_ids->at(iweight)<<endl;
            // }

            // cout<<"ientry "<<ientry<<endl;
            // cout<<"mc_weight_originalValue "<<mc_weight_originalValue<<endl;
            // cout<<"originalXWGTUP "<<originalXWGTUP<<endl;

            for(int ivar=0; ivar<v_var.size(); ivar++)
            {
                // if(v_var[ivar].Contains("Z_") && index_Z == -1) {continue;} //if Z not found
                // if(v_var[ivar].Contains("antitop", TString::kIgnoreCase) && index_antitop == -1) {continue;} //if antitop not found
                // if(v_var[ivar].Contains("top", TString::kIgnoreCase) && index_top == -1) {continue;} //if top not found

                if(v_var[ivar].Contains("Zreco_") && v_var_floats[ivar] <= 0) {continue;} //No Z->ll reco

                //Count nentries for debugging
                v3_nentries_var_proc_bin[ivar][iproc][v3_histos_var_proc_reweight[ivar][iproc][0]->GetXaxis()->FindBin(v_var_floats[ivar])]++; //add +1 entry to relevant bin

                //Loop on requested reweight points (from MG)
                for(int iweight=0; iweight<v_reweightNames_fromMG.size(); iweight++)
                {
                    // cout<<"v_reweightNames_fromMG[iweight] "<<v_reweightNames_fromMG[iweight]<<endl;

                    //Loop on reweights available in sample
                    for(int iweightid=0; iweightid<v_reweights_ids->size(); iweightid++)
                    {
                        // cout<<"TString(v_reweights_ids->at(iweightid)) "<<TString(v_reweights_ids->at(iweightid))<<endl;

                        if(v_reweightNames_fromMG[iweight] == TString(v_reweights_ids->at(iweightid)) ) //Requested weight found
                        {
                            // cout<<"v_reweights_floats->at(iweightid) "<<v_reweights_floats->at(iweightid)<<endl;

                            if(isnan(v_var_floats[ivar]) || isinf(v_var_floats[ivar]))
                            {
                                cout<<FRED("Warning ! "<<v_var[ivar]<<" = "<<v_var_floats[ivar]<<" // SKIP !")<<endl;
                                continue;
                            }

                            float w = v_reweights_floats->at(iweightid)/(mc_weight_originalValue * v_SWE[iweightid]);
                            // float w = weight_SF * v_reweights_floats->at(iweightid)/ v_SWE[iweightid];

                            if(show_overflow) {Fill_TH1F_UnderOverflow(v3_histos_var_proc_reweight[ivar][iproc][iweight], v_var_floats[ivar], w);}
                            // else {Fill_TH1F_NoUnderOverflow(v3_histos_var_proc_reweight[ivar][iproc][iweight], v_var_floats[ivar], w);}
                            else {v3_histos_var_proc_reweight[ivar][iproc][iweight]->Fill(v_var_floats[ivar], w);}

                            // cout<<"v3_histos_var_proc_reweight[ivar][iproc][iweight]->Integral() "<<v3_histos_var_proc_reweight[ivar][iproc][iweight]->Integral()<<endl;

                            //DEBUG uncert : compute error of 1 bin myself to make sure it's OK
                            if(!ivar && !iproc && !iweight && v3_histos_var_proc_reweight[0][0][0]->GetXaxis()->FindBin(v_var_floats[ivar]) == 1) {debug_uncert+= pow(v_reweights_floats->at(iweightid)/mc_weight_originalValue, 2);}

                            break; //weight has been found
                        }

                    } //Loop on event weights -- for matching
                } //Loop on selected weights -- for matching

                FillTH1EFT(v2_TH1EFT_var_proc[ivar][iproc], v_var_floats[ivar], v_reweights_ids, v_reweights_floats, originalXWGTUP, v_SWE, weight_SF, show_overflow);

                // cout<<"v2_TH1EFT_var_proc[ivar][iproc]->Integral() "<<v2_TH1EFT_var_proc[ivar][iproc]->Integral()<<endl;
            } //Loop on vars
        } //Loop on entries

        delete h_SWE;
        f->Close();

        delete v_reweights_floats; delete v_reweights_ids;
    } //Loop o processes

    if(normalize) //Normalize to 1
    {
        for(int ivar=0; ivar<v_var.size(); ivar++)
        {
            for(int iproc=0; iproc<v_process.size(); iproc++)
            {
                for(int iweight=0; iweight<v_reweightNames_fromMG.size(); iweight++)
                {
                    v3_histos_var_proc_reweight[ivar][iproc][iweight]->Scale(1./v3_histos_var_proc_reweight[ivar][iproc][iweight]->Integral());
                }
            }
        }
    }

    bool printBinContent = false; //debug
    if(printBinContent)
    {
        for(int ivar=0; ivar<1; ivar++)
        {
            cout<<"var "<<v_var[ivar]<<endl;

            for(int iproc=0; iproc<v_process.size(); iproc++)
            {
                cout<<"proc "<<v_process[iproc]<<endl;

                for(int iweight=0; iweight<v_reweightNames_fromMG.size(); iweight++)
                {
                    cout<<"//--------------------------------------------"<<endl;
                    cout<<"weight "<<v_reweightNames_fromMG[iweight]<<endl;
                    for(int ibin=1; ibin<v3_histos_var_proc_reweight[ivar][iproc][iweight]->GetNbinsX()+1; ibin++)
                    {
                        cout<<"bin "<<ibin<<" : "<<v3_histos_var_proc_reweight[ivar][iproc][iweight]->GetBinContent(ibin)<<" +- "<<v3_histos_var_proc_reweight[ivar][iproc][iweight]->GetBinError(ibin)<<" ("<<v3_nentries_var_proc_bin[ivar][iproc][ibin+1]<<" entries)"<<endl; //NB : first v3_nentries_var_proc_bin element is underflow
                    }
                }
            }
        }

        cout<<"sqrt(debug_uncert) = "<<sqrt(debug_uncert)<<endl;
    }


// #    # #####  # ##### ######    #    # #  ####  #####  ####   ####
// #    # #    # #   #   #         #    # # #        #   #    # #
// #    # #    # #   #   #####     ###### #  ####    #   #    #  ####
// # ## # #####  #   #   #         #    # #      #   #   #    #      #
// ##  ## #   #  #   #   #         #    # # #    #   #   #    # #    #
// #    # #    # #   #   ######    #    # #  ####    #    ####   ####

//--- Write histograms to an output rootfile, so that they could later be superimposed on top of data/mc comparison plots (to compare shapes, ...)
    bool write_histos_toRootfile = false;
    if(write_histos_toRootfile)
    {
        TFile* f_out = new TFile("GenPlotter_histos.root", "RECREATE");

        for(int ivar=0; ivar<v_var.size(); ivar++)
        {
            for(int iproc=0; iproc<v_process.size(); iproc++)
            {
                TString outhistoname = "TH1EFT_" + v_var[ivar]+"_"+v_process[iproc];
                v2_TH1EFT_var_proc[ivar][iproc]->Write(outhistoname);

                for(int iweight=0; iweight<v_reweightNames_fromMG.size(); iweight++)
                {
                    outhistoname = v_var[ivar]+"_"+v_process[iproc]+"_"+v_reweightNames_fromMG[iweight];

                    v3_histos_var_proc_reweight[ivar][iproc][iweight]->Write(outhistoname);
                }
            }
        }

        f_out->Close();

        cout<<endl<<FYEL("==> Created root file: ")<<f_out->GetName()<<FYEL(" containing GEN-level histograms")<<endl<<endl;
    }


// ########  ##        #######  ########
// ##     ## ##       ##     ##    ##
// ##     ## ##       ##     ##    ##
// ########  ##       ##     ##    ##
// ##        ##       ##     ##    ##
// ##        ##       ##     ##    ##
// ##        ########  #######     ##

    bool setlog = false;

    for(int ivar=0; ivar<v_var.size(); ivar++)
    {
        //For each extrapolated reweight, will scale the TH1EFT accordingly and store its content into a TH1F which will be drawn on the canvas
        vector<vector<TH1F*>> v2_TH1FfromTH1EFT_proc_reweight(v_process.size());
        vector<vector<TH1F*>> v2_TH1FfromTH1EFT_proc_reweight_subplot(v_process.size());

        float ymax = -1;
        float SFmax = -1;
        float SFmin = 1.;

        // Compute max Y value (to adapt Y range) and min/max 'BSM/SM' scale factor (to adapt ratio plot range)
        for(int iproc=0; iproc<v_process.size(); iproc++)
        {

 // #####  ###### #    # ###### #  ####  #    # #####
 // #    # #      #    # #      # #    # #    #   #
 // #    # #####  #    # #####  # #      ######   #
 // #####  #      # ## # #      # #  ### #    #   #
 // #   #  #      ##  ## #      # #    # #    #   #
 // #    # ###### #    # ###### #  ####  #    #   #
 //
 // ###### #    # ##### #####    ##   #####   ####  #
 // #       #  #    #   #    #  #  #  #    # #    # #
 // #####    ##     #   #    # #    # #    # #    # #
 // #        ##     #   #####  ###### #####  #    # #
 // #       #  #    #   #   #  #    # #      #    # #
 // ###### #    #   #   #    # #    # #       ####  ######

            //Create reweighted (extrapolation) histograms
            v2_TH1FfromTH1EFT_proc_reweight[iproc].resize(v_reweightNames_extrapol.size());
            v2_TH1FfromTH1EFT_proc_reweight_subplot[iproc].resize(v_reweightNames_extrapol.size());
            for(int iweight=0; iweight<v_reweightNames_extrapol.size(); iweight++)
            {
                //Rescale TH1EFT accordingly to current reweight
                WCPoint wcp = WCPoint((string) v_reweightNames_extrapol[iweight], 1.);
                v2_TH1EFT_var_proc[ivar][iproc]->Scale(wcp);

                if(normalize) {v2_TH1EFT_var_proc[ivar][iproc]->Scale(1. / v2_TH1EFT_var_proc[ivar][iproc]->Integral());} //Normalize

                //Store contents of rescaled TH1EFT into a TH1F for drawing
                v2_TH1FfromTH1EFT_proc_reweight[iproc][iweight] = (TH1F*) v2_TH1EFT_var_proc[ivar][iproc]->Clone();
            }

 // ###### # #    # #####     #   #    #    #   ##   #    #
 // #      # ##   # #    #     # #     ##  ##  #  #   #  #
 // #####  # # #  # #    #      #      # ## # #    #   ##
 // #      # #  # # #    #      #      #    # ######   ##
 // #      # #   ## #    #      #      #    # #    #  #  #
 // #      # #    # #####       #      #    # #    # #    #

            //Find ymax among *all* histograms for proper plot rescaling
            //--------------------------------------------

            //Loop on MG reweights
            for(int iweight=0; iweight<v_reweightNames_fromMG.size(); iweight++)
            {
                if(v3_histos_var_proc_reweight[ivar][iproc][iweight]->GetMaximum() > ymax)
                {
                    ymax = v3_histos_var_proc_reweight[ivar][iproc][iweight]->GetMaximum();
                    // cout<<"ymax = "<<ymax<<endl;
                }

                //Reference for ratio is either the SM histo, or the histo of the first process
                TH1F* h_compare_tmp = v3_histos_var_proc_reweight[ivar][iproc][0];
                if(v_reweightNames_fromMG.size() + v_reweightNames_extrapol.size() == 1 && v_process.size() > 1) {h_compare_tmp = v3_histos_var_proc_reweight[ivar][0][iweight];}

                for(int ibin=1; ibin<v3_histos_var_proc_reweight[ivar][iproc][iweight]->GetNbinsX()+1; ibin++)
                {
                    if(v3_histos_var_proc_reweight[ivar][iproc][iweight]->GetBinContent(ibin)/h_compare_tmp->GetBinContent(ibin) > SFmax)
                    {
                        SFmax = v3_histos_var_proc_reweight[ivar][iproc][iweight]->GetBinContent(ibin)/h_compare_tmp->GetBinContent(ibin);
                        // cout<<"SFmax = "<<SFmax<<endl;
                    }
                    if(v3_histos_var_proc_reweight[ivar][iproc][iweight]->GetBinContent(ibin)/h_compare_tmp->GetBinContent(ibin) < SFmin)
                    {
                        SFmin = v3_histos_var_proc_reweight[ivar][iproc][iweight]->GetBinContent(ibin)/h_compare_tmp->GetBinContent(ibin);
                        // cout<<"SFmin = "<<SFmin<<endl;
                    }
                }
            }

            //Loop on extrapolation reweights
            for(int iweight=0; iweight<v_reweightNames_extrapol.size(); iweight++)
            {
                if(v2_TH1FfromTH1EFT_proc_reweight[iproc][iweight]->GetMaximum() > ymax)
                {
                    ymax = v2_TH1FfromTH1EFT_proc_reweight[iproc][iweight]->GetMaximum();
                    // cout<<"ymax = "<<ymax<<endl;
                }

                //Reference for ratio is either the SM histo, or the histo of the first process
                TH1F* h_compare_tmp = v3_histos_var_proc_reweight[ivar][iproc][0];
                if(v_reweightNames_fromMG.size() + v_reweightNames_extrapol.size() == 1 && v_process.size() > 1) {h_compare_tmp = v3_histos_var_proc_reweight[ivar][0][iweight];}

                for(int ibin=1; ibin<v2_TH1FfromTH1EFT_proc_reweight[iproc][iweight]->GetNbinsX()+1; ibin++)
                {
                    if(v2_TH1FfromTH1EFT_proc_reweight[iproc][iweight]->GetBinContent(ibin)/h_compare_tmp->GetBinContent(ibin) > SFmax)
                    {
                        SFmax = v2_TH1FfromTH1EFT_proc_reweight[iproc][iweight]->GetBinContent(ibin)/h_compare_tmp->GetBinContent(ibin);
                        // cout<<"SFmax = "<<SFmax<<endl;
                    }
                    if(v2_TH1FfromTH1EFT_proc_reweight[iproc][iweight]->GetBinContent(ibin)/h_compare_tmp->GetBinContent(ibin) < SFmin)
                    {
                        SFmin = v2_TH1FfromTH1EFT_proc_reweight[iproc][iweight]->GetBinContent(ibin)/h_compare_tmp->GetBinContent(ibin);
                        // cout<<"SFmin = "<<SFmin<<endl;
                    }
                }
            }
        }
        // double SFmax = RoundUp(ymax / h_compare_tmp->GetMaximum()) + 0.99; //in %
        //--------------------------------------------

 //                                                    #
 //  ####    ##   #    # #    #   ##    ####          #     #      ######  ####
 // #    #  #  #  ##   # #    #  #  #  #             #      #      #      #    #
 // #      #    # # #  # #    # #    #  ####        #       #      #####  #
 // #      ###### #  # # #    # ######      #      #        #      #      #  ###
 // #    # #    # #   ##  #  #  #    # #    #     #         #      #      #    #
 //  ####  #    # #    #   ##   #    #  ####     #          ###### ######  ####

        //-- Canvas definition
        TCanvas* c1 = new TCanvas("c1","c1", 1000, 800);
        // c1->SetTopMargin(0.1);
        c1->SetBottomMargin(0.43);
        if(setlog) c1->SetLogy();

        //--- LEGEND FOR REWEIGHTS
        TLegend* legend_weights = 0;
        // legend_weights = new TLegend(0.82,0.77,0.99,0.99); //single-column top-right legend

        //1 column for each 2 entries (round up with ceil)
        // float width_col = 0.32; //hardcoded value
        float width_col = 0.17;
        float x_left = 0.99-ceil((v_reweightNames_fromMG.size()+v_reweightNames_extrapol.size())/2.)*width_col; //each column takes same x-space
        float bottom_legend = 0.90;
        // legend_weights = new TLegend(x_left,0.88,0.99,0.99);
        legend_weights = new TLegend(x_left,bottom_legend,0.99,0.99);
        legend_weights->SetNColumns(ceil((v_reweightNames_fromMG.size()+v_reweightNames_extrapol.size())/2.));
        legend_weights->SetLineColor(1);
        // legend_weights->SetTextSize(0.025);
        legend_weights->SetTextSize(0.03);

        //--- LEGEND FOR PROCESSES (if >1 processes)
        TLegend* legend_proc = 0;
        if(v_process.size() > 1)
        {
            legend_proc = new TLegend(x_left - 0.10,0.88,x_left - 0.01,0.99);
            legend_proc->SetTextSize(0.03);
        }

 // #####  #####    ##   #    #
 // #    # #    #  #  #  #    #
 // #    # #    # #    # #    #
 // #    # #####  ###### # ## #
 // #    # #   #  #    # ##  ##
 // #####  #    # #    # #    #

        //-- Style and draw histos
        c1->cd();
        for(int iproc=0; iproc<v_process.size(); iproc++)
        {
            //Loop on pre-existing MG reweights
            for(int iweight=0; iweight<v_reweightNames_fromMG.size(); iweight++)
            {
                // v3_histos_var_proc_reweight[ivar][iproc][iweight]->SetLineColor(iweight+1);
                // if(iweight+1==5) {v3_histos_var_proc_reweight[ivar][iproc][iweight]->SetLineColor(8);} //don't like yellow
                v3_histos_var_proc_reweight[ivar][iproc][iweight]->SetLineColor(Get_Color(iweight));

                v3_histos_var_proc_reweight[ivar][iproc][iweight]->SetLineWidth(4);

                if(!iweight) //only needed for first histo
                {
                    v3_histos_var_proc_reweight[ivar][iproc][iweight]->SetTitle(""); //No main title

                    // v3_histos_var_proc_reweight[ivar][iproc][iweight]->GetYaxis()->SetTitle("Events");
                    v3_histos_var_proc_reweight[ivar][iproc][iweight]->GetYaxis()->SetTitle("a.u.");
                    // v3_histos_var_proc_reweight[ivar][iproc][iweight]->GetYaxis()->SetTitle("#frac{d#sigma}{dX} (a.u.)");
                    v3_histos_var_proc_reweight[ivar][iproc][iweight]->SetLineStyle(1);

                    v3_histos_var_proc_reweight[ivar][iproc][iweight]->GetXaxis()->SetLabelFont(42);
                    v3_histos_var_proc_reweight[ivar][iproc][iweight]->GetYaxis()->SetLabelFont(42);
                    v3_histos_var_proc_reweight[ivar][iproc][iweight]->GetYaxis()->SetTitleFont(42);
                    v3_histos_var_proc_reweight[ivar][iproc][iweight]->GetYaxis()->SetTitleSize(0.06);
                    v3_histos_var_proc_reweight[ivar][iproc][iweight]->GetXaxis()->SetTitleSize(0.06);
                    v3_histos_var_proc_reweight[ivar][iproc][iweight]->GetYaxis()->SetTickLength(0.04);
                    v3_histos_var_proc_reweight[ivar][iproc][iweight]->GetYaxis()->SetLabelSize(0.048);
                    v3_histos_var_proc_reweight[ivar][iproc][iweight]->GetXaxis()->SetNdivisions(505);
                    v3_histos_var_proc_reweight[ivar][iproc][iweight]->GetYaxis()->SetNdivisions(506);
                    v3_histos_var_proc_reweight[ivar][iproc][iweight]->GetYaxis()->SetTitleOffset(1.2);
                    v3_histos_var_proc_reweight[ivar][iproc][iweight]->GetXaxis()->SetLabelSize(0.0); //subplot axis instead


                    if(setlog) {v3_histos_var_proc_reweight[ivar][iproc][iweight]->SetMaximum(ymax * 5.);}
                    else {v3_histos_var_proc_reweight[ivar][iproc][iweight]->SetMaximum(ymax * 1.2);}
                }

                if(v_process.size() > 1 && !iweight) //only fill legend once per process, not for each weight
                {
                    legend_proc->AddEntry(v3_histos_var_proc_reweight[ivar][iproc][iweight], " "+GetProcessLegendName(v_process[iproc]), "L");
                }

                if(!iproc) //Different style per process ; only fill legend once per reweight, not for each process
                {
                    legend_weights->AddEntry(v3_histos_var_proc_reweight[ivar][iproc][iweight], GetReweightLegendName(v_reweightNames_fromMG[iweight]), "L");
                }
                else {v3_histos_var_proc_reweight[ivar][iproc][iweight]->SetLineStyle(iproc+1);}

                v3_histos_var_proc_reweight[ivar][iproc][iweight]->Draw("hist E same");
            }

            //Loop on extrapolated reweights
            v2_TH1FfromTH1EFT_proc_reweight[iproc].resize(v_reweightNames_extrapol.size());
            v2_TH1FfromTH1EFT_proc_reweight_subplot[iproc].resize(v_reweightNames_extrapol.size());
            for(int iweight=0; iweight<v_reweightNames_extrapol.size(); iweight++)
            {
                //Rescale TH1EFT accordingly to current reweight
                WCPoint wcp = WCPoint((string) v_reweightNames_extrapol[iweight], 1.);
                v2_TH1EFT_var_proc[ivar][iproc]->Scale(wcp);
                v2_TH1EFT_var_proc[ivar][iproc]->Scale(1. / v2_TH1EFT_var_proc[ivar][iproc]->Integral()); //Normalize

                //Store contents of rescaled TH1EFT into a TH1F for drawing
                v2_TH1FfromTH1EFT_proc_reweight[iproc][iweight] = (TH1F*) v2_TH1EFT_var_proc[ivar][iproc]->Clone();

                // v2_TH1FfromTH1EFT_proc_reweight[iproc][iweight]->SetLineColor(v_reweightNames_fromMG.size() + iweight + 1);
                // if(v_reweightNames_fromMG.size() + iweight + 1==5) {v2_TH1FfromTH1EFT_proc_reweight[iproc][iweight]->SetLineColor(8);} //don't like yellow
                v2_TH1FfromTH1EFT_proc_reweight[iproc][iweight]->SetLineColor(Get_Color(v_reweightNames_fromMG.size() + iweight));

                if(!iproc) //Different style per process ; only fill legend once per reweight, not for each process
                {
                    legend_weights->AddEntry(v2_TH1FfromTH1EFT_proc_reweight[iproc][iweight], GetReweightLegendName(v_reweightNames_extrapol[iweight]), "L");
                }
                else {v2_TH1FfromTH1EFT_proc_reweight[iproc][iweight]->SetLineStyle(iproc+1);}

                v2_TH1FfromTH1EFT_proc_reweight[iproc][iweight]->SetLineWidth(2);

                v2_TH1FfromTH1EFT_proc_reweight[iproc][iweight]->Draw("hist E same");
            } //weight loop
        } //proc loop

        legend_weights->Draw("same");
        if(v_process.size() > 1) {legend_proc->Draw("same");}

//  ####  #    # #####  #####  #       ####  #####
// #      #    # #    # #    # #      #    #   #
//  ####  #    # #####  #    # #      #    #   #
//      # #    # #    # #####  #      #    #   #
// #    # #    # #    # #      #      #    #   #
//  ####   ####  #####  #      ######  ####    #

        //-- Subplot (for all reweights, not nominal)
        //-- create subpad to plot ratio
        TPad *pad_ratio = new TPad("pad_ratio", "pad_ratio", 0.0, 0.0, 1.0, 1.0);
        pad_ratio->SetTopMargin(0.6);
        pad_ratio->SetFillColor(0);
        pad_ratio->SetFillStyle(0);
        pad_ratio->SetGridy(1);
        pad_ratio->Draw();
        pad_ratio->cd(0);

        for(int iproc=0; iproc<v_process.size(); iproc++)
        {
            //Loop on MG reweights
            for(int iweight=0; iweight<v_reweightNames_fromMG.size(); iweight++)
            {
                TString subplot_y_title = "";
                //Compare ratio EFT/SM
                if(v_reweightNames_fromMG.size()+v_reweightNames_extrapol.size() > 1)
                {
                    subplot_y_title = "BSM/SM"; // [%]
                    if(v_reweightNames_fromMG[iweight] != "sm")
                    {
                        v3_histos_var_proc_reweight_subplot[ivar][iproc][iweight] = (TH1F*) v3_histos_var_proc_reweight[ivar][iproc][iweight]->Clone(); //Copy histo
                        // v3_histos_var_proc_reweight_subplot[ivar][iproc][iweight]->Add(v3_histos_var_proc_reweight[ivar][iproc][0], -1); //Substract nominal
                        v3_histos_var_proc_reweight_subplot[ivar][iproc][iweight]->Divide(v3_histos_var_proc_reweight[ivar][iproc][0]); //Divide by nominal
                    }
                }
                else if(v_reweightNames_fromMG.size() + v_reweightNames_extrapol.size() == 1 && v_process.size() > 1) //Compare ratio of processes
                {
                    subplot_y_title = "Process ratio"; // [%]
                    if(iproc > 0)
                    {
                        v3_histos_var_proc_reweight_subplot[ivar][iproc][iweight] = (TH1F*) v3_histos_var_proc_reweight[ivar][iproc][iweight]->Clone(); //Copy histo
                        v3_histos_var_proc_reweight_subplot[ivar][iproc][iweight]->Divide(v3_histos_var_proc_reweight[ivar][0][iweight]); //Divide by nominal
                    }
                }

                // for(int i=0; i<10; i++)
                // {
                //     cout<<"v3_histos_var_proc_reweight_subplot[ivar][iproc][iweight]->GetBinContent(i+1)"<<v3_histos_var_proc_reweight_subplot[ivar][iproc][iweight]->GetBinContent(i+1)<<endl;
                // }

                v3_histos_var_proc_reweight_subplot[ivar][iproc][iweight]->GetXaxis()->SetTitle(Get_Variable_Name(v_var[ivar]));
                // v3_histos_var_proc_reweight_subplot[ivar][iproc][iweight]->GetXaxis()->SetTitle(v_var[ivar]);
                v3_histos_var_proc_reweight_subplot[ivar][iproc][iweight]->GetYaxis()->CenterTitle();
                v3_histos_var_proc_reweight_subplot[ivar][iproc][iweight]->GetYaxis()->SetTitle(subplot_y_title);
                // v3_histos_var_proc_reweight_subplot[ivar][iproc][iweight]->GetYaxis()->SetTitle("#frac{(X-#mu)}{#mu} [%]");
                v3_histos_var_proc_reweight_subplot[ivar][iproc][iweight]->GetYaxis()->SetTitleOffset(1.4);
                v3_histos_var_proc_reweight_subplot[ivar][iproc][iweight]->GetYaxis()->SetTickLength(0.);
                v3_histos_var_proc_reweight_subplot[ivar][iproc][iweight]->GetXaxis()->SetTitleOffset(0.8);
                v3_histos_var_proc_reweight_subplot[ivar][iproc][iweight]->GetYaxis()->SetLabelSize(0.048);
                v3_histos_var_proc_reweight_subplot[ivar][iproc][iweight]->GetYaxis()->SetNdivisions(507); //grid draw on primary tick marks only
                v3_histos_var_proc_reweight_subplot[ivar][iproc][iweight]->GetYaxis()->SetTitleSize(0.05);
                v3_histos_var_proc_reweight_subplot[ivar][iproc][iweight]->GetXaxis()->SetTickLength(0.04);
                v3_histos_var_proc_reweight_subplot[ivar][iproc][iweight]->GetXaxis()->SetLabelSize(0.04);

                // v3_histos_var_proc_reweight_subplot[ivar][iproc][iweight]->Scale(100.); //express in %
                v3_histos_var_proc_reweight_subplot[ivar][iproc][iweight]->SetMinimum(0.);
                v3_histos_var_proc_reweight_subplot[ivar][iproc][iweight]->SetMaximum(SFmax*1.2);

                // v3_histos_var_proc_reweight_subplot[ivar][iproc][iweight]->SetLineColor(iweight+1);
                v3_histos_var_proc_reweight_subplot[ivar][iproc][iweight]->SetLineColor(Get_Color(iweight));
                v3_histos_var_proc_reweight_subplot[ivar][iproc][iweight]->SetLineWidth(4);

                v3_histos_var_proc_reweight_subplot[ivar][iproc][iweight]->Draw("hist E same");
            }

            //Loop on extrapolated reweights
            for(int iweight=0; iweight<v_reweightNames_extrapol.size(); iweight++)
            {
                v2_TH1FfromTH1EFT_proc_reweight_subplot[iproc][iweight] = (TH1F*) v2_TH1FfromTH1EFT_proc_reweight[iproc][iweight]->Clone(); //Copy corresponding 'main' histo
                v2_TH1FfromTH1EFT_proc_reweight_subplot[iproc][iweight]->Divide(v3_histos_var_proc_reweight[ivar][iproc][0]); //Divide by nominal = SM histo

                v2_TH1FfromTH1EFT_proc_reweight_subplot[iproc][iweight]->SetMinimum(0.);
                v2_TH1FfromTH1EFT_proc_reweight_subplot[iproc][iweight]->SetMaximum(SFmax*1.2);

                v2_TH1FfromTH1EFT_proc_reweight_subplot[iproc][iweight]->SetLineWidth(2);

                v2_TH1FfromTH1EFT_proc_reweight_subplot[iproc][iweight]->Draw("hist E same");
            }
        }

        //Only write process name on plot if considering 1 single process
        if(v_process.size() == 1)
        {
            TString text = "pp #rightarrow " + GetProcessLegendName(v_process[0]) + " (13 TeV)";
            TLatex latex;
            latex.SetNDC();
            latex.SetTextAlign(11);
            latex.SetTextFont(52);
            latex.SetTextSize(0.04);
            latex.DrawLatex(0.17, 0.95, text);
        }

        //Write once the list of names of the operators which are considered
        TString example_rwgt_name = ""; //Need 1 example of rwgt name
        if(v_reweightNames_extrapol.size()>0) {example_rwgt_name = v_reweightNames_extrapol[v_reweightNames_extrapol.size()-1];}
        else if(v_reweightNames_fromMG.size()>1) {example_rwgt_name = v_reweightNames_fromMG[v_reweightNames_fromMG.size()-1];}
        TString list_operators = Get_List_Operators(example_rwgt_name);
        if(example_rwgt_name != "")
        {
            TLatex latex;
            latex.SetNDC();
            latex.SetTextAlign(11);
            latex.SetTextFont(52);
            latex.SetTextSize(0.03);
            latex.DrawLatex(x_left+0.04, bottom_legend-0.04, list_operators);
        }

// #    # #####  # ##### ######
// #    # #    # #   #   #
// #    # #    # #   #   #####
// # ## # #####  #   #   #
// ##  ## #   #  #   #   #
// #    # #    # #   #   ######

        TString outputname = "./" + v_var[ivar] + ".png";
        c1->SaveAs(outputname);

        delete pad_ratio;
        delete legend_weights;
        if(v_process.size() > 1) {delete legend_proc;}
        delete c1;

        for(int iproc=0; iproc<v_process.size(); iproc++)
        {
            for(int iweight=0; iweight<v_reweightNames_extrapol.size(); iweight++)
            {
                delete v2_TH1FfromTH1EFT_proc_reweight[iproc][iweight];
                delete v2_TH1FfromTH1EFT_proc_reweight_subplot[iproc][iweight];
            }
        }

    } //end var loop

    for(int ivar=0; ivar<v_var.size(); ivar++)
    {
        for(int iproc=0; iproc<v_process.size(); iproc++)
        {
            delete v2_TH1EFT_var_proc[ivar][iproc];

            for(int iweight=0; iweight<v_reweightNames_fromMG.size(); iweight++)
            {
                delete v3_histos_var_proc_reweight[ivar][iproc][iweight];
                delete v3_histos_var_proc_reweight_subplot[ivar][iproc][iweight];
            }
        }
    }

    return;
}






//--------------------------------------------
//--------------------------------------------
//--------------------------------------------
//--------------------------------------------
//--------------------------------------------
//--------------------------------------------
//--------------------------------------------
//--------------------------------------------
//--------------------------------------------







//--------------------------------------------
// ##     ##    ###    #### ##    ##
// ###   ###   ## ##    ##  ###   ##
// #### ####  ##   ##   ##  ####  ##
// ## ### ## ##     ##  ##  ## ## ##
// ##     ## #########  ##  ##  ####
// ##     ## ##     ##  ##  ##   ###
// ##     ## ##     ## #### ##    ##
//--------------------------------------------

int main()
{
    //-- List of processes to superimpose
    vector<TString> v_process;
    // v_process.push_back("ttz");
    // v_process.push_back("tzq");
    // v_process.push_back("tllq");
    // v_process.push_back("ttll");
    v_process.push_back("tllq_top19001");
    // v_process.push_back("ttll_top19001");

    //-- List of variables to plot, and their ranges
    //NB : "Top_x" and "Antitop_x" contain only events which have a top or antitop respectively. 'LeadingTop_x' considers leading top/antitop
    vector<TString> v_var; vector<pair<float, float>> v_min_max;
    v_var.push_back("Z_pt"); v_min_max.push_back(std::make_pair(0, 400));
    // v_var.push_back("Z_eta"); v_min_max.push_back(std::make_pair(-5, 5));
    // v_var.push_back("Z_m"); v_min_max.push_back(std::make_pair(70, 110));
    // v_var.push_back("Zreco_m"); v_min_max.push_back(std::make_pair(50, 110));
    // v_var.push_back("Zreco_dPhill"); v_min_max.push_back(std::make_pair(0, 4));
    // v_var.push_back("LeadingTop_pt"); v_min_max.push_back(std::make_pair(0, 500));
    // v_var.push_back("LeadingTop_eta"); v_min_max.push_back(std::make_pair(-5, 5));
    // v_var.push_back("TopZsystem_m"); v_min_max.push_back(std::make_pair(250, 1000));

    // v_var.push_back("cosThetaStarPol_Z"); v_min_max.push_back(std::make_pair(-1, 1));
    // v_var.push_back("cosThetaStarPol_Top"); v_min_max.push_back(std::make_pair(-1, 1));
    // v_var.push_back("Top_pt"); v_min_max.push_back(std::make_pair(0, 500));
    // v_var.push_back("Top_eta"); v_min_max.push_back(std::make_pair(-5, 5));
    // v_var.push_back("Top_m"); v_min_max.push_back(std::make_pair(100, 300));
    // v_var.push_back("LeadingTop_m"); v_min_max.push_back(std::make_pair(-5, 5));

    //-- List of weights to plot *which are present in the rootfile (evaluated by MG)* (+ colors)
    //--> Will create corresponding TH1F and plot it (can use it e.g. to control that some base points are well modeled by TH1EFT extrapolation)
    //WARNING : here the names must match exactly those stored in the tree !
    vector<TString> v_reweightNames_fromMG; vector<int> v_colors;
    v_reweightNames_fromMG.push_back("rwgt_sm"); //Nominal SM weight -- ALWAYS KEEP FIRST !!!

    // v_reweightNames_fromMG.push_back("rwgt_ctz_2.0_ctw_2.0_cpqm_2.0_cpq3_2.0_cpt_2.0");

    //-- List of weights to plot *which will be obtained from the TH1EFT extrapolation* (+ colors)
    //NB : 'rwgt_sm' will crash ; explicitely set all the WCs to 0 instead
    vector<TString> v_reweightNames_extrapol;
    // v_reweightNames_extrapol.push_back("rwgt_ctz_0_ctw_0_cpqm_0_cpq3_0_cpt_0");

    v_reweightNames_extrapol.push_back("rwgt_ctz_5_ctw_0_cpqm_0_cpq3_0_cpt_0");
    // v_reweightNames_extrapol.push_back("rwgt_ctz_0_ctw_5_cpqm_0_cpq3_0_cpt_0");

    Load_Canvas_Style();

    Compare_Distributions(v_process, v_var, v_reweightNames_fromMG, v_reweightNames_extrapol, v_min_max);

    return 0;
}
