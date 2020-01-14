#ifndef Func_other_h
#define Func_other_h

/* BASH COLORS */
#define RST   "\x1B[0m"
#define KRED  "\x1B[31m"
#define KGRN  "\x1B[32m"
#define KYEL  "\x1B[33m"
#define KBLU  "\x1B[34m"
#define KMAG  "\x1B[35m"
#define KCYN  "\x1B[36m"
#define KWHT  "\x1B[37m"
#define FRED(x) KRED x RST
#define FGRN(x) KGRN x RST
#define FYEL(x) KYEL x RST
#define FBLU(x) KBLU x RST
#define FMAG(x) KMAG x RST
#define FCYN(x) KCYN x RST
#define FWHT(x) KWHT x RST
#define BOLD(x) "\x1B[1m" x RST
#define UNDL(x) "\x1B[4m" x RST
#define ITAL(x) "\x1B[3m" x RST
#define STRK(x) "\x1B[9m" x RST

#include <sstream>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <vector>
#include <sys/stat.h> // to be able to check file existence
#include <dirent.h> //list dir content


#include "TStyle.h"
#include "TCanvas.h"
#include "TH1F.h"
#include "TH2F.h"
#include "TH3F.h"
#include "TFile.h"
#include "TTree.h"
#include "TPad.h"
#include "TLegend.h"
#include "TLine.h"
#include "TGraphAsymmErrors.h"

	void Load_Canvas_Style();
	TString Convert_Number_To_TString(double, int=3);
	double Convert_TString_To_Number(TString);
	float Find_Number_In_TString(TString);
	TString Convert_Sign_To_Word(TString);
	std::pair<TString,TString> Break_Cuts_In_Two(TString);
	TString Combine_Naming_Convention(TString);
	void Extract_Ranking_Info(TString, TString, TString);
	void Get_Ranking_Vectors(TString, TString, std::vector<TString>&, std::vector<double>&);
	bool Check_File_Existence(const TString&);
	void MoveFile(TString, TString);
	void CopyFile(TString, TString);
	void Order_BDTvars_By_Decreasing_Signif_Loss(TString);
	void Order_Cuts_By_Decreasing_Signif_Loss(TString);
	void Fill_Last_Bin_TH1F(TH1F*, double); //Increment last bin of histo by specified weight
	void Fill_First_Bin_TH1F(TH1F*, double); //Increment first bin of histo by specified weight
	void Set_Custom_ColorPalette(std::vector<TColor*>&, std::vector<int>&); //Set custom color palette
	bool Is_Event_Passing_Cut(TString, double);
	bool Get_Variable_Range(TString, int&, double&, double&);
	bool Apply_CommandArgs_Choices(int, char **, TString&, bool&, TString&);
	void Get_Samples_Colors(std::vector<int>&, std::vector<TString>, int);
	void Get_SampleList_tHq2016(std::vector<TString>&, bool);
	TString Find_Corresponding_Category_2016(TString, TString, TString="tHq");
    TString Get_Category_Boolean_Name(TString, TString, TString, TString, TString);
    TString Get_RootDirNtuple_Path(bool, TString, TString);
    bool Get_Dir_Content(std::string, std::vector<TString>&);
    void Get_Input_Variables(std::vector<TString>&, TString, bool);
    double Compute_StdDevScaled_Value(double, double, double);
    double Compute_RangeScaled_Value(double, double, double, double, double);
    std::pair<double,double> Get_MinMax_Variable(TString, TString, TString, TString, std::vector<TString>);
	void Make_Pileup_Plots(std::vector<TString>, TString, TString);
	int Get_ttH2017_Binning(TH2F*, double, double);
    int Get_tHq2017_Binning(double, double, TString);
    int Get_tHq2017_Binning_compPallabi(double, double, TString);
    int Get_FCNC_Binning(double, double, TString, TString);
	bool Check_isEvent_passSubCategory(TString, float, float, float);
    float lnN1D_p1(float, float, float, float);
    float Get_lnN1D_Syst_Weight(TString, TString, TString, TString, float, float, TString);
	void Get_Mirror_Histogram(TH1F*&, TH1F*&, TH1F*&, bool);
	void Get_TemplateSymm_Histogram(TH1F*&, TH1F*&, TH1F*&, bool);
    void Inflate_Syst_inShapeTemplate(TH1F*&, TH1F*, float);
    void Set_Histogram_FlatZero(TH1F*&, bool, TString="");
    float Get_Binning_SubCategory(TString, TString);
    TString Get_CERN_CombineNamingConvention(TString, TString, TString);
    void Copy_TemplateFile_allCouplings(std::vector<TString>, TString, TString);
	void Get_SumWeights_allCouplings(TString, std::vector<float>&, float&);
	float Get_scalingFactor_toSMxsec(TString, TString);
	float Get_SF_DifferentCoupling(TString, TString, std::vector<float>*, float, float, float);
    int Test_Binnings(int, float, float, bool);
    // std::pair<double,double> Get_Mean_RMS_Variable(TString, TString, TString, TString); //needs testing
	// void Fill_BDT_specialBinning(TH1F*, double, double, double, bool, TString);

    //Baseline categ. studies
    void Plot_Overlap_Categories(TFile*, std::ofstream&, TString);
    void Represent_Overlapping_Categories();
    void Compare_Before_After_Orthogonalization();
    void Get_Yield_Per_Jet_Categ(TString);
    void Check_ChangeYields_Orthogonalization_FwdJet();
	void Make_1DPlot_Yield_EachCat_Overlap();

    //=== TESTING, TEMPRARY
    // TString Get_Category_Boolean_Name_AlternativeCat(TString, TString, bool, TString, TString);


//--------------------------------------------
	//Increment weight of first bin by 'weight'
	//Inline functions must be declared in header file
	inline void Fill_TH1F_UnderOverflow(TH1F* h, double value, double weight)
	{
		//--- Deprecated !! Must use Fill() not to loose error info !
		// if(value >= h->GetXaxis()->GetXmax() ) {h->AddBinContent(h->GetNbinsX(), weight);} //overflow
		// else if(value <= h->GetXaxis()->GetXmin() ) {h->AddBinContent(1, weight);} //underflow
		// else {h->Fill(value, weight);}

		if(value >= h->GetXaxis()->GetXmax() ) {h->Fill(h->GetXaxis()->GetXmax() - (h->GetXaxis()->GetBinWidth(1) / 2), weight);} //overflow in last bin
		else if(value <= h->GetXaxis()->GetXmin() ) {h->Fill(h->GetXaxis()->GetXmin() + (h->GetXaxis()->GetBinWidth(1) / 2), weight);} //underflow in first bin
		else {h->Fill(value, weight);}

		return;
	};

#endif
