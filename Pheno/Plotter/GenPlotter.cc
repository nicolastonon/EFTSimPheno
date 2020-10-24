/**
 * Compare distributions obtained under the SM or EFT hypotheses
 * Input root file obtained with the GenAnalyzer code
 * Also other functions : plot xsec vs operators, etc.
*/

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

//-- Hardcoded values if want to plot real nof events... (corresponding to cuts applied in GenAnalyzer)
#define xsec_tllq 30.51
#define xsec_ttll 99.37
#define xsec_twll 13.487
//tWZ: 11.23

#define lumi 41.5

#define DEFVAL -9

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

void Compare_Distributions(vector<TString> v_process, vector<TString> v_var, vector<TString> v_reweightNames_fromMG, vector<TString> v_reweightNames_extrapol, vector<pair<float, float>> v_min_max, int max_entries)
{
    cout<<endl<<BOLD(UNDL(FYEL("=== Compare Distributions (different processes, EFT points, ...) ===")))<<endl<<endl;

//--------------------------------------------
    bool normalize = true; //true <--> scale all histos to 1
    bool show_overflow = true; //true <--> include under- and over-flow (from bins 0 and nbins+1)
//--------------------------------------------

    Load_Canvas_Style();

    //For each var, each process and each reweight, fill/store an histo
    vector<vector<vector<TH1F*>>> v3_histos_var_proc_reweight(v_var.size());
    vector<vector<vector<TH1F*>>> v3_histos_var_proc_reweight_subplot(v_var.size());

    //Idem for TH1EFT (but only need 1 TH1EFT for all reweights)
    vector<vector<TH1EFT*>> v2_TH1EFT_var_proc(v_var.size());

    vector<vector<vector<int>>> v3_nentries_var_proc_bin(v_var.size()); //For each bin of each var of each process, count nof entries -- for debug, understand uncerts.

    TString example_reweight_namingConvention = ""; //Store 1 example of weight ID, to know which are the operators used to parameterized the event weights in samples //Assume that all considered samples follow same naming convention

    double debug_uncert=0;
    TH1::SetDefaultSumw2();

    //Allocate memory to histos
    const int nbins_histos = 15;
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

            float xmin = v_min_max[ivar].first, xmax = v_min_max[ivar].second;
            if(xmin == xmax)
            {
                xmin = 0; xmax = 300; //Default range
                if(v_var[ivar].Contains("_eta", TString::kIgnoreCase)) {xmin = -5; xmax = 5;}
                else if(v_var[ivar].Contains("_phi", TString::kIgnoreCase) || v_var[ivar].Contains("_dPhi", TString::kIgnoreCase)) {xmin = -3.2; xmax = 3.2;}
                else if(v_var[ivar].Contains("_dR", TString::kIgnoreCase) || v_var[ivar].Contains("dR_", TString::kIgnoreCase)) {xmin = 0; xmax = 6;}
                else if(v_var[ivar].Contains("_eta", TString::kIgnoreCase)) {xmin = -5; xmax = 5;}
                else if(v_var[ivar].Contains("_dEta", TString::kIgnoreCase) || v_var[ivar].Contains("dEta_", TString::kIgnoreCase)) {xmin = 0; xmax = 8;}
            }

            v2_TH1EFT_var_proc[ivar][iproc] = new TH1EFT("", "", nbins_histos, xmin, xmax);

            v3_nentries_var_proc_bin[ivar][iproc].resize(nbins_histos+2); //Also under/over flow bins

            for(int iweight=0; iweight<v_reweightNames_fromMG.size(); iweight++)
            {
                v3_histos_var_proc_reweight[ivar][iproc][iweight] = new TH1F("", "", nbins_histos, xmin, xmax);
                v3_histos_var_proc_reweight_subplot[ivar][iproc][iweight] = new TH1F("", "", nbins_histos, xmin, xmax);
            }
        }
    }

    //-- LOOP ON PROCESSES
    for(int iproc=0; iproc<v_process.size(); iproc++)
    {
        bool is_SMEFT_sample = true;
        if(v_process[iproc].Contains("central")) {is_SMEFT_sample = false;} //Convention: assume that unless the filename contains the keyword "central", it corresponds to a private SMEFT sample
        if(!is_SMEFT_sample && (v_reweightNames_fromMG.size()>1 || v_reweightNames_extrapol.size()>1)) {cout<<"ERROR: can't plot EFT points for central samples ! "<<endl;} //Central sample --> only SM point

        TString dir = "./";
        TString filepath = dir + "output_" + v_process[iproc] + ".root";
        if(!Check_File_Existence(filepath)) {cout<<endl<<BOLD(FRED("--- File "<<filepath<<" not found ! Exit !"))<<endl<<endl; return;}

        TFile* f = TFile::Open(filepath);
        if(f == 0) {cout<<endl<<BOLD(FRED("--- File not found ! Exit !"))<<endl<<endl; return;}

        // TString treename = "tree";
        TString treename = "GenAnalyzer/tree"; //new name
        TTree* t = (TTree*) f->Get(treename);
        if(t == 0) {cout<<endl<<BOLD(FRED("--- Tree not found ! Exit !"))<<endl<<endl; return;}

        cout<<FBLU("Process : "<<v_process[iproc]<<" // Reading file : ")<<filepath<<endl<<endl;

        //Try to get proper event normalization ? (but depends on cuts in GenAnalyzer)
        float weight_SF = xsec_tllq * lumi;
        if(v_process[iproc] == "tllq" || v_process[iproc] == "tzq") {weight_SF = xsec_tllq * lumi;}
        else if(v_process[iproc] == "ttll" || v_process[iproc] == "ttz") {weight_SF = xsec_ttll * lumi;}
        else if(v_process[iproc] == "twll" || v_process[iproc] == "twz") {weight_SF = xsec_twll * lumi;}

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

        //Check that all requested weights from MG are found in sample
        TH1F* h_SWE = NULL;
        if(is_SMEFT_sample)
        {
            t->GetEntry(0);
            example_reweight_namingConvention = v_reweights_ids->at(2); //Arbitrary name example (don't take first element because it may be 'rwgt_sm')
            for(int iweight=0; iweight<v_reweightNames_fromMG.size(); iweight++)
            {
                bool weightFound = false;
                for(int iweightid=0; iweightid<v_reweights_ids->size(); iweightid++)
                {
                    // cout<<"v_reweights_ids->at(iweightid) "<<v_reweights_ids->at(iweightid)<<endl;

                    if(ToLower(v_reweightNames_fromMG[iweight]) == ToLower(TString(v_reweights_ids->at(iweightid))) ) {weightFound = true;}
                }
                if(!weightFound)
                {
                    cout<<FRED("ERROR ! Weight "<<v_reweightNames_fromMG[iweight]<<" not found ! The reweights found in sample are :")<<endl;
                    for(int iweightid=0; iweightid<v_reweights_ids->size(); iweightid++) {cout<<"v_reweights_ids->at(iweightid) "<<v_reweights_ids->at(iweightid)<<endl;}
                    return;
                }
            }

            //-- Read and store sums of weights (SWE)
            // TString hSWEname = "h_SWE";
            TString hSWEname = "GenAnalyzer/h_SWE"; //new name
            h_SWE = (TH1F*) f->Get(hSWEname);
            vector<float> v_SWE;
            for(int ibin=0; ibin<h_SWE->GetNbinsX(); ibin++)
            {
                v_SWE.push_back(h_SWE->GetBinContent(ibin+1)); //1 SWE stored for each stored weight
                // cout<<"v_SWE[ibin] = "<<v_SWE[ibin]<<endl;
            }
        }


 // ###### #    # ###### #    # #####  ####     #       ####   ####  #####
 // #      #    # #      ##   #   #   #         #      #    # #    # #    #
 // #####  #    # #####  # #  #   #    ####     #      #    # #    # #    #
 // #      #    # #      #  # #   #        #    #      #    # #    # #####
 // #       #  #  #      #   ##   #   #    #    #      #    # #    # #
 // ######   ##   ###### #    #   #    ####     ######  ####   ####  #

        vector<double> v_means(v_var.size()); //Can printout mean of variables

        // int nentries = 1000;
        int nentries = t->GetEntries();
        if(max_entries>0 && nentries>max_entries) {nentries = max_entries;}
        cout<<FMAG("Processing "<<nentries<<" entries...")<<endl;
        // if(v_reweightNames_extrapol.size() > 0 && nentries > 50000) {nentries = 50000;} //Parameterization is slow
        for(int ientry=0; ientry<nentries; ientry++)
        {
            t->GetEntry(ientry);

            if(ientry%1000==0) {cout<<ientry<<" entries..."<<endl;}

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

                if(abs(v_var_floats[ivar]) == abs(DEFVAL)) {continue;} //Dummy value, don't plot this entry for this variable

                v_means[ivar]+= v_var_floats[ivar];

                //Count nentries for debugging
                v3_nentries_var_proc_bin[ivar][iproc][v3_histos_var_proc_reweight[ivar][iproc][0]->GetXaxis()->FindBin(v_var_floats[ivar])]++; //add +1 entry to relevant bin

                //Loop on requested reweight points (from MG)
                for(int iweight=0; iweight<v_reweightNames_fromMG.size(); iweight++)
                {
                    // cout<<"v_reweightNames_fromMG[iweight] "<<v_reweightNames_fromMG[iweight]<<endl;

                    if(!is_SMEFT_sample) //Only SM point/weight
                    {
                        float w = mc_weight_originalValue; //Default (e.g. central samples)

                        if(show_overflow) {Fill_TH1F_UnderOverflow(v3_histos_var_proc_reweight[ivar][iproc][iweight], v_var_floats[ivar], w);}
                        else {v3_histos_var_proc_reweight[ivar][iproc][iweight]->Fill(v_var_floats[ivar], w);}
                        // cout<<"v3_histos_var_proc_reweight[ivar][iproc][iweight]->Integral() "<<v3_histos_var_proc_reweight[ivar][iproc][iweight]->Integral()<<endl;
                        continue;
                    }

                    //Loop on reweights available in sample
                    for(int iweightid=0; iweightid<v_reweights_ids->size(); iweightid++)
                    {
                        // cout<<"TString(v_reweights_ids->at(iweightid)) "<<TString(v_reweights_ids->at(iweightid))<<endl;

                        if(ToLower(v_reweightNames_fromMG[iweight]) == ToLower(TString(v_reweights_ids->at(iweightid))) ) //Requested weight found
                        {
                            if(isnan(v_var_floats[ivar]) || isinf(v_var_floats[ivar]))
                            {
                                cout<<FRED("Warning ! "<<v_var[ivar]<<" = "<<v_var_floats[ivar]<<" // SKIP !")<<endl;
                                continue;
                            }

                            // cout<<v_reweights_ids->at(iweightid)<<" --> "<<v_reweights_floats->at(iweightid)<<endl;

                            float w = v_reweights_floats->at(iweightid);
                            // float w = weight_SF * v_reweights_floats->at(iweightid)/ v_SWE[iweightid];

                            if(v_process[iproc].Contains("tllq") && w>70) {cout<<"w = "<<w<<endl; continue;} //FIXME -- got a spurious weight in 1 sample... !

                            if(show_overflow) {Fill_TH1F_UnderOverflow(v3_histos_var_proc_reweight[ivar][iproc][iweight], v_var_floats[ivar], w);}
                            // else {Fill_TH1F_NoUnderOverflow(v3_histos_var_proc_reweight[ivar][iproc][iweight], v_var_floats[ivar], w);}
                            else {v3_histos_var_proc_reweight[ivar][iproc][iweight]->Fill(v_var_floats[ivar], w);}
                            // cout<<"v3_histos_var_proc_reweight[ivar][iproc][iweight]->Integral() "<<v3_histos_var_proc_reweight[ivar][iproc][iweight]->Integral()<<endl;

                            //DEBUG uncert : compute error of 1 bin myself to make sure it's OK
                            if(!ivar && !iproc && !iweight && v3_histos_var_proc_reweight[0][0][0]->GetXaxis()->FindBin(v_var_floats[ivar]) == 1) {debug_uncert+= pow(v_reweights_floats->at(iweightid), 2);}

                            break; //weight has been found
                        } //MG reweights

                    } //Loop on event weights -- for matching
                } //Loop on selected weights -- for matching
            } //Loop on vars

            if(!v_reweightNames_extrapol.size()) {continue;} //Don't need to fill TH1EFT (long) if not extrapolating weights

            float sm_wgt = 0;
            WCFit eftfit = Get_EFT_Fit(v_reweights_ids, v_reweights_floats, sm_wgt);

            FillTH1EFT_ManyVars(v2_TH1EFT_var_proc, eftfit, v_var_floats, iproc, sm_wgt, show_overflow);
        } //Loop on entries

        //-- Debug printout
        // for(int ivar=0; ivar<v_var.size(); ivar++) {cout<<"Variable "<<v_var[ivar]<<" / Mean = "<<v_means[ivar]/nentries<<endl;}

        if(h_SWE) {delete h_SWE; h_SWE = NULL;}
        f->Close();

        delete v_reweights_floats; delete v_reweights_ids;
    } //Loop on processes

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
        for(int ivar=0; ivar<2; ivar++)
        {
            cout<<"var "<<v_var[ivar]<<endl;

            for(int iproc=0; iproc<v_process.size(); iproc++)
            {
                cout<<"proc "<<v_process[iproc]<<endl;

                for(int iweight=0; iweight<v_reweightNames_fromMG.size(); iweight++)
                {
                    cout<<"//--------------------------------------------"<<endl;
                    cout<<"weight "<<v_reweightNames_fromMG[iweight]<<endl;
                    for(int ibin=0; ibin<v3_histos_var_proc_reweight[ivar][iproc][iweight]->GetNbinsX()+2; ibin++)
                    {
                        cout<<"bin "<<ibin<<" : "<<v3_histos_var_proc_reweight[ivar][iproc][iweight]->GetBinContent(ibin)<<" +- "<<v3_histos_var_proc_reweight[ivar][iproc][iweight]->GetBinError(ibin)<<" "<<"("<<v3_histos_var_proc_reweight[ivar][iproc][iweight]->GetBinError(ibin)*100/v3_histos_var_proc_reweight[ivar][iproc][iweight]->GetBinContent(ibin)<<"%)"<<" ["<<v3_nentries_var_proc_bin[ivar][iproc][ibin]<<" entries]"<<endl; //NB : first v3_nentries_var_proc_bin element is underflow
                    }
                }
            }
        }
        // cout<<"sqrt(debug_uncert) = "<<sqrt(debug_uncert)<<endl; //NB: not normalized
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

    mkdir("plots", 0777);
    mkdir("plots/input_vars", 0777);

    bool contains_ACreweights = false; //Check whether some 'extrapolated reweights' are set in the AC framework -- if yes, want to extend legend size (larger names)
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
                // cout<<"v_reweightNames_extrapol[iweight] "<<v_reweightNames_extrapol[iweight]<<endl;

                //Rescale TH1EFT accordingly to current reweight
                WCPoint wcp;
                if(v_reweightNames_extrapol[iweight].BeginsWith("rwgt_c", TString::kExact)) {wcp = WCPoint((string) v_reweightNames_extrapol[iweight], 1.);} //EFT
                else if(v_reweightNames_extrapol[iweight].BeginsWith("rwgt_C", TString::kExact) || v_reweightNames_extrapol[iweight].BeginsWith("rwgt_D", TString::kExact)) {wcp = WCPoint((string) Convert_ACtoEFT(v_reweightNames_extrapol[iweight], example_reweight_namingConvention), 1.); contains_ACreweights = true;} //AC --> convert in terms of Wilson coeff. for reweighting
                else {cout<<"ERROR: naming convention not recognized !"<<endl; return;}

                v2_TH1EFT_var_proc[ivar][iproc]->Scale(wcp);

                if(normalize) {v2_TH1EFT_var_proc[ivar][iproc]->Scale(1. / v2_TH1EFT_var_proc[ivar][iproc]->Integral());} //Normalize

                //Store contents of rescaled TH1EFT into a TH1F for drawing
                v2_TH1FfromTH1EFT_proc_reweight[iproc][iweight] = (TH1F*) v2_TH1EFT_var_proc[ivar][iproc]->Clone();
            }

 //                                                 #                         #     #
 // ###### # #    # #####     #    # # #    #      #  #    #   ##   #    #     #   #
 // #      # ##   # #    #    ##  ## # ##   #     #   ##  ##  #  #   #  #       # #
 // #####  # # #  # #    #    # ## # # # #  #    #    # ## # #    #   ##         #
 // #      # #  # # #    #    #    # # #  # #   #     #    # ######   ##         #
 // #      # #   ## #    #    #    # # #   ##  #      #    # #    #  #  #        #
 // #      # #    # #####     #    # # #    # #       #    # #    # #    #       #

            //Find ymax among *all* histograms for proper plot rescaling
            //--------------------------------------------

            //Loop on MG reweights
            for(int iweight=0; iweight<v_reweightNames_fromMG.size(); iweight++)
            {
                // cout<<"iweight "<<iweight<<endl;

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
                    // cout<<"ibin "<<ibin<<endl;

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
            } //extrapol. weight loop
        } //process loop
        // double SFmax = RoundUp(ymax / h_compare_tmp->GetMaximum()) + 0.99; //in %

        //-- Arbitrary threshold, for lisibility
        if(SFmax > 5) {SFmax = 5.;}
        //--------------------------------------------


 //                                                    #
 //  ####    ##   #    # #    #   ##    ####          #     #      ######  ####  ###### #    # #####
 // #    #  #  #  ##   # #    #  #  #  #             #      #      #      #    # #      ##   # #    #
 // #      #    # # #  # #    # #    #  ####        #       #      #####  #      #####  # #  # #    #
 // #      ###### #  # # #    # ######      #      #        #      #      #  ### #      #  # # #    #
 // #    # #    # #   ##  #  #  #    # #    #     #         #      #      #    # #      #   ## #    #
 //  ####  #    # #    #   ##   #    #  ####     #          ###### ######  ####  ###### #    # #####

        //-- Canvas definition
        TCanvas* c1 = new TCanvas("c1","c1", 1000, 800);
        // c1->SetTopMargin(0.1);
        c1->SetBottomMargin(0.43);
        if(setlog) c1->SetLogy();
        c1->SetTopMargin(0.1); //space for legend

        //--- LEGEND FOR REWEIGHTS
        TLegend* legend_weights = 0;
        // legend_weights = new TLegend(0.82,0.77,0.99,0.99); //single-column top-right legend

        //1 column for each 2 entries (round up with ceil)
        // float width_col = 0.32; //hardcoded value
        float width_col = 0.17;
        float x_left = 0.99-ceil((v_reweightNames_fromMG.size()+v_reweightNames_extrapol.size())/2.)*width_col; //each column takes same x-space
        if(contains_ACreweights) {x_left = 0.2;} //AC names are larger
        float bottom_legend = 0.90;
        // legend_weights = new TLegend(x_left,bottom_legend,0.99,0.99);
        legend_weights = new TLegend(x_left,0.905,0.995,0.995);
        legend_weights->SetNColumns(ceil((v_reweightNames_fromMG.size()+v_reweightNames_extrapol.size())/2.));
        legend_weights->SetLineColor(1);
        // legend_weights->SetTextSize(0.025);
        legend_weights->SetTextSize(0.03);

        //--- LEGEND FOR PROCESSES (if >1 processes)
        TLegend* legend_proc = 0;
        if(v_process.size() > 1)
        {
            // legend_proc = new TLegend(x_left - 0.10,0.88,x_left - 0.01,0.99);
            legend_proc = new TLegend(0.20, 0.92, x_left-0.02,0.99);
            legend_weights->SetNColumns(v_process.size());
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
                if((v_reweightNames_fromMG.size() + v_reweightNames_extrapol.size()) == 1) {v3_histos_var_proc_reweight[ivar][iproc][iweight]->SetLineColor(Get_Color(iproc));} //Special case: if only considering 1 scenario (e.g. SM), apply different colors to different samples

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

                    if(v_process.size() == 1) {v3_histos_var_proc_reweight[ivar][iproc][iweight]->SetFillColorAlpha(18, 0.4);} //Fill SM light gray

                    if(setlog) {v3_histos_var_proc_reweight[ivar][iproc][iweight]->SetMaximum(ymax * 5.);}
                    else {v3_histos_var_proc_reweight[ivar][iproc][iweight]->SetMaximum(ymax * 1.2);}

                    v3_histos_var_proc_reweight[ivar][iproc][iweight]->SetMinimum(0.); //If no bin is 0, minimum is shifted and can give a wrong impression (inflated error bars, ...)
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

            //-- Loop on extrapolated reweights
            //REMOVE -- already done earlier ?
            // v2_TH1FfromTH1EFT_proc_reweight[iproc].resize(v_reweightNames_extrapol.size());
            // v2_TH1FfromTH1EFT_proc_reweight_subplot[iproc].resize(v_reweightNames_extrapol.size());
            for(int iweight=0; iweight<v_reweightNames_extrapol.size(); iweight++)
            {
                //REMOVE -- already done earlier ?
                //Rescale TH1EFT accordingly to current reweight
                // WCPoint wcp = WCPoint((string) v_reweightNames_extrapol[iweight], 1.);
                // v2_TH1EFT_var_proc[ivar][iproc]->Scale(wcp);
                // v2_TH1EFT_var_proc[ivar][iproc]->Scale(1. / v2_TH1EFT_var_proc[ivar][iproc]->Integral()); //Normalize
                //Store contents of rescaled TH1EFT into a TH1F for drawing
                // v2_TH1FfromTH1EFT_proc_reweight[iproc][iweight] = (TH1F*) v2_TH1EFT_var_proc[ivar][iproc]->Clone();

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
                    subplot_y_title = "EFT/SM"; // [%]
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

                if(!iweight) {v3_histos_var_proc_reweight_subplot[ivar][iproc][iweight]->SetFillColor(0);} //No fill color for SM in subplot

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
                // v3_histos_var_proc_reweight_subplot[ivar][iproc][iweight]->SetMinimum(0.);
                v3_histos_var_proc_reweight_subplot[ivar][iproc][iweight]->SetMinimum(SFmin*0.7);
                v3_histos_var_proc_reweight_subplot[ivar][iproc][iweight]->SetMaximum(SFmax*1.1);

                v3_histos_var_proc_reweight_subplot[ivar][iproc][iweight]->SetLineColor(Get_Color(iweight));
                if((v_reweightNames_fromMG.size() + v_reweightNames_extrapol.size()) == 1) {v3_histos_var_proc_reweight_subplot[ivar][iproc][iweight]->SetLineColor(Get_Color(iproc));} //Special case: if only considering 1 scenario (e.g. SM), apply different colors to different samples
                v3_histos_var_proc_reweight_subplot[ivar][iproc][iweight]->SetLineWidth(4);

                v3_histos_var_proc_reweight_subplot[ivar][iproc][iweight]->Draw("hist E same");
            }

            //Loop on extrapolated reweights
            for(int iweight=0; iweight<v_reweightNames_extrapol.size(); iweight++)
            {
                v2_TH1FfromTH1EFT_proc_reweight_subplot[iproc][iweight] = (TH1F*) v2_TH1FfromTH1EFT_proc_reweight[iproc][iweight]->Clone(); //Copy corresponding 'main' histo
                v2_TH1FfromTH1EFT_proc_reweight_subplot[iproc][iweight]->Divide(v3_histos_var_proc_reweight[ivar][iproc][0]); //Divide by nominal = SM histo

                // v2_TH1FfromTH1EFT_proc_reweight_subplot[iproc][iweight]->SetMinimum(0.);
                v2_TH1FfromTH1EFT_proc_reweight_subplot[iproc][iweight]->SetMinimum(SFmin*0.7);
                v2_TH1FfromTH1EFT_proc_reweight_subplot[iproc][iweight]->SetMaximum(SFmax*1.1);

                v2_TH1FfromTH1EFT_proc_reweight_subplot[iproc][iweight]->SetLineWidth(2);

                v2_TH1FfromTH1EFT_proc_reweight_subplot[iproc][iweight]->Draw("hist E same");
            }
        }

        //Only write process name on plot if considering 1 single process
        if(v_process.size() == 1)
        {
            TString text = "pp #rightarrow " + GetProcessLegendName(v_process[0]);
            TLatex latex;
            latex.SetNDC();
            latex.SetTextAlign(11);
            latex.SetTextFont(52);
            latex.SetTextSize(0.03);
            // latex.DrawLatex(0.17, 0.95, text);

            if(x_left < 0.45 || v_process.size()>1) {latex.DrawLatex(0.19, 0.85, text);}
            else {latex.DrawLatex(0.26, 0.91, text);}
        }

        //Write once the list of names of the operators which are considered
        //NB: assumes that all the reweighting points (MG and extrapolation) will follow the exact same convention
        // TString example_rwgt_name = ""; //Need 1 example of rwgt name
        // if(v_reweightNames_extrapol.size()>0) {example_rwgt_name = v_reweightNames_extrapol[v_reweightNames_extrapol.size()-1];}
        // else if(v_reweightNames_fromMG.size()>1) {example_rwgt_name = v_reweightNames_fromMG[v_reweightNames_fromMG.size()-1];}
        // TString list_operators = Get_List_Operators(example_rwgt_name);
        TString list_operators = Get_List_Operators(example_reweight_namingConvention);
        if(example_reweight_namingConvention != "")
        {
            TLatex latex;
            latex.SetNDC();
            latex.SetTextAlign(11);
            latex.SetTextFont(52);
            latex.SetTextSize(0.03);
            // latex.DrawLatex(0.65, bottom_legend-0.04, list_operators); //inside plot, top left

            if(x_left < 0.45 || v_process.size()>1) {latex.DrawLatex(0.50, bottom_legend-0.05, list_operators);}
            else {latex.DrawLatex(0.18, 0.96, list_operators);} //outside plot, top left
        }

// #    # #####  # ##### ######
// #    # #    # #   #   #
// #    # #    # #   #   #####
// # ## # #####  #   #   #
// ##  ## #   #  #   #   #
// #    # #    # #   #   ######

        TString dir_proc = "";
        if(v_process.size() == 1) //Single signal process --> store plots in dedicated dir.
        {
            if(v_process[0].Contains("tllq")) {dir_proc = "tZq/";}
            else if(v_process[0].Contains("ttll")) {dir_proc = "ttZ/";}
            else if(v_process[0].Contains("twll")) {dir_proc = "tWZ/";}

            mkdir(("plots/input_vars/"+dir_proc).Data(), 0777);
        }

        TString outputname = "./plots/input_vars/" + dir_proc + v_var[ivar] + ".png";
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
//--------------------------------------------

    int max_entries = -1; //-1 <-> process all available entries

    //-- List of processes to superimpose
    vector<TString> v_process;
    // v_process.push_back("tllq");
    v_process.push_back("ttll");
    // v_process.push_back("twll");

//--------------------------------------------

    //-- List of variables to plot, and their ranges
    //NB : "Top_x" and "Antitop_x" contain only events which have a top or antitop respectively. 'LeadingTop_x' considers leading top/antitop
    vector<TString> v_var; vector<pair<float, float>> v_min_max;
    v_var.push_back("Z_pt"); v_min_max.push_back(std::make_pair(0, 500));
    v_var.push_back("Z_eta"); v_min_max.push_back(std::make_pair(-1, -1));
    v_var.push_back("Z_phi"); v_min_max.push_back(std::make_pair(-1, -1));
    v_var.push_back("Z_m"); v_min_max.push_back(std::make_pair(75, 105));
    v_var.push_back("Zreco_pt"); v_min_max.push_back(std::make_pair(0, 500));
    v_var.push_back("Zreco_eta"); v_min_max.push_back(std::make_pair(-1, -1));
    v_var.push_back("Zreco_phi"); v_min_max.push_back(std::make_pair(-1, -1));
    v_var.push_back("Zreco_m"); v_min_max.push_back(std::make_pair(75, 105));
    v_var.push_back("Zreco_dPhill"); v_min_max.push_back(std::make_pair(0, 3));
    v_var.push_back("LepTop_pt"); v_min_max.push_back(std::make_pair(0, 400));
    v_var.push_back("LepTop_eta"); v_min_max.push_back(std::make_pair(-1, -1));
    v_var.push_back("LepTop_phi"); v_min_max.push_back(std::make_pair(-1, -1));
    v_var.push_back("LepTop_m"); v_min_max.push_back(std::make_pair(160, 190));
    v_var.push_back("mTW"); v_min_max.push_back(std::make_pair(70, 400));

    v_var.push_back("TopZsystem_pt"); v_min_max.push_back(std::make_pair(0, 500));
    v_var.push_back("TopZsystem_eta"); v_min_max.push_back(std::make_pair(-1, -1));
    v_var.push_back("TopZsystem_phi"); v_min_max.push_back(std::make_pair(-1, -1));
    v_var.push_back("TopZsystem_m"); v_min_max.push_back(std::make_pair(200, 2000));
    v_var.push_back("recoilQuark_pt"); v_min_max.push_back(std::make_pair(-1, -1));
    v_var.push_back("recoilQuark_eta"); v_min_max.push_back(std::make_pair(-1, -1));
    v_var.push_back("recoilQuark_phi"); v_min_max.push_back(std::make_pair(-1, -1));
    v_var.push_back("Wlep_pt"); v_min_max.push_back(std::make_pair(0, 500));
    v_var.push_back("Wlep_eta"); v_min_max.push_back(std::make_pair(-1, -1));
    v_var.push_back("Wlep_phi"); v_min_max.push_back(std::make_pair(-1, -1));
    v_var.push_back("cosThetaStarPol_Top"); v_min_max.push_back(std::make_pair(-1, 1));
    v_var.push_back("cosThetaStarPol_Z"); v_min_max.push_back(std::make_pair(-1, 1));
    v_var.push_back("maxDiJet_pt"); v_min_max.push_back(std::make_pair(0, 500));
    v_var.push_back("maxDiJet_m"); v_min_max.push_back(std::make_pair(0, 3000));
    v_var.push_back("minDiJet_dEta"); v_min_max.push_back(std::make_pair(0, 0.5));
    v_var.push_back("maxDiJet_dEta"); v_min_max.push_back(std::make_pair(2, 12));
    v_var.push_back("maxDiJet_dPhi"); v_min_max.push_back(std::make_pair(2.6, 3.2));
    v_var.push_back("minDiJet_dR"); v_min_max.push_back(std::make_pair(0.3, 1));
    v_var.push_back("maxDiJet_dR"); v_min_max.push_back(std::make_pair(2, 12));
    v_var.push_back("Mass_3l"); v_min_max.push_back(std::make_pair(80, 500));
    v_var.push_back("dR_tZ"); v_min_max.push_back(std::make_pair(-1, -1));
    v_var.push_back("dR_ZlW"); v_min_max.push_back(std::make_pair(-1, -1));
    v_var.push_back("dR_blW"); v_min_max.push_back(std::make_pair(0, 4));
    v_var.push_back("dR_bW"); v_min_max.push_back(std::make_pair(0, 4));
    v_var.push_back("dR_tClosestLep"); v_min_max.push_back(std::make_pair(0, 4));
    v_var.push_back("dR_jprimeClosestLep"); v_min_max.push_back(std::make_pair(-1, -1));
    v_var.push_back("dR_tjprime"); v_min_max.push_back(std::make_pair(-1, -1));
    v_var.push_back("dEta_tjprime"); v_min_max.push_back(std::make_pair(-1, -1));
    v_var.push_back("dR_bjprime"); v_min_max.push_back(std::make_pair(-1, -1));
    v_var.push_back("dEta_bjprime"); v_min_max.push_back(std::make_pair(-1, -1));
    v_var.push_back("dR_lWjprime"); v_min_max.push_back(std::make_pair(-1, -1));
    v_var.push_back("dEta_lWjprime"); v_min_max.push_back(std::make_pair(-1, -1));
    v_var.push_back("dR_Zjprime"); v_min_max.push_back(std::make_pair(-1, -1));
    v_var.push_back("dEta_Zjprime"); v_min_max.push_back(std::make_pair(-1, -1));
    v_var.push_back("maxEtaJet"); v_min_max.push_back(std::make_pair(0, 6));
    v_var.push_back("dR_ttbar"); v_min_max.push_back(std::make_pair(-1, -1));
    v_var.push_back("dEta_ttbar"); v_min_max.push_back(std::make_pair(-1, -1));
    v_var.push_back("ptLepSum"); v_min_max.push_back(std::make_pair(0, 500));
    v_var.push_back("ptHadSum"); v_min_max.push_back(std::make_pair(0, 2000));
    v_var.push_back("mHT"); v_min_max.push_back(std::make_pair(0, 600));
    v_var.push_back("lepAsym"); v_min_max.push_back(std::make_pair(-5, 5));
    v_var.push_back("njets"); v_min_max.push_back(std::make_pair(0, 15));

//--------------------------------------------

    //-- List of weights to plot *which are present in the rootfile* (evaluated by MG) (+ specify colors)
    //--> Will create corresponding TH1F and plot it (can use it e.g. to control that some base points are well modeled by TH1EFT extrapolation)
    //WARNING : here the names must match exactly those stored in the tree !
    vector<TString> v_reweightNames_fromMG; vector<int> v_colors;
    //=====
    v_reweightNames_fromMG.push_back("rwgt_sm"); //Nominal SM weight -- ALWAYS KEEP FIRST !
    //=====

    v_reweightNames_fromMG.push_back("rwgt_ctz_5.0_ctw_0.0_cpqm_0.0_cpq3_0.0_cpt_0.0");
    v_reweightNames_fromMG.push_back("rwgt_ctz_0.0_ctw_5.0_cpqm_0.0_cpq3_0.0_cpt_0.0");
    v_reweightNames_fromMG.push_back("rwgt_ctz_0.0_ctw_0.0_cpqm_15.0_cpq3_0.0_cpt_0.0");
    v_reweightNames_fromMG.push_back("rwgt_ctz_0.0_ctw_0.0_cpqm_0.0_cpq3_15.0_cpt_0.0");
    v_reweightNames_fromMG.push_back("rwgt_ctz_0.0_ctw_0.0_cpqm_0.0_cpq3_0.0_cpt_15.0");

    // v_reweightNames_fromMG.push_back("rwgt_ctZ_5.0_ctW_0.0_cpQM_0.0_cpQ3_0.0_cpt_0.0");
    // v_reweightNames_fromMG.push_back("rwgt_ctZ_0.0_ctW_5.0_cpQM_0.0_cpQ3_0.0_cpt_0.0");
    // v_reweightNames_fromMG.push_back("rwgt_ctZ_0.0_ctW_0.0_cpQM_15.0_cpQ3_0.0_cpt_0.0");
    // v_reweightNames_fromMG.push_back("rwgt_ctZ_0.0_ctW_0.0_cpQM_0.0_cpQ3_15.0_cpt_0.0");
    // v_reweightNames_fromMG.push_back("rwgt_ctZ_0.0_ctW_0.0_cpQM_0.0_cpQ3_0.0_cpt_15.0");

    // v_reweightNames_fromMG.push_back("rwgt_ctZ_0.0_ctW_0.0_cpQM_-3.0_cpQ3_0.0_cpt_0.0");
    // v_reweightNames_fromMG.push_back("rwgt_ctZ_0.0_ctW_0.0_cpQM_0.0_cpQ3_0.0_cpt_-3.0");

    //-- List of weights to plot which will be obtained from the TH1EFT extrapolation
    //Examples: 'rwgt_ctZ_0' (SM), 'rwgt_ctZ_2' (EFT), 'rwgt_C1A_0.3' (AC, abs), 'rwgt_C1A_0.3' (AC, abs) ; last 2 examples correspond to anomalous couplings (C1A,C1V,C2V), which get automatically converted in terms of Wilson coeff. (in which the event weights are actually parameterized) //NB: C1V_SM = 0.2448, C1A_SM = -0.6012 ; can either choose to set a 'DeltaC1A/V' difference or an absolute 'C1A/V' value) //Default for AC: C2V=DeltaC1A=DeltaC1V=0
    vector<TString> v_reweightNames_extrapol;
    // v_reweightNames_extrapol.push_back("rwgt_ctz_0_ctw_0_cpqm_0_cpq3_0_cpt_0");
    // v_reweightNames_extrapol.push_back("rwgt_C2V_-0.25_C1A_0_C1V_0");
    // v_reweightNames_extrapol.push_back("rwgt_DeltaC1V_0.5_DeltaC1A_0.5");
    // v_reweightNames_extrapol.push_back("rwgt_DeltaC1A_1");
    // v_reweightNames_extrapol.push_back("rwgt_C2V_0.4");

    // v_reweightNames_extrapol.push_back("rwgt_ctz_1_ctw_0_cpqm_0_cpq3_0_cpt_0");
    // v_reweightNames_extrapol.push_back("rwgt_ctz_2_ctw_0_cpqm_0_cpq3_0_cpt_0");
    // v_reweightNames_extrapol.push_back("rwgt_ctz_3_ctw_0_cpqm_0_cpq3_0_cpt_0");
    // v_reweightNames_extrapol.push_back("rwgt_ctz_0_ctw_1_cpqm_0_cpq3_0_cpt_0");

//--------------------------------------------

    Compare_Distributions(v_process, v_var, v_reweightNames_fromMG, v_reweightNames_extrapol, v_min_max, max_entries);

    return 0;
}
