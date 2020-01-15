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

#include "Func_other.h"

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

/**
 * Hardcoded yields, e.g. taken from someone else, for comparison of the numbers in the printout
 */
float Compare_OtherYield(TString nlep, TString sample, TString subcat, bool remove_totalSF, bool remove_prefiring)
{
	int choice_comparison = 1;

	//-- Yields from Pallabi, 10 april
	if(choice_comparison == 1)
	{
        //--------------------------------------------
		{
			if(nlep == "3l")
			{
				if(sample == "ttH" || sample == "ttH_ctcvcp") {return 23.82;}
				else if(sample == "tHq_ITC") {return 9.96;}
				else if(sample == "tHW_ITC") {return 8.82;}
				else if(sample == "tHq_SM") {return 0.5910;}
				else if(sample == "tHW_SM") {return 0.69;}
				else if(sample == "ttW") {return 29.98;}
                else if(sample == "TTZ") {return 44.53;}
                else if(sample == "ttZ") {return 30.97;}
				else if(sample == "ttWW") {return 2.84;}
				else if(sample == "WZ") {return 9.56;}
				else if(sample == "ZZ") {return 0.98;}
                else if(sample == "tZq") {return 5.74;}
                else if(sample == "Rares") {return 5.38;}
				else if(sample == "Convs") {return 5.72;}
                else if(sample == "Fakes") {return 36.32;}
                else if(sample == "SIGNAL") {return 42.59;}
                else if(sample == "BKG") {return 141.20;}
			}
			else if(nlep == "2l")
			{
				if(subcat == "uu" || subcat == "mm")
				{
					if(sample == "ttH" || sample == "ttH_ctcvcp") {return 27.85;}
					else if(sample == "tHq_ITC") {return 21.77;}
					else if(sample == "tHW_ITC") {return 8.49;}
					else if(sample == "tHq_SM") {return 1.55;}
					else if(sample == "tHW_SM") {return 0.73;}
					else if(sample == "ttW") {return 80.39;}
					else if(sample == "ttWW") {return 2.91;}
					else if(sample == "TTZ") {return 28.19;}
					else if(sample == "ttZ") {return 26.12;}
					else if(sample == "EWK") {return 20.55;}
                    else if(sample == "tZq") {return 9.84;}
                    else if(sample == "Rares") {return 16.58;}
					else if(sample == "Fakes") {return 114.86;}
                    else if(sample == "SIGNAL") {return 58.12;}
                    else if(sample == "BKG") {return 272.58;}
				}
				else if(subcat == "eu" || subcat == "em")
				{
					if(sample == "ttH" || sample == "ttH_ctcvcp") {return 43.70;}
					else if(sample == "tHq_ITC") {return 33.86;}
					else if(sample == "tHW_ITC") {return 13.22;}
					else if(sample == "tHq_SM") {return 2.533;}
					else if(sample == "tHW_SM") {return 1.18;}
					else if(sample == "ttW") {return 123.00;}
					else if(sample == "ttWW") {return 4.82;}
					else if(sample == "TTZ") {return 77.16;}
					else if(sample == "ttZ") {return 48.20;}
					else if(sample == "EWK") {return 51.36;}
                    else if(sample == "tZq") {return 18.74;}
                    else if(sample == "Rares") {return 25.17;}
					else if(sample == "Convs") {return 16.31;}
					else if(sample == "Fakes") {return 185.27;}
					else if(sample == "QFlip") {return 33.03;}
                    else if(sample == "SIGNAL") {return 90.79;}
                    else if(sample == "BKG") {return 534.17;}
				}
			}
		}
        //--------------------------------------------
	}

	return -1;
}



//--------------------------------------------
//  ######  ##     ## ######## ######## ##        #######  ##      ##
// ##    ## ##     ##    ##    ##       ##       ##     ## ##  ##  ##
// ##       ##     ##    ##    ##       ##       ##     ## ##  ##  ##
// ##       ##     ##    ##    ######   ##       ##     ## ##  ##  ##
// ##       ##     ##    ##    ##       ##       ##     ## ##  ##  ##
// ##    ## ##     ##    ##    ##       ##       ##     ## ##  ##  ##
//  ######   #######     ##    ##       ########  #######   ###  ###
//--------------------------------------------


void Compute_Write_Yields(vector<TString> v_samples, vector<TString> v_label, TString nLep, TString region, TString analysis, TString subcat = "")
{
    bool remove_totalSF = false; //SFs are applied to default weights ; can divide weight by total SF again to get nominal weight
    bool remove_prefiring = false; //divide weight by prefiring weight

	cout<<FYEL("--- Will count the yields in Final State for each sample in the list ---")<<endl;
	cout<<FYEL("nLep "<<nLep<<"")<<endl;
	cout<<FYEL("region "<<region<<"")<<endl;
	cout<<FYEL("analysis "<<analysis<<"")<<endl;
	cout<<FYEL("subcat "<<subcat<<"")<<endl;

	TString dir_ntuples = "./input_ntuples/";
    if(analysis == "tth") {dir_ntuples+= "ttH2017/";}
    else if(analysis == "thq") {dir_ntuples+= "tHq2017/";}
	else {dir_ntuples+= "FCNC/";}
    if(region != "SR" && region != "SR_ATLAS") {region = "CR_" + region;}
	dir_ntuples+= nLep + "/" + region + "/";

    {
        // dir_ntuples = "./input_ntuples/tHq2017/MEM/3l/SR/";
    }

    TString outname_latex = "cutflow/latex/Yields_";
    if(analysis == "tth") {outname_latex+= "ttH";}
    if(analysis == "tqh") {outname_latex+= "tHq";}
    else {outname_latex+= "FCNC";}
    outname_latex+= "_" + nLep + "_" +region;
    if(subcat != "") {outname_latex+= "_" + subcat;}
    outname_latex+= ".tex";

    mkdir("cutflow", 0777);
    mkdir("cutflow/latex", 0777);

//--------------------------------------------
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

//--------------------------------------------
    TString outname = "cutflow/Yields_";
    if(analysis == "tth") {outname+= "ttH";}
    if(analysis == "thq") {outname+= "tHq";}
    else {outname+= "FCNC";}
    outname+= "_" + nLep + "_" +region;
    if(subcat != "") {outname+= "_" + subcat;}
    outname+= ".txt";

	ofstream file_out(outname.Data());
	file_out<<"## Yields  in "<<nLep<<" "<<region<<" region ("<<analysis<<" analysis) ##"<<endl;
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
		if(v_samples[isample] == "QFlip" && (nLep != "2l" || subcat == "mm" || subcat == "uu")) {continue;}
		if(v_samples[isample].Contains("GammaConv") && (subcat == "mm" || subcat == "uu")) {continue;}

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
		if(v_samples[isample] == "QFlip" && (nLep != "2l" || subcat == "mm" || subcat == "uu"))
		{
			file_latex<<" - & ";
			continue;
		}
		if(v_samples[isample].Contains("GammaConv") && (subcat == "mm" || subcat == "uu"))
		{
			file_latex<<" - & ";
			continue;
		}

		TString filepath = dir_ntuples + v_samples[isample]+".root";
		cout<<"-- File "<<filepath<<endl;

		if(!Check_File_Existence(filepath) )
		{
			cout<<FRED("File "<<filepath<<" not found !")<<endl;
			continue;
		}

		// cout<<FBLU("Sample : "<<v_samples[isample]<<"")<<endl;

		TFile* f = new TFile(filepath);
		TTree* t = (TTree*) f->Get("Tree");
        if(!t) {continue;}

        t->SetBranchStatus("*", 0); //disable all branches, speed up

		Float_t weight = 1.;
		Float_t weight_FR = 1.;
		Float_t total_SF = 1.;
		// Float_t prefiringWeight = 1.;
        Float_t SMcoupling_SF = 1.;

		TString cat_name = Get_Category_Boolean_Name(nLep, region, analysis, v_samples[isample], "");

		t->SetBranchStatus("weight", 1);
		t->SetBranchAddress("weight", &weight);
		t->SetBranchStatus("total_SF", 1);
        t->SetBranchAddress("total_SF", &total_SF);
		// t->SetBranchStatus("prefiringWeight", 1);
        // t->SetBranchAddress("prefiringWeight", &prefiringWeight);
        if(v_label[isample].Contains("-SM")) {t->SetBranchStatus("SMcoupling_SF", 1); t->SetBranchAddress("SMcoupling_SF", &SMcoupling_SF);}

        t->SetBranchStatus("weightfake", 1);
        t->SetBranchStatus("weightflip", 1);
		if(v_samples[isample].Contains("Fakes") ) {t->SetBranchAddress("weightfake", &weight_FR);}
		else if(v_samples[isample] == "QFlip") {t->SetBranchAddress("weightflip", &weight_FR);}

        //ADDED to reproduce ttH2017 categorization
        float channel, nMediumBJets, lepCharge, chargeLeadingLep, nJets;
        t->SetBranchStatus("channel", 1);
        t->SetBranchAddress("channel", &channel);
        t->SetBranchStatus("nMediumBJets", 1);
        t->SetBranchAddress("nMediumBJets", &nMediumBJets);
        t->SetBranchStatus("lepCharge", 1);
        t->SetBranchAddress("lepCharge", &lepCharge);
        t->SetBranchStatus("chargeLeadingLep", 1);
        t->SetBranchAddress("chargeLeadingLep", &chargeLeadingLep); //Special case : QFlip events are +-. Look at charge of leading lepton to decide if goes into "pos" or "neg" categ.
        t->SetBranchStatus("nJets", 1);
        t->SetBranchAddress("nJets", &nJets); //Special case : QFlip events are +-. Look at charge of leading lepton to decide if goes into "pos" or "neg" categ.

		double weight_avg = 0, SMcoupling_avg = 0;

 // ###### #    # ###### #    # #####    #       ####   ####  #####
 // #      #    # #      ##   #   #      #      #    # #    # #    #
 // #####  #    # #####  # #  #   #      #      #    # #    # #    #
 // #      #    # #      #  # #   #      #      #    # #    # #####
 // #       #  #  #      #   ##   #      #      #    # #    # #
 // ######   ##   ###### #    #   #      ######  ####   ####  #

		int nentries = t->GetEntries();
		for(int ientry=0; ientry<nentries; ientry++)
		{
			weight=1;
            total_SF = 1;
            SMcoupling_SF = 1;

			t->GetEntry(ientry);

			// if(nJets != 2) {continue;}

			if(isnan(weight) || isinf(weight))
			{
				cout<<BOLD(FRED("* Found event with weight = "<<weight<<" ; remove it..."))<<endl; continue;
			}

            if(!weight) {cout<<"weight = 0 ! Normal ?"<<endl;}

			weight_avg+= weight;

			if(v_samples[isample] == "QFlip") {lepCharge = chargeLeadingLep;}

            if(subcat != "" && !Check_isEvent_passSubCategory(subcat, channel, nMediumBJets, lepCharge) ) {continue;}

			if(remove_totalSF && total_SF != 0) {weight/= total_SF;}
			// if(remove_prefiring && prefiringWeight != 0) {weight/= prefiringWeight;}

			weight*= weight_FR;

            if(analysis == "fcnc" && v_samples[isample] == "Fakes_MC" && weight_FR == 1) {continue;} //FIXME -- tmp bug with FCNC Fakes_MC : some events have weightfake==1, should be lower

            if(v_label[isample].Contains("-SM"))
			{
                // cout<<"weight : "<<weight<<" -> weight * SMcoupling_SF = "<<weight * SMcoupling_SF<<endl;

				weight*= SMcoupling_SF;
			}

            //FCNC samples scaled arbitrarily in NTA => Rescale here //NOW USE ARBITRARY BR = 1%
            //NB : xsecs corrected for the fact that samples do not have top->tau decays
            if(v_samples[isample] == "tH_ST_hut_FCNC") {weight*= 0.3311;}
            else if(v_samples[isample] == "tH_ST_hct_FCNC") {weight*= 0.0456;}
            else if(v_samples[isample].Contains("tH_TT") ) {weight*= 1.093;}

            if(region == "SR_ATLAS") {weight*= 36.1/41.5;} //lumi factor

            if(analysis == "thq" && (v_label[isample].Contains("ITC") || v_label[isample].Contains("ctcvcp") ) ) //Signals, group together
			{
				yield_tmp+= weight;
				statErr_tmp+= weight*weight;
				yield_signals+= weight;
				statErr_signals+= weight*weight;

				// cout<<"yield_signals "<<yield_signals<<endl;
			}
            else if(analysis == "tth" && v_label[isample] == "ttH") //Signals, group together
            {
                yield_tmp+= weight;
                statErr_tmp+= weight*weight;
                yield_signals+= weight;
                statErr_signals+= weight*weight;
            }
            else if(analysis == "fcnc" && v_label[isample].Contains("FCNC")) //Signals, group together
			{
				yield_tmp+= weight;
				statErr_tmp+= weight*weight;
				yield_signals+= weight;
				statErr_signals+= weight*weight;

				// cout<<"yield_signals "<<yield_signals<<endl;
			}
			else if(v_label[isample].Contains("SM") || v_samples[isample].Contains("FCNC") || (v_label[isample] == "ttH" && analysis != "fcnc")) //don't count as signal
			{
				yield_tmp+= weight;
				statErr_tmp+= weight*weight;
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
			// cout<<"SMcoupling_avg "<<setprecision(15)<<SMcoupling_avg / nentries<<endl;
		} //event loop

		// cout<<"yield_bkg = "<<yield_bkg<<endl;
		// cout<<"yield_tmp "<<yield_tmp<<endl;

		//Check if restart counter, or merge processes
		if(v_samples[isample] != "DATA" && isample < v_label.size()-1 && v_label[isample] != v_label[isample+1])
		{
			file_out<<"--------------------------------------------"<<endl;
			file_out<<left<<setw(25)<<v_label[isample]<<setprecision(4)<<yield_tmp;
			// file_out<<v_label[isample]<<"\\t"<<yield_tmp;

			if(!v_label[isample].Contains("SM"))
			{
				file_latex<<setprecision(3)<<yield_tmp<<" & ";
			}

			if(analysis == "thq")
			{
				float compareYield = Compare_OtherYield(nLep, v_label[isample], subcat, remove_totalSF, remove_prefiring);
				if(compareYield != -1)
				{
					TString sign = "";
					float ratio = ((yield_tmp - compareYield)/compareYield) * 100;
					if(ratio > 0) {sign = "+";}
					file_out<<" ("<<sign<<setprecision(2)<<ratio<<"%)";

					//Debug printout
					// cout<<"yield_tmp "<<yield_tmp<<endl;
					// cout<<"compareYield "<<compareYield<<endl;
				}
			}

			if(!v_samples[isample].Contains("Fakes") && v_samples[isample] != "QFlip") {file_out<<" (+/- "<<statErr_tmp<<" stat.)"<<endl;}
			else {file_out<<endl;}
			// cout<<left<<setw(25)<<v_label[isample]<<yield_tmp<<endl;

			yield_tmp = 0; //Reset after writing to file
			statErr_tmp = 0;
		} //write result

	} //sample loop

	float compareYield;
	file_out<<endl<<"____________________________________________"<<endl;
	file_out<<"____________________________________________"<<endl;
	file_out<<left<<setw(25)<<"Signals"<<setprecision(5)<<yield_signals;
    if(analysis == "thq")
    {
    	compareYield = Compare_OtherYield(nLep, "SIGNAL", subcat, remove_totalSF, remove_prefiring);
    	if(compareYield != -1)
    	{
    		TString sign = "";
    		float ratio = ((yield_signals - compareYield)/compareYield) * 100;
    		if(ratio > 0) {sign = "+";}
    		file_out<<" ("<<sign<<setprecision(2)<<ratio<<"%)";
    	}
    	// file_out<<" (+/- "<<statErr_signals<<" stat.)"<<endl;
    }
	file_out<<endl;

	file_out<<endl<<"____________________________________________"<<endl;
	file_out<<"____________________________________________"<<endl;
	file_out<<left<<setw(25)<<"Total backgrounds"<<setprecision(5)<<yield_bkg;
    if(analysis == "thq")
    {
        compareYield = Compare_OtherYield(nLep, "BKG", subcat, remove_totalSF, remove_prefiring);
        if(compareYield != -1)
        {
            TString sign = "";
            float ratio = ((yield_bkg - compareYield)/compareYield) * 100;
            if(ratio > 0) {sign = "+";}
            file_out<<" ("<<sign<<setprecision(2)<<ratio<<"%)";
        }
    }
	file_out<<endl;

	file_out<<endl<<"____________________________________________"<<endl;
	file_out<<"____________________________________________"<<endl;
	file_out<<left<<setw(25)<<"DATA"<<setprecision(5)<<yield_DATA<<endl;
	file_out<<"____________________________________________"<<endl;
	file_out<<"____________________________________________"<<endl;

	file_latex<<setprecision(3)<<yield_bkg<<""; //Total bkg

	file_latex<<" \\\\ \\hline"<<endl;
	file_latex<<"\\end{tabular}"<<endl;
	file_latex<<"\\caption{xxx}"<<endl;
	file_latex<<"\\label{tab:my-table}"<<endl;
	file_latex<<"\\end{table}"<<endl;

	cout<<endl<<"-- Wrote file : "<<outname<<endl;
	cout<<endl<<"-- Wrote file : "<<outname_latex<<endl;

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
    TString nLep = "2l";
    TString region = ""; //SR, CR_ttW/ttZ/WZ
    TString analysis = "";
    TString subcat = ""; //Can require events to belong to a specific subcat of HIG-18-019

	if(argc > 1)
	{
        if(!strcmp(argv[1],"thq") || !strcmp(argv[1],"tth") || !strcmp(argv[1],"fcnc")) {analysis = argv[1];}
		else {cout<<"Wrong first arg ! Should be 'thq' or 'tth' or 'fcnc' !"<<endl; return 0;}

		if(argc > 2)
		{
			if(!strcmp(argv[2],"2l") || !strcmp(argv[2],"3l")) {nLep = argv[2];}
			else {cout<<"Wrong second arg ! Should be '2l' or '3l' !"<<endl; return 0;}

			if(argc > 3)
			{
				if(!strcmp(argv[3],"SR") || !strcmp(argv[3],"SR_ATLAS") || !strcmp(argv[3],"ttW") || !strcmp(argv[3],"ttZ") || !strcmp(argv[3],"Fake") || !strcmp(argv[3],"WZ") || !strcmp(argv[3],"Z") ) {region = argv[3];}
				else {cout<<"Wrong third arg ! Should be 'SR' ((_ATLAS)) or 'ttW' or 'Fake' or 'ttZ' or 'WZ'or 'Z' !"<<endl; return 0;}

				if(argc > 4)
				{
					subcat = argv[4];
					// else {cout<<"Wrong third arg ! Should be 'SR' or 'ttW' or 'ttZ' or 'WZ' !"<<endl; return 0;}
				}
			}
		}
	}
	if(((region == "ttW" || region == "Fake") && nLep == "3l") || (nLep == "2l" && (region == "WZ" || region == "ttZ" || region == "Z")) )  {cout<<"Wrong region !"<<endl; return 0;}

	//Sample names and labels
	//NB : labels must be latex-compatible
	vector<TString> v_samples; vector<TString> v_label;

    if(analysis == "thq")
    {
        v_samples.push_back("tHq"); v_label.push_back("tHq-ITC"); //ITC coupling
        v_samples.push_back("tHW"); v_label.push_back("tHW-ITC"); //ITC coupling
    }
    else if(analysis == "fcnc")
    {
        v_samples.push_back("tH_ST_hut_FCNC"); v_label.push_back("ST-hut");
        v_samples.push_back("tH_ST_hct_FCNC"); v_label.push_back("ST-hct");
        v_samples.push_back("tH_TT_hut_FCNC"); v_label.push_back("TT-hut");
        v_samples.push_back("tH_TT_hct_FCNC"); v_label.push_back("TT-hct");
    }

	v_samples.push_back("tHq"); v_label.push_back("tHq-SM"); //SM coupling
	v_samples.push_back("tHW"); v_label.push_back("tHW-SM"); //SM coupling

    //Different ttH samples
    if(analysis == "thq") {v_samples.push_back("ttH_ctcvcp"); v_label.push_back("ttH-ctcvcp");}
	else {v_samples.push_back("ttH"); v_label.push_back("ttH");}

	v_samples.push_back("ttWW"); v_label.push_back("ttW");

	v_samples.push_back("ttW_PSweights"); v_label.push_back("ttW");

    v_samples.push_back("ttZ"); v_label.push_back("TTZ");
    v_samples.push_back("TTJets_DiLep_MLM"); v_label.push_back("TTZ"); //TTbar MC
    v_samples.push_back("TTJets_SemiLep_MLM"); v_label.push_back("TTZ"); //TTbar MC

	v_samples.push_back("WZ"); v_label.push_back("WZ");
	v_samples.push_back("ZZ"); v_label.push_back("ZZ");
	// v_samples.push_back("WJets"); v_label.push_back("EWK");
	// v_samples.push_back("DY_LO"); v_label.push_back("EWK");

	v_samples.push_back("tZq"); v_label.push_back("tZq");

	v_samples.push_back("GGHZZ4L"); v_label.push_back("Rares");
	v_samples.push_back("ZZZ"); v_label.push_back("Rares");
	v_samples.push_back("WZZ"); v_label.push_back("Rares");
	v_samples.push_back("WWZ"); v_label.push_back("Rares");
    v_samples.push_back("WWW"); v_label.push_back("Rares");
	v_samples.push_back("WW_DPS"); v_label.push_back("Rares");
	v_samples.push_back("WpWp"); v_label.push_back("Rares");
	v_samples.push_back("TTTT"); v_label.push_back("Rares");
	v_samples.push_back("tttW"); v_label.push_back("Rares");
	v_samples.push_back("ttWH"); v_label.push_back("Rares");
	v_samples.push_back("WZG"); v_label.push_back("Rares");
	v_samples.push_back("tWZ"); v_label.push_back("Rares");
    v_samples.push_back("VHToNonbb"); v_label.push_back("Rares");

    // v_samples.push_back("ST_schan"); v_label.push_back("STschan");
    // v_samples.push_back("ST_tchan"); v_label.push_back("STtchan");
    // v_samples.push_back("ST_tWchan"); v_label.push_back("STtWchan");

	v_samples.push_back("GammaConv_MC"); v_label.push_back("Convs");

	v_samples.push_back("Fakes"); v_label.push_back("Nonprompt");
	v_samples.push_back("Fakes_MC"); v_label.push_back("Nonprompt"); //Fakes MC (substract)

	v_samples.push_back("QFlip"); v_label.push_back("QFlip");

	v_samples.push_back("DATA"); v_label.push_back("DATA");

	for(int isample=0; isample<v_samples.size(); isample++)
	{
		//Hard-coded : group WZ+ZZ as 'EWK' in 2lSS
		if((nLep == "2l" || analysis == "fcnc") && (v_samples[isample] == "WZ" || v_samples[isample] == "ZZ")) {v_label[isample] = "EWK";}
	}

//--------------------------------------------
//--------------------------------------------
	Compute_Write_Yields(v_samples, v_label, nLep, region, analysis, subcat);

	return 0;
}
