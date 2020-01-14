#ifndef TopEFT_analysis_h
#define TopEFT_analysis_h

#include <TROOT.h>
#include <TChain.h>
#include <TFile.h>
#include <TH1F.h>
#include <TH2F.h>
#include <TStyle.h>
#include <TCanvas.h>
#include <TLorentzVector.h>
#include "TTree.h"
#include "TObjString.h"
#include "TSystem.h"
#include "TString.h"
#include "TColor.h"
#include "TCut.h"
#include "TLegend.h"
#include "TLine.h"
#include "THStack.h"
#include "TString.h"
#include "TRandom.h"
#include "TLatex.h"
#include "TGraphErrors.h"
#include "TGraphAsymmErrors.h"
#include "TObject.h"
#include "TRandom3.h"
#include "TRandom1.h"
#include "TObjArray.h"
#include "TF1.h"
#include "TLegendEntry.h"
#include "TGaxis.h"
#include "TLeaf.h"
#include "TFractionFitter.h"
#include "TVirtualFitter.h"
#include "TFitResultPtr.h"
#include "TFitResult.h"

#include <iostream>
#include <cstdlib>
#include <iostream>
#include <map>
#include <string>
#include <cmath>
#include <sstream>
#include <fstream>

#include "TMVA/Tools.h"
#include "TMVA/Factory.h"
#include "TMVA/DataLoader.h"
#include "TMVA/Reader.h"
#include "TMVA/MethodCuts.h"
#include "TMVA/Timer.h"
#include "TMVA/Config.h"

#include <algorithm>
#include <cassert> 	//Can be used to terminate program if argument is not true. Ex : assert(test > 0 && "Error message");
#include <sys/stat.h> //for mkdir

#include "Func_other.h" //Helper functions

using namespace std;

class TopEFT_analysis
{

	public :

	TopEFT_analysis(); //Default constructor
	TopEFT_analysis(vector<TString>, vector<TString>, vector<TString>, vector<TString>, vector<TString>, vector<TString>, vector<TString>, vector<TString>, vector<bool>, vector<TString>, TString, bool, double, bool, bool, TString, bool, bool, bool, TString, bool, TString, bool, TString, TString, double, bool, bool, bool, TString, bool, vector<TString>, vector<TString>); //Overloaded constructor
	~TopEFT_analysis(); //Default destructor

//--- METHODS
	void Train_BDT(TString, TString, bool); //Train BDT
    // void Produce_Templates_old(TString, bool, bool=false); //Produce BDT templates //OLD
    void Produce_Templates(TString, bool, int=-1); //Produce BDT templates //NEW
	// void Produce_Control_Histograms(bool); //Produce histograms of input variables, etc.
	void Draw_Templates(bool, TString, TString="", bool=true, bool=false); //Draw BDT templates
	// void Draw_Control_Plots(TString); //Draw input variables
	void Draw_InputVars_NoStack(TString); //Draw input variables without stacking (for comparison with AN)
	void Draw_ColZ_Templates(TString); //Draw ColZ BDT2D templates (for comparison with AN)
	void Draw_ColZ_All(); //Call 'Draw_ColZ_Templates' for all templates
	void Produce_KerasDNN_Histos_For_ROC(TString, int=100); //Produce templates for Keras-only NN, that can be used to plot ROCs
	void Plot_KerasDNN_Outputs(TString, vector<TString>, TString);
    void Make_Overlap_Plots();
	void Merge_Templates_ByProcess(TString, TString, bool);
	void Modify_Template_Histograms(TString, TString, bool, bool);
    void Make_ScaleVariations_Histograms(bool, bool, bool);
    void Plot_ScaleVariations_Histograms(vector<TString>, vector<TString>, vector<Int_t>, bool, bool, bool);
    void Make_PDFvariations_Plot();
    void Compare_TemplateShapes_Processes(TString, TString);
    void Copy_Templates_withCERNconventions(TString, bool, TString);
    void Compare_Histograms();
	void Produce_Templates_ManyBinnings(TString);

//--- MEMBERS
	bool stop_program;


	private :

//--- METHODS
    void Set_Luminosity(double, TString);

//--- MEMBERS
    TMVA::Reader *reader;
    TMVA::Reader *reader1; //1 reader for BDT xxx
    TMVA::Reader *reader2; //1 reader for BDT yyy

    std::vector<TString> sample_list; //List of samples
    std::vector<TString> sample_groups; //List of "group naming", 1 per sample (many samples can have same naming)
    std::vector<TString> syst_list; //List of systematics stored as event weights
    std::vector<TString> systTree_list; //List of systematics stored as separate TTrees
	std::vector<TString> channel_list; //List of subchannels

	std::vector<TString> v_cut_name; //List of cut variable
	std::vector<TString> v_cut_def; //Cut definition
	std::vector<Float_t> v_cut_float; //Store variables needed for cut
	std::vector<Char_t> v_cut_char; //Categories are encoded into Char_t, because vector<bool> is 'broken' in C++
	std::vector<bool> v_cut_IsUsedForBDT; //true <-> the variable used for cut is also a input variable

	std::vector<TString> var_list; std::vector<Float_t> var_list_floats; //TMVA variables
	std::vector<TString> v_add_var_names; vector<Float_t> v_add_var_floats; //Additional vars only for CR plots

	std::vector<int> color_list;
	std::vector<TColor*> v_custom_colors;

	bool use_NeuralNetwork;
	TString DNN_type;
	TString classifier_name;

    TString region; //"SR" / "CR_ttZ" / "CR_ttW"
	TString categ_bool_name; //Name of boolean associated to category
	TString signal_process;
    TString dir_ntuples; //Path to base dir. containing Ntuples
    TString t_name;
	TString plot_extension;
	double luminosity_rescale;
	double ref_luminosity;
	int nbins; //nbin_templates
	TString filename_suffix;
	bool show_pulls_ratio;
    bool is_blind;
};

#endif
