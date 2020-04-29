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

// See http://www.martinbroadhurst.com/how-to-split-a-string-in-c.html //Included in headers
// template <class Container>
// void split_string(const std::string& str, Container& cont, const std::string& delims = " ") {boost::split(cont,str,boost::is_any_of(delims));}

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





//--------------------------------------------
// ######## ######## ########    ######## ##     ## ##    ##  ######   ######
// ##       ##          ##       ##       ##     ## ###   ## ##    ## ##    ##
// ##       ##          ##       ##       ##     ## ####  ## ##       ##
// ######   ######      ##       ######   ##     ## ## ## ## ##        ######
// ##       ##          ##       ##       ##     ## ##  #### ##             ##
// ##       ##          ##       ##       ##     ## ##   ### ##    ## ##    ##
// ######## ##          ##       ##        #######  ##    ##  ######   ######
//--------------------------------------------

/**
 * For given event, create the WCFit object and fill the TH1EFT with it
*/
void FillTH1EFT(TH1EFT*& h, const float& x, vector<string>* v_reweights_ids, vector<float>* v_reweights_floats, const float& originalXWGTUP, const vector<float>& v_SWE, const float& weight_SF=0, bool show_overflow=false)
{
    bool debug = false;
//--------------------------------------------
    float sm_wgt = 0.; //Weight of SM point
    float sm_swe = 0.; //SWE of SM point
    WCFit eft_fit("myfit");

    //May only loop on minimal required number of points for WCFit (depends on n.of WCs) -- will get fit warning otherwise

    // for(int iwgt=0; iwgt<25; iwgt++) //just the necessary nof weights to overconstrain fit with 5 WCs
    for(int iwgt=0; iwgt<v_reweights_ids->size(); iwgt++)
    {
        // cout<<"v_reweights_ids->at(iwgt) "<<v_reweights_ids->at(iwgt)<<" / w = "<<v_reweights_floats->at(iwgt)<<" / v_SWE[iwgt] "<<v_SWE[iwgt]<<endl;

        TString ts = (TString) v_reweights_ids->at(iwgt);
        if(ts == "rwgt_1") {continue;} //obsolete

        if(!ts.Contains("_sm", TString::kIgnoreCase))
        {
            float w = v_reweights_floats->at(iwgt);
            // float w = v_reweights_floats->at(iwgt) / v_SWE[iwgt];
            // float w = v_reweights_floats->at(iwgt)/(originalXWGTUP * v_SWE[iwgt]);
            // float w = v_reweights_floats->at(iwgt) / originalXWGTUP;

            WCPoint wc_pt(v_reweights_ids->at(iwgt), w);

            eft_fit.points.push_back(wc_pt);
        }
        else
        {
            sm_wgt = v_reweights_floats->at(iwgt);
            sm_swe = v_SWE[iwgt];
        }
    } //weights loop

    //-- Include 'manually' the SM point as first element (not included automatically because named 'SM' and not via its operator values)
    eft_fit.points.insert(eft_fit.points.begin(), eft_fit.points[0]); //Duplicate the first element
    eft_fit.points[0].setSMPoint(); //Set (new) first element to SM coeffs (all null)
    eft_fit.points[0].wgt = sm_wgt; //Set (new) first element to SM weight

    eft_fit.fitPoints();

    if(debug) //Printout WC values, compare true weight to corresponding fit result
    {
        cout<<"//-------------------------------------------- "<<endl;

        eft_fit.dump(); //Printout all names and coefficients of WC pairs

        for (uint i=0; i < eft_fit.points.size(); i++)
        {
            WCPoint wc_pt = eft_fit.points.at(i);
            double fit_val = eft_fit.evalPoint(&wc_pt);
            wc_pt.dump(); //Printout names and values of all WCs for this point
            std::cout << "===> " << std::setw(3) << i << ": " << std::setw(12) << wc_pt.wgt << " | " << std::setw(12) << fit_val << " | " << std::setw(12) << (wc_pt.wgt-fit_val) << std::endl; //Printout : i / true weight / evaluated weight / diff
        }

        // cout<<"originalXWGTUP "<<originalXWGTUP<<endl;
        cout<<endl<<endl<<endl;
    }

    //Fill with SM weight by default
    if(show_overflow) {Fill_TH1EFT_UnderOverflow(h, x, v_reweights_floats->at(1), eft_fit);}
    else {h->Fill(x, v_reweights_floats->at(1), eft_fit);}

    return;
}

/**
 * Parameterization of the xsec as a function of operator1 (x) and operator2 (y). See definition of each coeff. Normalize to SM coeff=weight
 */
Double_t EFT_Fit_Parameterization_1D(Double_t* val, Double_t* par)
{
    Float_t x = val[0];

    return (par[0] + x*par[1] + x*x*par[2]) / par[0];
}

Double_t EFT_Fit_Parameterization_2D(Double_t* val, Double_t* par)
{
    Float_t x = val[0];
    Float_t y = val[1];

    return (par[0] + x*par[1] + x*x*par[2] + y*par[3] + x*y*par[4] + y*y*par[5]) / par[0];
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
