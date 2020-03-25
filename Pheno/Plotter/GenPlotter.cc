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
#include "/home/ntonon/Postdoc/EFTSimulations/myAnalysis/Utils/TH1EFT.h"
#include "/home/ntonon/Postdoc/EFTSimulations/myAnalysis/Utils/WCPoint.h"
#include "/home/ntonon/Postdoc/EFTSimulations/myAnalysis/Utils/WCFit.h"

#include <cassert>     //Can be used to terminate program if argument is not true.
//Ex : assert(test > 0 && "Error message");
#include <sys/stat.h> // to be able to use mkdir

//Hardcoded values if want to plot real nof events... (corresponding to cuts applied in GenAnalyzer)
#define xsec_tllq 30.5
#define xsec_ttll 281
#define lumi 41.5

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

// See http://www.martinbroadhurst.com/how-to-split-a-string-in-c.html //Included in headers
// template <class Container>
// void split_string(const std::string& str, Container& cont, const std::string& delims = " ") {
//     boost::split(cont,str,boost::is_any_of(delims));
// }

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
	gStyle->SetOptTitle(1); //CHANGED //Main title
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

//Get proper name for operator
//NB : ts1.CompareTo("ts2", TString::kIgnoreCase) returns 0 if both string are equal, case-insensitive
TString Get_Operator_Name(TString ts)
{
    if(!ts.CompareTo("ctz", TString::kIgnoreCase)) {return "c_{tZ}";}
    else if(!ts.CompareTo("ctw", TString::kIgnoreCase)) {return "c_{tW}";}
    else if(!ts.CompareTo("cpqm", TString::kIgnoreCase)) {return "c^{-}_{#varphiQ}";}
    else if(!ts.CompareTo("cpq3", TString::kIgnoreCase)) {return "c^{3}_{#varphiQ}";}
    else if(!ts.CompareTo("cpt", TString::kIgnoreCase)) {return "c_{#varphit}";}

    return ts;
}

//Provided the full name of a rwgt point, returns the corresponding list of operators names
//NB : order of operators in the reweights must always be the same !
TString Get_List_Operators(TString full_rwgt_name)
{
    if(!full_rwgt_name.BeginsWith("rwgt_", TString::kIgnoreCase) ) {return "";}
    // cout<<"full_rwgt_name "<<full_rwgt_name<<endl;

    TString result = "{ ";
    std::vector<std::string> words;
    split_string((string) full_rwgt_name, words, "_");
    for(int iw=1; iw<words.size()-1; iw+=2) //ignore first word 'rwgt', ignore WC values
    {
        result+= Get_Operator_Name(words[iw]) + " / ";
    }
    result = result.Strip(TString::kTrailing, ' '); //remove trailing hchar
    result = result.Strip(TString::kTrailing, '/'); //remove trailing hchar
    result+= " }";

    return result;
}

//Get proper reweight name for legend
// NB : looks like reweight names are automatically made lowercase-only !
TString GetReweightLegendName(TString variationname)
{
    TString result = variationname;

    if(variationname.Contains("sm", TString::kIgnoreCase) ) {result = "SM";}
    else if(variationname.Contains("rwgt_", TString::kIgnoreCase))
    {
        result = "(";
        std::vector<std::string> words;
        split_string((string) variationname, words, "_");
        for(int iw=1; iw<words.size()-1; iw+=2) //ignore first word 'rwgt'
        {
            TString ts_val = (TString) words[iw+1];
            // if(ts_val == '0') {continue;} //don't printout null WCs
            if(ts_val.EndsWith(".0")) {ts_val.Remove(TString::kTrailing, '0'); ts_val.Remove(TString::kTrailing, '.');} //remove trailing 0
            // result+= Get_Operator_Name(words[iw]) + " " + ts_val + "/";
            result+= ts_val + '/';
        }
        result = result.Strip(TString::kTrailing, '/'); //remove trailing hchar
        result+= ")";
    }

    // result+= " [TeV^{-2}]";

    return result;
}

TString GetProcessLegendName(TString proc)
{
    if(!proc.CompareTo("ttz", TString::kIgnoreCase)) {return "t#bar{t}Z";}
    else if(!proc.CompareTo("tzq", TString::kIgnoreCase)) {return "tZq";}
    else if(!proc.CompareTo("ttll", TString::kIgnoreCase)) {return "t#bar{t}ll";}
    else if(!proc.CompareTo("tllq", TString::kIgnoreCase)) {return "tllq";}

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

inline void Fill_TH1EFT_UnderOverflow(TH1EFT* h, double value, float weight, WCFit fit)
{
    if(value >= h->GetXaxis()->GetXmax() ) {h->Fill(h->GetXaxis()->GetXmax() - (h->GetXaxis()->GetBinWidth(1) / 2), weight, fit);} //overflow in last bin
    else if(value <= h->GetXaxis()->GetXmin() ) {h->Fill(h->GetXaxis()->GetXmin() + (h->GetXaxis()->GetBinWidth(1) / 2), weight, fit);} //underflow in first bin
    else {h->Fill(value, weight, fit);}
    return;
};

inline void Fill_TH1F_NoUnderOverflow(TH1F* h, double value, double weight)
{
    if(value < h->GetXaxis()->GetXmax() && value > h->GetXaxis()->GetXmin() ) {h->Fill(value, weight);}
    return;
};

//Rename variables for axis title
TString Get_Variable_Name(TString var)
{
    if(var == "Z_pt") {return "p_{T}(Z)";}
    if(var == "Z_eta") {return "#eta(Z)";}
    if(var == "Z_m") {return "m(Z)";}
    if(var == "Zreco_m") {return "m(Z_{reco})";}
    if(var == "Top_pt") {return "p_{T}(t)";}
    if(var == "Top_eta") {return "#eta(t)";}
    if(var == "Top_m") {return "m(t)";}
    if(var == "TopZsystem_m") {return "m(tZ)";}
    if(var == "LeadingTop_pt") {return "p_{T}(t^{lead})";}
    if(var == "LeadingTop_eta") {return "#eta(t^{lead})";}
    if(var == "Zreco_dPhill") {return "#Delta#varphi_{ll}";}
    if(var == "cosThetaStarPol_Z") {return "cos(#theta^{*}_{Z})";}
    if(var == "cosThetaStarPol_Top") {return "cos(#theta^{*}_{t})";}

    return var;
}

/**
 * Returns a color depending on index given in arg
 */
int Get_Color(int index)
{
	vector<int> v_colors;

    //Try to get many colors as different as possible
    v_colors.push_back(kBlack); //SM is black
	v_colors.push_back(kRed);
	v_colors.push_back(kBlue);
	v_colors.push_back(kGreen+1);
	v_colors.push_back(kViolet-1);
    v_colors.push_back(kOrange-1);
    v_colors.push_back(kPink+5);
    v_colors.push_back(kCyan-3);

	if(index < v_colors.size() ) {return v_colors[index];}
	else {return index;}
}


 // ###### ###### #####    ###### #    # #    #  ####   ####
 // #      #        #      #      #    # ##   # #    # #
 // #####  #####    #      #####  #    # # #  # #       ####
 // #      #        #      #      #    # #  # # #           #
 // #      #        #      #      #    # #   ## #    # #    #
 // ###### #        #      #       ####  #    #  ####   ####

/**
 * For given event, create the WCFit object and fill the TH1EFT with it
*/
void Fill_TH1EFT(TH1EFT*& h, float x, vector<string>* v_reweights_ids, vector<float>* v_reweights_floats, float originalXWGTUP, vector<float> v_SWE, float weight_SF=0, bool show_overflow=false)
{
    bool debug = false;
//--------------------------------------------
    float sm_wgt = 0.; //Weight of SM point
    // float sm_swe = 0.; //SWE of SM point
    std::vector<WCPoint> wc_pts;

    // if(isPrivMC) // Add EFT weights
    {
        for(int iwgt=1; iwgt<v_reweights_ids->size(); iwgt++) //First weight is always 'rwgt_1' (corresponding to originalXWGTUP weight ?)
        {
            // cout<<"v_reweights_ids->at(iwgt) "<<v_reweights_ids->at(iwgt)<<" / w = "<<v_reweights_floats->at(iwgt)<<" / v_SWE[iwgt] "<<v_SWE[iwgt]<<endl;

            TString ts = v_reweights_ids->at(iwgt);
            if(ts.Contains("rwgt_") && ts != "rwgt_1" && !ts.Contains("_sm", TString::kIgnoreCase))
            // std::size_t foundstr = v_reweights_ids[iwgt].find("rwgt_");// only save EFT weights
            // if(foundstr != std::string::npos)
            {
                //FIXME -- which is correct ?
                float w = v_reweights_floats->at(iwgt);
                // float w = v_reweights_floats->at(iwgt)/(originalXWGTUP * v_SWE[iwgt]);
                // float w = v_reweights_floats->at(iwgt) / v_SWE[iwgt];
                // float w = v_reweights_floats->at(iwgt) / originalXWGTUP;

                WCPoint wc_pt(v_reweights_ids->at(iwgt), w);

                wc_pts.push_back(wc_pt);
            }
            else if(!ts.CompareTo("rwgt_sm", TString::kIgnoreCase)) //Store SM weight manually
            {

                float w = v_reweights_floats->at(iwgt);
                // float w = v_reweights_floats->at(iwgt)/(originalXWGTUP * v_SWE[iwgt]);
                // float w = v_reweights_floats->at(iwgt) / v_SWE[iwgt];
                // float w = v_reweights_floats->at(iwgt) / originalXWGTUP;

                sm_wgt = w;
                // sm_swe = v_SWE[iwgt];
            }
        } //weights loop

        //-- Include 'manually' the SM point as first element (not included automatically because named 'SM' and not via its operator values)
        wc_pts.insert(wc_pts.begin(), wc_pts[0]); //Duplicate the first element
        wc_pts[0].setSMPoint(); //Set (new) first element to SM coeffs (all null)
        wc_pts[0].wgt = sm_wgt; //Set (new) first element to SM weight
    }

    WCFit eft_fit(wc_pts, "");

    if(debug) //Printout WC values, compare true weight to corresponding fit result
    {
        cout<<"//-------------------------------------------- "<<endl;

        eft_fit.dump(); //Printout all names and coefficients of WC pairs

        for (uint i=0; i < wc_pts.size(); i++)
        {
            WCPoint wc_pt = wc_pts.at(i);
            double fit_val = eft_fit.evalPoint(&wc_pt);
            wc_pt.dump(); //Printout names and values of all WCs for this point
            std::cout << "===> " << std::setw(3) << i << ": " << std::setw(12) << wc_pt.wgt << " | " << std::setw(12) << fit_val << " | " << std::setw(12) << (wc_pt.wgt-fit_val) << std::endl; //Printout : i / true weight / evaluated weight / diff
        }

        // cout<<endl<<endl<<endl;
        // WCPoint wcp;
        // string name = "rwgt_ctz_0_ctw_0_cpqm_0_cpq3_0_cpt_0";
        // wcp = WCPoint(name, 0.);
        // wcp.dump();
        // cout<<"eft_fit.evalPoint(SM) => "<<eft_fit.evalPoint(&wcp)<<endl;
        // name = "rwgt_ctz_3.0_ctw_3.0_cpqm_3.0_cpq3_3.0_cpt_3.0";
        // wcp = WCPoint(name, 0.);
        // wcp.dump();
        // cout<<"eft_fit.evalPoint("<<name<<") => "<<eft_fit.evalPoint(&wcp)<<endl;
        // name = "rwgt_ctz_2.0_ctw_2.0_cpqm_2.0_cpq3_2.0_cpt_2.0";
        // wcp = WCPoint(name, 0.);
        // wcp.dump();
        // cout<<"eft_fit.evalPoint("<<name<<") => "<<eft_fit.evalPoint(&wcp)<<endl;
        // name = "rwgt_ctz_2.0_ctw_2.0_cpqm_2.0_cpq3_2.0_cpt_-2.0";
        // wcp = WCPoint(name, 0.);
        // wcp.dump();
        // cout<<"eft_fit.evalPoint("<<name<<") => "<<eft_fit.evalPoint(&wcp)<<endl;

        // cout<<"originalXWGTUP "<<originalXWGTUP<<endl;
        cout<<endl<<endl<<endl;
    }

    //Fill with SM weight by default
    if(show_overflow) {Fill_TH1EFT_UnderOverflow(h, x, sm_wgt, eft_fit);}
    else {h->Fill(x, sm_wgt, eft_fit);}

    return;
}

/**
 * Parameterization of the xsec as a function of operator1 (x) and operator2 (y). See definition of each coeff
 */
Double_t EFT_Fit_Parameterization(Double_t* val, Double_t* par)
{
   Float_t x = val[0];
   Float_t y = val[1];

   return (par[0] + x*par[1] + y*par[2] + x*y*par[3] + x*x*par[4] + y*y*par[5]) / par[0];
}

















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

        TString treename = "tree";
        // TString treename = "GenAnalyzer/tree"; //new name
        TTree* t = (TTree*) f->Get(treename);
        if(t == 0) {cout<<endl<<BOLD(FRED("--- Tree not found ! Exit !"))<<endl<<endl; return;}

        cout<<FBLU("Process : "<<v_process[iproc]<<" // Reading file : ")<<filepath<<endl<<endl;

        //Try to get proper event normalization ? (but depends on cuts in GenAnalyzer)
        float weight_SF = xsec_tllq * lumi;
        if(v_process[iproc] == "tllq" || v_process[iproc] == "ttz") {weight_SF = xsec_ttll * lumi;}

        //Read branches
        vector<float>* v_reweights_floats = new vector<float>(v_reweightNames_fromMG.size());
        vector<string>* v_reweights_ids = new vector<string>(v_reweightNames_fromMG.size());
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
        TString hSWEname = "h_SWE";
        // TString hSWEname = "GenAnalyzer/h_SWE"; //new name
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
                if(v_var[ivar].Contains("Z_") && index_Z == -1) {continue;} //if Z not found

                if(v_var[ivar].Contains("antitop", TString::kIgnoreCase) && index_antitop == -1) {continue;} //if antitop not found
                if(v_var[ivar].Contains("top", TString::kIgnoreCase) && index_top == -1) {continue;} //if top not found

                //FIXME -- tmp check
                if(v_var[ivar].Contains("Zreco_") && v_var_floats[ivar] <= 0) {continue;} //No Z->ll reco

                //Count nentries for debugging
                v3_nentries_var_proc_bin[ivar][iproc][v3_histos_var_proc_reweight[ivar][iproc][0]->GetXaxis()->FindBin(v_var_floats[ivar])]++; //add +1 entry to relevant bin

                for(int iweight=0; iweight<v_reweightNames_fromMG.size(); iweight++)
                {
                    // cout<<"v_reweightNames_fromMG[iweight] "<<v_reweightNames_fromMG[iweight]<<endl;

                    for(int iweightid=0; iweightid<v_reweights_ids->size(); iweightid++)
                    {
                        // cout<<"TString(v_reweights_ids->at(iweightid)) "<<TString(v_reweights_ids->at(iweightid))<<endl;

                        if(v_reweightNames_fromMG[iweight] == TString(v_reweights_ids->at(iweightid)) )
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
                        }

                    } //Loop on event weights -- for matching
                } //Loop on selected weights -- for matching

                Fill_TH1EFT(v2_TH1EFT_var_proc[ivar][iproc], v_var_floats[ivar], v_reweights_ids, v_reweights_floats, originalXWGTUP, v_SWE, weight_SF, show_overflow);

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
// ##     ##  ######  ########  ######     ##     ##  ######     ######## ######## ########
//  ##   ##  ##    ## ##       ##    ##    ##     ## ##    ##    ##       ##          ##
//   ## ##   ##       ##       ##          ##     ## ##          ##       ##          ##
//    ###     ######  ######   ##          ##     ##  ######     ######   ######      ##
//   ## ##         ## ##       ##           ##   ##        ##    ##       ##          ##
//  ##   ##  ##    ## ##       ##    ##      ## ##   ##    ##    ##       ##          ##
// ##     ##  ######  ########  ######        ###     ######     ######## ##          ##
//--------------------------------------------

/**
 * Plot the process cross section as a function of the WCs of 2 operators (2D and 3D plots)
 * First, loop on all entries and fill a TH1EFT object with all the events fits
 * Then, rescale the TH1EFT at all points of a 2D scan (2 operators), store the sums of weights (= xsecs if properly normalized), and plot values
 */
void Plot_CrossSection_VS_WilsonCoeff(TString process, TString operator1, TString operator2)
{
    bool debug = false;
    bool relative_to_SM = true; //true <-> compare all points to SM value

 //  ####  ###### ##### #    # #####
 // #      #        #   #    # #    #
 //  ####  #####    #   #    # #    #
 //      # #        #   #    # #####
 // #    # #        #   #    # #
 //  ####  ######   #    ####  #

    cout<<endl<<BOLD(UNDL(FYEL("=== Plot XSEC .vs. WCs ===")))<<endl<<endl;

    if(operator1 == "" || operator2 == "") {cout<<FRED("Error ! Wrong operator name ! Abort ! ")<<endl; return;}

    TString dir = "./";
    TString filepath = dir + "output_" + process + ".root";

    TFile* f = TFile::Open(filepath);
    if(f == 0) {cout<<endl<<BOLD(FRED("--- File not found ! Exit !"))<<endl<<endl; return;}

    // TString treename = "tree";
    TString treename = "GenAnalyzer/tree"; //new name
    TTree* t = (TTree*) f->Get(treename);
    if(t == 0) {cout<<endl<<BOLD(FRED("--- Tree not found ! Exit !"))<<endl<<endl; return;}

    cout<<FBLU("Process : "<<process<<" // Reading file : ")<<filepath<<endl<<endl;

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

    //Read branches
    vector<float>* v_reweights_floats = new vector<float>();
    vector<string>* v_reweights_ids = new vector<string>();
    t->SetBranchAddress("v_weightIds", &v_reweights_ids);
    t->SetBranchAddress("v_weights", &v_reweights_floats);

    float originalXWGTUP;
    t->SetBranchAddress("originalXWGTUP", &originalXWGTUP);

    TH1EFT* h = new TH1EFT("", "", 1, 0, 1); //TH1EFT storing the weights and fits for all events

    int nentries = 1000;
    // int nentries = t->GetEntries();
    cout<<FMAG("Processing "<<nentries<<" entries...")<<endl;
    for(int ientry=0; ientry<nentries; ientry++)
    {
        t->GetEntry(ientry);

        Fill_TH1EFT(h, 0.5, v_reweights_ids, v_reweights_floats, originalXWGTUP, v_SWE, 0, false);
    }

    // ###### #    # ##### #####    ##   #####   ####  #
    // #       #  #    #   #    #  #  #  #    # #    # #
    // #####    ##     #   #    # #    # #    # #    # #
    // #        ##     #   #####  ###### #####  #    # #
    // #       #  #    #   #   #  #    # #      #    # #
    // ###### #    #   #   #    # #    # #       ####  ######

    cout<<FMAG("Creating 2D plot...")<<endl;

    //1D vectors of WC values to cover for each operator
    vector<float> v_grid_op1;
    vector<float> v_grid_op2;
    // int min_op1 = -100., max_op1=100; //min max operator1
    // int min_op2 = -100., max_op2=100; //min max operator2
    // float step = 5; //Steps between WC values
    int min_op1 = -25., max_op1=25; //min max operator1
    int min_op2 = -25., max_op2=25; //min max operator2
    float step = 1; //Steps between WC values

    //Final TH2F to plot -- store sums of weights at each point of the 2D scan
    // TH2F* h2 = new TH2F("test1", "test2", (max_op1-min_op1)/step, min_op1, max_op1, (max_op2-min_op2)/step, min_op2, max_op2);

    //To parameterize the xsec as a function of 2 operators, 6 independent coefficients are required
    //Extract these parameters, and use them to parameterize smoothly the xsec as a function of the 2 operators -> plot
    WCFit fit = h->GetSumFit(); //Get summed fit (over all bins)
    Double_t c0, c1, c2, c3, c4, c5; //Get all necessary fit coeffs
    c0 = fit.getCoefficient("rwgt_SM", "rwgt_SM");
    c1 = fit.getCoefficient("rwgt_SM", (string) operator1);
    c2 = fit.getCoefficient("rwgt_SM", (string) operator2);
    c3 = fit.getCoefficient((string) operator1, (string) operator2);
    c4 = fit.getCoefficient((string) operator1, (string) operator1);
    c5 = fit.getCoefficient((string) operator2, (string) operator2);

    //Create a TF2 object using the proper xsec parameterization
    TF2 *tf2 = new TF2("f2", EFT_Fit_Parameterization, min_op1, max_op1, min_op2, max_op2, 6);
    tf2->SetParameters(c0,c1,c2,c3,c4,c5);

/*
    //Get SM integral, for rescaling
    TString rwgt_name_SM = "rwgt_"+operator1+"_0_"+operator2+"_0";
    WCPoint wcp_SM = WCPoint((string) rwgt_name_SM, 1.);
    h->Scale(wcp_SM);
    float SM_integral = h->Integral();

    for(float x1=min_op1; x1<=max_op1; x1+=step)
    {
        for(float x2=min_op2; x2<=max_op2; x2+=step)
        {
            //Get reweight ID corresponding to current scanning point
            TString rwgt_name_tmp = "rwgt_"+operator1+"_"+std::to_string(x1)+"_"+operator2+"_"+std::to_string(x2);
            if(debug) cout<<"rwgt_name_tmp "<<rwgt_name_tmp<<endl;

            //Rescale TH1EFT accordingly to current reweight
            //NB : no need to rescale each bin separately ! Only care about total integral, so can rescale global fit manually instead...
            WCPoint wcp = WCPoint((string) rwgt_name_tmp, 1.);
            // h->Scale(wcp);
            // float value = h->Integral();
            WCFit fit = h->GetSumFit();
            float value = fit.evalPoint(&wcp);

            Int_t binx = h2->GetXaxis()->FindBin(x1);
            Int_t biny = h2->GetYaxis()->FindBin(x2);
            Int_t bin = h2->GetBin(binx, biny, 0);

            if(relative_to_SM) {value/= SM_integral;}

            if(debug) cout<<"value "<<value<<endl;

            h2->SetBinContent(bin, value);
        }
    }
*/

    TCanvas* c = new TCanvas("c","c", 1000, 800);
    c->SetTopMargin(0.13);
    c->SetRightMargin(0.11);
    c->SetGridx(1);
    c->SetGridy(1);

 //  ####   ####   ####  #    # ###### ##### #  ####   ####
 // #    # #    # #      ##  ## #        #   # #    # #
 // #      #    #  ####  # ## # #####    #   # #       ####
 // #      #    #      # #    # #        #   # #           #
 // #    # #    # #    # #    # #        #   # #    # #    #
 //  ####   ####   ####  #    # ######   #   #  ####   ####

    //Color palettes : https://root.cern.ch/doc/master/classTColor.html
    // gStyle->SetPalette(kDeepSea);

    TString plot_title = "#sigma_{EFT}/#sigma_{SM} vs "+Get_Operator_Name(operator1)+" , "+Get_Operator_Name(operator2);
    // TString plot_title = "#frac{#sigma_{EFT}}{#sigma_{SM}} vs { "+Get_Operator_Name(operator1)+" , "+Get_Operator_Name(operator2)+" }";
    // h2->SetTitle(plot_title);
    // h2->GetXaxis()->SetTitle(Get_Operator_Name(operator1));
    // h2->GetYaxis()->SetTitle(Get_Operator_Name(operator2));
    // h2->GetZaxis()->SetTitle("a.u.");
    // h2->GetZaxis()->SetLabelSize(0.03);

    //Increase resolution //Default is 20 for 2d func //Use at least 100 //Slows down significantly
    tf2->SetNpx(100);
    tf2->SetNpy(100);

    tf2->GetHistogram()->SetTitle(plot_title);
    tf2->GetHistogram()->GetXaxis()->SetTitle(Get_Operator_Name(operator1));
    tf2->GetHistogram()->GetYaxis()->SetTitle(Get_Operator_Name(operator2));
    tf2->GetHistogram()->GetZaxis()->SetLabelSize(0.03);
    // tf2->GetHistogram()->GetZaxis()->SetTitle("a.u.");

    //Draw marker on SM point //See : https://root.cern.ch/doc/master/classTAttMarker.html
    TGraph *SM_marker = new TGraph();
    SM_marker->SetPoint(0, 0, 0);
    SM_marker->SetMarkerStyle(5);
    SM_marker->SetMarkerSize(2.);
    SM_marker->SetMarkerColor(kBlack);

    TString text = "pp #rightarrow " + GetProcessLegendName(process);
    TLatex latex;
    latex.SetNDC();
    latex.SetTextAlign(11);
    latex.SetTextFont(52);
    latex.SetTextSize(0.04);
    latex.DrawLatex(0.17, 0.95, text);

 // #####  #       ####  #####
 // #    # #      #    #   #
 // #    # #      #    #   #
 // #####  #      #    #   #
 // #      #      #    #   #
 // #      ######  ####    #

    //2D plot
    tf2->Draw("colz");
    SM_marker->Draw("P same");
    TString outname = "xsec_vs_"+operator1+"_"+operator2+"_2D.png";
    c->SaveAs(outname);

    //3D plot
    c->SetRightMargin(0.05);
    // c->SetLeftMargin(0.05);
    tf2->GetHistogram()->GetXaxis()->SetTitleOffset(1.5);
    tf2->GetHistogram()->GetYaxis()->SetTitleOffset(1.5);
    tf2->GetHistogram()->GetXaxis()->SetNdivisions(505);
    tf2->GetHistogram()->GetYaxis()->SetNdivisions(505);
    tf2->Draw("SURF2");
    outname = "xsec_vs_"+operator1+"_"+operator2+"_3D.png";
    c->SaveAs(outname);

    delete h;
    // delete h2;
    delete tf2;
    delete c;
    delete SM_marker;

    delete v_reweights_floats;
    delete v_reweights_ids;

    return;
}

/**
 * Call function Plot_CrossSection_VS_WilsonCoeff() for each pair of operators
*/
void Plot_CrossSection_VS_WilsonCoeff_AllPairsOperators(TString process)
{
    vector<TString> v_operators;
    v_operators.push_back("ctz");
    v_operators.push_back("ctw");
    v_operators.push_back("cpqm");
    v_operators.push_back("cpq3");
    v_operators.push_back("cpt");

    cout<<endl<<BOLD(UNDL(FYEL("=== Plot XSEC .vs. WCs for all pairs of operators ===")))<<endl<<endl;

    for(int i1=0; i1<v_operators.size(); i1++)
    {
        for(int i2=0; i2<v_operators.size(); i2++)
        {
            if(i1==i2) {continue;}

            cout<<endl<<FMAG("=== Pair : "<<v_operators[i1]<<", "<<v_operators[i2]<<"")<<endl<<endl;

            Plot_CrossSection_VS_WilsonCoeff(process, v_operators[i1], v_operators[i2]);
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
    //-- List of processes to superimpose
    vector<TString> v_process;
    // v_process.push_back("ttz");
    // v_process.push_back("tzq");
    v_process.push_back("tllq");
    // v_process.push_back("ttll");

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
    // v_reweightNames_fromMG.push_back("rwgt_ctz_-2.0_ctw_2.0_cpqm_2.0_cpq3_-2.0_cpt_2.0");
    // v_reweightNames_fromMG.push_back("rwgt_ctz_-2.0_ctw_-2.0_cpqm_-2.0_cpq3_-2.0_cpt_2.0");

    //-- List of weights to plot *which will be obtained from the TH1EFT extrapolation* (+ colors)
    //NB : 'rwgt_sm' will crash ; explicitely set all the WCs to 0 instead
    vector<TString> v_reweightNames_extrapol;
    // v_reweightNames_extrapol.push_back("rwgt_ctz_0_ctw_0_cpqm_0_cpq3_0_cpt_0");
    // v_reweightNames_extrapol.push_back("rwgt_ctz_2.0_ctw_2.0_cpqm_2.0_cpq3_2.0_cpt_2.0");

    v_reweightNames_extrapol.push_back("rwgt_ctz_5_ctw_0_cpqm_0_cpq3_0_cpt_0");
    // v_reweightNames_extrapol.push_back("rwgt_ctz_0_ctw_5_cpqm_0_cpq3_0_cpt_0");
    // v_reweightNames_extrapol.push_back("rwgt_ctz_0_ctw_0_cpqm_5_cpq3_0_cpt_0");
    // v_reweightNames_extrapol.push_back("rwgt_ctz_0_ctw_0_cpqm_0_cpq3_5_cpt_0");
    // v_reweightNames_extrapol.push_back("rwgt_ctz_0_ctw_0_cpqm_0_cpq3_0_cpt_5");

    Load_Canvas_Style();

    // Compare_Distributions(v_process, v_var, v_reweightNames_fromMG, v_reweightNames_extrapol, v_min_max);

    Plot_CrossSection_VS_WilsonCoeff("tllq", "ctw", "ctz");

    // Plot_CrossSection_VS_WilsonCoeff_AllPairsOperators("tllq");

    return 0;
}
