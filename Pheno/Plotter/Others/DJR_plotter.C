//ROOT macro to produce DJR (Differential Jet Rate) plots from GEN/SIM/RECO (miniAOD) samples
//Adapted from: https://github.com/cms-sw/genproductions/blob/master/bin/MadGraph5_aMCatNLO/macros/plotdjr.C
//Must be run from CMSSW env

#include "TFile.h"
#include "TTree.h"
#include "TH1D.h"
#include "TCanvas.h"
#include "TCut.h"
#include "TROOT.h"
#include "TStyle.h"
#include "TChain.h"
#include "TLegend.h"
#include <vector>

void setcanvas(TCanvas *c1, TPad **pad)
{
    c1->SetLeftMargin(0.0);
    c1->SetTopMargin(0.00);
    c1->SetRightMargin(0.00);
    c1->SetTopMargin(0.0);

    pad[0]  =new TPad("pad0","pad",0,0.67,0.5,1.0);
    pad[1]  =new TPad("pad1","pad",0.5,0.67,1.0,1.0);
    pad[2]  =new TPad("pad2","pad",0,0.33,0.5,0.67);
    pad[3]  =new TPad("pad3","pad",0.5,0.33,1.0,0.67);
    pad[4]  =new TPad("pad4","pad",0.,0.,0.5,0.33);
    for(int k=0;k<5;k++) {pad[k]->Draw();}

    return;
}

void setlegend(TLegend *legend, TH1D *hall, TH1D *hmult0, TH1D *hmult1, TH1D *hmult2, TH1D *hmult3, TH1D *hmult4)
{
    legend->SetTextSize(0.050);
    legend->SetBorderSize(0);
    legend->SetTextFont(62);
    legend->SetLineColor(0);
    legend->SetLineStyle(1);
    legend->SetLineWidth(1);
    legend->SetFillColor(0);
    legend->SetFillStyle(1001);

    legend->AddEntry(hall,"all partons");
    legend->AddEntry(hmult0,"0 partons");
    legend->AddEntry(hmult1,"1 parton");
    legend->AddEntry(hmult2,"2 partons");
    legend->AddEntry(hmult3,"3 partons");
    legend->AddEntry(hmult4,"4 partons");

    return;
}

void makeplot(const char *name, TTree *tree, TCut weight, const char *drawstring, const char *xlabel, int nbins, double xlow, double xhigh, int typeMC)
{
    TCut mult0 = "GenEvent.nMEPartons()==0";
    TCut mult1 = "GenEvent.nMEPartons()==1";
    TCut mult2 = "GenEvent.nMEPartons()==2";
    TCut mult3 = "GenEvent.nMEPartons()==3";
    TCut mult4 = "GenEvent.nMEPartons()==4";

    if (typeMC == 0) //this is for NLO with FXFX merging
    {
        mult0 = "LHEEvent.npNLO()==0";
        mult1 = "LHEEvent.npNLO()==1";
        mult2 = "LHEEvent.npNLO()==2";
        mult3 = "LHEEvent.npNLO()==3";
        mult4 = "LHEEvent.npNLO()==4";
    }
    else if(typeMC == 1) //this is for LO with MLM; default values
    {
        mult0 = "GenEvent.nMEPartons()==0";
        mult1 = "GenEvent.nMEPartons()==1";
        mult2 = "GenEvent.nMEPartons()==2";
        mult3 = "GenEvent.nMEPartons()==3";
        mult4 = "GenEvent.nMEPartons()==4";
    }
    else if(typeMC == 2) //this is for LO with MLM (plotting partons after excluding non-matched partons in wbb/vbf type processes)
    {
        mult0 = "GenEvent.nMEPartonsFiltered()==0";
        mult1 = "GenEvent.nMEPartonsFiltered()==1";
        mult2 = "GenEvent.nMEPartonsFiltered()==2";
        mult3 = "GenEvent.nMEPartonsFiltered()==3";
        mult4 = "GenEvent.nMEPartonsFiltered()==4";
    }

    TH1D *hall = new TH1D(TString::Format("hall_%s",name),"",nbins,xlow,xhigh);
    TH1D *hmult0 = new TH1D(TString::Format("hmult0_%s",name),"",nbins,xlow,xhigh);
    TH1D *hmult1 = new TH1D(TString::Format("hmult1_%s",name),"",nbins,xlow,xhigh);
    TH1D *hmult2 = new TH1D(TString::Format("hmult2_%s",name),"",nbins,xlow,xhigh);
    TH1D *hmult3 = new TH1D(TString::Format("hmult3_%s",name),"",nbins,xlow,xhigh);
    TH1D *hmult4 = new TH1D(TString::Format("hmult4_%s",name),"",nbins,xlow,xhigh);

    //Colors
    hall->SetLineColor(kBlack);
    hmult0->SetLineColor(kRed);
    hmult1->SetLineColor(kBlue);
    hmult2->SetLineColor(kGreen);
    hmult3->SetLineColor(kViolet);
    hmult4->SetLineColor(kOrange);

    //Style
    hall->SetLineWidth(3);
    hmult0->SetLineStyle(2); hmult0->SetLineWidth(2);
    hmult1->SetLineStyle(2); hmult1->SetLineWidth(2);
    hmult2->SetLineStyle(2); hmult2->SetLineWidth(2);
    hmult3->SetLineStyle(2); hmult3->SetLineWidth(2);
    hmult4->SetLineStyle(2); hmult4->SetLineWidth(2);

    tree->Draw(TString::Format("%s>>%s",drawstring,hall->GetName()),weight,"goff");
    tree->Draw(TString::Format("%s>>%s",drawstring,hmult0->GetName()),weight*mult0,"goff");
    tree->Draw(TString::Format("%s>>%s",drawstring,hmult1->GetName()),weight*mult1,"goff");
    tree->Draw(TString::Format("%s>>%s",drawstring,hmult2->GetName()),weight*mult2,"goff");
    tree->Draw(TString::Format("%s>>%s",drawstring,hmult3->GetName()),weight*mult3,"goff");
    tree->Draw(TString::Format("%s>>%s",drawstring,hmult4->GetName()),weight*mult4,"goff");

    hall->GetXaxis()->SetTitle(xlabel);
    // cout<<hall->GetXaxis()->GetTitleSize()<<endl;
    hall->GetXaxis()->SetTitleSize(0.040);
    hall->GetXaxis()->SetTitleOffset(1.2);

    TLegend *legend=new TLegend(0.72,0.87-4*0.06,0.90,0.87);
    setlegend(legend, hall, hmult0, hmult1, hmult2, hmult3, hmult4);

    hall->Draw("EHIST");
    hmult0->Draw("EHISTSAME");
    hmult1->Draw("EHISTSAME");
    hmult2->Draw("EHISTSAME");
    hmult3->Draw("EHISTSAME");
    hmult4->Draw("EHISTSAME");

    gStyle->SetOptStat(0);
    legend->Draw();

    return;
}

//Example path for file stored at DESY T2 : /pnfs/desy.de/cms/tier2/store/user/ntonon/ttlldim6_top19001_FASTSIM1_v2/ttlldim6_top19001_FASTSIM1_v2/200428_093324/0000/FASTSIM1_inAODSIM_9.root
void plotdjr(const TString& infile, const TString& outfile="DJR.png")
{
    TString cmsswbase = getenv("CMSSW_BASE");
    if(cmsswbase.Length() == 0) {std::cout<<"Error ! Must run from CMSSW area"<<std::endl; return 1;}

    //Load FW libraries
    cout << "Loading FW Lite setup." << endl;
    gSystem->Load("libFWCoreFWLite.so");
    FWLiteEnabler::enable();
    gSystem->Load("libDataFormatsFWLite.so");
    gSystem->Load("libDataFormatsPatCandidates.so");

    TH1::SetDefaultSumw2();

    TChain *tree = new TChain("Events");
    tree->Add(infile);

    //tree->SetAlias("GenEvent","GenEventInfoProduct_generator__GEN.obj"); //For GEN-level files
    //tree->SetAlias("GenEvent","GenEventInfoProduct_generator__SIM.obj"); //For SIM-level files
    tree->SetAlias("GenEvent","GenEventInfoProduct_generator__RECO.obj"); //For RECO-level files, e.g. miniAOD

    //tree->SetAlias("LHEEvent","LHEEventProduct_externalLHEProducer__LHE.obj"); //For GEN-level files
    //tree->SetAlias("LHEEvent","LHEEventProduct_externalLHEProducer__SIM.obj"); //For SIM-level files
    tree->SetAlias("LHEEvent","LHEEventProduct_externalLHEProducer__RECO.obj"); //For RECO-level files, e.g. miniAOD

    TCut weight = "GenEvent.weight()";
    int nbins = 50.;
    double djrmin = -0.5;
    double djrmax = 3.;

    //typeMC sets the kind of sample we are looking at:
    //0 is for NLO with FXFX merging;
    //1 is for LO with MLM;
    //2 is for LO with MLM (plotting partons after excluding non-matched partons in wbb/vbf type processes)
    int typeMC = 1;

    cout << "typeMC = " << typeMC ;
    if(typeMC==0) {cout<<" (NLO with FXFX merging)"<<endl;}
    else if(typeMC==1) {cout<<" (LO with MLM)"<<endl;}
    else if(typeMC==2) {cout<<" (LO with MLM, after excluding non-matched partons in some processes)"<<endl;}

    // TCanvas *c1 = new TCanvas("c1", "c1", 1600, 1200);
    TCanvas *c1 = new TCanvas("c1", "c1", 3200, 2400); //Huge canvas, good resolution
    // gStyle->SetImageScaling(3.); //Make image larger than canvas, looks cleaner //ROOT6 only
    TPad *pad[5];
    setcanvas(c1,pad);

    pad[0]->cd(); pad[0]->SetBottomMargin(0.10);
    makeplot("djr0",tree,weight,"log10(GenEvent.DJRValues_[0])","log_{10}#left(#frac{DRJ0#rightarrow1}{GeV}#right)",nbins,djrmin,djrmax,typeMC); //Or : x title : DJR 0->1
    pad[1]->cd(); pad[1]->SetBottomMargin(0.10);
    makeplot("djr1",tree,weight,"log10(GenEvent.DJRValues_[1])","log_{10}#left(#frac{DRJ1#rightarrow2}{GeV}#right)",nbins,djrmin,djrmax,typeMC);
    pad[2]->cd(); pad[2]->SetBottomMargin(0.10);
    makeplot("djr2",tree,weight,"log10(GenEvent.DJRValues_[2])","log_{10}#left(#frac{DRJ2#rightarrow3}{GeV}#right)",nbins,djrmin,djrmax,typeMC);
    pad[3]->cd(); pad[3]->SetBottomMargin(0.10);
    makeplot("djr3",tree,weight,"log10(GenEvent.DJRValues_[3])","log_{10}#left(#frac{DRJ3#rightarrow4}{GeV}#right)",nbins,djrmin,djrmax,typeMC);
    pad[4]->cd(); pad[4]->SetBottomMargin(0.10);
    makeplot("djr4",tree,weight,"log10(GenEvent.DJRValues_[4])","log_{10}#left(#frac{DRJ4#rightarrow5}{GeV}#right)",nbins,djrmin,djrmax,typeMC);

    c1->Print(outfile);

    return;
}
