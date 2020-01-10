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
#include "TObject.h"

#include <iostream>
#include <cstdlib>
#include <iostream>
#include <map>
#include <string>
#include <cmath>
#include <sstream>
#include <fstream>

//#include "TMVA/Tools.h"
//#include "TMVA/Factory.h"
//#include "TMVA/Reader.h"
//#include "TMVA/MethodCuts.h"
//#include "TMVA/Config.h"

#include <cassert>     //Can be used to terminate program if argument is not true.
//Ex : assert(test > 0 && "Error message");
#include <sys/stat.h> // to be able to use mkdir

using namespace std;


//--------------------------------------------
// ##     ## ######## ##       ########  ######## ########
// ##     ## ##       ##       ##     ## ##       ##     ##
// ##     ## ##       ##       ##     ## ##       ##     ##
// ######### ######   ##       ########  ######   ########
// ##     ## ##       ##       ##        ##       ##   ##
// ##     ## ##       ##       ##        ##       ##    ##
// ##     ## ######## ######## ##        ######## ##     ##
//--------------------------------------------


//Use stat function (from library sys/stat) to check whether a file exists
bool Check_File_Existence(const TString& name)
{
  struct stat buffer;
  return (stat (name.Data(), &buffer) == 0); //true if file exists
}


void Load_Canvas_Style()
{
    TH1::SetDefaultSumw2();

	// For the canvas:
	gStyle->SetCanvasBorderMode(0);
	gStyle->SetCanvasColor(0); // must be kWhite but I dunno how to do that in PyROOT
	gStyle->SetCanvasDefH(600); //Height of canvas
	gStyle->SetCanvasDefW(600); //Width of canvas
	gStyle->SetCanvasDefX(0);   //POsition on screen
	gStyle->SetCanvasDefY(0);
	gStyle->SetPadBorderMode(0);
	gStyle->SetPadColor(0); // kWhite
	gStyle->SetPadGridX(0); //false
	gStyle->SetPadGridY(0); //false
	gStyle->SetGridColor(0);
	gStyle->SetGridStyle(3);
	gStyle->SetGridWidth(1);
	gStyle->SetFrameBorderMode(0);
	gStyle->SetFrameBorderSize(1);
	gStyle->SetFrameFillColor(0);
	gStyle->SetFrameFillStyle(0);
	gStyle->SetFrameLineColor(1);
	gStyle->SetFrameLineStyle(1);
	gStyle->SetFrameLineWidth(1);
	gStyle->SetHistLineColor(1);
	gStyle->SetHistLineStyle(0);
	gStyle->SetHistLineWidth(1);
	gStyle->SetEndErrorSize(2);
	gStyle->SetOptFit(1011);
	gStyle->SetFitFormat("5.4g");
	gStyle->SetFuncColor(2);
	gStyle->SetFuncStyle(1);
	gStyle->SetFuncWidth(1);
	gStyle->SetOptDate(0);
	gStyle->SetOptFile(0);
	gStyle->SetOptStat(0); // To display the mean and RMS:   SetOptStat("mr");
	gStyle->SetStatColor(0); // kWhite
	gStyle->SetStatFont(42);
	gStyle->SetStatFontSize(0.04);
	gStyle->SetStatTextColor(1);
	gStyle->SetStatFormat("6.4g");
	gStyle->SetStatBorderSize(1);
	gStyle->SetStatH(0.1);
	gStyle->SetStatW(0.15);
	gStyle->SetPadTopMargin(0.07);
	gStyle->SetPadBottomMargin(0.13);
	gStyle->SetPadLeftMargin(0.16);
	gStyle->SetPadRightMargin(0.03);
	gStyle->SetOptTitle(0);
	gStyle->SetTitleFont(42);
	gStyle->SetTitleColor(1);
	gStyle->SetTitleTextColor(1);
	gStyle->SetTitleFillColor(10);
	gStyle->SetTitleFontSize(0.05);
	gStyle->SetTitleColor(1, "XYZ");
	gStyle->SetTitleFont(42, "XYZ");
	gStyle->SetTitleSize(0.06, "XYZ");
	gStyle->SetTitleXOffset(0.9);
	gStyle->SetTitleYOffset(1.25);
	gStyle->SetLabelColor(1, "XYZ");
	gStyle->SetLabelFont(42, "XYZ");
	gStyle->SetLabelOffset(0.007, "XYZ");
	gStyle->SetLabelSize(0.05, "XYZ");
	gStyle->SetAxisColor(1, "XYZ");
	gStyle->SetStripDecimals(1); // kTRUE
	gStyle->SetTickLength(0.03, "XYZ");
	gStyle->SetNdivisions(510, "XYZ");
	gStyle->SetPadTickX(1);  // To get tick marks on the opposite side of the frame
	gStyle->SetPadTickY(1);
	gStyle->SetOptLogx(0);
	gStyle->SetOptLogy(0);
	gStyle->SetOptLogz(0);
	gStyle->SetPaperSize(20.,20.);
}

TString GetReweightLegendName(TString variationname)
{
    TString result = variationname;

    if(variationname == "sm") {result = "SM";}
    else if(variationname.Contains("_"))
    {
        TString tmp = variationname;
        int i = tmp.Index("_"); //Find index of character

        if(variationname.Contains("ctz", TString::kIgnoreCase)) {result = "c_{tZ} = ";}
        else if(variationname.Contains("ctw", TString::kIgnoreCase)) {result = "c_{tW} = ";}
        else
        {
            tmp.Remove(i); //Remove subsequent characters
            result = tmp + " = ";
        }

        tmp = variationname;
        tmp.Remove(0, i+1); //Remove previous characters

        TString first_digit = tmp;
        first_digit.Remove(tmp.Index("p"));
        result+= first_digit;

        TString second_digit = tmp;
        second_digit.Remove(0,tmp.Index("p")+1);
        if(second_digit != '0') {result+= "." + second_digit;}
    }

    // result+= " [TeV^{-2}]";

    return result;
}

TString GetProcessLegendName(TString proc)
{
    if(proc == "ttz") {return "t#bar{t}Z";}
    else if(proc == "tzq") {return "tZq";}
    else if(proc == "ttll") {return "t#bar{t}ll";}
    else if(proc == "tllq") {return "tllq";}

    return proc;
}

//Round to upper 10
int RoundUp(int toRound)
{
    if (toRound % 10 == 0) return toRound;
    return (10 - toRound % 10) + toRound;
}

inline void Fill_TH1F_UnderOverflow(TH1F* h, double value, double weight)
{
    if(value >= h->GetXaxis()->GetXmax() ) {h->Fill(h->GetXaxis()->GetXmax() - (h->GetXaxis()->GetBinWidth(1) / 2), weight);} //overflow in last bin
    else if(value <= h->GetXaxis()->GetXmin() ) {h->Fill(h->GetXaxis()->GetXmin() + (h->GetXaxis()->GetBinWidth(1) / 2), weight);} //underflow in first bin
    else {h->Fill(value, weight);}

    return;
};

inline void Fill_TH1F_NoUnderOverflow(TH1F* h, double value, double weight)
{
    if(value < h->GetXaxis()->GetXmax() && value > h->GetXaxis()->GetXmin() ) {h->Fill(value, weight);}

    return;
};



















//--------------------------------------------
//  ######   #######  ##     ## ########     ###    ########  ########    ######## ######## ########
// ##    ## ##     ## ###   ### ##     ##   ## ##   ##     ## ##          ##       ##          ##
// ##       ##     ## #### #### ##     ##  ##   ##  ##     ## ##          ##       ##          ##
// ##       ##     ## ## ### ## ########  ##     ## ########  ######      ######   ######      ##
// ##       ##     ## ##     ## ##        ######### ##   ##   ##          ##       ##          ##
// ##    ## ##     ## ##     ## ##        ##     ## ##    ##  ##          ##       ##          ##
//  ######   #######  ##     ## ##        ##     ## ##     ## ########    ######## ##          ##
//--------------------------------------------

void Compare_Distributions(vector<TString> v_process, vector<TString> v_var, vector<TString> v_reweight_names, vector<pair<float, float>> v_min_max)
{
    cout<<endl<<UNDL(FYEL("=== Compare Distributions (different processes, EFT points, ...) ==="))<<endl<<endl;

//--------------------------------------------
    bool normalize = true; //true <--> ...
//--------------------------------------------

    //For each var, each process and each reweight, fill/store an histo
    vector<vector<vector<TH1F*>>> v3_histos_var_proc_reweight(v_var.size());
    vector<vector<vector<TH1F*>>> v3_histos_var_proc_reweight_subplot(v_var.size());

    vector<vector<vector<int>>> v3_nentries_var_proc_bin(v_var.size()); //For each bin of each var of each process, count nof entries -- for debug, understand uncerts.

    //Allocate memory to histos
    const int nbins_histos = 10;
    for(int ivar=0; ivar<v_var.size(); ivar++)
    {
        v3_histos_var_proc_reweight[ivar].resize(v_process.size());
        v3_histos_var_proc_reweight_subplot[ivar].resize(v_process.size());

        v3_nentries_var_proc_bin[ivar].resize(v_process.size());

        for(int iproc=0; iproc<v_process.size(); iproc++)
        {
            v3_histos_var_proc_reweight[ivar][iproc].resize(v_reweight_names.size());
            v3_histos_var_proc_reweight_subplot[ivar][iproc].resize(v_reweight_names.size());

            v3_nentries_var_proc_bin[ivar][iproc].resize(nbins_histos+2); //Also under/over flow bins

            for(int iweight=0; iweight<v_reweight_names.size(); iweight++)
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

        TString treename = "tree";
        TTree* t = (TTree*) f->Get(treename);
        if(t == 0) {cout<<endl<<BOLD(FRED("--- Tree not found ! Exit !"))<<endl<<endl; return;}

        cout<<FBLU("Process "<<v_process[iproc]<<" // Reading file : ")<<filepath<<endl;

        //Read branches
        vector<float>* v_reweights_floats = new vector<float>(v_reweight_names.size());
        vector<string>* v_reweights_ids = new vector<string>(v_reweight_names.size());
        vector<float> v_var_floats(v_var.size());
        t->SetBranchAddress("v_weightIds", &v_reweights_ids);
        t->SetBranchAddress("v_weights", &v_reweights_floats);
        for(int ivar=0; ivar<v_var.size(); ivar++)
        {
            t->SetBranchAddress(v_var[ivar], &v_var_floats[ivar]);
        }

        int index_Z, index_top, index_antitop;
        t->SetBranchAddress("index_Z", &index_Z);
        t->SetBranchAddress("index_top", &index_top);
        t->SetBranchAddress("index_antitop", &index_antitop);

        float mc_weight_originalValue, originalXWGTUP;
        t->SetBranchAddress("mc_weight_originalValue", &mc_weight_originalValue);
        t->SetBranchAddress("originalXWGTUP", &originalXWGTUP);

        //Protections
        t->GetEntry(0);
        for(int iweight=0; iweight<v_reweight_names.size(); iweight++)
        {
            bool weightFound = false;
            for(int iweightid=0; iweightid<v_reweights_ids->size(); iweightid++)
            {
                if(v_reweight_names[iweight] == TString(v_reweights_ids->at(iweightid)) ) {weightFound = true;}
            }
            if(!weightFound) {cout<<"ERROR ! Weight "<<v_reweight_names[iweight]<<" not found ! Abort !"<<endl; return;}
        }

        //Read and store sums of weights (SWE)
        // TH1F* h_SWE = (TH1F*) f->Get("h_SWE");
        // vector<float> v_SWE;
        // for(int ibin=0; ibin<h_SWE->GetNbinsX(); ibin++)
        // {
        //     v_SWE.push_back(h_SWE->GetBinContent(ibin+1)); //1 SWE stored for each stored weight
        //     cout<<"v_SWE[ibin] = "<<v_SWE[ibin]<<endl;
        // }

     // ###### #    # ###### #    # #####  ####     #       ####   ####  #####
     // #      #    # #      ##   #   #   #         #      #    # #    # #    #
     // #####  #    # #####  # #  #   #    ####     #      #    # #    # #    #
     // #      #    # #      #  # #   #        #    #      #    # #    # #####
     // #       #  #  #      #   ##   #   #    #    #      #    # #    # #
     // ######   ##   ###### #    #   #    ####     ######  ####   ####  #

        int nentries = t->GetEntries();
        for(int ientry=0; ientry<nentries; ientry++)
        {
            t->GetEntry(ientry);

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
                if(v_var[ivar].Contains("Z") && index_Z == -1) {continue;} //if Z not found

                if(v_var[ivar].Contains("antitop", TString::kIgnoreCase) && index_antitop == -1) {continue;} //if antitop not found
                else if(v_var[ivar].Contains("top", TString::kIgnoreCase) && index_top == -1) {continue;} //if top not found

                //-- DEBUG --
                v3_nentries_var_proc_bin[ivar][iproc][v3_histos_var_proc_reweight[ivar][iproc][0]->GetXaxis()->FindBin(v_var_floats[ivar])]++; //count +1 entry to relevant bin

                for(int iweight=0; iweight<v_reweight_names.size(); iweight++)
                {
                    // cout<<"v_reweight_names[iweight] "<<v_reweight_names[iweight]<<endl;

                    for(int iweightid=0; iweightid<v_reweights_ids->size(); iweightid++)
                    {
                        // cout<<"TString(v_reweights_ids->at(iweightid)) "<<TString(v_reweights_ids->at(iweightid))<<endl;

                        if(v_reweight_names[iweight] == TString(v_reweights_ids->at(iweightid)) )
                        {
                            // cout<<"v_reweights_floats->at(iweightid) "<<v_reweights_floats->at(iweightid)<<endl;

                            // if(normalize) {Fill_TH1F_UnderOverflow(v3_histos_var_proc_reweight[ivar][iproc][iweight], v_var_floats[ivar], v_reweights_floats->at(iweightid)/(mc_weight_originalValue * v_SWE[iweightid]));}
                            // else {Fill_TH1F_UnderOverflow(v3_histos_var_proc_reweight[ivar][iproc][iweight], v_var_floats[ivar], v_reweights_floats->at(iweightid)/mc_weight_originalValue);}

                            if(isnan(v_var_floats[ivar]) || isinf(v_var_floats[ivar]))
                            {
                                cout<<FRED("Warning ! "<<v_var[ivar]<<" = "<<v_var_floats[ivar]<<" // SKIP !")<<endl;
                                continue;
                            }

                            Fill_TH1F_UnderOverflow(v3_histos_var_proc_reweight[ivar][iproc][iweight], v_var_floats[ivar], v_reweights_floats->at(iweightid)/mc_weight_originalValue);
                            // Fill_TH1F_NoUnderOverflow(v3_histos_var_proc_reweight[ivar][iproc][iweight], v_var_floats[ivar], v_reweights_floats->at(iweightid)/mc_weight_originalValue);

                            // if(v_reweight_names[iweight] == "ctz_4p0")
                            // if(v_reweight_names[iweight] == "sm")
                            // if(v_reweights_floats->at(iweightid)/mc_weight_originalValue > 100)
                            // {
                            //     cout<<"v_var_floats[ivar] = "<<v_var_floats[ivar]<<" / SF = "<<v_reweights_floats->at(iweightid)/mc_weight_originalValue<<endl;
                            // }

                            // if(v_reweight_names[iweight] == "ctz_3p0")
                            // {
                            //     cout<<"3p0 reweight = "<<v_reweights_floats->at(iweightid)/mc_weight_originalValue<<endl;
                            // }
                            // else if(v_reweight_names[iweight] == "sm")
                            // {
                            //     cout<<"sm reweight = "<<v_reweights_floats->at(iweightid)/mc_weight_originalValue<<endl;
                            // }
                        }

                    } //Loop on event weights -- for matching
                } //Loop on selected weights -- for matching
            } //Loop on vars
        } //Loop on entries

        // delete h_SWE;
        f->Close();

        delete v_reweights_floats; delete v_reweights_ids;
    } //Loop on processes

    if(normalize) //Normalize to 1
    {
        for(int ivar=0; ivar<v_var.size(); ivar++)
        {
            for(int iproc=0; iproc<v_process.size(); iproc++)
            {
                for(int iweight=0; iweight<v_reweight_names.size(); iweight++)
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

                for(int iweight=0; iweight<v_reweight_names.size(); iweight++)
                {
                    cout<<"//--------------------------------------------"<<endl;
                    cout<<"weight "<<v_reweight_names[iweight]<<endl;
                    for(int ibin=1; ibin<v3_histos_var_proc_reweight[ivar][iproc][iweight]->GetNbinsX()+1; ibin++)
                    {
                        cout<<"bin "<<ibin<<" : "<<v3_histos_var_proc_reweight[ivar][iproc][iweight]->GetBinContent(ibin)<<" +- "<<v3_histos_var_proc_reweight[ivar][iproc][iweight]->GetBinError(ibin)<<" ("<<v3_nentries_var_proc_bin[ivar][iproc][ibin+1]<<" entries)"<<endl; //NB : first v3_nentries_var_proc_bin element is underflow
                    }
                }
            }
        }
    }

 // #####  #       ####  #####
 // #    # #      #    #   #
 // #    # #      #    #   #
 // #####  #      #    #   #
 // #      #      #    #   #
 // #      ######  ####    #

    bool setlog = false;

    for(int ivar=0; ivar<v_var.size(); ivar++)
    {
        //Compute max Y value (to adapt Y range) and min/max 'BSM/SM' scale factor (to adapt ratio plot range)
        float ymax = -1;
        float SFmax = -1;
        float SFmin = 1.;
        for(int iproc=0; iproc<v_process.size(); iproc++)
        {
            for(int iweight=0; iweight<v_reweight_names.size(); iweight++)
            {
                if(v3_histos_var_proc_reweight[ivar][iproc][iweight]->GetMaximum() > ymax)
                {
                    ymax = v3_histos_var_proc_reweight[ivar][iproc][iweight]->GetMaximum();
                    // cout<<"ymax = "<<ymax<<endl;
                }

                for(int ibin=1; ibin<v3_histos_var_proc_reweight[ivar][iproc][iweight]->GetNbinsX()+1; ibin++)
                {
                    if(v3_histos_var_proc_reweight[ivar][iproc][iweight]->GetBinContent(ibin)/v3_histos_var_proc_reweight[ivar][iproc][0]->GetBinContent(ibin) > SFmax)
                    {
                        SFmax = v3_histos_var_proc_reweight[ivar][iproc][iweight]->GetBinContent(ibin)/v3_histos_var_proc_reweight[ivar][iproc][0]->GetBinContent(ibin);
                        // cout<<"SFmax = "<<SFmax<<endl;
                    }
                    if(v3_histos_var_proc_reweight[ivar][iproc][iweight]->GetBinContent(ibin)/v3_histos_var_proc_reweight[ivar][iproc][0]->GetBinContent(ibin) < SFmin)
                    {
                        SFmin = v3_histos_var_proc_reweight[ivar][iproc][iweight]->GetBinContent(ibin)/v3_histos_var_proc_reweight[ivar][iproc][0]->GetBinContent(ibin);
                        // cout<<"SFmin = "<<SFmin<<endl;
                    }
                }
            }
        }
        // double SFmax = RoundUp(ymax / v3_histos_var_proc_reweight[ivar][iproc][0]->GetMaximum()) + 0.99; //in %

        //Canvas definition
        // Load_Canvas_Style();
        TCanvas* c1 = new TCanvas("c1","c1", 1000, 800);
        // c1->SetTopMargin(0.1);
        c1->SetBottomMargin(0.45);
        if(setlog) c1->SetLogy();
        // c1->SetGridy(1);
        // c1->SetGridx(1);

        //--- LEGEND FOR REWEIGHTS
        TLegend* legend_weights = 0;

        // legend_weights = new TLegend(0.82,0.77,0.99,0.99); //single-column top-right legend

        //1 column for each 2 entries (round up with ceil)
        float x_left = 0.99-ceil(v_reweight_names.size()/2.)*0.13; //each column takes 0.13 x-space
        legend_weights = new TLegend(x_left,0.88,0.99,0.99);
        legend_weights->SetNColumns(ceil(v_reweight_names.size()/2.));
        legend_weights->SetLineColor(1);
		legend_weights->SetTextSize(0.04);

        //--- LEGEND FOR PROCESSES (if >1 processes)
        TLegend* legend_proc = 0;
        if(v_process.size() > 1)
        {
            legend_proc = new TLegend(x_left - 0.08,0.86,x_left - 0.01,0.99);
            legend_proc->SetTextSize(0.03);
        }

        c1->cd();
        for(int iproc=0; iproc<v_process.size(); iproc++)
        {
            for(int iweight=0; iweight<v_reweight_names.size(); iweight++)
            {
                v3_histos_var_proc_reweight[ivar][iproc][iweight]->SetLineColor(iweight+1);

                if(!iweight) //only needed for first histo
                {
                    // v3_histos_var_proc_reweight[ivar][iproc][iweight]->GetYaxis()->SetTitle("Events");
                    v3_histos_var_proc_reweight[ivar][iproc][iweight]->GetYaxis()->SetTitle("a.u.");
                    // v3_histos_var_proc_reweight[ivar][iproc][iweight]->SetLineStyle(1);
                    v3_histos_var_proc_reweight[ivar][iproc][iweight]->SetLineWidth(2);

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
                    legend_weights->AddEntry(v3_histos_var_proc_reweight[ivar][iproc][iweight], GetReweightLegendName(v_reweight_names[iweight]), "L");
                }
                else {v3_histos_var_proc_reweight[ivar][iproc][iweight]->SetLineStyle(iproc+1);}

                v3_histos_var_proc_reweight[ivar][iproc][iweight]->Draw("hist E same");
            } //weight loop
        } //proc loop

        legend_weights->Draw("same");
        if(v_process.size() > 1) {legend_proc->Draw("same");}

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
            for(int iweight=0; iweight<v_reweight_names.size(); iweight++)
            {
                if(v_reweight_names[iweight] != "sm")
                {
                    v3_histos_var_proc_reweight_subplot[ivar][iproc][iweight] = (TH1F*) v3_histos_var_proc_reweight[ivar][iproc][iweight]->Clone(); //Copy histo
                    // v3_histos_var_proc_reweight_subplot[ivar][iproc][iweight]->Add(v3_histos_var_proc_reweight[ivar][iproc][0], -1); //Substract nominal
                    v3_histos_var_proc_reweight_subplot[ivar][iproc][iweight]->Divide(v3_histos_var_proc_reweight[ivar][iproc][0]); //Divide by nominal

                    // for(int i=0; i<10; i++)
                    // {
                    //     cout<<"v3_histos_var_proc_reweight_subplot[ivar][iproc][iweight]->GetBinContent(i+1)"<<v3_histos_var_proc_reweight_subplot[ivar][iproc][iweight]->GetBinContent(i+1)<<endl;
                    // }

                    v3_histos_var_proc_reweight_subplot[ivar][iproc][iweight]->GetXaxis()->SetTitle(v_var[ivar]);
                    v3_histos_var_proc_reweight_subplot[ivar][iproc][iweight]->GetYaxis()->CenterTitle();
                    v3_histos_var_proc_reweight_subplot[ivar][iproc][iweight]->GetYaxis()->SetTitle("BSM/SM");
                    // v3_histos_var_proc_reweight_subplot[ivar][iproc][iweight]->GetYaxis()->SetTitle("BSM/SM [%]");
                    // v3_histos_var_proc_reweight_subplot[ivar][iproc][iweight]->GetYaxis()->SetTitle("#frac{(X-#mu)}{#mu} [%]");
                    v3_histos_var_proc_reweight_subplot[ivar][iproc][iweight]->GetYaxis()->SetTitleOffset(1.4);
                    v3_histos_var_proc_reweight_subplot[ivar][iproc][iweight]->GetYaxis()->SetTickLength(0.);
                    v3_histos_var_proc_reweight_subplot[ivar][iproc][iweight]->GetXaxis()->SetTitleOffset(1.);
                    v3_histos_var_proc_reweight_subplot[ivar][iproc][iweight]->GetYaxis()->SetLabelSize(0.048);
                    // v3_histos_var_proc_reweight_subplot[ivar][iproc][iweight]->GetXaxis()->SetLabelFont(42);
                    // v3_histos_var_proc_reweight_subplot[ivar][iproc][iweight]->GetYaxis()->SetLabelFont(42);
                    // v3_histos_var_proc_reweight_subplot[ivar][iproc][iweight]->GetXaxis()->SetTitleFont(42);
                    // v3_histos_var_proc_reweight_subplot[ivar][iproc][iweight]->GetYaxis()->SetTitleFont(42);
                    // v3_histos_var_proc_reweight_subplot[ivar][iproc][iweight]->GetYaxis()->SetNdivisions(503); //grid draw on primary tick marks only
                    v3_histos_var_proc_reweight_subplot[ivar][iproc][iweight]->GetYaxis()->SetNdivisions(507); //grid draw on primary tick marks only
                    v3_histos_var_proc_reweight_subplot[ivar][iproc][iweight]->GetYaxis()->SetTitleSize(0.05);
                    v3_histos_var_proc_reweight_subplot[ivar][iproc][iweight]->GetXaxis()->SetTickLength(0.04);

                    // v3_histos_var_proc_reweight_subplot[ivar][iproc][iweight]->Scale(100.); //express in %
                    // v3_histos_var_proc_reweight_subplot[ivar][iproc][iweight]->SetMinimum(0.80);
                    // v3_histos_var_proc_reweight_subplot[ivar][iproc][iweight]->SetMaximum(+3.);
                    v3_histos_var_proc_reweight_subplot[ivar][iproc][iweight]->SetMinimum(SFmin*0.8);
                    v3_histos_var_proc_reweight_subplot[ivar][iproc][iweight]->SetMaximum(SFmax*1.2);

                    v3_histos_var_proc_reweight_subplot[ivar][iproc][iweight]->SetLineColor(iweight+1);
                    v3_histos_var_proc_reweight_subplot[ivar][iproc][iweight]->SetLineWidth(2);
                    // v3_histos_var_proc_reweight_subplot[ivar][iproc][iweight]->SetLineStyle(2);

                    v3_histos_var_proc_reweight_subplot[ivar][iproc][iweight]->Draw("hist E same");
                }
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
            latex.SetTextSize(0.03);
            latex.DrawLatex(0.17, 0.95, text);
        }

        TString outputname = "./" + v_var[ivar] + ".png";
        c1->SaveAs(outputname);

        delete pad_ratio;
        delete legend_weights;
        if(v_process.size() > 1) {delete legend_proc;}
        delete c1;
    }

    for(int ivar=0; ivar<v_var.size(); ivar++)
    {
        for(int iproc=0; iproc<v_process.size(); iproc++)
        {
            for(int iweight=0; iweight<v_reweight_names.size(); iweight++)
            {
                delete v3_histos_var_proc_reweight[ivar][iproc][iweight];
                delete v3_histos_var_proc_reweight_subplot[ivar][iproc][iweight];
            }
        }
    }

    return;
}
















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
    vector<TString> v_process;
    v_process.push_back("ttz");
    // v_process.push_back("tzq");
    // v_process.push_back("tllq");
    // v_process.push_back("ttll");

    vector<TString> v_var; vector<pair<float, float>> v_min_max;
    v_var.push_back("Z_pt"); v_min_max.push_back(std::make_pair(0, 500));
    v_var.push_back("Z_eta"); v_min_max.push_back(std::make_pair(-5, 5));
    // v_var.push_back("Z_m"); v_min_max.push_back(std::make_pair(70, 100));
    // v_var.push_back("Top_pt"); v_min_max.push_back(std::make_pair(0, 500));
    // v_var.push_back("Top_eta"); v_min_max.push_back(std::make_pair(-5, 5));
    // v_var.push_back("Top_m"); v_min_max.push_back(std::make_pair(0, 300));
    // v_var.push_back("TopZsystem_m"); v_min_max.push_back(std::make_pair(250, 1000));
    // v_var.push_back("LeadingTop_pt"); v_min_max.push_back(std::make_pair(0, 500));
    // v_var.push_back("LeadingTop_eta"); v_min_max.push_back(std::make_pair(-5, 5));
    // v_var.push_back("Zreco_dPhill"); v_min_max.push_back(std::make_pair(0, 6));
    // v_var.push_back("cosThetaStarPol_Z"); v_min_max.push_back(std::make_pair(-1, 1));
    // v_var.push_back("cosThetaStarPol_Top"); v_min_max.push_back(std::make_pair(-1, 1));

    vector<TString> v_reweight_names; vector<int> v_colors;
    v_reweight_names.push_back("sm"); //Nominal SM weight -- keep

    // v_reweight_names.push_back("ctz_0p1");
    // v_reweight_names.push_back("ctz_0p5");
    // v_reweight_names.push_back("ctz_1p0");
    // v_reweight_names.push_back("ctz_1p5");
    v_reweight_names.push_back("ctz_2p0");
    // v_reweight_names.push_back("ctz_2p5");
    // v_reweight_names.push_back("ctz_3p0");
    v_reweight_names.push_back("ctz_4p0");
    v_reweight_names.push_back("ctz_5p0");
    v_reweight_names.push_back("ctz_8p0");
    // v_reweight_names.push_back("ctz_10p0");

    // v_reweight_names.push_back("ctw_0p5");
    // v_reweight_names.push_back("ctw_1p0");
    // v_reweight_names.push_back("ctw_3p0");
    // v_reweight_names.push_back("ctw_5p0");
    // v_reweight_names.push_back("ctw_8p0");

    Load_Canvas_Style();

    Compare_Distributions(v_process, v_var, v_reweight_names, v_min_max);

    return 0;
}
