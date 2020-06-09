/* BASH CUSTOM */
#define RST   "\e[0m"
#define KRED  "\e[31m"
#define KGRN  "\e[32m"
#define KYEL  "\e[33m"
#define KBLU  "\e[34m"
#define KMAG  "\e[35m"
#define KCYN  "\e[36m"
#define KWHT  "\e[37m"
#define FRED(x) KRED x RST
#define FGRN(x) KGRN x RST
#define FYEL(x) KYEL x RST
#define FBLU(x) KBLU x RST
#define FMAG(x) KMAG x RST
#define FCYN(x) KCYN x RST
#define FWHT(x) KWHT x RST
#define BOLD(x) "\e[1m" x RST
#define ITAL(x) "\e[3m" x RST
#define UNDL(x) "\e[4m" x RST
#define STRIKE(x) "\e[9m" x RST
#define DIM(x) "\e[2m" x RST
#define DOUBLEUNDERLINE(x) "\e[21m" x RST
#define CURLYUNDERLINE(x) "\e[4:3m" x RST
#define BLINK(x) "\e[5m" x RST
#define REVERSE(x) "\e[7m" x RST
#define INVISIBLE(x) "\e[8m" x RST
#define OVERLINE(x) "\e[53m" x RST
#define TURQUOISE  "\e[38;5;42m"
#define SALMON  "\e[38;2;240;143;104m"
#define FTURQUOISE(x) TURQUOISE x RST
#define FSALMON(x) SALMON x RST
#define YELBKG(x) "\e[43m" x RST
#define CYANBKG(x) "\e[46m" x RST

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
#include "TH2F.h"
#include <TStyle.h>
#include <TCanvas.h>
#include <TLorentzVector.h>
#include "TTree.h"
#include "TColor.h"
#include "TCut.h"
#include "TLegend.h"
#include "TLine.h"
#include "THStack.h"
#include "TString.h"
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

//Convert a double into a TString
// precision --> can choose if TString how many digits the TString should display
TString Convert_Number_To_TString(double number, int precision=3)
{
    if(number == 0.) {return "0";}

	stringstream ss;
	ss << std::setprecision(precision) << number;
	TString ts = ss.str();
	return ts;
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
    result = result.Strip(TString::kTrailing, ' '); //remove trailing char
    result = result.Strip(TString::kTrailing, '/'); //remove trailing char
    result+= " }";

    return result;
}

//Get proper reweight name for legend
// NB : looks like reweight names are automatically made lowercase-only !
TString GetReweightLegendName(TString variationname)
{
    TString result = variationname;

    if(variationname.Contains("sm", TString::kIgnoreCase) ) {result = "SM";} //SM
    else if(variationname.Contains("C1V") || variationname.Contains("C1A") || variationname.Contains("C2V") || variationname.Contains("C2A")) //AC
    {
        TString ts = variationname.ReplaceAll("rwgt_", "");

        std::vector<std::string> words_tmp; //Split string 'name_val_name_val...' into words --> return 'name=val,name=val,...'
        split_string((string) ts, words_tmp, "_");
        TString tmp = "";
        for(int iw=0; iw<words_tmp.size()-1; iw+=2)
        {
            tmp+= words_tmp[iw] + "=" + words_tmp[iw+1] + " /";
        }
        tmp = tmp.Strip(TString::kTrailing, '/'); //remove trailing hchar
        //Restyle AC names
        tmp = tmp.ReplaceAll("Delta", "#Delta");
        tmp = tmp.ReplaceAll("C1V", "C_{1,V}");
        tmp = tmp.ReplaceAll("C1A", "C_{1,A}");
        tmp = tmp.ReplaceAll("C2V", "C_{2,V}");
        tmp = tmp.ReplaceAll("C2A", "C_{2,A}");
        return tmp;
    }
    else if(variationname.Contains("rwgt_", TString::kIgnoreCase)) //EFT
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
    //-- Exact match only
    // if(!proc.CompareTo("ttz", TString::kIgnoreCase)) {return "t#bar{t}Z";}
    // else if(!proc.CompareTo("tzq", TString::kIgnoreCase)) {return "tZq";}
    // else if(!proc.CompareTo("ttll", TString::kIgnoreCase)) {return "t#bar{t}ll";}
    // else if(!proc.CompareTo("tllq", TString::kIgnoreCase)) {return "tllq";}

    //-- Substring matching
    if(proc.Contains("ttz", TString::kIgnoreCase)) {return "t#bar{t}Z";}
    else if(proc.Contains("tzq", TString::kIgnoreCase)) {return "tZq";}
    else if(proc.Contains("ttll", TString::kIgnoreCase)) {return "t#bar{t}ll";}
    else if(proc.Contains("tllq", TString::kIgnoreCase)) {return "tllq";}

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

inline void Fill_TH1EFT_UnderOverflow(TH1EFT* h, double value, float weight, WCFit& fit)
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

void FillTH1EFT_SingleVar(TH1EFT*& h, const float& x,  WCFit& eft_fit, float sm_wgt, bool show_overflow=false)
{
    //Fill with SM weight by default
    if(show_overflow) {Fill_TH1EFT_UnderOverflow(h, x, sm_wgt, eft_fit);}
    else {h->Fill(x, sm_wgt, eft_fit);}

    return;
}

void FillTH1EFT_ManyVars(vector<vector<TH1EFT*>>& v_h, WCFit& eft_fit, const vector<float>& v_x, int idx_proc, float sm_wgt, bool show_overflow=false)
{
    for (size_t ivar = 0; ivar < v_x.size(); ivar++)
    {
        //Fill with SM weight by default
        if(show_overflow) {Fill_TH1EFT_UnderOverflow(v_h[ivar][idx_proc], v_x[ivar], sm_wgt, eft_fit);}
        else {v_h[ivar][idx_proc]->Fill(v_x[ivar], sm_wgt, eft_fit);}
    }

    return;
}

WCFit Get_EFT_Fit(vector<string>* v_reweights_ids, vector<float>* v_reweights_floats, float& sm_wgt)
{
    bool debug = false;

//--------------------------------------------

    WCFit eft_fit("myfit");

    //May only loop on minimal required number of points for WCFit (depends on n.of WCs) -- will get fit warning otherwise
    for(int iwgt=0; iwgt<25; iwgt++) //just the necessary nof weights to overconstrain fit with 5 WCs
    // for(int iwgt=0; iwgt<v_reweights_ids->size(); iwgt++)
    {
        // cout<<"v_reweights_ids->at(iwgt) "<<v_reweights_ids->at(iwgt)<<" / w = "<<v_reweights_floats->at(iwgt)<<" / v_SWE[iwgt] "<<v_SWE[iwgt]<<endl;

        TString ts = (TString) v_reweights_ids->at(iwgt);
        if(ts == "rwgt_1") {continue;} //obsolete

        if(!ts.Contains("_sm", TString::kIgnoreCase))
        {
            float w = v_reweights_floats->at(iwgt);
            // float w = v_reweights_floats->at(iwgt)*1000;
            // float w = v_reweights_floats->at(iwgt) / v_SWE[iwgt]; //Should divide all reweights by SWE(SM)

            WCPoint wc_pt(v_reweights_ids->at(iwgt), w);

            eft_fit.points.push_back(wc_pt);
        }
        else
        {
            sm_wgt = v_reweights_floats->at(iwgt);
            // sm_wgt = v_reweights_floats->at(iwgt)*1000;
        }
    } //weights loop

    //-- Include 'manually' the SM point as first element (not included automatically because named 'SM' and not via its operator values)
    //-- Return the value of sm_wgt by reference
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
    }

    return eft_fit;
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


//XXX
void Draw_MG_Reference_Points(TString operator1, TGraph*& graph, vector<double>* v_sumsReweights_afterSel, vector<string>* v_reweights_ids)
{
    TString weightname_SM = "rwgt_SM";
    vector<TString> v_reweightNames_fromMG; vector<double> v_Xaxis;
    if(operator1 == "ctZ")
    {
        v_reweightNames_fromMG.push_back("rwgt_ctZ_-3.0_ctW_0.0_cpQM_0.0_cpQ3_0.0_cpt_0.0"); v_Xaxis.push_back(-3);
        v_reweightNames_fromMG.push_back("rwgt_ctZ_5.0_ctW_0.0_cpQM_0.0_cpQ3_0.0_cpt_0.0"); v_Xaxis.push_back(5);
    }
    else if(operator1 == "ctW")
    {
        v_reweightNames_fromMG.push_back("rwgt_ctZ_0.0_ctW_-3.0_cpQM_0.0_cpQ3_0.0_cpt_0.0"); v_Xaxis.push_back(-3);
        v_reweightNames_fromMG.push_back("rwgt_ctZ_0.0_ctW_5.0_cpQM_0.0_cpQ3_0.0_cpt_0.0"); v_Xaxis.push_back(5);
    }
    else if(operator1 == "cpQM")
    {
        v_reweightNames_fromMG.push_back("rwgt_ctZ_0.0_ctW_0.0_cpQM_-3.0_cpQ3_0.0_cpt_0.0"); v_Xaxis.push_back(-3);
        v_reweightNames_fromMG.push_back("rwgt_ctZ_0.0_ctW_0.0_cpQM_5.0_cpQ3_0.0_cpt_0.0"); v_Xaxis.push_back(5);
    }
    else if(operator1 == "cpQ3")
    {
        v_reweightNames_fromMG.push_back("rwgt_ctZ_0.0_ctW_0.0_cpQM_0.0_cpQ3_-3.0_cpt_0.0"); v_Xaxis.push_back(-3);
        v_reweightNames_fromMG.push_back("rwgt_ctZ_0.0_ctW_0.0_cpQM_0.0_cpQ3_5.0_cpt_0.0"); v_Xaxis.push_back(5);
    }
    else if(operator1 == "cpt")
    {
        v_reweightNames_fromMG.push_back("rwgt_ctZ_0.0_ctW_0.0_cpQM_0.0_cpQ3_0.0_cpt_-3.0"); v_Xaxis.push_back(-3);
        v_reweightNames_fromMG.push_back("rwgt_ctZ_0.0_ctW_0.0_cpQM_0.0_cpQ3_0.0_cpt_5.0"); v_Xaxis.push_back(5);
    }

/*
    TH1F* h_SM = NULL;

    //Define root interactive command to draw desired histogram
    TString variable = weightname_SM + " >> h_SM(1, 0, 1)";

    //Produce histogram (interactively)
    t->Draw(variable);

    //Retrieve generated histogram
    h_SM = (TH1F*) gDirectory->Get("h_SM")->Clone();

    float int_SM = h_SM->Integral();
    cout<<"int_SM "<<int_SM<<endl;

    delete h_SM;

    for(int ipoint=0; ipoint<v_reweightNames_fromMG.size(); ipoint++)
    {
        for(int id=0; id<v_reweights_ids->size(); id++)
        {
            if((TString) v_reweights_ids->at(id) != v_reweightNames_fromMG[ipoint]) {continue;}

            TH1F* h = NULL;

            //Define root interactive command to draw desired histogram
        	TString variable = v_weightIds + "["+id+"]" + " >> h(" + Convert_Number_To_TString(nbins) + "," + Convert_Number_To_TString(xmin) + "," + Convert_Number_To_TString(xmax) + ")";

        	//Produce histogram (interactively)
        	t->Draw(variable);

            //Retrieve generated histogram
            h = (TH1F*) gDirectory->Get("h")->Clone();
            h->SetDirectory(0); //NECESSARY so that histo is not associated with TFile, and doesn't get deleted when file closed !
            if(!h || h->GetEntries() == 0) {cout<<BOLD(FRED("Null or void histogram (made from TMVA TTree) ! Abort !"))<<endl; return 0;}

            double int_EFT = h->Integral();
            cout<<"int_EFT "<<int_EFT<<endl;

            // c->cd();

            delete h;
        }
    }
*/

    // cout<<"v_reweights_ids->size() "<<v_reweights_ids->size()<<endl;
    for(int ipoint=0; ipoint<v_reweightNames_fromMG.size(); ipoint++)
    {
        double int_SM = 0;
        double int_EFT = 0;
        for(int id=0; id<v_reweights_ids->size(); id++)
        {
            if((TString) v_reweights_ids->at(id) == "rwgt_sm" || (TString) v_reweights_ids->at(id) == "rwgt_SM") {int_SM = v_sumsReweights_afterSel->at(id);}
            else if((TString) v_reweights_ids->at(id) == v_reweightNames_fromMG[ipoint]) {int_EFT = v_sumsReweights_afterSel->at(id);}
        }

        // cout<<"int_SM "<<int_SM<<endl;
        // cout<<"int_EFT "<<int_EFT<<endl;
        // cout<<"int_EFT/int_SM "<<int_EFT/int_SM<<endl;
        graph->SetPoint(ipoint, v_Xaxis[ipoint], int_EFT/int_SM);
    }

    return;
}

//Convert the name of an anomalous coupling point (e.g. 'rwgt_C1A_0.5') to an EFT name (e.g. 'rwgt_ctZ_0.3')
TString Convert_ACtoEFT(TString name_AC, TString example_reweight_namingConvention)
{
    TString name_EFT = ""; //TString to return

    if(!name_AC.BeginsWith("rwgt_C2V") && !name_AC.BeginsWith("rwgt_C1V") && !name_AC.BeginsWith("rwgt_C1A") && !name_AC.BeginsWith("rwgt_D")) {cout<<"ERROR: AC naming convention not recognized !"<<endl; return name_EFT;} //Only care about C2V, C1V, C1A anomalous couplings
    if(name_AC.Contains("C2A")) {cout<<"ERROR: only consider C2V, C1A, C1V anomalous couplings !"<<endl; return name_EFT;}

    double thetaW = 0.502; //Weinberg angle (in radians)
    double vev = 246.2 * 0.001; //Higgs vacuum expectation value = 246.2 GeV //Convert to TeV (since WC values are expressed in TeV^-1)

    //SM values for the neutral vector and axial-vector couplings
    double C1V_SM = 0.2448, C1A_SM = -0.6012;

    //dX values satisfying C1,A/V = C1A/V_SM + dX <== these are the values we need to convert (C1,A/V) <-> EFT
    double DeltaC1V = 0, DeltaC1A = 0, C2V_value = 0; //Set to 0 (<-> SM) by default

    std::vector<std::string> words_reweight; //Split the reweight name passed by arg (to be used for event reweighting) into words
    split_string((string) name_AC, words_reweight, "_");
    for(int iw=1; iw<words_reweight.size()-1; iw+=2) //ignore first word 'rwgt', ignore WC values
    {
        if(((TString) words_reweight[iw]).Contains("C1A"))
        {
            if((TString) words_reweight[iw] == "C1A") {DeltaC1A = std::stod(words_reweight.at(iw+1)) - C1A_SM;} //Absolute -> take difference with SM
            if((TString) words_reweight[iw] == "DeltaC1A") {DeltaC1A = std::stod(words_reweight.at(iw+1));} //Relative -> can use this value directly
        }
        else if(((TString) words_reweight[iw]).Contains("C1V"))
        {
            if((TString) words_reweight[iw] == "C1V") {DeltaC1V = std::stod(words_reweight.at(iw+1)) - C1V_SM;} //Absolute -> take difference with SM
            if((TString) words_reweight[iw] == "DeltaC1V") {DeltaC1V = std::stod(words_reweight.at(iw+1));} //Relative -> can use this value directly
        }
        else if((TString) words_reweight[iw] == "C2V")  {C2V_value = std::stod(words_reweight.at(iw+1));}
        else {cout<<"ERROR: AC name ["<<words_reweight[iw]<<"] not recognized !"<<endl; return name_EFT;}
    }
    if(DeltaC1A == 0 && DeltaC1V == 0 && C2V_value == 0) {cout<<"ERROR: all AC values found to be null !"<<endl; return name_EFT;}

    // cout<<"C2V_value = "<<C2V_value<<endl;
    // cout<<"DeltaC1V = "<<DeltaC1V<<endl;
    // cout<<"DeltaC1A = "<<DeltaC1A<<endl;

    //Get corresponding EFT WC values
    double ctZ_WC = 0.;
    double cpQM_WC = 0.;
    double cpt_WC = 0.;
    if(C2V_value != 0) //Direct relation C2V <-> ctZ
    {
        ctZ_WC = sqrt(2) * cos(thetaW) * sin(thetaW) * C2V_value / pow(vev, 2);
    }
    else if(DeltaC1V != 0 || DeltaC1A != 0) //Linear relations between (cpqm,cpt) <-> (DeltaC1A,DeltaC1V)
    {
        //Solve system of linear equations with 2 degrees of freedom x (cpqm) and y (cpt)
        //(x+y) = a (<-> C1,V)
        //(x-y) = b (<-> C1,A)

        double a = -4 * DeltaC1V * cos(thetaW) * sin(thetaW) / pow(vev, 2);
        double b = 4 * DeltaC1A * cos(thetaW) * sin(thetaW) / pow(vev, 2);

        // --> x = (a+b)/2 ; y = (a-b) / 2
        cpQM_WC = (a+b) / 2.;
        cpt_WC = (a-b) / 2.;
    }

    //Translate WC numerical values into TString
    name_EFT = "rwgt";
    std::vector<std::string> words_example; //Split full reweight name example (containing the list of operators needed for weight parameterization) into words
    split_string((string) example_reweight_namingConvention, words_example, "_");
    for(int iw=1; iw<words_example.size()-1; iw+=2) //ignore first word 'rwgt', ignore WC values
    {
        TString opname_tmp = words_example[iw]; //Name of operator found in sample, needed for weight parameterization
        name_EFT+= "_" + opname_tmp + "_";
        if(((TString) words_example[iw]).Contains("ctZ", TString::kIgnoreCase) && ctZ_WC != 0) {name_EFT+= Convert_Number_To_TString(ctZ_WC, 3);}
        else if(((TString) words_example[iw]).Contains("cpQM", TString::kIgnoreCase) && cpQM_WC != 0) {name_EFT+= Convert_Number_To_TString(cpQM_WC, 3);}
        else if(((TString) words_example[iw]).Contains("cpt", TString::kIgnoreCase) && cpt_WC != 0) {name_EFT+= Convert_Number_To_TString(cpt_WC, 3);}
        else {name_EFT+= (TString) "0";}
    }

    // cout<<"name_AC = "<<name_AC<<endl;
    // cout<<"name_EFT = "<<name_EFT<<endl;

    return name_EFT;
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
