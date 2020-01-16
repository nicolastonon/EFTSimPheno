#include <TROOT.h>
#include <TChain.h>
#include <TFile.h>
#include <TH1F.h>
#include <TStyle.h>
#include <TCanvas.h>
#include <TLorentzVector.h>
#include "TTree.h"
#include "TString.h"

#include <iostream>
#include <cstdlib>
#include <iomanip>
#include <map>
#include <string>
#include <cmath>
#include <sstream>
#include <fstream>

#include "Helper.h"

#include <cassert> 	//Can be used to terminate program if argument is not true.
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



//--------------------------------------------
//  ######  ##     ## ######## ######## ##        #######  ##      ##
// ##    ## ##     ##    ##    ##       ##       ##     ## ##  ##  ##
// ##       ##     ##    ##    ##       ##       ##     ## ##  ##  ##
// ##       ##     ##    ##    ######   ##       ##     ## ##  ##  ##
// ##       ##     ##    ##    ##       ##       ##     ## ##  ##  ##
// ##    ## ##     ##    ##    ##       ##       ##     ## ##  ##  ##
//  ######   #######     ##    ##       ########  #######   ###  ###
//--------------------------------------------


void Compute_Write_Yields(vector<TString> v_samples, vector<TString> v_label, TString region, TString signal)
{
    bool remove_totalSF = false; //SFs are applied to default weights ; can divide weight by total SF again to get nominal weight

    cout<<endl<<YELBKG("                          ")<<endl<<endl;
	cout<<FYEL("--- Will count the yields for each sample ---")<<endl;
	cout<<"(region : "<<region<<")"<<endl;
    cout<<endl<<YELBKG("                          ")<<endl<<endl;

	TString dir_ntuples = "./input_ntuples/";

    TString outname_latex = "cutflow/latex/Yields_"+region+".tex";

    mkdir("cutflow", 0777);
    mkdir("cutflow/latex", 0777);

//--------------------------------------------
/*
    ofstream file_latex(outname_latex.Data()); //NEW : directly write yields into latex table

	//NB : '\' character must be escaped as '\\' in C++!
    file_latex<<"\\begin{table}[]"<<endl;
    file_latex<<"\\centering"<<endl;
    file_latex<<"\\begin{tabular}{|c|";
	for(int isample=0; isample<v_label.size(); isample++)
	{
		// if(v_samples[isample] == "QFlip" && (nLep != "2l" || subcat == "mm" || subcat == "uu")) {continue;}
		// if(v_samples[isample].Contains("GammaConv") && (subcat == "mm" || subcat == "uu")) {continue;}

		if(v_samples[isample] != "DATA" && !v_label[isample].Contains("SM") && isample < v_label.size()-1 && v_label[isample] != v_label[isample+1])
		{
			file_latex<<"c|"; //1 column per process
		}
	}
	file_latex<<"c|"; //also add total bkg
	file_latex<<"}"<<endl;
	file_latex<<"\\hline"<<endl;
	file_latex<<" & ";
	for(int isample=0; isample<v_label.size(); isample++)
	{
		// if(v_samples[isample] == "QFlip" && (nLep != "2l" || subcat == "mm" || subcat == "uu")) {continue;}
		// if(v_samples[isample].Contains("GammaConv") && (subcat == "mm" || subcat == "uu")) {continue;}

		if(v_samples[isample] != "DATA" && !v_label[isample].Contains("SM") && isample < v_label.size()-1 && v_label[isample] != v_label[isample+1])
		{
			if(v_label[isample] == "TTZ") {file_latex<<"$\\mathbf{t\\bar{t}Z}$ & ";}
			else if(v_label[isample] == "ttW") {file_latex<<"$\\mathbf{t\\bar{t}W}$ & ";}
			else {file_latex<<"\\textbf{"<<v_label[isample]<<"} & ";}
		}
	}
	file_latex<<"Total SM"; //1 column per process
	file_latex<<"\\\\ \\hline"<<endl;
	if(subcat != "")
    {
        file_latex<<"\\textbf{";
        if(subcat == "uu") {file_latex<<"\\mumu";}
        else if(subcat == "ue") {file_latex<<"\\emu";}
        else if(subcat == "ee") {file_latex<<"ee";}
        else {file_latex<<nLep;}
        file_latex<<"} & ";
    }
	else {file_latex<<"\\textbf{"<<nLep<<"} & ";}
*/
//--------------------------------------------

//--------------------------------------------
    TString outname = "cutflow/Yields_"+region+".txt";

	ofstream file_out(outname.Data());
	file_out<<"## Yields  in "<<region<<" region ##"<<endl;
	file_out<<"____________________________________________"<<endl;
	file_out<<"____________________________________________"<<endl;

	//NB : don't declare inside sample loop, cause might want to sum samples
	double yield_tmp = 0;
	double yield_signals = 0;
	double yield_bkg = 0;
	double yield_DATA = 0;

	double statErr_tmp = 0;
	double statErr_signals = 0;

	//FIRST LOOP ON SAMPLES : check here if files are missing ; else, may interfer with summing of several processes (TTZ, Rares, ...)
	for(int isample=0; isample<v_samples.size(); isample++)
	{
		TString filepath = dir_ntuples + v_samples[isample]+".root";
		// cout<<"-- File "<<filepath<<endl;
		if(!Check_File_Existence(filepath) )
		{
			//ERASE MISSING SAMPLES FROM VECTORS
			v_samples.erase(v_samples.begin() + isample);
			v_label.erase(v_label.begin() + isample);

			cout<<FRED("File "<<filepath<<" not found ! Erased index '"<<isample<<"' from vectors")<<endl;
		}
	}

//  ####    ##   #    # #####  #      ######    #       ####   ####  #####
// #       #  #  ##  ## #    # #      #         #      #    # #    # #    #
//  ####  #    # # ## # #    # #      #####     #      #    # #    # #    #
//      # ###### #    # #####  #      #         #      #    # #    # #####
// #    # #    # #    # #      #      #         #      #    # #    # #
//  ####  #    # #    # #      ###### ######    ######  ####   ####  #

	for(int isample=0; isample<v_samples.size(); isample++)
	{
		TString filepath = dir_ntuples + v_samples[isample]+".root";
		cout<<"-- File "<<filepath<<endl;

		if(!Check_File_Existence(filepath) )
		{
			cout<<FRED("File "<<filepath<<" not found !")<<endl;
			continue;
		}

		// cout<<FBLU("Sample : "<<v_samples[isample]<<"")<<endl;

        TString treename = "result";
		TFile* f = new TFile(filepath);
		TTree* t = 0;
        t = (TTree*) f->Get(treename);
        if(!t) {cout<<FRED("Tree '"<<treename<<"' not found ! Skip !")<<endl; continue;}

        t->SetBranchStatus("*", 0); //disable all branches, speed up

		Double_t weight = 1., weight_avg = 0.;
        Float_t eventMCFactor;
        //FIXME -- check channel value

        t->SetBranchStatus("eventWeight", 1);
		t->SetBranchAddress("eventWeight", &weight);
        t->SetBranchStatus("eventMCFactor", 1);
		t->SetBranchAddress("eventMCFactor", &eventMCFactor);

        //ADDED to reproduce ttH2017 categorization
        float channel;
        t->SetBranchStatus("channel", 1);
        t->SetBranchAddress("channel", &channel);

 // ###### #    # ###### #    # #####    #       ####   ####  #####
 // #      #    # #      ##   #   #      #      #    # #    # #    #
 // #####  #    # #####  # #  #   #      #      #    # #    # #    #
 // #      #    # #      #  # #   #      #      #    # #    # #####
 // #       #  #  #      #   ##   #      #      #    # #    # #
 // ######   ##   ###### #    #   #      ######  ####   ####  #

		int nentries = t->GetEntries();
		for(int ientry=0; ientry<nentries; ientry++)
		{
			weight=1.; eventMCFactor = 1.;

			t->GetEntry(ientry);

			// if(nJets != 2) {continue;}

			if(isnan(weight*eventMCFactor) || isinf(weight*eventMCFactor))
			{
				cout<<BOLD(FRED("* Found event with weight*eventMCFactor = "<<weight<<"*"<<eventMCFactor<<" ; remove it..."))<<endl; continue;
			}

            if(!weight*eventMCFactor) {cout<<"weight*eventMCFactor = "<<weight<<"*"<<eventMCFactor<<" ! Is it expected ?"<<endl;}

            //After sanity checks, can compute final event weight
            weight*= eventMCFactor;

			weight_avg+= weight;

            //FIXME -- check chan value
            // if(subcat != "" && !Check_isEvent_passSubCategory(subcat, channel, nMediumBJets, lepCharge) ) {continue;}

            if(signal == "signal" && v_label[isample].Contains("tZq") ) //Signals, group together
			{
				yield_tmp+= weight;
				statErr_tmp+= weight*weight;
				yield_signals+= weight;
				statErr_signals+= weight*weight;

				// cout<<"yield_signals "<<yield_signals<<endl;
			}
			else if(v_samples[isample] != "DATA") //Backgrounds
			{
				if(isample > 0 && v_samples[isample] == "Fakes_MC" && v_label[isample]==v_label[isample-1]) {yield_tmp-= weight;} //substract Fakes_MC to DD Fakes
				else {yield_tmp+= weight; statErr_tmp+= weight*weight;}

				if(v_samples[isample] == "Fakes_MC") {yield_bkg-= weight;} //susbtract MC prompt contributions to fakes/flip
				else {yield_bkg+= weight;}
				// statErr_bkg+= weight*weight;
			}
			else if(v_samples[isample] == "DATA") //DATA
			{
				yield_DATA+= weight;
				// statErr_DATA+= weight;
			}

			// cout<<"yield_tmp "<<yield_tmp<<endl;
			// cout<<"weight_avg "<<setprecision(15)<<weight_avg / nentries<<endl;
		} //event loop

		// cout<<"yield_bkg = "<<yield_bkg<<endl;
		// cout<<"yield_tmp "<<yield_tmp<<endl;

		//Check if restart counter, or merge processes
		if(v_samples[isample] != "DATA" && isample < v_label.size()-1 && v_label[isample] != v_label[isample+1])
		{
			file_out<<"--------------------------------------------"<<endl;
			file_out<<left<<setw(25)<<v_label[isample]<<setprecision(4)<<yield_tmp;
			// file_out<<v_label[isample]<<"\\t"<<yield_tmp;

			file_out<<" (+/- "<<statErr_tmp<<" stat.)"<<endl;
			// cout<<left<<setw(25)<<v_label[isample]<<yield_tmp<<endl;

			yield_tmp = 0; //Reset after writing to file
			statErr_tmp = 0;
		} //write result

	} //sample loop

	float compareYield;
	file_out<<endl<<"____________________________________________"<<endl;
	file_out<<"____________________________________________"<<endl;
	file_out<<left<<setw(25)<<"Signals"<<setprecision(5)<<yield_signals;
	file_out<<endl;

	file_out<<endl<<"____________________________________________"<<endl;
	file_out<<"____________________________________________"<<endl;
	file_out<<left<<setw(25)<<"Total backgrounds"<<setprecision(5)<<yield_bkg;
	file_out<<endl;

	file_out<<endl<<"____________________________________________"<<endl;
	file_out<<"____________________________________________"<<endl;
	file_out<<left<<setw(25)<<"DATA"<<setprecision(5)<<yield_DATA<<endl;
	file_out<<"____________________________________________"<<endl;
	file_out<<"____________________________________________"<<endl;

	// file_latex<<setprecision(3)<<yield_bkg<<""; //Total bkg
	// file_latex<<" \\\\ \\hline"<<endl;
	// file_latex<<"\\end{tabular}"<<endl;
	// file_latex<<"\\caption{xxx}"<<endl;
	// file_latex<<"\\label{tab:my-table}"<<endl;
	// file_latex<<"\\end{table}"<<endl;

	cout<<endl<<FYEL("-- Wrote file : "<<outname<<"")<<endl;
	// cout<<FYEL("-- Wrote file : "<<outname_latex<<"")<<endl;

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

int main(int argc, char **argv)
{
    TString signal = "tZq";
    TString region = "SR"; //SR

	if(argc > 1)
	{
        if(!strcmp(argv[1],"SR") ) {region = argv[1];}
		else {cout<<"Wrong first arg ! Should be 'SR' !"<<endl; return 0;}
	}

    //-- Protections
	if(region != "SR")  {cout<<"Wrong region !"<<endl; return 0;}

	//Sample names and labels
	//NB : labels must be latex-compatible
	vector<TString> v_samples; vector<TString> v_label;

    // v_samples.push_back("DATA"); v_label.push_back("DATA");

    v_samples.push_back("tZq"); v_label.push_back("tZq");
    v_samples.push_back("ttZ"); v_label.push_back("ttZ");

//--------------------------------------------
//--------------------------------------------
	Compute_Write_Yields(v_samples, v_label, region, signal);

	return 0;
}
