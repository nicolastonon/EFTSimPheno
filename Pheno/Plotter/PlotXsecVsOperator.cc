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

//Hardcoded values if want to plot real nof events... (corresponding to cuts applied in GenAnalyzer)
#define xsec_tllq 30.5
#define xsec_ttll 281
#define lumi 41.5

using namespace std;

//--------------------------------------------
// ##     ##  ######  ########  ######     ##     ##  ######     ######## ######## ########
//  ##   ##  ##    ## ##       ##    ##    ##     ## ##    ##    ##       ##          ##
//   ## ##   ##       ##       ##          ##     ## ##          ##       ##          ##
//    ###     ######  ######   ##          ##     ##  ######     ######   ######      ##
//   ## ##         ## ##       ##           ##   ##        ##    ##       ##          ##
//  ##   ##  ##    ## ##       ##    ##      ## ##   ##    ##    ##       ##          ##
// ##     ##  ######  ########  ######        ###     ######     ######## ##          ##
//--------------------------------------------

 //                                       #
 //  ####  ###### #####    ##### #    #  ##   ###### ###### #####
 // #    # #        #        #   #    # # #   #      #        #
 // #      #####    #        #   ######   #   #####  #####    #
 // #  ### #        #        #   #    #   #   #      #        #
 // #    # #        #        #   #    #   #   #      #        #
 //  ####  ######   #        #   #    # ##### ###### #        #

void Get_TH1EFT_ForXsecPlot(TH1EFT*& h, TString process, vector<double>*& v_sumsReweights_afterSel, vector<string>*& v_reweights_ids)
{
    bool debug = false;

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

    // vector<double> v_sumsReweights_afterSel(100); //v_SWE stores sums of weights (for each EFT reweight point) for the total of all entries in the sample. This vector stores the SWE but only for the entries which have been processed (can be used for debugging, without running over all entries)

    //Read branches
    vector<float>* v_reweights_floats = new vector<float>();
    // vector<string>* v_reweights_ids = new vector<string>();
    t->SetBranchAddress("v_weightIds", &v_reweights_ids);
    t->SetBranchAddress("v_weights", &v_reweights_floats);

    float originalXWGTUP;
    t->SetBranchAddress("originalXWGTUP", &originalXWGTUP);

    //We just need to fill the TH1EFT to later parameterize the TF2 fit ; no need to use full statistics, few Ks events are enough for approx.
    int nentries = 10000;
    // int nentries = 20000;
    // int nentries = t->GetEntries();
    cout<<FMAG("Processing "<<nentries<<" entries...")<<endl;

    //Draw progress bar
    bool draw_progress_bar = true;
    TMVA::Timer timer(nentries, "", true);
    TMVA::gConfig().SetDrawProgressBar(1);
    TMVA::gConfig().SetUseColor(1);

    for(int ientry=0; ientry<nentries; ientry++)
    {
        if(draw_progress_bar && ientry%1000==0) {timer.DrawProgressBar(ientry, ""); cout<<ientry<<" / "<<nentries<<endl;}

        t->GetEntry(ientry);

        // FillTH1EFT(h, 0.5, v_reweights_ids, v_reweights_floats, originalXWGTUP, v_SWE, 0, false);
        float sm_wgt;
        WCFit eftfit = Get_EFT_Fit(v_reweights_ids, v_reweights_floats, sm_wgt);
        FillTH1EFT_SingleVar(h, 0.5, eftfit, sm_wgt, true);

        // if(debug)
        {
            for(int iwgt=0; iwgt<v_reweights_floats->size(); iwgt++)
            {
                // v_sumsReweights_afterSel[iwgt]+= v_reweights_floats->at(iwgt);
                v_sumsReweights_afterSel->at(iwgt)+= v_reweights_floats->at(iwgt);
            }
        }
    }

    if(debug) //Printout WC values, compare true weight to corresponding fit result
    {
        WCFit fit = h->GetSumFit(); //Get summed fit (over all bins)

        cout<<"//-------------------------------------------- "<<endl;
        fit.dump(); //Printout all names and coefficients of WC pairs
        for (uint i=0; i < fit.points.size(); i++)
        {
            WCPoint wc_pt = fit.points.at(i);
            double fit_val = fit.evalPoint(&wc_pt);
            wc_pt.dump(); //Printout names and values of all WCs for this point
            // std::cout << "===> " << std::setw(3) << i << ": " << std::setw(12) << v_sumsReweights_afterSel[i] << " | " << std::setw(12) << fit_val << " | " << std::setw(12) << (v_sumsReweights_afterSel[i]-fit_val) << std::endl; //Printout : i / true weight / evaluated weight / diff
            std::cout << "===> " << std::setw(3) << i << ": " << std::setw(12) << v_sumsReweights_afterSel->at(i) << " | " << std::setw(12) << fit_val << " | " << std::setw(12) << (v_sumsReweights_afterSel->at(i)-fit_val) << std::endl; //Printout : i / true weight / evaluated weight / diff
        }
        cout<<endl<<endl<<endl;
    }

    delete v_reweights_floats;
    // delete v_reweights_ids;
    f->Close();

    return;
}

 // ###### #    # ##### #####    ##   #####   ####  #
 // #       #  #    #   #    #  #  #  #    # #    # #
 // #####    ##     #   #    # #    # #    # #    # #
 // #        ##     #   #####  ###### #####  #    # #
 // #       #  #    #   #   #  #    # #      #    # #
 // ###### #    #   #   #    # #    # #       ####  ######

// #####  #       ####  #####
// #    # #      #    #   #
// #    # #      #    #   #
// #####  #      #    #   #
// #      #      #    #   #
// #      ######  ####    #

/**
 * Plot the process cross section as a function of the WCs of 2 operators (2D and 3D plots)
 * First, loop on all entries and fill a TH1EFT object with all the events fits
 * Then, rescale the TH1EFT at all points of a 2D scan (2 operators), store the sums of weights (= xsecs if properly normalized), and plot values
 */
void Plot_CrossSection_VS_WilsonCoeff(TString process, TString operator1, TString operator2, vector<double>*& v_sumsReweights_afterSel, vector<string>* v_reweights_ids, TH1EFT* h, TString type="1D")
{
    bool debug = false;
    bool relative_to_SM = true; //true <-> compare all points to SM value

    string kSMstr = "SM"; //Must be exact same name as 'kSMstr' in WCFit

    cout<<endl<<BOLD(UNDL(FYEL("=== Plot XSEC .vs. WCs ===")))<<endl<<endl;
    if(operator1 == "") {cout<<FRED("Error ! Wrong operator name ! Abort ! ")<<endl; return;}
    if(type != "1D" && type != "2D") {cout<<FRED("Error ! Wrong type name ! Abort ! ")<<endl; return;}
    cout<<ITAL("Creating "<<type<<" plot...")<<endl;

    //1D vectors of WC values to cover for each operator
    vector<float> v_grid_op1;
    vector<float> v_grid_op2;
    int min_op1 = -25., max_op1=25; //min max operator1
    int min_op2 = -25., max_op2=25; //min max operator2

    //To parameterize the xsec as a function of 2 operators, 6 independent coefficients are required
    //Extract these parameters, and use them to parameterize smoothly the xsec as a function of the 2 operators -> plot
    WCFit fit = h->GetSumFit(); //Get summed fit (over all bins)
    Double_t c0, c1, c2, c3, c4, c5; //Get all necessary fit coeffs (2D -> 5 params)
    c0 = fit.getCoefficient(kSMstr, kSMstr);
    c1 = fit.getCoefficient(kSMstr, (string) operator1);
    c2 = fit.getCoefficient((string) operator1, (string) operator1);

    if(debug)
    {
        cout<<kSMstr<<"*"<<kSMstr<<" --> c0 "<<c0<<endl;
        cout<<kSMstr<<"*"<<operator1<<" --> c1 "<<c1<<endl;
        cout<<operator1<<"*"<<operator1<<" --> c2 "<<c2<<endl;
    }

    if(type == "1D")
    {
        //Due to fit error, there may be a very small slope instead of flat, making the plot visually wrong... correct for that
        if(abs(c1) < pow(10, -6)) {c1=0.;}
        if(abs(c2) < pow(10, -6)) {c2=0.;}
    }
    else if(type == "2D")
    {
        c3 = fit.getCoefficient(kSMstr, (string) operator2);
        c4 = fit.getCoefficient((string) operator1, (string) operator2);
        c5 = fit.getCoefficient((string) operator2, (string) operator2);
    }

    //1D plot -> Create a TF1 object using the proper xsec parameterization
    TF1 *tf1 = 0;
    if(type == "1D")
    {
        tf1 = new TF1("f1", EFT_Fit_Parameterization_1D, min_op1, max_op1, 3);
        tf1->SetParameters(c0,c1,c2);
    }

    //2D plot -> Create a TF2 object using the proper xsec parameterization
    TF2 *tf2 = 0;
    if(type == "2D")
    {
        tf2 = new TF2("f2", EFT_Fit_Parameterization_2D, min_op1, max_op1, min_op2, max_op2, 6);
        tf2->SetParameters(c0,c1,c2,c3,c4,c5);
    }

    TCanvas* c = new TCanvas("c","c", 1000, 800);
    if(type == "2D")
    {
        c->SetTopMargin(0.13);
        c->SetRightMargin(0.11);
    }
    c->SetGridx(1);
    c->SetGridy(1);

    //Draw marker on SM point //See : https://root.cern.ch/doc/master/classTAttMarker.html
    TGraph *SM_marker = new TGraph();
    SM_marker->SetPoint(0, 0, 1.); //SM point -> 1
    SM_marker->SetMarkerStyle(29);
    SM_marker->SetMarkerSize(3.);
    SM_marker->SetMarkerColor(kBlack);

    //Color palettes : https://root.cern.ch/doc/master/classTColor.html
    // gStyle->SetPalette(kDeepSea);

    TString text = "pp #rightarrow " + GetProcessLegendName(process);
    TLatex latex;
    latex.SetNDC();
    latex.SetTextAlign(11);
    latex.SetTextFont(52);
    latex.SetTextSize(0.04);

    mkdir("plots", 0777);
    mkdir("plots/xsec", 0777);

 //   #
 //  ##   #####
 // # #   #    #
 //   #   #    #
 //   #   #    #
 //   #   #    #
 // ##### #####

    TGraph* graph = new TGraph;

    if(type == "1D")
    {
        TString plot_title = "#sigma_{EFT} / #sigma_{SM}";
        tf1->GetHistogram()->SetTitle("");
        tf1->GetHistogram()->GetXaxis()->SetTitle(Get_Operator_Name(operator1));
        tf1->GetHistogram()->GetYaxis()->SetTitle(Get_Operator_Name(plot_title));
        // if(tf1->GetMinimum() > 0.95) {tf1->SetMinimum(1.);}
        tf1->SetLineWidth(3.);
        tf1->SetLineColor(kAzure-2);

        //1D plot
        tf1->Draw("L");
        SM_marker->Draw("P same");

        latex.DrawLatex(0.18, 0.95, text);

        Draw_MG_Reference_Points(operator1, graph, v_sumsReweights_afterSel, v_reweights_ids);
        graph->Draw("P same");
        graph->SetMarkerStyle(8);
        graph->SetMarkerSize(1.5);
        graph->SetMarkerColor(kViolet);

        TLegend* leg = new TLegend(0.46, 0.75, 0.68, 0.9);
        leg->SetTextSize(0.03);
        leg->AddEntry(tf1, "Extrapolation", "L");
        leg->AddEntry(graph, "MG reweighting", "P");
        leg->AddEntry(SM_marker, "SM", "P");
        leg->Draw("same");

        TString outname = "plots/xsec/" + process + "_xsec_vs_"+operator1+"_1D.png";
        c->SaveAs(outname);

        delete leg;
    }

 //  #####
 // #     # #####
 //       # #    #
 //  #####  #    #
 // #       #    #
 // #       #    #
 // ####### #####
    else if(type == "2D")
    {
        TString plot_title = "#sigma_{EFT} / #sigma_{SM}";
        // TString plot_title = "#sigma_{EFT} / #sigma_{SM} vs "+Get_Operator_Name(operator1)+" , "+Get_Operator_Name(operator2);
        // TString plot_title = "#frac{#sigma_{EFT}}{#sigma_{SM}} vs { "+Get_Operator_Name(operator1)+" , "+Get_Operator_Name(operator2)+" }";
        // h2->SetTitle(plot_title);
        // h2->GetXaxis()->SetTitle(Get_Operator_Name(operator1));
        // h2->GetYaxis()->SetTitle(Get_Operator_Name(operator2));
        // h2->GetZaxis()->SetTitle("a.u.");
        // h2->GetZaxis()->SetLabelSize(0.03);

        //Increase resolution //Default is 20 for 2d func //Use at least 100 //Slows down significantly
        tf2->SetNpx(200);
        tf2->SetNpy(200);

        tf2->GetHistogram()->SetTitle(plot_title);
        tf2->GetHistogram()->GetXaxis()->SetTitle(Get_Operator_Name(operator1));
        tf2->GetHistogram()->GetYaxis()->SetTitle(Get_Operator_Name(operator2));
        tf2->GetHistogram()->GetZaxis()->SetLabelSize(0.03);
        // tf2->GetHistogram()->GetZaxis()->SetTitle("a.u.");
        tf2->GetHistogram()->SetMaximum(tf2->GetHistogram()->GetMaximum()*0.8);

        //2D plot
        tf2->Draw("colz");
        SM_marker->Draw("P same");
        TString outname = "plots/xsec/" + process + "_xsec_vs_"+operator1+"_"+operator2+"_2D.png";

        latex.DrawLatex(0.18, 0.88, text);

        c->SaveAs(outname);

        //3D plot
        c->SetRightMargin(0.05);
        // c->SetLeftMargin(0.05);
        tf2->GetHistogram()->GetXaxis()->SetTitleOffset(1.5);
        tf2->GetHistogram()->GetYaxis()->SetTitleOffset(1.5);
        tf2->GetHistogram()->GetXaxis()->SetNdivisions(505);
        tf2->GetHistogram()->GetYaxis()->SetNdivisions(505);
        tf2->Draw("SURF2");
        outname = "plots/xsec/" + process + "_xsec_vs_"+operator1+"_"+operator2+"_3D.png";
        c->SaveAs(outname);
    }

    if(type == "1D") {delete tf1;}
    else if(type == "2D") {delete tf2;}
    delete c;
    delete SM_marker;
    delete graph;

    return;
}

 // #       ####   ####  #####      ####  #####  ###### #####    ##   #####  ####  #####   ####
 // #      #    # #    # #    #    #    # #    # #      #    #  #  #    #   #    # #    # #
 // #      #    # #    # #    #    #    # #    # #####  #    # #    #   #   #    # #    #  ####
 // #      #    # #    # #####     #    # #####  #      #####  ######   #   #    # #####       #
 // #      #    # #    # #         #    # #      #      #   #  #    #   #   #    # #   #  #    #
 // ######  ####   ####  #          ####  #      ###### #    # #    #   #    ####  #    #  ####

/**
 * Call function Plot_CrossSection_VS_WilsonCoeff() for each operators => 1D plots for all single operators
*/
void Plot_CrossSection_VS_WilsonCoeff_SingleOperator(TString process, vector<TString> v_operators, TString op="")
{
    //Get TH1EFT object
    TH1EFT* h = new TH1EFT("", "", 1, 0, 1); //TH1EFT storing the weights and fits for all events (single bin, only care about integral)
    vector<double>* v_sumsReweights_afterSel = new vector<double>(100); //v_SWE stores sums of weights (for each EFT reweight point) for the total of all entries in the sample. This vector stores the SWE but only for the entries which have been processed
    vector<string>* v_reweights_ids = new vector<string>();
    Get_TH1EFT_ForXsecPlot(h, process, v_sumsReweights_afterSel, v_reweights_ids);

    cout<<endl<<BOLD(UNDL(FYEL("=== Plot XSEC .vs. WCs for all unique operators ===")))<<endl<<endl;

    for(int i1=0; i1<v_operators.size(); i1++)
    {
        if(op != "" && v_operators[i1]!= op) {continue;} //Plot only 1 operator

        cout<<endl<<FMAG("=== Operator : "<<v_operators[i1]<<"")<<endl<<endl;

        Plot_CrossSection_VS_WilsonCoeff(process, v_operators[i1], "", v_sumsReweights_afterSel, v_reweights_ids, h, "1D");
    }

    delete h;
    delete v_sumsReweights_afterSel;
    delete v_reweights_ids;

    return;
}

/**
 * Call function Plot_CrossSection_VS_WilsonCoeff() for each pair of operators => 2D plots for all pairs of operators
*/
void Plot_CrossSection_VS_WilsonCoeff_PairOperators(TString process, vector<TString> v_operators, TString op1="", TString op2="")
{
    //Get TH1EFT object
    TH1EFT* h = new TH1EFT("", "", 1, 0, 1); //TH1EFT storing the weights and fits for all events (single bin, only care about integral)
    vector<double>* v_sumsReweights_afterSel = new vector<double>(100); //v_SWE stores sums of weights (for each EFT reweight point) for the total of all entries in the sample. This vector stores the SWE but only for the entries which have been processed
    vector<string>* v_reweights_ids = new vector<string>();
    Get_TH1EFT_ForXsecPlot(h, process, v_sumsReweights_afterSel, v_reweights_ids);

    cout<<endl<<BOLD(UNDL(FYEL("=== Plot XSEC .vs. WCs for all pairs of operators ===")))<<endl<<endl;

    for(int i1=0; i1<v_operators.size(); i1++)
    {
        if(op1 != "" && op2 != "" && v_operators[i1]!= op1) {continue;} //Plot only 1 pair of operator

        for(int i2=0; i2<v_operators.size(); i2++)
        {
            if(i1==i2) {continue;}
            if(op1 != "" && op2 != "" && v_operators[i2]!= op2) {continue;} //Plot only 1 pair of operator

            cout<<endl<<FMAG("=== Pair : "<<v_operators[i1]<<", "<<v_operators[i2]<<"")<<endl<<endl;

            Plot_CrossSection_VS_WilsonCoeff(process, v_operators[i1], v_operators[i2], v_sumsReweights_afterSel, v_reweights_ids, h, "2D");
        }
    }

    delete h;
    delete v_sumsReweights_afterSel;
    delete v_reweights_ids;

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
    bool lowercase_operators = true; //true <-> operator names are forced to lowercase (if needed to match reweight names)

    TString process;
    // process = "tllq_training";
    process = "ttll_training";

    vector<TString> v_operators;
    v_operators.push_back("ctz");
    v_operators.push_back("ctw");
    v_operators.push_back("cpqm");
    v_operators.push_back("cpq3");
    v_operators.push_back("cpt");

    if(lowercase_operators) {for(int i=0; i<v_operators.size(); i++) {v_operators[i].ToLower();} }
    Load_Canvas_Style();

    Plot_CrossSection_VS_WilsonCoeff_SingleOperator(process, v_operators, "ctz");
    // Plot_CrossSection_VS_WilsonCoeff_SingleOperator(process, v_operators); //Plot all

    // Plot_CrossSection_VS_WilsonCoeff_PairOperators(process, v_operators, "ctw", "ctw");
    // Plot_CrossSection_VS_WilsonCoeff_PairOperators(process, v_operators); //Plot all

    return 0;
}
