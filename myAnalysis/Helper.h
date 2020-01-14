#ifndef Helper_h
#define Helper_h

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
#define CYANBKG(x) "\e[46m" x RST

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

//-- Basic helper functions
    bool Check_File_Existence(const TString&);
    void MoveFile(TString, TString);
    void CopyFile(TString, TString);
	TString Convert_Number_To_TString(double, int=3);
	double Convert_TString_To_Number(TString);
	float Find_Number_In_TString(TString);
	TString Convert_Sign_To_Word(TString);
	std::pair<TString,TString> Break_Cuts_In_Two(TString);
    bool Is_Event_Passing_Cut(TString, double);
    double Compute_RangeScaled_Value(double, double, double, double, double);
    double Compute_StdDevScaled_Value(double, double, double);
    bool Get_Dir_Content(std::string, std::vector<TString>&);

//-- ROOT-cpecific helper functions
    void Fill_Last_Bin_TH1F(TH1F*, double); //Increment last bin of histo by specified weight
    void Fill_First_Bin_TH1F(TH1F*, double); //Increment first bin of histo by specified weight
    void Load_Canvas_Style();
	TString Combine_Naming_Convention(TString);
    void Extract_Ranking_Info(TString, TString);
    void Get_Ranking_Vectors(TString, std::vector<TString>&, std::vector<double>&);
    void Compare_Histograms(TString, TString, TString, TString);

//-- Analysis-specific helper functions
    bool Apply_CommandArgs_Choices(int, char **, TString&, TString&);
    void Get_Samples_Colors(std::vector<int>&, std::vector<TString>, int);
    void Set_Custom_ColorPalette(std::vector<TColor*>&, std::vector<int>&); //Set custom color palette
    bool Get_Variable_Range(TString, int&, double&, double&);
    TString Get_Category_Boolean_Name();

//--------------------------------------------
	//Increment weight of first bin by 'weight'
	//Inline functions must be declared in header file
	inline void Fill_TH1F_UnderOverflow(TH1F* h, double value, double weight)
	{
		if(value >= h->GetXaxis()->GetXmax() ) {h->Fill(h->GetXaxis()->GetXmax() - (h->GetXaxis()->GetBinWidth(1) / 2), weight);} //overflow in last bin
		else if(value <= h->GetXaxis()->GetXmin() ) {h->Fill(h->GetXaxis()->GetXmin() + (h->GetXaxis()->GetBinWidth(1) / 2), weight);} //underflow in first bin
		else {h->Fill(value, weight);}

		return;
	};

#endif
