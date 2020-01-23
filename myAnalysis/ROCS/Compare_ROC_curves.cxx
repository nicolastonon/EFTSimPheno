//by Nicolas Tonon (DESY)

//This code makes it possible to compare ROC curves
//The histograms are either 1) produced from a TMVA output TTree, or 2) taken directly from a root file containing histograms

//--------------------------------------------
// == REMARKS ==
// -- In TMVA files, TestTree contains only half of the events, whereas my user-histos use the full datasets... ?
// -- In case we read user-histos, need to modify variable name !
// -- /!\ ROC for TMVA training sample is wron for now ! Because the "weight" variable in TrainTree is incorrect (don't know what it means...)
//==> Should use directly the training ROC histo ! (make TGraph from histo ? check)
/**
* == USAGE ==
* # Should be pretty straightforward to obtain ROCs from TMVA TTree files :
* -- Just fill the vectors correctly in main, make sure variable name is ok (hard-code if needed), ...
*
*  # To get ROC from user-file containing histograms :
*  -- File should be produced with sufficient amount of bins for precision (make it consistent in main)
*  -- Set correctly the name of object=histogram to retrieve : for my purposes, I automatically get the name from my ownn naming conventions (tHq analysis)
*  -- If want to use just any histogram, should bypass this and hard-code the names of each histo to retrieve !
*/

//--------------------------------------------
#include "../Helper.h"

#include <iostream>
#include <cstdlib>
#include <map>
#include <string>
#include <iomanip>
#include <cmath>
#include <sstream>

#include "TFile.h"
#include "TTree.h"
#include "TLegend.h"
#include "TString.h"
#include "TLatex.h"
#include "TCanvas.h"
#include "TGraphErrors.h"
#include "TF1.h"
#include "TH1F.h"
#include "TH2F.h"
#include "TLine.h"
#include "TStyle.h"

#include <cassert>     //Can be used to terminate program if argument is not true.
//Ex : assert(test > 0 && "Error message");
#include <sys/stat.h> // to be able to use mkdir

#define DEBUG false

using namespace std;



//--------------------------------------------
// ##     ## ######## ##       ########  ######## ########     ######## ##     ## ##    ##  ######   ######
// ##     ## ##       ##       ##     ## ##       ##     ##    ##       ##     ## ###   ## ##    ## ##    ##
// ##     ## ##       ##       ##     ## ##       ##     ##    ##       ##     ## ####  ## ##       ##
// ######### ######   ##       ########  ######   ########     ######   ##     ## ## ## ## ##        ######
// ##     ## ##       ##       ##        ##       ##   ##      ##       ##     ## ##  #### ##             ##
// ##     ## ##       ##       ##        ##       ##    ##     ##       ##     ## ##   ### ##    ## ##    ##
// ##     ## ######## ######## ##        ######## ##     ##    ##        #######  ##    ##  ######   ######
//--------------------------------------------

/**
 * Sum all histograms from a vector into a single histogram, passed as pointer in argument
 */
void Sum_Histograms(TH1F* &h, vector<TH1F*> v_histos)
{
	cout<<endl<<"--- Sum_Histograms()"<<endl<<endl;

	if(v_histos.size() < 2) {cout<<FRED("No histogram to sum !")<<endl; return;}

	h = 0; //histogram to be filled

	for(int ihisto=0; ihisto<v_histos.size(); ihisto++)
	{
		if(!ihisto) {h = (TH1F*) v_histos[ihisto]->Clone();}
		else {h->Add(v_histos[ihisto]);}
	}

	return;
}

/**
 * Returns a different color for each index
 */
int Get_Color(int index)
{
	vector<int> v_colors;

	//-- Can uncomment these 4 lines to get 2*2 same colors (4 ROCs total)
	// v_colors.push_back(kRed);
	// v_colors.push_back(kBlue);
	// v_colors.push_back(kRed);
	// v_colors.push_back(kBlue);

	//-- Can use this palette to get 4*2 same colors (8+ ROCs)
	v_colors.push_back(kRed);
	v_colors.push_back(kBlue);
	v_colors.push_back(kGreen+1);
	v_colors.push_back(kBlack);
	v_colors.push_back(kRed);
	v_colors.push_back(kBlue);
	v_colors.push_back(kGreen+1);
	v_colors.push_back(kBlack);
	v_colors.push_back(kViolet-1);
	v_colors.push_back(kOrange-1);



	if(index < v_colors.size() ) {return v_colors[index];}
	else {return index;}
}

void Get_Signal_and_Backgrounds(TString filepath, TString signal, vector<TString>& v_sig, vector<TString>& v_bkg)
{
	v_sig.push_back(signal); //Only 1 signal accounted for (modify if want THQ+THW, etc.)

    v_bkg.push_back("bkg"); //not needed anymore ?

	return;
}

void Apply_Cosmetics(TCanvas* &c1)
{
// -- using https://twiki.cern.ch/twiki/pub/CMS/Internal/FigGuidelines

	float l = c1->GetLeftMargin();
	float t = c1->GetTopMargin();

	TString cmsText = "CMS";
	TLatex latex;
	latex.SetNDC();
	latex.SetTextAngle(0);
	latex.SetTextColor(kBlack);

	latex.SetTextFont(61);
	latex.SetTextAlign(11);
	latex.SetTextSize(0.06);
	// latex.DrawLatex(l + 0.01, 0.92, cmsText);

	TString extraText = "Preliminary";
	latex.SetTextFont(52);
	latex.SetTextSize(0.05);
	// if(draw_preliminary_label)
	{
		// latex.DrawLatex(l + 0.12, 0.92, extraText);
	}

	c1->SetFrameLineWidth(2.);
}

/**
 * Try to infer varname directly from filename, and return full variable name to plot
 * If not found, return original string (<-> not modified)
 */
// TString Get_VarName_From_FileName(TString filename, TString variable, TString signal = "")
// {
// 	TString tmp = variable;
// 	return tmp; //If substring not found, don't modify variable name
// }


double GetFloatPrecision(double value, double precision)
{
    return (floor((value * pow(10, precision) + 0.5)) / pow(10, precision));
}










//--------------------------------------------
//  ######   ######## ########       ###    ##     ##  ######
// ##    ##  ##          ##         ## ##   ##     ## ##    ##
// ##        ##          ##        ##   ##  ##     ## ##
// ##   #### ######      ##       ##     ## ##     ## ##
// ##    ##  ##          ##       ######### ##     ## ##
// ##    ##  ##          ##       ##     ## ##     ## ##    ##
//  ######   ########    ##       ##     ##  #######   ######
//--------------------------------------------

/**
 * Compute AUC (ROC integral) from histogram produced by TMVA (not available for Custom files)
 */
double Get_AUC_From_TMVAfile(TString filename, TString variable, bool use_TrainSample, TString signal)
{
	if(DEBUG) {cout<<FYEL("-- Get_AUC_From_TMVAfile() --")<<endl;}

	TH1F* h = 0; //Pointer to histogram passed as argument, filled inside function

	if(!Check_File_Existence(filename) ) {cout<<FRED("File "<<filename<<" not found ! Abort")<<endl; return 0;}
	TFile* f = TFile::Open(filename);

	//Command to interactively produce an histo from variable, with desired range
	// variable = Get_VarName_From_FileName(filename, variable, signal); //auto infer var name

	TString dir_name = "weights/Method_BDT/" + variable; //Path of subdir, hard-coded
	TString h_name = "MVA_" + variable + "_"; //Name of ROC histo, hard-coded

	if(use_TrainSample) {h_name+= "trainingRejBvsS";} //Training ROC
	else {h_name+= "rejBvsS";} //Testing ROC

	h = (TH1F*) f->Get(dir_name + "/" + h_name);
	if(!h) {cout<<FRED("Histo "<<dir_name<<"/"<<h_name<<" not found ! Abort")<<endl; return 0;}

	// cout<<"h->Integral() = "<<h->Integral()<<endl;
	double AUC = roundf(h->Integral() ); //Change precision
	AUC/= 100.; //Needs renorm.

	if(DEBUG) {cout<<"AUC = "<<AUC<<endl;}

	return AUC;
}








//--------------------------------------------
//  ######   ######## ########    ##     ## ####  ######  ########  #######
// ##    ##  ##          ##       ##     ##  ##  ##    ##    ##    ##     ##
// ##        ##          ##       ##     ##  ##  ##          ##    ##     ##
// ##   #### ######      ##       #########  ##   ######     ##    ##     ##
// ##    ##  ##          ##       ##     ##  ##        ##    ##    ##     ##
// ##    ##  ##          ##       ##     ##  ##  ##    ##    ##    ##     ##
//  ######   ########    ##       ##     ## ####  ######     ##     #######
//--------------------------------------------


/**
 * Get histogram from a TMVA output TTree, for given range, nbins, cuts, etc.
 * @param h        histogram to be filled
 * @param t        TTree to read
 * @param variable variable to plot
 * @param nbins
 * @param xmin
 * @param xmax
 * @param cuts     cuts to apply -- "1" <-> no cut
 */
bool Create_Histogram_From_TMVA_Tree(TH1F*& h, TString filename, TString treename, TString variable, TString signal, bool is_bkg, int nbins, double xmin, double xmax, bool use_TrainSample, TString cuts="1")
{
	if(DEBUG) {cout<<FYEL("-- Create_Histogram_From_TMVA_Tree() --")<<endl;}

	h = 0; //Pointer to histogram passed as argument, filled inside function

	if(!Check_File_Existence(filename) ) {cout<<FRED("File "<<filename<<" not found ! Abort")<<endl; return 0;}
	TFile* f = TFile::Open(filename);

	//Hard-code tree name
	treename = "weights/TestTree";
	if(use_TrainSample) {treename = "weights/TrainTree";}

	TTree* t = (TTree*) f->Get(treename);
	if(!t) {cout<<FRED("Null TTree '"<<treename<<"' ! Are you sure you are indeed reading a TMVA-produced file (else change option) ? Abort")<<endl; return 0;}

	//Command to interactively produce an histo from variable, with desired range
	// variable = Get_VarName_From_FileName(filename, variable, signal); //auto infer var name

	variable = variable + " >> h(" + Convert_Number_To_TString(nbins) + "," + Convert_Number_To_TString(xmin) + "," + Convert_Number_To_TString(xmax) + ")";
	// cout<<"variable = "<<variable<<endl;

	//Option to interactively apply weights to histogram, and apply set of cuts
	//NB : in TMVA, sig <-> "classID==0" & bkg <-> "classID==1"
	// NB : in TrainTree, weight variable is wrong (don't' know what it correspond to...!) //But weight ok for testTree
	cuts = "weight * (" + cuts + " && classID==" + Convert_Number_To_TString(is_bkg) + ")";
	// cout<<"cuts = "<<cuts<<endl;

	//Produce histogram interactively
	t->Draw(variable, cuts);

	//Retrieve generated histogram
	h = (TH1F*) gDirectory->Get("h")->Clone();
	h->SetDirectory(0); //NECESSARY so that histo is not associated with TFile, and doesn't get deleted when file closed !
	if(!h || h->GetEntries() == 0) {cout<<BOLD(FRED("Null or void histogram (made from TMVA TTree) ! Abort !"))<<endl; return 0;}

	//Retrieve underflow content
	double underflow = h->GetBinContent(0);
	if(underflow>0)
	{
		h->AddBinContent(1, underflow); //Add underflow
		h->SetBinContent(0, 0); //Remove underflow
	}
	//Retrieve overflow content
	double overflow = h->GetBinContent(nbins+1);
	if(overflow>0)
	{
		h->AddBinContent(nbins, overflow); //Add overflow
		h->SetBinContent(nbins+1, 0); //Remove overflow
	}

	f->Close();

	return true;
}

/**
 * Can directly retrieve ROC histogram from TMVA files
 * NB : not used for now, since implemented use of TGraph... keep it as before (Else should slightly modify main func, to plot 1 single hist directly instead of computing eff. from TGraphs...)
 */
bool Retrieve_Histogram_From_TMVA_File(TH1F*& h, TString filename, TString variable, bool is_bkg, bool use_TrainSample)
{
	if(DEBUG) {cout<<FYEL("-- Retrieve_Histogram_From_TMVA_File() --")<<endl;}

	h = 0; //Pointer to histogram passed as argument, filled inside function

	if(!Check_File_Existence(filename) ) {cout<<FRED("File "<<filename<<" not found ! Abort")<<endl; return 0;}
	TFile* f = TFile::Open(filename);

	// variable = Get_VarName_From_FileName(filename, variable); //auto infer var name

    TString dir_name = "weights/Method_BDT/" + variable; //Path of subdir, hard-coded
	TString h_name = "MVA_" + variable + "_"; //Name of ROC histo, hard-coded

	if(use_TrainSample) {h_name+= "trainingRejBvsS";} //Training ROC
	else {h_name+= "rejBvsS";} //Testing ROC

	h = (TH1F*) f->Get(dir_name + "/" + h_name);
	if(!h) {cout<<FRED("Histo "<<dir_name<<"/"<<h_name<<" not found ! Abort")<<endl; return 0;}

	cout<<"Integral = "<<h->Integral()<<endl;

	return true;
}




/**
 * Get histogram directly from root file (must have been produced before)
 * @param h          histogram to be filled
 * @param filename   path of TFile containing histogram
 * @param histo_name name of histogram to retrieve in file
 * NB : problem : THIS USES THE FULL SAMPLE, NOT JUST TEST SET ! Use other function to get test sample from Keras
 */
bool Get_Histogram_From_CustomFile(TH1F*& h, TString filename, TString histo_name)
{
	if(DEBUG) {cout<<FYEL("-- Get_Histogram_From_CustomFile() --")<<endl;}

	h = 0; //Pointer to histogram passed as argument, filled inside function

	if(!Check_File_Existence(filename) ) {cout<<FRED("File "<<filename<<" not found ! Abort")<<endl; return 0;}
	TFile* f = TFile::Open(filename);

	if(DEBUG) {cout<<"- Opening histo : "<<histo_name<<endl;}

	h = (TH1F*) f->Get(histo_name);
	h->SetDirectory(0); //NECESSARY so that histo is not associated with TFile, and doesn't get deleted when file closed !
	if(!h || h->GetEntries() == 0) {cout<<BOLD(FRED("Null or void histogram '"<<histo_name<<"' (from user-file) ! Abort !"))<<endl; return 0;}

	//Retrieve underflow content
	double underflow = h->GetBinContent(0);
	if(underflow>0)
	{
		h->AddBinContent(1, underflow); //Add underflow
		h->SetBinContent(0, 0); //Remove underflow
	}
	//Retrieve overflow content
	int nbins = h->GetNbinsX();
	double overflow = h->GetBinContent(nbins+1);
	if(overflow>0)
	{
		h->AddBinContent(nbins, overflow); //Add overflow
		h->SetBinContent(nbins+1, 0); //Remove overflow
	}

	f->Close();

	return 1;
}




bool Get_Histogram_From_KerasFile(TH1F*& h, TString filename, TString histo_name)
{
	if(DEBUG) {cout<<FYEL("-- Get_Histogram_From_KerasFile() --")<<endl;}

	h = 0; //Pointer to histogram passed as argument, filled inside function

	if(!Check_File_Existence(filename) ) {cout<<FRED("File "<<filename<<" not found ! Abort")<<endl; return 0;}
	TFile* f = TFile::Open(filename);

	if(DEBUG) {cout<<"- Opening histo : "<<histo_name<<endl;}

	h = (TH1F*) f->Get(histo_name);
	h->SetDirectory(0); //NECESSARY so that histo is not associated with TFile, and doesn't get deleted when file closed !
	if(!h || h->GetEntries() == 0) {cout<<BOLD(FRED("Null or void histogram '"<<histo_name<<"' (from user-file) ! Abort !"))<<endl; return 0;}

	//Retrieve underflow content
	double underflow = h->GetBinContent(0);
	if(underflow>0)
	{
		h->AddBinContent(1, underflow); //Add underflow
		h->SetBinContent(0, 0); //Remove underflow
	}
	//Retrieve overflow content
	int nbins = h->GetNbinsX();
	double overflow = h->GetBinContent(nbins+1);
	if(overflow>0)
	{
		h->AddBinContent(nbins, overflow); //Add overflow
		h->SetBinContent(nbins+1, 0); //Remove overflow
	}

	f->Close();

	return 1;
}










//--------------------------------------------
//  ######   ######## ########    ########  ######   ########     ###    ########  ##     ##
// ##    ##  ##          ##          ##    ##    ##  ##     ##   ## ##   ##     ## ##     ##
// ##        ##          ##          ##    ##        ##     ##  ##   ##  ##     ## ##     ##
// ##   #### ######      ##          ##    ##   #### ########  ##     ## ########  #########
// ##    ##  ##          ##          ##    ##    ##  ##   ##   ######### ##        ##     ##
// ##    ##  ##          ##          ##    ##    ##  ##    ##  ##     ## ##        ##     ##
//  ######   ########    ##          ##     ######   ##     ## ##     ## ##        ##     ##


// ######## ######## ######## ####  ######  #### ######## ##    ##  ######  ##    ##
// ##       ##       ##        ##  ##    ##  ##  ##       ###   ## ##    ##  ##  ##
// ##       ##       ##        ##  ##        ##  ##       ####  ## ##         ####
// ######   ######   ######    ##  ##        ##  ######   ## ## ## ##          ##
// ##       ##       ##        ##  ##        ##  ##       ##  #### ##          ##
// ##       ##       ##        ##  ##    ##  ##  ##       ##   ### ##    ##    ##
// ######## ##       ##       ####  ######  #### ######## ##    ##  ######     ##
//

//  ######  ####  ######            ##    ########  ##    ##  ######
// ##    ##  ##  ##    ##          ##     ##     ## ##   ##  ##    ##
// ##        ##  ##               ##      ##     ## ##  ##   ##
//  ######   ##  ##   ####       ##       ########  #####    ##   ####
//       ##  ##  ##    ##       ##        ##     ## ##  ##   ##    ##
// ##    ##  ##  ##    ##      ##         ##     ## ##   ##  ##    ##
//  ######  ####  ######      ##          ########  ##    ##  ######
//--------------------------------------------

/**
 * For each bin from signal/bkg histograms, will compute efficiency and store it in a TGraph (passed as arg)
 * @param g     TGraph to be filled
 * @param h_sig signal histo
 * @param h_bkg bkg histo
 */
bool Produce_Efficiency_TGraph(TGraph* &g, TH1F* h_sig, TH1F* h_bkg)
{
	if(DEBUG) {cout<<FYEL("-- Produce_Efficiency_TGraph()")<<endl;}

	if(!h_sig || h_sig->GetEntries() == 0) {cout<<BOLD(FRED("Null or void signal TGraph ! Abort !"))<<endl; return 0;}
	if(!h_bkg || h_bkg->GetEntries() == 0) {cout<<BOLD(FRED("Null or void bkg TGraph ! Abort !"))<<endl; return 0;}

	int nbins = h_sig->GetNbinsX();

	g = new TGraph(nbins); //TGraph to be filled

	if(h_sig->GetNbinsX() != h_bkg->GetNbinsX()) {cout<<BOLD(FRED("Different nbins for sig & bkg histograms ! Abort"))<<endl;}
	if(nbins < 30) {cout<<"Warning : only "<<nbins<<" bins in histograms (Low precision) !"<<endl;}

	double integral_sig = h_sig->Integral();
	double integral_bkg = h_bkg->Integral();

	// cout<<"Integral sig = "<<integral_sig<<endl;
	// cout<<"Integral bkg = "<<integral_bkg<<endl;

	double AUC = 0;

	for (int ibin=1; ibin<nbins+1; ibin++)
	{
		//Compute efficiency for sig and bkg, from current bin to xmax
		double integral_sig_tmp = h_sig->Integral(ibin, nbins);
		double integral_bkg_tmp = h_bkg->Integral(ibin, nbins);

		double eff_sig_tmp = integral_sig_tmp / integral_sig;
		double eff_bkg_tmp = integral_bkg_tmp / integral_bkg;

		// eff_sig_tmp = eff_bkg_tmp; //set for xcheck, so that auc = 0.5

		//Fill efficiency graph
        // g->SetPoint(ibin-1, 1-eff_sig_tmp, eff_bkg_tmp);
        g->SetPoint(ibin-1, eff_sig_tmp, 1-eff_bkg_tmp);

		// if(ibin > 70 && ibin < 75)
		// {
			// cout<<"//--------------------------------------------"<<endl;
			// cout<<"Point "<<ibin<<" / x = "<<eff_sig_tmp<<" / y = "<<1-eff_bkg_tmp<<endl;
			// cout<<"integral_sig_tmp "<<integral_sig_tmp<<endl;
			// cout<<"integral_bkg_tmp "<<integral_bkg_tmp<<endl;
			// cout<<"eff_sig_tmp "<<eff_sig_tmp<<endl;
			// cout<<"eff_bkg_tmp "<<eff_bkg_tmp<<endl;
		// }
	}

	return 1;
}











//--------------------------------------------
// ########  ##        #######  ########    ########   #######   ######   ######
// ##     ## ##       ##     ##    ##       ##     ## ##     ## ##    ## ##    ##
// ##     ## ##       ##     ##    ##       ##     ## ##     ## ##       ##
// ########  ##       ##     ##    ##       ########  ##     ## ##        ######
// ##        ##       ##     ##    ##       ##   ##   ##     ## ##             ##
// ##        ##       ##     ##    ##       ##    ##  ##     ## ##    ## ##    ##
// ##        ########  #######     ##       ##     ##  #######   ######   ######
//--------------------------------------------

/**
 * For each ROC, will : 1) get sig & bkg histos, 2) produce efficiency graph, 3) superimpose all graphs
 */
void Superimpose_ROC_Curves(vector<TString> v_filepath, vector<TString> v_objName, vector<TString> v_label, vector<TString> v_isTMVA_file, vector<bool> v_isTrainSample, TString variable, TString region, int nbins, double xmin, double xmax, TString signal, TString cuts="1")
{
	cout<<endl<<YELBKG("                          ")<<endl<<endl;
	cout<<FYEL("--- Will superimpose all ROC curves on plot ---")<<endl;
	cout<<endl<<YELBKG("                          ")<<endl<<endl;

	if(!v_filepath.size() || !v_label.size() || !v_isTMVA_file.size() )
	{
		cout<<"Passed void vector as argument ! Abort !"<<endl; return;
	}

	TCanvas* c = new TCanvas("", "", 1000, 800);
	c->SetTopMargin(0.1);
	// c->SetGrid();

	//Draw custom background (coordinates hard-coded...)
	TPad *p = new TPad("p","p",0.16,0.13,0.97,0.9);
	p->SetFillColorAlpha(kGray, 0.15);

	TLine* l_randGuess = new TLine(0, 1, 1, 0); //Display "random guess" ROC
	l_randGuess->SetLineStyle(2);

	vector<TLine*> v_gridlines_Y(9);
	vector<TLine*> v_gridlines_X(9);
	double ticklength = 0.03;

	for(int i=0; i<9; i++)
	{
		v_gridlines_Y[i] = new TLine((i+1)*0.1, 0+ticklength, (i+1)*0.1, 1-ticklength);

		v_gridlines_X[i] = new TLine(0+ticklength, (i+1)*0.1, 1-ticklength, (i+1)*0.1);

		v_gridlines_Y[i]->SetLineColor(0);
		v_gridlines_Y[i]->SetLineWidth(4);
		// v_gridlines_Y[i]->SetLineStyle(2);

		v_gridlines_X[i]->SetLineColor(0);
		v_gridlines_X[i]->SetLineWidth(4);
		// v_gridlines_X[i]->SetLineStyle(2);
	}

	//Need this to set the axis on plot
	TH1F* h_axis = new TH1F("", "", 10, 0, 1);
	h_axis->SetMinimum(0.0001); //Remove 0 from axis
	h_axis->SetMaximum(1);

	h_axis->GetXaxis()->SetTitle("Signal efficiency");
	h_axis->GetXaxis()->SetTitleOffset(1);
	h_axis->GetXaxis()->SetLabelFont(42);
	h_axis->GetXaxis()->SetTitleFont(42);
	h_axis->GetXaxis()->SetTickLength(0.04);

	h_axis->GetYaxis()->SetTitle("Background rejection");
	h_axis->GetYaxis()->SetTitleOffset(1);
	h_axis->GetYaxis()->SetLabelFont(42);
	h_axis->GetYaxis()->SetTitleFont(42);
	h_axis->GetYaxis()->SetTickLength(0.04);

    float y_min_leg = 0.4 - v_filepath.size() * 0.08;
	TLegend* legend = new TLegend(0.19, y_min_leg, 0.60, 0.4);
    legend->SetTextSize(0.03);
    legend->SetLineColor(kGray);

	//1 TGraph per ROC
	vector<TGraph*> v_graph(v_filepath.size() );

	//1 value of AUC (area under curve) per ROC -- NB : not available yet for PureKeras files (only TMVA)
	vector<double> v_AUC(v_filepath.size() );

	//For each file, will get/produce sig & bkg histograms, compute efficiency, and plot it
	for(int iroc=0; iroc<v_filepath.size(); iroc++)
	{
		cout<<endl<<endl<<endl<<UNDL("* File "<<v_filepath[iroc]<<" :")<<endl;

		//For each file, get list of signal and background processes involved
		vector<TString> v_sig_names, v_bkg_names;
		Get_Signal_and_Backgrounds(v_filepath[iroc], signal, v_sig_names, v_bkg_names);


		//1 histo per process
		vector<TH1F*> v_h_sig(v_sig_names.size());
		vector<TH1F*> v_h_bkg(v_bkg_names.size());

		//In TMVA TTree, only 1 signal & 1 background class
		if(v_isTMVA_file[iroc] == "TMVA" || v_isTMVA_file[iroc] == "Keras") {v_h_sig.resize(1); v_h_bkg.resize(1);}

		if(v_isTMVA_file[iroc] == "TMVA") {v_AUC[iroc] = Get_AUC_From_TMVAfile(v_filepath[iroc], variable, v_isTrainSample[iroc], signal);}
		else {v_AUC[iroc] = 0;} //No 'integral' for TGraphs...
		// cout<<"v_AUC[iroc] "<<v_AUC[iroc]<<endl;

		//-- For each sig & bkg process, get histogram
		for(int isig=0; isig<v_h_sig.size(); isig++)
		{
			cout<<"- Signal n°"<<isig+1<<" : "<<v_sig_names[isig]<<endl;

			// variable = Get_VarName_From_FileName(v_filepath[iroc], variable);

			if(v_isTMVA_file[iroc] == "TMVA") //Produce histo from TMVA TTree
			{
                // if(!Retrieve_Histogram_From_TMVA_File(v_h_sig[isig], v_filepath[iroc], variable, false, v_isTrainSample[iroc]) )
                if(!Create_Histogram_From_TMVA_Tree(v_h_sig[isig], v_filepath[iroc], v_objName[iroc], variable, signal, false, nbins, xmin, xmax, v_isTrainSample[iroc], cuts) )
				{
					return;
				}
			}
			else if(v_isTMVA_file[iroc] == "Custom") //Retrieve directly histogram from user-file
			{
				TString hname_tmp = "DNN"+variable+"__"+v_sig_names[isig];

				if(!Get_Histogram_From_CustomFile(v_h_sig[isig], v_filepath[iroc], "DNN"+variable+"__"+v_sig_names[isig]) ) //Check naming convention used in pure-Keras NN
				{
					return;
				}
			}
			else if(v_isTMVA_file[iroc] == "Keras") //Retrieve directly histogram from file produced in python script
			{
				// TString hname_tmp = "DNN"+variable+"__"+v_sig_names[isig];
				TString hname_tmp = "hist_test_sig";

				if(!Get_Histogram_From_KerasFile(v_h_sig[isig], v_filepath[iroc], hname_tmp) ) //Check naming convention used in pure-Keras NN
				{
					return;
				}
			}
			else {cout<<"Problem !"<<endl; return;}
		}

		for(int ibkg=0; ibkg<v_h_bkg.size(); ibkg++)
		{
			cout<<"- Background n°"<<ibkg+1<<" : "<<v_bkg_names[ibkg]<<endl;
			if(v_h_bkg.size()==1 && v_bkg_names.size() == 2) //Fix, because need to resize vector for TMVA files
			{
				cout<<"- Background n°"<<ibkg+1+1<<" : "<<v_bkg_names[ibkg+1]<<endl;
			}

			if(v_isTMVA_file[iroc] == "TMVA") //Produce histo from TMVA TTree
			{
                // if(!Retrieve_Histogram_From_TMVA_File(v_h_bkg[ibkg], v_filepath[iroc], variable, false, v_isTrainSample[iroc]) )
				if(!Create_Histogram_From_TMVA_Tree(v_h_bkg[ibkg], v_filepath[iroc], v_objName[iroc], variable, signal, true, nbins, xmin, xmax, v_isTrainSample[iroc], cuts) )
				{
					return;
				}
			}
			else if(v_isTMVA_file[iroc] == "Custom") //Retrieve directly histogram from user-file
			{
				TString hname_tmp = "DNN"+variable+"__"+v_bkg_names[ibkg];

				if(!Get_Histogram_From_CustomFile(v_h_bkg[ibkg], v_filepath[iroc], "DNN"+variable+"__"+v_bkg_names[ibkg]) ) //Check naming convention used in pure-Keras NN
				{
					return;
				}
			}
			else if(v_isTMVA_file[iroc] == "Keras")
			{
				// TString hname_tmp = "DNN"+variable+"__"+v_bkg_names[ibkg];
				TString hname_tmp = "hist_test_bkg";

				if(!Get_Histogram_From_KerasFile(v_h_bkg[ibkg], v_filepath[iroc], hname_tmp) ) //Check naming convention used in pure-Keras NN
				{
					return;
				}
			}
			else {cout<<"Problem !"<<endl; return;}
		}

		TH1F* h_sig = 0;
		if(v_h_sig.size() > 1) {Sum_Histograms(h_sig, v_h_sig);}
		else if(v_h_sig.size() == 1) {h_sig = (TH1F*) v_h_sig[0]->Clone();}
		else {cout<<BOLD(FRED("Wrong vector size !"))<<endl; return;}
		if(!h_sig) {cout<<"h_sig is null !"<<endl; return;}

		TH1F* h_bkg = 0;
		if(v_h_bkg.size() > 1) {Sum_Histograms(h_bkg, v_h_bkg);}
		else if(v_h_bkg.size() == 1) {h_bkg = (TH1F*) v_h_bkg[0]->Clone();}
		else {cout<<BOLD(FRED("Wrong vector size !"))<<endl; return;}

		//Produce efficiency TGraph from sig & bkg histograms
		if(!Produce_Efficiency_TGraph(v_graph[iroc], h_sig, h_bkg) ) {return;}

		for(int isig=0; isig<v_h_sig.size(); isig++)
		{
			delete v_h_sig[isig]; v_h_sig[isig] = 0;
		}
		for(int ibkg=0; ibkg<v_h_bkg.size(); ibkg++)
		{
			delete v_h_bkg[ibkg]; v_h_bkg[ibkg] = 0;
		}
		delete h_sig; h_sig = 0;
		delete h_bkg; h_bkg = 0;
	}

	c->cd();

	h_axis->Draw(); //Draw axis first

	//Draw custom grid
	p->Draw("same");
	for(int i=0; i<v_gridlines_Y.size(); i++)
	{
		v_gridlines_Y[i]->Draw("same");
		v_gridlines_X[i]->Draw("same");
	}

	l_randGuess->Draw("same"); //random guess

	for(int iroc=0; iroc<v_filepath.size(); iroc++)
	{
		// v_graph[iroc]->SetLineColor(Get_Color(iroc));
		v_graph[iroc]->SetLineColorAlpha(Get_Color(iroc), 0.75);
		v_graph[iroc]->SetLineWidth(4);
		// v_graph[iroc]->SetMarkerColor(Get_Color(iroc));
		// v_graph[iroc]->SetLineStyle(2);

		//-- dashed lines for some ROCs ?
		if(iroc > 3)
		// if(iroc > 1)
		{
			v_graph[iroc]->SetLineColorAlpha(Get_Color(iroc), 0.75);
			v_graph[iroc]->SetLineStyle(8);
		}

		// cout<<"v_AUC[iroc] "<<v_AUC[iroc]<<endl;

		// if(v_AUC[iroc] != 0) {legend->AddEntry(v_graph[iroc], v_label[iroc], "L");} //FIXME
        if(v_AUC[iroc] != 0) {legend->AddEntry(v_graph[iroc], v_label[iroc] + " (AUC="+Convert_Number_To_TString(v_AUC[iroc]) + ")", "L");}
		else {legend->AddEntry(v_graph[iroc], v_label[iroc], "L");}

		v_graph[iroc]->Draw("same C"); //'C' <-> smooth curve
	}

	legend->Draw("same");
	Apply_Cosmetics(c);

	c->SaveAs("ROC_comparison.png");

	for(int igraph=0; igraph<v_graph.size(); igraph++)
	{
		delete v_graph[igraph]; v_graph[igraph] = 0;
	}
	delete h_axis; h_axis = 0;
	delete p; p = NULL;
	delete legend; legend = 0;
	delete c; c = 0;

	for(int i=0; i<v_gridlines_X.size(); i++)
	{
		delete v_gridlines_X[i]; v_gridlines_X[i] = NULL;
		delete v_gridlines_Y[i]; v_gridlines_Y[i] = NULL;
	}
	delete l_randGuess; l_randGuess = NULL;

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
	Load_Canvas_Style();

//-- Select options, etc.
//--------------------------------------------
	TString signal = "tZq";
	TString region = "SR";
    TString lumiYear = "Run2";
    // TString channel = "";

	TString cuts = "1"; //1 <-> No cut

	int nbins = 100; //Choose nof bins to use to produce ROC (if use user-histos, make sure binning is same)
	double xmin = -1; double xmax = 1; //Normally, BDT and NN range from -1 to +1
//--------------------------------------------


//-- Fill vectors
//--------------------------------------------
//Push_back 1 element to each vector, for each ROC curve
//NB : within tHq framework, objNames are expected to be the following :
//-- for regular BDT : "weights/TestTree"

	vector<TString> v_filepath; //Path of TFile containing TMVA TTree or histograms
	vector<TString> v_objName; //Complete path of TMVA TTree or histogram
	vector<TString> v_label; //Label to be displayed on plot
    vector<TString> v_isTMVA_file; //'TMVA' <-> looking for TMVA TTree ; else (?) <-> looking for histograms
    vector<bool> v_isTrainSample; //True <-> looking for ROC from train sample ; else test sample
//--------------------------------------------

//--------------------------------------------
    v_filepath.push_back("../outputs/BDT_"+signal+"_"+lumiYear+".root");
    v_objName.push_back("");
    v_label.push_back("tZq "+lumiYear);
    v_isTMVA_file.push_back("TMVA"); v_isTrainSample.push_back(false);

//--------------------------------------------




//--------------------------------------------
//-- Hard-coded from here
//--------------------------------------------

	//Name of variable to plot from TMVA TTree. Automatically infered (can be bypassed via this hardcoding)
	TString variable = signal; //Must correspond to 'method_title' variable from TopEFT_analysis code (?)

	Superimpose_ROC_Curves(v_filepath, v_objName, v_label, v_isTMVA_file, v_isTrainSample, variable, region, nbins, xmin, xmax, signal, cuts);
//--------------------------------------------

	return 0;
}
