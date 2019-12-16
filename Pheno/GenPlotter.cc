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


//Use stat function (from library sys/stat) to check if a file exists
bool Check_File_Existence(const TString& name)
{
  struct stat buffer;
  return (stat (name.Data(), &buffer) == 0); //true if file exists
}


void Load_Canvas_Style()
{
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

TString GetVariationLegendName(TString variationname)
{
    TString result = variationname;

    if(variationname == "SM") {return result;}
    else
    {
        if(variationname.Contains("ctz", TString::kIgnoreCase)) {result = "c_{tZ}";}
        else if(variationname.Contains("ctw", TString::kIgnoreCase)) {result = "c_{tW}";}

        if(variationname.Contains("0p1", TString::kIgnoreCase)) {result+= " = 0.1";}
        if(variationname.Contains("0p5", TString::kIgnoreCase)) {result+= " = 0.5";}
        if(variationname.Contains("1p0", TString::kIgnoreCase)) {result+= " = 1";}
        if(variationname.Contains("1p5", TString::kIgnoreCase)) {result+= " = 1.5";}
        if(variationname.Contains("2p0", TString::kIgnoreCase)) {result+= " = 2";}
        if(variationname.Contains("2p5", TString::kIgnoreCase)) {result+= " = 2.5";}
        if(variationname.Contains("3p0", TString::kIgnoreCase)) {result+= " = 3";}
        if(variationname.Contains("4p0", TString::kIgnoreCase)) {result+= " = 4";}
        if(variationname.Contains("5p0", TString::kIgnoreCase)) {result+= " = 5";}
    }

    // result+= " [TeV^{-2}]";

    return result;
}









//--------------------------------------------
// ##     ##    ###    ##    ## ########    ########  ##        #######  ########  ######
// ###   ###   ## ##   ##   ##  ##          ##     ## ##       ##     ##    ##    ##    ##
// #### ####  ##   ##  ##  ##   ##          ##     ## ##       ##     ##    ##    ##
// ## ### ## ##     ## #####    ######      ########  ##       ##     ##    ##     ######
// ##     ## ######### ##  ##   ##          ##        ##       ##     ##    ##          ##
// ##     ## ##     ## ##   ##  ##          ##        ##       ##     ##    ##    ##    ##
// ##     ## ##     ## ##    ## ########    ##        ########  #######     ##     ######
//--------------------------------------------

void MakePlots(TString process, vector<TString> v_var, vector<TString> v_reweight_names, vector<pair<float, float>> v_min_max)
{
    cout<<endl<<FYEL("=== MakePlots ===")<<endl<<endl;

    TString dir = "./";
    TString filepath = dir + "test_" + process + ".root";

    TFile* f = TFile::Open(filepath);
    if(f == 0) {cout<<endl<<BOLD(FRED("--- File not found ! Exit !"))<<endl<<endl; return;}

    TString treename = "tree";
    TTree* t = (TTree*) f->Get(treename);
    if(t == 0) {cout<<endl<<BOLD(FRED("--- Tree not found ! Exit !"))<<endl<<endl; return;}

    vector<vector<TH1F*>> v_histos_var_reweight(v_var.size());
    vector<vector<TH1F*>> v_histos_var_reweight_subplot(v_var.size());

    vector<float>* v_reweights_floats = new vector<float>(v_reweight_names.size());
    vector<string>* v_reweights_ids = new vector<string>(v_reweight_names.size());

    vector<float> v_var_floats(v_var.size());

    int index_Z, index_top, index_antitop;

    t->SetBranchAddress("v_weightIds", &v_reweights_ids);
    t->SetBranchAddress("v_weights", &v_reweights_floats);

    t->SetBranchAddress("index_Z", &index_Z);
    t->SetBranchAddress("index_top", &index_top);
    t->SetBranchAddress("index_antitop", &index_antitop);

    float mc_weight_originalValue;
    t->SetBranchAddress("mc_weight_originalValue", &mc_weight_originalValue);

    for(int ivar=0; ivar<v_var.size(); ivar++)
    {
        t->SetBranchAddress(v_var[ivar], &v_var_floats[ivar]);

        v_histos_var_reweight[ivar].resize(v_reweight_names.size());
        v_histos_var_reweight_subplot[ivar].resize(v_reweight_names.size());

        for(int iweight=0; iweight<v_reweight_names.size(); iweight++)
        {
            v_histos_var_reweight[ivar][iweight] = new TH1F("", "", 15, v_min_max[ivar].first, v_min_max[ivar].second);
            v_histos_var_reweight_subplot[ivar][iweight] = new TH1F("", "", 15, v_min_max[ivar].first, v_min_max[ivar].second);
        }
    }

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

        // cout<<"ientry "<<ientry<<endl;
        // cout<<"mc_weight_originalValue "<<mc_weight_originalValue<<endl;

        for(int ivar=0; ivar<v_var.size(); ivar++)
        {
            if(v_var[ivar].Contains("Z") && index_Z == -1) {continue;} //if Z not found

            if(v_var[ivar].Contains("antitop", TString::kIgnoreCase) && index_antitop == -1) {continue;} //if antitop not found
            else if(v_var[ivar].Contains("top", TString::kIgnoreCase) && index_top == -1) {continue;} //if top not found

            for(int iweight=0; iweight<v_reweight_names.size(); iweight++)
            {
                // cout<<"v_reweights_ids[iweight] "<<v_reweights_ids->at(iweight)<<endl;

                if(v_reweight_names[iweight] == "SM") //Nominal weight
                {
                    // v_histos_var_reweight[ivar][iweight]->Fill(v_var_floats[ivar], mc_weight_originalValue);
                    v_histos_var_reweight[ivar][iweight]->Fill(v_var_floats[ivar], 1.);
                }
                else //Reweights
                {
                    for(int iweightid=0; iweightid<v_reweights_ids->size(); iweightid++)
                    {
                        if(v_reweight_names[iweight] == TString(v_reweights_ids->at(iweightid)) )
                        {
                            // cout<<"v_reweights_floats->at(iweight) "<<v_reweights_floats->at(iweight)<<endl;
                            v_histos_var_reweight[ivar][iweight]->Fill(v_var_floats[ivar], v_reweights_floats->at(iweight)/mc_weight_originalValue);

                            //FIXME
                            // if(v_reweight_names[iweight] == "ctz_3p0" && v_var_floats[ivar]>200) {cout<<"reweight = "<<v_reweights_floats->at(iweight)/mc_weight_originalValue<<endl;}
                        }
                    }
                }
            }
        }
    } //Loop on entries


 // #####  #       ####  #####
 // #    # #      #    #   #
 // #    # #      #    #   #
 // #####  #      #    #   #
 // #      #      #    #   #
 // #      ######  ####    #

    bool setlog = true;

    for(int ivar=0; ivar<v_var.size(); ivar++)
    {
        //Canvas definition
        // Load_Canvas_Style();
        TCanvas* c1 = new TCanvas("c1","c1", 1000, 800);
        // c1->SetTopMargin(0.1);
        c1->SetBottomMargin(0.45);
        if(setlog) c1->SetLogy();
        // c1->SetGridy(1);
        // c1->SetGridx(1);

        TLegend* qw = new TLegend(0.82,0.77,0.99,0.99);

        c1->cd();
        for(int iweight=0; iweight<v_reweight_names.size(); iweight++)
        {

            v_histos_var_reweight[ivar][iweight]->SetLineColor(iweight+1);

            if(!iweight) //only needed for first histo
            {
                v_histos_var_reweight[ivar][iweight]->GetYaxis()->SetTitle("Events");
                v_histos_var_reweight[ivar][iweight]->SetLineStyle(1);
                v_histos_var_reweight[ivar][iweight]->SetLineWidth(3);

                v_histos_var_reweight[ivar][iweight]->GetXaxis()->SetLabelFont(42);
                v_histos_var_reweight[ivar][iweight]->GetYaxis()->SetLabelFont(42);
                v_histos_var_reweight[ivar][iweight]->GetYaxis()->SetTitleFont(42);
                v_histos_var_reweight[ivar][iweight]->GetYaxis()->SetTitleSize(0.06);
                v_histos_var_reweight[ivar][iweight]->GetXaxis()->SetTitleSize(0.06);
                v_histos_var_reweight[ivar][iweight]->GetYaxis()->SetTickLength(0.04);
                v_histos_var_reweight[ivar][iweight]->GetYaxis()->SetLabelSize(0.048);
                v_histos_var_reweight[ivar][iweight]->GetXaxis()->SetNdivisions(505);
                v_histos_var_reweight[ivar][iweight]->GetYaxis()->SetNdivisions(506);
                v_histos_var_reweight[ivar][iweight]->GetYaxis()->SetTitleOffset(1.2);
                v_histos_var_reweight[ivar][iweight]->GetXaxis()->SetLabelSize(0.0); //subplot axis instead

                if(setlog) {v_histos_var_reweight[ivar][iweight]->SetMaximum(v_histos_var_reweight[ivar][iweight]->GetMaximum() * 5.);}
                else {v_histos_var_reweight[ivar][iweight]->SetMaximum(v_histos_var_reweight[ivar][iweight]->GetMaximum() * 1.3);}
            }

            if(v_reweight_names[iweight] != "SM")
            {
                v_histos_var_reweight[ivar][iweight]->SetLineWidth(2);
                // v_histos_var_reweight[ivar][iweight]->SetLineStyle(2);
            }

            v_histos_var_reweight[ivar][iweight]->Draw("hist same");

            // qw->AddEntry(v_histos_var_reweight[ivar][iweight], v_reweight_names[iweight], "L");
            qw->AddEntry(v_histos_var_reweight[ivar][iweight], GetVariationLegendName(v_reweight_names[iweight]), "L");
        }

        qw->Draw("same");

        //-- Subplot (for all reweights, not nominal)
        //-- create subpad to plot ratio
        TPad *pad_ratio = new TPad("pad_ratio", "pad_ratio", 0.0, 0.0, 1.0, 1.0);
        pad_ratio->SetTopMargin(0.6);
        pad_ratio->SetFillColor(0);
        pad_ratio->SetFillStyle(0);
        pad_ratio->SetGridy(1);
        pad_ratio->Draw();
        pad_ratio->cd(0);

        for(int iweight=0; iweight<v_reweight_names.size(); iweight++)
        {
            if(v_reweight_names[iweight] != "SM")
            {
                v_histos_var_reweight_subplot[ivar][iweight] = (TH1F*) v_histos_var_reweight[ivar][iweight]->Clone(); //Copy histo
                v_histos_var_reweight_subplot[ivar][iweight]->Add(v_histos_var_reweight[ivar][0], -1); //Substract nominal
                v_histos_var_reweight_subplot[ivar][iweight]->Divide(v_histos_var_reweight[ivar][0]); //Divide by nominal

                // for(int i=0; i<10; i++)
                // {
                //     cout<<"v_histos_var_reweight_subplot[ivar][iweight]->GetBinContent(i+1)"<<v_histos_var_reweight_subplot[ivar][iweight]->GetBinContent(i+1)<<endl;
                // }

                v_histos_var_reweight_subplot[ivar][iweight]->GetXaxis()->SetTitle(v_var[ivar]);
                v_histos_var_reweight_subplot[ivar][iweight]->GetYaxis()->CenterTitle();
                v_histos_var_reweight_subplot[ivar][iweight]->GetYaxis()->SetTitle("BSM/SM [%]");
                // v_histos_var_reweight_subplot[ivar][iweight]->GetYaxis()->SetTitle("#frac{(X-#mu)}{#mu} [%]");
                v_histos_var_reweight_subplot[ivar][iweight]->GetYaxis()->SetTitleOffset(1.4);
                v_histos_var_reweight_subplot[ivar][iweight]->GetYaxis()->SetTickLength(0.);
                v_histos_var_reweight_subplot[ivar][iweight]->GetXaxis()->SetTitleOffset(1.);
                v_histos_var_reweight_subplot[ivar][iweight]->GetYaxis()->SetLabelSize(0.048);
                // v_histos_var_reweight_subplot[ivar][iweight]->GetXaxis()->SetLabelFont(42);
                // v_histos_var_reweight_subplot[ivar][iweight]->GetYaxis()->SetLabelFont(42);
                // v_histos_var_reweight_subplot[ivar][iweight]->GetXaxis()->SetTitleFont(42);
                // v_histos_var_reweight_subplot[ivar][iweight]->GetYaxis()->SetTitleFont(42);
                v_histos_var_reweight_subplot[ivar][iweight]->GetYaxis()->SetNdivisions(503); //grid draw on primary tick marks only
                // v_histos_var_reweight_subplot[ivar][iweight]->GetYaxis()->SetNdivisions(507); //grid draw on primary tick marks only
                v_histos_var_reweight_subplot[ivar][iweight]->GetYaxis()->SetTitleSize(0.04);
                v_histos_var_reweight_subplot[ivar][iweight]->GetXaxis()->SetTickLength(0.04);

                v_histos_var_reweight_subplot[ivar][iweight]->Scale(100.); //express in %
                v_histos_var_reweight_subplot[ivar][iweight]->SetMinimum(-1.99); //%
                v_histos_var_reweight_subplot[ivar][iweight]->SetMaximum(+50.99); //%

                v_histos_var_reweight_subplot[ivar][iweight]->SetLineColor(iweight+1);
                v_histos_var_reweight_subplot[ivar][iweight]->SetLineWidth(2);
                // v_histos_var_reweight_subplot[ivar][iweight]->SetLineStyle(2);

                v_histos_var_reweight_subplot[ivar][iweight]->Draw("same");
            }
        }

        TString text = "pp #rightarrow " + process + " (13 TeV)";
		TLatex latex;
		latex.SetNDC();
		latex.SetTextAlign(11);
        latex.SetTextFont(52);
		latex.SetTextSize(0.03);
		latex.DrawLatex(0.61, 0.95, text);

        TString outputname = "./plot_" + v_var[ivar] + ".png";
        c1->SaveAs(outputname);

        delete pad_ratio;
        delete qw;
        delete c1;
    }

    for(int ivar=0; ivar<v_var.size(); ivar++)
    {
        for(int iweight=0; iweight<v_reweight_names.size(); iweight++)
        {
            delete v_histos_var_reweight[ivar][iweight];
            delete v_histos_var_reweight_subplot[ivar][iweight];
        }
    }

    f->Close();

    delete v_reweights_floats; delete v_reweights_ids;

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
    // TString process = "ttZ";
    TString process = "tZq";

    vector<TString> v_var; vector<pair<float, float>> v_min_max;
    v_var.push_back("Z_pt"); v_min_max.push_back(std::make_pair(20, 400));
    // v_var.push_back("Z_m"); v_min_max.push_back(std::make_pair(70, 100));
    // v_var.push_back("Top_pt"); v_min_max.push_back(std::make_pair(20, 400));
    // v_var.push_back("TopZsystem_m"); v_min_max.push_back(std::make_pair(250, 1000));

    vector<TString> v_reweight_names; vector<int> v_colors;
    v_reweight_names.push_back("SM"); //Nominal SM weight
    v_reweight_names.push_back("ctz_0p1"); v_colors.push_back(kBlue);
    v_reweight_names.push_back("ctz_0p5");
    v_reweight_names.push_back("ctz_1p0");
    v_reweight_names.push_back("ctz_1p5");
    v_reweight_names.push_back("ctz_2p0");
    // v_reweight_names.push_back("ctz_2p5");
    v_reweight_names.push_back("ctz_3p0");
    // v_reweight_names.push_back("ctz_4p0");
    // v_reweight_names.push_back("ctz_5p0");

    // v_reweight_names.push_back("ctw_0p5");
    // v_reweight_names.push_back("ctw_1p0");
    // v_reweight_names.push_back("ctw_5p0");

    Load_Canvas_Style();

    MakePlots(process, v_var, v_reweight_names, v_min_max);

    return 0;
}
