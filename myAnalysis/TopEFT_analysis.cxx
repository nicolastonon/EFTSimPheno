//by Nicolas Tonon (DESY)

//--- LIST OF FUNCTIONS (for quick search) :
//--------------------------------------------
// Train_BDT

// Produce_Templates

//--------------------------------------------

#include "TopEFT_analysis.h"

// #include "TMVA/PyMethodBase.h" //PYTHON -- Keras interface
// #include "TMVA/MethodPyKeras.h" //PYTHON -- Keras interface
// #include "TMVA/HyperParameterOptimisation.h"
// #include "TMVA/VariableImportance.h"
// #include "TMVA/CrossValidation.h" //REMOVABLE -- needed for BDT optim

//To read weights from DNN trained with Keras (without TMVA) -- requires c++14, etc.
// #include "fdeep/fdeep.hpp" //REMOVABLE

#define MYDEBUG(msg) cout<<endl<<ITAL("-- DEBUG: " << __FILE__ << ":" << __LINE__ <<":")<<FRED(" " << msg  <<"")<<endl

using namespace std;





//---------------------------------------------------------------------------
// ####    ##    ##    ####    ########
//  ##     ###   ##     ##        ##
//  ##     ####  ##     ##        ##
//  ##     ## ## ##     ##        ##
//  ##     ##  ####     ##        ##
//  ##     ##   ###     ##        ##
// ####    ##    ##    ####       ##
//---------------------------------------------------------------------------

/////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////

//Overloaded constructor
TopEFT_analysis::TopEFT_analysis(vector<TString> thesamplelist, vector<TString> thesamplegroups, vector<TString> thesystlist, vector<TString> thesystTreelist, vector<TString> thechannellist, vector<TString> thevarlist, vector<TString> set_v_cut_name, vector<TString> set_v_cut_def, vector<bool> set_v_cut_IsUsedForBDT, vector<TString> set_v_add_var_names, TString theplotextension, double lumi, bool show_pulls, TString region, TString signal_process, TString classifier_name, TString DNN_type)
{
    //Canvas definition
    Load_Canvas_Style();

    TH1::SetDefaultSumw2();
    gStyle->SetErrorX(0.);

    nbins = 10; //default

	mkdir("outputs", 0777);
	mkdir("plots", 0777);

	stop_program = false;

	this->region = region;

    this->signal_process = signal_process;

	// this->is_blind = is_blind;

    sample_list.resize(thesamplelist.size());
    sample_groups.resize(thesamplelist.size());
	for(int i=0; i<thesamplelist.size(); i++)
	{
        sample_list[i] = thesamplelist[i];
        sample_groups[i] = thesamplegroups[i];
	}

	dir_ntuples = "./input_ntuples/";
	// cout<<"dir_ntuples : "<<dir_ntuples<<endl;

	//-- Get colors
	color_list.resize(sample_list.size());
	Get_Samples_Colors(color_list, sample_list, 0);

	this->classifier_name = classifier_name;
	if(classifier_name == "DNN")
	{
		if(DNN_type.Contains("TMVA", TString::kIgnoreCase)) {this->DNN_type = "TMVA";}
		else if(DNN_type.Contains("PyKeras", TString::kIgnoreCase)) {this->DNN_type = "PyKeras";}
		else if(DNN_type.Contains("Keras", TString::kIgnoreCase)) {this->DNN_type = "Keras";}
		else {cout<<BOLD(FRED("Wrong value of DNN_type !"))<<endl; stop_program = true;}
	}

    // t_name = "Tree";
    t_name = "result";

	Set_Luminosity(lumi, "2017");
	// if(use_2016_ntuples) {Set_Luminosity(35.862, "2016");} //Use 2016 lumi instead

	plot_extension = theplotextension;

	show_pulls_ratio = show_pulls;

	syst_list.resize(thesystlist.size());
	for(int i=0; i<thesystlist.size(); i++)
	{
		syst_list[i] = thesystlist[i];
	}
	if(syst_list.size() == 0 || syst_list[0] != "") {cout<<"ERROR : first element of 'syst_list' is not empty (<-> nominal event weight) ! If that's what you want, remove this protection !"<<endl; stop_program = true;}

	systTree_list.resize(thesystTreelist.size());
	for(int i=0; i<thesystTreelist.size(); i++)
	{
		systTree_list[i] = thesystTreelist[i];
	}
	if(systTree_list.size() == 0 || systTree_list[0] != "") {cout<<"ERROR : first element of 'systTree_list' is not empty (<-> nominal TTree) ! If that's what you want, remove this protection !"<<endl; stop_program = true;}

	channel_list.resize(thechannellist.size());
	for(int i=0; i<thechannellist.size(); i++)
	{
		channel_list[i] = thechannellist[i];
	}
	if(channel_list.size() == 0 || channel_list[0] != "") {cout<<"ERROR : first element of 'channel_list' is not empty (<-> no subcat.) or vector is empty ! If that's what you want, remove this protection !"<<endl; stop_program = true;}

	int nof_categories_activated = 0; //Make sure only 1 orthogonal cat. is activated at once
	for(int i=0; i<set_v_cut_name.size(); i++) //Region cuts vars (e.g. NJets)
	{
		if(set_v_cut_name[i].Contains("is_") && set_v_cut_def[i] == "==1") //Consider that this variable is an event category, encoded as Char_t
		{
			nof_categories_activated++;
		}

		v_cut_name.push_back(set_v_cut_name[i]);
		v_cut_def.push_back(set_v_cut_def[i]);
		v_cut_IsUsedForBDT.push_back(set_v_cut_IsUsedForBDT[i]);
		v_cut_float.push_back(-999);
		v_cut_char.push_back(0);

		//NOTE : it is a problem if a variable is present in more than 1 list, because it will cause SetBranchAddress conflicts (only the last SetBranchAddress to a branch will work)
		//---> If a variable is present in 2 lists, erase it from other lists !
		for(int ivar=0; ivar<thevarlist.size(); ivar++)
		{
			if(thevarlist[ivar].Contains("is_") )
			{
				cout<<BOLD(FRED("## Warning : categories should not been used as input/spectator variables ! Are you sure ? "))<<endl;
			}
			if(thevarlist[ivar] == set_v_cut_name[i])
			{
				cout<<FGRN("** Constructor")<<" : erased variable "<<thevarlist[ivar]<<" from vector thevarlist (possible conflict) !"<<endl;
				thevarlist.erase(thevarlist.begin() + ivar);
				ivar--; //modify index accordingly
			}

		}
		for(int ivar=0; ivar<set_v_add_var_names.size(); ivar++)
		{
			if(set_v_add_var_names[ivar].Contains("is_") )
			{
				cout<<BOLD(FRED("## Warning : categories should not been used as input/spectator variables ! Are you sure ? "))<<endl;
			}
			if(set_v_add_var_names[ivar] == set_v_cut_name[i])
			{
				cout<<FGRN("** Constructor")<<" : erased variable "<<set_v_add_var_names[ivar]<<" from vector set_v_add_var_names (possible conflict) !"<<endl;
				set_v_add_var_names.erase(set_v_add_var_names.begin() + ivar);
				ivar--; //modify index accordingly
			}
		}

		// cout<<"Cuts : name = "<<v_cut_name[i]<<" / def = "<<v_cut_def[i]<<endl;
	}
	for(int i=0; i<thevarlist.size(); i++) //TMVA vars
	{
		var_list.push_back(thevarlist[i]);
		var_list_floats.push_back(-999);

		for(int ivar=0; ivar<set_v_add_var_names.size(); ivar++)
		{
			if(set_v_add_var_names[ivar] == thevarlist[i])
			{
				cout<<FGRN("** Constructor")<<" : erased variable "<<set_v_add_var_names[ivar]<<" from vector set_v_add_var_names (possible conflict) !"<<endl;
				set_v_add_var_names.erase(set_v_add_var_names.begin() + ivar);
				ivar--; //modify index accordingly
			}
		}
	}

	for(int i=0; i<set_v_add_var_names.size(); i++) //Additional vars, only for CR plots
	{
		v_add_var_names.push_back(set_v_add_var_names[i]);
		v_add_var_floats.push_back(-999);
	}

	//Make sure that the "==" sign is written properly, or rewrite it
	for(int ivar=0; ivar<v_cut_name.size(); ivar++)
	{
		if( v_cut_def[ivar].Contains("=") && !v_cut_def[ivar].Contains("!") && !v_cut_def[ivar].Contains("==") && !v_cut_def[ivar].Contains("<") && !v_cut_def[ivar].Contains(">") )
		{
			v_cut_def[ivar] = "==" + Convert_Number_To_TString(Find_Number_In_TString(v_cut_def[ivar]));

			cout<<endl<<BOLD(FBLU("##################################"))<<endl;
			cout<<"--- Changed cut on "<<v_cut_name[ivar]<<" to: "<<v_cut_def[ivar]<<" ---"<<endl;
			cout<<BOLD(FBLU("##################################"))<<endl<<endl;
		}
	}

	// color_list.resize(thecolorlist.size());
	// for(int i=0; i<thecolorlist.size(); i++)
	// {
	// 	color_list[i] = thecolorlist[i];
	// }
	// if(use_custom_colorPalette) {Set_Custom_ColorPalette(v_custom_colors, color_list);}

	//Store the "cut name" that will be written as a suffix in the name of each output file
	this->filename_suffix = "";
	TString tmp = "";
	for(int ivar=0; ivar<v_cut_name.size(); ivar++)
	{
		if(v_cut_name[ivar].Contains("is_") ) {continue;} //No need to appear in filename
		else if(v_cut_name[ivar] == "nLightJets_Fwd40") {this->filename_suffix+= "_fwdCut"; continue;} //No need to appear in filename

		if(v_cut_def[ivar] != "")
		{
            if(!v_cut_def[ivar].Contains("&&") && !v_cut_def[ivar].Contains("||") ) //Single condition
            {
                tmp+= "_" + v_cut_name[ivar] + Convert_Sign_To_Word(v_cut_def[ivar]) + Convert_Number_To_TString(Find_Number_In_TString(v_cut_def[ivar]));
            }
            else if(v_cut_def[ivar].Contains("&&")) //Double '&&' condition
            {
                TString cut1 = Break_Cuts_In_Two(v_cut_def[ivar]).first, cut2 = Break_Cuts_In_Two(v_cut_def[ivar]).second;
                tmp = "_" + v_cut_name[ivar] + Convert_Sign_To_Word(cut1) + Convert_Number_To_TString(Find_Number_In_TString(cut1));
                tmp+= Convert_Sign_To_Word(cut2) + Convert_Number_To_TString(Find_Number_In_TString(cut2));
            }
            else if(v_cut_def[ivar].Contains("||") )
            {
                TString cut1 = Break_Cuts_In_Two(v_cut_def[ivar]).first, cut2 = Break_Cuts_In_Two(v_cut_def[ivar]).second;
                tmp = "_" + v_cut_name[ivar] + Convert_Sign_To_Word(cut1) + Convert_Number_To_TString(Find_Number_In_TString(cut1));
                tmp+= "OR" + Convert_Sign_To_Word(cut2) + Convert_Number_To_TString(Find_Number_In_TString(cut2));

            }

			this->filename_suffix+= tmp;
		}
	}

	cout<<endl<<endl<<BOLD(FBLU("[Region : "<<region<<"]"))<<endl<<endl<<endl;

    //-- Protections

    usleep(2000000); //Pause for 3s (in microsec)
}

TopEFT_analysis::~TopEFT_analysis()
{

}


/////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////
/**
 * Compute the luminosity re-scaling factor (MC),  to be used thoughout the code
 * @param desired_luminosity [Value of the desired lumi in fb-1]
 */
void TopEFT_analysis::Set_Luminosity(double desired_luminosity, TString reference)
{
	if(reference == "2017") {ref_luminosity = 41.5;}
	else if(reference == "2016") {ref_luminosity = 35.862;} //Moriond 2017 lumi ref.
	assert(ref_luminosity > 0 && "Using wrong lumi reference -- FIX !"); //Make sure we use 2016 or 2017 as ref.

	this->luminosity_rescale = desired_luminosity / ref_luminosity;

	if(luminosity_rescale != 1)
	{
		cout<<endl<<BOLD(FBLU("##################################"))<<endl;
		cout<<"--- Using luminosity scale factor : "<<desired_luminosity<<" / "<<ref_luminosity<<" = "<<luminosity_rescale<<" ! ---"<<endl;
		cout<<BOLD(FBLU("##################################"))<<endl<<endl;
	}
}













//---------------------------------------------------------------------------
// ########    ########        ###       ####    ##    ##    ####    ##    ##     ######
//    ##       ##     ##      ## ##       ##     ###   ##     ##     ###   ##    ##    ##
//    ##       ##     ##     ##   ##      ##     ####  ##     ##     ####  ##    ##
//    ##       ########     ##     ##     ##     ## ## ##     ##     ## ## ##    ##   ####
//    ##       ##   ##      #########     ##     ##  ####     ##     ##  ####    ##    ##
//    ##       ##    ##     ##     ##     ##     ##   ###     ##     ##   ###    ##    ##
//    ##       ##     ##    ##     ##    ####    ##    ##    ####    ##    ##     ######
//---------------------------------------------------------------------------


/////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////

void TopEFT_analysis::Train_BDT(TString channel, bool write_ranking_info)
{
	//--- Options
	bool use_relative_weights = true; //if false, will use fabs(weight)

//--------------------------------------------
	cout<<endl<<BOLD(FYEL("##################################"))<<endl;
	cout<<FYEL("--- TRAINING ---")<<endl;
	cout<<BOLD(FYEL("##################################"))<<endl<<endl;

	if(use_relative_weights) {cout<<"-- Using "<<BOLD(FGRN("*RELATIVE weights*"))<<" --"<<endl<<endl<<endl;}
	else {cout<<"-- Using "<<BOLD(FGRN("*ABSOLUTE weights*"))<<" --"<<endl<<endl<<endl;}

	mkdir("weights", 0777);
	mkdir("weights/BDT", 0777);
	// mkdir("weights/DNN", 0777);
	// mkdir("weights/DNN/TMVA", 0777);
	// mkdir("weights/DNN/PyKeras", 0777);
	// mkdir("weights/DNN/Keras", 0777);

	usleep(1000000); //Pause for 1s (in microsec)

//--------------------------------
//  ####  #    # #####  ####
// #    # #    #   #   #
// #      #    #   #    ####
// #      #    #   #        #
// #    # #    #   #   #    #
//  ####   ####    #    ####
//--------------------------------------------

	//---Apply additional cuts on the signal and background samples (can be different)
	TCut mycuts = "";
	TCut mycutb = "";
	TString tmp = "";

	//--- CHOOSE TRAINING EVENTS <--> cut on corresponding category
	TString cat_tmp = "";
	// cat_tmp = Get_Category_Boolean_Name(nLep_cat, region, analysis_type, "", scheme);

	//Even if ask templates in the SR, need to use training (looser) category for training !
	// if(cat_tmp.Contains("_SR") )
	// {
	// 	int i = cat_tmp.Index("_SR"); //Find index of substring
	// 	cat_tmp.Remove(i); //Remove substring
	// }
    // tmp+= cat_tmp + "==1";

	//--- Define additionnal cuts
	for(int ivar=0; ivar<v_cut_name.size(); ivar++)
	{
		if(v_cut_def[ivar] != "")
		{
			tmp+= " && ";

			if(!v_cut_def[ivar].Contains("&&") && !v_cut_def[ivar].Contains("||")) {tmp+= v_cut_name[ivar] + v_cut_def[ivar];} //If cut contains only 1 condition
			else if(v_cut_def[ivar].Contains("&&") && v_cut_def[ivar].Contains("||")) {cout<<BOLD(FRED("ERROR ! Wrong cut definition !"))<<endl;}
			else if(v_cut_def[ivar].Contains("&&") )//If '&&' in the cut, break it in 2
			{
				tmp+= v_cut_name[ivar] + Break_Cuts_In_Two(v_cut_def[ivar]).first;
				tmp+= " && ";
				tmp+= v_cut_name[ivar] + Break_Cuts_In_Two(v_cut_def[ivar]).second;
			}
			else if(v_cut_def[ivar].Contains("||") )//If '||' in the cut, break it in 2
			{
				tmp+= v_cut_name[ivar] + Break_Cuts_In_Two(v_cut_def[ivar]).first;
				tmp+= " || ";
				tmp+= v_cut_name[ivar] + Break_Cuts_In_Two(v_cut_def[ivar]).second;
			}
		}
	}

	bool split_by_leptonChan = false;
	if(split_by_leptonChan && (channel != "all" && channel != ""))
	{
		if(channel == "uuu" || channel == "uu")	{mycuts = "Channel==0"; mycutb = "Channel==0";}
		else if(channel == "uue" || channel == "ue") {mycuts = "Channel==1"; mycutb = "Channel==1";}
		else if(channel == "eeu" || channel == "ee") {mycuts = "Channel==2"; mycutb = "Channel==2";}
		else if(channel == "eee") {mycuts = "Channel==3"; mycutb = "Channel==3";}
		else {cout << "WARNING : wrong channel name while training " << endl;}
	}

	cout<<"-- Will apply the following cut(s) : "<<BOLD(FGRN(""<<tmp<<""))<<endl<<endl<<endl<<endl;
	usleep(2000000); //Pause for 2s (in microsec)

	if(tmp != "") {mycuts+= tmp; mycutb+= tmp;}

	//--------------------------------------------
	//---------------------------------------------------------------
    // This loads the TMVA libraries
    TMVA::Tools::Instance();

	//Allows to bypass a protection in TMVA::Transplot_extensionionHandler, cf. description in source file:
	// if there are too many input variables, the creation of correlations plots blows up memory and basically kills the TMVA execution --> avoid above critical number (which can be user defined)
	(TMVA::gConfig().GetVariablePlotting()).fMaxNumOfAllowedVariablesForScatterPlots = 300;

	TString output_file_name = "outputs/" + classifier_name;
	if(channel != "") {output_file_name+= "_" + channel;}
	if(classifier_name == "DNN") {output_file_name+= "_" + DNN_type;}
	output_file_name+= "__" + signal_process;
	output_file_name+= this->filename_suffix + ".root";

	TFile* output_file = TFile::Open( output_file_name, "RECREATE" );

	// Create the factory object
	// TMVA::Factory* factory = new TMVA::Factory(type.Data(), output_file, "!V:!Silent:Color:DrawProgressBar:AnalysisType=Classification" );
	TString weights_dir = "weights";
	TMVA::DataLoader *dataloader = new TMVA::DataLoader(weights_dir); //If no TString given in arg, will store weights in : default/weights/...

	//--- Could modify here the name of dir. containing the BDT weights (default = "weights")
	//By setting it to "", weight files will be stored directly at the path given to dataloader
	//Complete path for weight files is : [path_given_toDataloader]/[fWeightFileDir]
	//Apparently, TMVAGui can't handle nested repos in path given to dataloader... so split path in 2 here
	TMVA::gConfig().GetIONames().fWeightFileDir = "BDT";
	if(classifier_name == "DNN" && DNN_type == "TMVA") {TMVA::gConfig().GetIONames().fWeightFileDir = "DNN/TMVA";}

//--------------------------------------------
 // #    #   ##   #####  #   ##   #####  #      ######  ####
 // #    #  #  #  #    # #  #  #  #    # #      #      #
 // #    # #    # #    # # #    # #####  #      #####   ####
 // #    # ###### #####  # ###### #    # #      #           #
 //  #  #  #    # #   #  # #    # #    # #      #      #    #
 //   ##   #    # #    # # #    # #####  ###### ######  ####
//--------------------------------------------

	// Define the input variables that shall be used for the MVA training
	for(int i=0; i<var_list.size(); i++)
	{
		dataloader->AddVariable(var_list[i].Data(), 'F');
	}

	//Choose if the cut variables are used in BDT or not
	//Spectator vars are not used for training/evalution, but possible to check their correlations, etc.
	for(int i=0; i<v_cut_name.size(); i++)
	{
		// cout<<"Is "<<v_cut_name[i]<<" used ? "<<(v_cut_IsUsedForBDT[i] && !v_cut_def[i].Contains("=="))<<endl;

		// if we ask "var == x", all the selected events will be equal to x, so can't use it as discriminant variable !
		if(v_cut_IsUsedForBDT[i] && !v_cut_def[i].Contains("==")) {dataloader->AddVariable(v_cut_name[i].Data(), 'F');}
		// else {dataloader->AddSpectator(v_cut_name[i].Data(), v_cut_name[i].Data(), 'F');}
	}
	for(int i=0; i<v_add_var_names.size(); i++)
	{
		// dataloader->AddSpectator(v_add_var_names[i].Data(), v_add_var_names[i].Data(), 'F');
	}

	double nEvents_sig = 0;
	double nEvents_bkg = 0;


//--------------------------------------------
 //                          #
 //  ####  #  ####          #     #####  #    #  ####
 // #      # #    #        #      #    # #   #  #    #
 //  ####  # #            #       #####  ####   #
 //      # # #  ###      #        #    # #  #   #  ###
 // #    # # #    #     #         #    # #   #  #    #
 //  ####  #  ####     #          #####  #    #  ####
//--------------------------------------------
	//--- Only use few samples for training
	std::vector<TFile *> files_to_close;
	for(int isample=0; isample<sample_list.size(); isample++)
    {
		cout<<"-- Sample : "<<sample_list[isample]<<endl;

		TString samplename_tmp = sample_list[isample];

        // --- Register the training and test trees
        TString inputfile = dir_ntuples + sample_list[isample] + ".root";

//--------------------------------------------
// ##### ##### #####  ###### ######  ####
//   #     #   #    # #      #      #
//   #     #   #    # #####  #####   ####
//   #     #   #####  #      #           #
//   #     #   #   #  #      #      #    #
//   #     #   #    # ###### ######  ####
//--------------------------------------------

	    TFile *file_input = 0, *file_input_train = 0, *file_input_test = 0;
		TTree *tree = 0, *tree_train = 0, *tree_test = 0;

        file_input = TFile::Open(inputfile);
        if(!file_input) {cout<<BOLD(FRED(<<inputfile<<" not found!"))<<endl; continue;}
        files_to_close.push_back(file_input);
        tree = (TTree*) file_input->Get(t_name);
        if(tree==0) {cout<<BOLD(FRED("ERROR :"))<<" file "<<inputfile<<" --> *tree = 0 !"<<endl; continue;}
        else {cout<<endl<<FMAG("=== Opened file : ")<<inputfile<<endl<<endl;}

        // global event weights per tree (see below for setting event-wise weights)
		//NB : in tHq2016, different global weights were attributed to each sample !
		// ( see : https://github.com/stiegerb/cmgtools-lite/blob/80X_M17_tHqJan30/TTHAnalysis/python/plotter/tHq-multilepton/signal_mva/trainTHQMVA.py#L78-L98)
        Double_t signalWeight     = 1.0;
        Double_t backgroundWeight = 1.0;

    //-- Choose between absolute/relative weights for training
		if(samplename_tmp.Contains(signal_process) )
		{
            nEvents_sig+= tree->GetEntries(mycuts); dataloader->AddSignalTree(tree, signalWeight);

			if(use_relative_weights)
			{
                // TString weightExp = "weight";
                TString weightExp = "eventWeight";
				dataloader->SetSignalWeightExpression(weightExp);
				cout<<"Signal sample : "<<samplename_tmp<<" / Weight expression : "<<weightExp<<endl<<endl;
			}
			else
			{
                // TString weightExp = "fabs(weight)";
                TString weightExp = "fabs(eventWeight)";
				dataloader->SetSignalWeightExpression(weightExp);
				cout<<"Signal sample : "<<samplename_tmp<<" / Weight expression : "<<weightExp<<endl<<endl;
			}
		}
		else
		{
            nEvents_bkg+= tree->GetEntries(mycutb); dataloader->AddBackgroundTree(tree, backgroundWeight);

            if(use_relative_weights)
			{
                // TString weightExp = "weight";
                TString weightExp = "eventWeight";
				dataloader->SetBackgroundWeightExpression(weightExp);
				cout<<"Background sample : "<<samplename_tmp<<" / Weight expression : "<<weightExp<<endl;
			}
			else
			{
                // TString weightExp = "fabs(weight)";
                TString weightExp = "fabs(eventWeight)";
				dataloader->SetBackgroundWeightExpression(weightExp);
				cout<<"Background sample : "<<samplename_tmp<<" / Weight expression : "<<weightExp<<endl;
			}
		}
    }


//--------------------------------
// #####  #####  ###### #####    ##   #####  ######    ##### #####  ###### ######  ####
// #    # #    # #      #    #  #  #  #    # #           #   #    # #      #      #
// #    # #    # #####  #    # #    # #    # #####       #   #    # #####  #####   ####
// #####  #####  #      #####  ###### #####  #           #   #####  #      #           #
// #      #   #  #      #      #    # #   #  #           #   #   #  #      #      #    #
// #      #    # ###### #      #    # #    # ######      #   #    # ###### ######  ####
//--------------------------------------------

	if(mycuts != mycutb) {cout<<__LINE__<<FRED("PROBLEM : cuts are different for signal and background ! If this is normal, modify code -- Abort")<<endl; return;}

    // Tell the factory how to use the training and testing events

	// If nTraining_Events=nTesting_Events="0", half of the events in the tree are used for training, and the other half for testing
	//NB : ttH seem to use 80% of stat for training, 20% for testing. Kirill using 25K training and testing events for now
	//NB : when converting nEvents to TString, make sure to ask for sufficient precision !

	// float trainingEv_proportion = 0.5;
	float trainingEv_proportion = 0.7;

	//-- Choose dataset splitting
	TString nTraining_Events_sig = "", nTraining_Events_bkg = "", nTesting_Events_sig = "", nTesting_Events_bkg = "";

    int nmaxEv = 150000; //max nof events for train or test
    int nTrainEvSig = (nEvents_sig * trainingEv_proportion < nmaxEv) ? nEvents_sig * trainingEv_proportion : nmaxEv;
    int nTrainEvBkg = (nEvents_bkg * trainingEv_proportion < nmaxEv) ? nEvents_bkg * trainingEv_proportion : nmaxEv;
    int nTestEvSig = (nEvents_sig * (1-trainingEv_proportion) < nmaxEv) ? nEvents_sig * (1-trainingEv_proportion) : nmaxEv;
    int nTestEvBkg = (nEvents_bkg * (1-trainingEv_proportion) < nmaxEv) ? nEvents_bkg * (1-trainingEv_proportion) : nmaxEv;

    nTraining_Events_sig = Convert_Number_To_TString(nTrainEvSig, 12);
    nTraining_Events_bkg = Convert_Number_To_TString(nTrainEvBkg, 12);
    nTesting_Events_sig = Convert_Number_To_TString(nTestEvSig, 12);
    nTesting_Events_bkg = Convert_Number_To_TString(nTestEvBkg, 12);

	cout<<endl<<endl<<FBLU("===================================")<<endl;
	cout<<FBLU("-- Requesting "<<nTraining_Events_sig<<" Training events [SIGNAL]")<<endl;
	cout<<FBLU("-- Requesting "<<nTesting_Events_sig<<" Testing events [SIGNAL]")<<endl;
	cout<<FBLU("-- Requesting "<<nTraining_Events_bkg<<" Training events [BACKGROUND]")<<endl;
	cout<<FBLU("-- Requesting "<<nTesting_Events_bkg<<" Testing events [BACKGROUND]")<<endl;
	cout<<FBLU("===================================")<<endl<<endl<<endl;

    dataloader->PrepareTrainingAndTestTree(mycuts, mycutb, "nTrain_Signal="+nTraining_Events_sig+":nTrain_Background="+nTraining_Events_bkg+":nTest_Signal="+nTesting_Events_sig+":nTest_Background="+nTesting_Events_bkg+":SplitMode=Random:!V");

	//-- for quick testing
	// dataloader->PrepareTrainingAndTestTree(mycuts, mycutb, "nTrain_Signal=10:nTrain_Background=10:nTest_Signal=10:nTest_Background=10:SplitMode=Random:NormMode=NumEvents:!V");


	//--- Boosted Decision Trees -- Choose method
	TMVA::Factory *factory = new TMVA::Factory(classifier_name, output_file, "V:!Silent:Color:DrawProgressBar:Correlations=True:AnalysisType=Classification");

	// TString method_title = channel + this->filename_suffix; //So that the output weights are labelled differently for each channel
	TString method_title = ""; //So that the output weights are labelled differently for each channel
	if(channel != "") {method_title = channel;}
	else {method_title = "all";}
	method_title+= "_" + region;
	method_title+= "__" + signal_process;

//--------------------------------------------
//  ####  #####  ##### #  ####  #    #  ####     #    # ###### ##### #    #  ####  #####
// #    # #    #   #   # #    # ##   # #         ##  ## #        #   #    # #    # #    #
// #    # #    #   #   # #    # # #  #  ####     # ## # #####    #   ###### #    # #    #
// #    # #####    #   # #    # #  # #      #    #    # #        #   #    # #    # #    #
// #    # #        #   # #    # #   ## #    #    #    # #        #   #    # #    # #    #
//  ####  #        #   #  ####  #    #  ####     #    # ######   #   #    #  ####  #####
//--------------------------------------------
    TString method_options = "";

    //ttH2017
    method_options= "!H:!V:NTrees=200:BoostType=Grad:Shrinkage=0.10:!UseBaggedGrad:nCuts=200:nEventsMin=100:NNodesMax=5:MaxDepth=8:NegWeightTreatment=PairNegWeightsGlobal:CreateMVAPdfs:DoBoostMonitor=True";


//--------------------------------------------
 // ##### #####    ##   # #    #       ##### ######  ####  #####       ###### #    #   ##   #
 //   #   #    #  #  #  # ##   #         #   #      #        #         #      #    #  #  #  #
 //   #   #    # #    # # # #  #         #   #####   ####    #         #####  #    # #    # #
 //   #   #####  ###### # #  # #         #   #           #   #         #      #    # ###### #
 //   #   #   #  #    # # #   ##         #   #      #    #   #         #       #  #  #    # #
 //   #   #    # #    # # #    #         #   ######  ####    #         ######   ##   #    # ######
//--------------------------------------------

	if(classifier_name == "BDT") {factory->BookMethod(dataloader, TMVA::Types::kBDT, method_title, method_options);} //Book BDT
	else if(DNN_type == "TMVA") {factory->BookMethod(dataloader, TMVA::Types::kDNN, method_title, method_options);}

	output_file->cd();

	mkdir("outputs/Rankings", 0777); //Dir. containing variable ranking infos
	mkdir("outputs/ROCS", 0777); //Dir. containing variable ranking infos

	TString ranking_file_path = "outputs/Rankings/rank_"+classifier_name+"_"+region+".txt";

	if(write_ranking_info) cout<<endl<<endl<<endl<<FBLU("NB : Temporarily redirecting standard output to file '"<<ranking_file_path<<"' in order to save Ranking Info !!")<<endl<<endl<<endl;

	std::ofstream out("ranking_info_tmp.txt"); //Temporary name
	std::streambuf *coutbuf = std::cout.rdbuf(); //save old buf
	if(write_ranking_info) std::cout.rdbuf(out.rdbuf()); //redirect std::cout to text file --> Ranking info will be saved !

    // Train MVAs using the set of training events
    factory->TrainAllMethods();

	if(write_ranking_info) std::cout.rdbuf(coutbuf); //reset to standard output again

	//-- NB : Test & Evaluation recap in the output files
    factory->TestAllMethods(); // ---- Evaluate all MVAs using the set of test events
    factory->EvaluateAllMethods(); // ----- Evaluate and compare performance of all configured MVAs

	//Could retrieve ROC graph directly
	// TMultiGraph* rocgraph = f.GetROCCurveAsMultiGraph("<datasetname>");

    // --------------------------------------------------------------
    // Save the output
    output_file->Close();
    std::cout << "==> Wrote root file: " << output_file->GetName() << std::endl;
    std::cout << "==> TMVA is done!" << std::endl;

	if(write_ranking_info)
	{
		MoveFile("./ranking_info_tmp.txt", ranking_file_path);
		Extract_Ranking_Info(ranking_file_path, channel); //Extract only ranking info from TMVA output
	}
	else {system("rm ./ranking_info_tmp.txt");} //Else remove the temporary ranking file

	for(unsigned int i=0; i<files_to_close.size(); i++) {files_to_close[i]->Close(); delete files_to_close[i];}

	delete dataloader; dataloader = NULL;
	delete factory; factory = NULL;
	output_file->Close(); output_file = NULL;

	return;
}
































//---------------------------------------------------------------------------
//  ######  ########  ########    ###    ######## ########       ######## ######## ##     ## ########  ##          ###    ######## ########  ######
// ##    ## ##     ## ##         ## ##      ##    ##                ##    ##       ###   ### ##     ## ##         ## ##      ##    ##       ##    ##
// ##       ##     ## ##        ##   ##     ##    ##                ##    ##       #### #### ##     ## ##        ##   ##     ##    ##       ##
// ##       ########  ######   ##     ##    ##    ######            ##    ######   ## ### ## ########  ##       ##     ##    ##    ######    ######
// ##       ##   ##   ##       #########    ##    ##                ##    ##       ##     ## ##        ##       #########    ##    ##             ##
// ##    ## ##    ##  ##       ##     ##    ##    ##                ##    ##       ##     ## ##        ##       ##     ##    ##    ##       ##    ##
//  ######  ##     ## ######## ##     ##    ##    ########          ##    ######## ##     ## ##        ######## ##     ##    ##    ########  ######
//---------------------------------------------------------------------------


void TopEFT_analysis::Produce_Templates(TString template_name, bool makeHisto_inputVars)
{
//--------------------------------------------
    // bool xxx = xxx;
//--------------------------------------------

    cout<<endl<<BOLD(FYEL("##################################"))<<endl;
	if(makeHisto_inputVars) {cout<<FYEL("--- Producing Input variables histograms ---")<<endl;}
	else if(template_name == "") {cout<<FYEL("--- Producing "<<template_name<<" Templates ---")<<endl;}
	else {cout<<BOLD(FRED("--- ERROR : invalid arguments ! Exit !"))<<endl; cout<<"Valid template names are : ttbar / ttV / 2D / 2Dlin !"<<endl; return;}
	cout<<BOLD(FYEL("##################################"))<<endl<<endl;

	if(classifier_name != "BDT") {cout<<BOLD(FRED("Error : DNNs are not supported !"))<<endl; return;}

	//Don't make systematics shifted histos for input vars (too long)
	if(makeHisto_inputVars)
	{
		classifier_name = ""; //For naming conventions
		// syst_list.resize(1); syst_list[0] = ""; //Force Remove systematics
	}

//  ####  ###### ##### #    # #####
// #      #        #   #    # #    #
//  ####  #####    #   #    # #    #
//      # #        #   #    # #####
// #    # #        #   #    # #
//  ####  ######   #    ####  #

	TH1::SetDefaultSumw2();

	//Output file name
	//-- For BDT templates
	TString output_file_name = "outputs/Templates_" + classifier_name + template_name;
	output_file_name+= "_" + region + filename_suffix;
    output_file_name+= ".root";

	//-- For input vars
	if(makeHisto_inputVars)
	{
		output_file_name = "outputs/ControlHistograms";
		output_file_name+= "_" + region + filename_suffix +".root";
	}

	//Create output file
	TFile* file_output = 0;
    mkdir("./outputs/tests/", 0777);
    output_file_name = "outputs/tests/Templates_" + classifier_name + template_name;
    output_file_name+= "_" + region + filename_suffix + ".root";
    file_output = TFile::Open(output_file_name, "RECREATE");

    reader = new TMVA::Reader( "!Color:!Silent" );

	// Name & adress of local variables which carry the updated input values during the event loop
	// NB : the variable names MUST corresponds in name and type to those given in the weight file(s) used -- same order
	// NB : if booking 2 BDTs (e.g. template_name=="2Dlin", must make sure that they use the same input variables... or else, find some way to make it work in the code)

	for(int i=0; i<var_list.size(); i++)
	{
        reader->AddVariable(var_list[i].Data(), &var_list_floats[i]);
        //cout<<"Added variable "<<var_list[i]<<endl;
	}

	for(int i=0; i<v_cut_name.size(); i++)
	{
		if(v_cut_IsUsedForBDT[i] && !v_cut_def[i].Contains("=="))
		{
            reader->AddVariable(v_cut_name[i].Data(), &v_cut_float[i]);
		}
	}

	// --- Book the MVA methods (1 or 2, depending on template)
	TString dir = "weights/";
	dir+= classifier_name;

	TString MVA_method_name1 = "", MVA_method_name2 = "";
	TString weightfile = "";
	TString template_name_MVA = "";
	if(!makeHisto_inputVars)
	{
		if(template_name == "ttbar" || template_name == "ttV") //Book only 1 BDT
		{
			template_name_MVA = template_name + "_all";

			MVA_method_name1 = template_name_MVA + " method";
			weightfile = dir + "/" + classifier_name + "_" + template_name_MVA;
			weightfile+= "__" + signal_process + ".weights.xml";

			if(!Check_File_Existence(weightfile) ) {cout<<BOLD(FRED("Weight file "<<weightfile<<" not found ! Abort"))<<endl; return;}

			reader->BookMVA(MVA_method_name1, weightfile);
		}
		else if((template_name == "2Dlin" || template_name == "2D") && !makeHisto_inputVars) //Need to book 2 BDTs
		{
			template_name_MVA = "ttbar_all";
			MVA_method_name1 = template_name_MVA + " method";
			weightfile = dir + "/" + classifier_name + "_" + template_name_MVA + "__" + signal_process + ".weights.xml";
			if(!Check_File_Existence(weightfile) ) {cout<<BOLD(FRED("Weight file "<<weightfile<<" not found ! Abort"))<<endl; return;}
			reader1->BookMVA(MVA_method_name1, weightfile);

			template_name_MVA = "ttV_all";
			MVA_method_name2 = template_name_MVA + " method";
			weightfile = dir + "/" + classifier_name + "_" + template_name_MVA + "__" + signal_process + ".weights.xml";
			if(!Check_File_Existence(weightfile) ) {cout<<BOLD(FRED("Weight file "<<weightfile<<" not found ! Abort"))<<endl; return;}
			reader2->BookMVA(MVA_method_name2, weightfile);
		}

		// cout<<"MVA_method_name1 "<<MVA_method_name1<<endl;
		// cout<<"MVA_method_name2 "<<MVA_method_name2<<endl;
	}

	//Input TFile and TTree, called for each sample
	TFile* file_input;
	TTree* tree(0);

	//Template binning
	double xmin = -1, xmax = 1;
	nbins = 10;

	//Want to plot ALL selected variables
	vector<TString> total_var_list;
	if(makeHisto_inputVars)
	{
		for(int i=0; i<var_list.size(); i++)
		{
			total_var_list.push_back(var_list[i]);
		}
		for(int i=0; i<v_add_var_names.size(); i++)
		{
			total_var_list.push_back(v_add_var_names[i]);
		}
	}
	else
	{
		total_var_list.push_back(template_name);
	}
	vector<float> total_var_floats(total_var_list.size()); //NB : can not read/cut on BDT... (would conflict with input var floats ! Can not set address twice)

 // #    #      ##      #    #    #
 // ##  ##     #  #     #    ##   #
 // # ## #    #    #    #    # #  #
 // #    #    ######    #    #  # #
 // #    #    #    #    #    #   ##
 // #    #    #    #    #    #    #

 // #       ####   ####  #####   ####
 // #      #    # #    # #    # #
 // #      #    # #    # #    #  ####
 // #      #    # #    # #####       #
 // #      #    # #    # #      #    #
 // ######  ####   ####  #       ####

	cout<<endl<<endl<<"-- Ntuples dir. : "<<dir_ntuples<<endl<<endl;

	//SAMPLE LOOP
	for(int isample=0; isample<sample_list.size(); isample++)
	{
		cout<<endl<<endl<<UNDL(FBLU("Sample : "<<sample_list[isample]<<""))<<endl;

		//Open input TFile
		TString inputfile = dir_ntuples + sample_list[isample] + ".root";

		// cout<<"inputfile "<<inputfile<<endl;
		if(!Check_File_Existence(inputfile))
		{
			cout<<endl<<"File "<<inputfile<<FRED(" not found!")<<endl;
			continue;
		}

		file_input = TFile::Open(inputfile, "READ");

		//-- Loop on TTrees : first empty element corresponds to nominal TTree ; additional TTrees may correspond to JES/JER TTrees (defined in main)
		//NB : only nominal TTree contains systematic weights ; others only contain the nominal weight (but variables have different values)
		for(int itree=0; itree<systTree_list.size(); itree++)
		{
			tree = 0;
			if(systTree_list[itree] == "") {tree = (TTree*) file_input->Get(t_name);}
			else {tree = (TTree*) file_input->Get(systTree_list[itree]);}

			if(!tree)
			{
				cout<<BOLD(FRED("ERROR : tree '"<<systTree_list[itree]<<"' not found for sample : "<<sample_list[isample]<<" ! Skip !"))<<endl;
				continue; //Skip sample
			}


//   ##   #####  #####  #####  ######  ####   ####  ######  ####
//  #  #  #    # #    # #    # #      #      #      #      #
// #    # #    # #    # #    # #####   ####   ####  #####   ####
// ###### #    # #    # #####  #           #      # #           #
// #    # #    # #    # #   #  #      #    # #    # #      #    #
// #    # #####  #####  #    # ######  ####   ####  ######  ####

			//Disactivate all un-necessary branches ; below, activate only needed ones
			tree->SetBranchStatus("*", 0); //disable all branches, speed up
			// tree->SetBranchStatus("xxx", 1);

			if(makeHisto_inputVars)
			{
				for(int i=0; i<total_var_list.size(); i++)
				{
                    tree->SetBranchStatus(total_var_list[i], 1);
                    tree->SetBranchAddress(total_var_list[i], &total_var_floats[i]);
				}
			}
			else //Book input variables in same order as trained BDT
			{
                for(int i=0; i<var_list.size(); i++)
                {
                    tree->SetBranchStatus(var_list[i], 1);
                    tree->SetBranchAddress(var_list[i], &var_list_floats[i]);
                    // cout<<"Activate var '"<<var_list[i]<<"'"<<endl;
                }
			}

			for(int i=0; i<v_cut_name.size(); i++)
			{
				tree->SetBranchStatus(v_cut_name[i], 1);
				if(v_cut_name[i].Contains("is_") ) //Categories are encoded into Char_t, not float
				{
					tree->SetBranchAddress(v_cut_name[i], &v_cut_char[i]);
				}
				else //All others are floats
				{
					tree->SetBranchAddress(v_cut_name[i], &v_cut_float[i]);
				}
			}

			//--- Cut on relevant event selection (e.g. 3l SR, ttZ CR, etc.) -- stored as Char_t
			Char_t is_goodCategory; //Categ. of event
			// TString cat_name = Get_Category_Boolean_Name(nLep_cat, region, analysis_type, sample_list[isample], scheme);
			// tree->SetBranchStatus(cat_name, 1);
			// tree->SetBranchAddress(cat_name, &is_goodCategory);
			// cout<<"Categ <=> "<<cat_name<<endl;

			//--- Cut on relevant categorization (lepton flavour, btagging, charge)
			float channel, nMediumBJets, lepCharge;
			// tree->SetBranchStatus("channel", 1);
			// tree->SetBranchAddress("channel", &channel);
			// tree->SetBranchStatus("nMediumBJets", 1);
			// tree->SetBranchAddress("nMediumBJets", &nMediumBJets);
			// tree->SetBranchStatus("lepCharge", 1);
			// tree->SetBranchAddress("lepCharge", &lepCharge);
	//--- Event weights
			float weight; float weight_SF; //Stored separately
			float SMcoupling_SF, SMcoupling_weight, mc_weight_originalValue;
            float chargeLeadingLep;
			tree->SetBranchStatus("eventWeight", 1);
			tree->SetBranchAddress("eventWeight", &weight);
			// tree->SetBranchStatus("mc_weight_originalValue", 1);
			// tree->SetBranchAddress("mc_weight_originalValue", &mc_weight_originalValue);

            //Reserve 1 float for each systematic weight
			vector<Float_t> v_float_systWeights(syst_list.size());
			for(int isyst=0; isyst<syst_list.size(); isyst++)
			{
				//-- Protections : not all syst weights apply to all samples, etc.
				if(sample_list[isample] == "DATA" || sample_list[isample] == "QFlip") {break;}
				else if(systTree_list[itree] != "") {break;} //Syst event weights only stored in nominal TTree
				else if((syst_list[isyst].Contains("FR_") ) && !sample_list[isample].Contains("Fake") ) {continue;}
				else if(sample_list[isample].Contains("Fake") && !syst_list[isyst].Contains("FR_") && syst_list[isyst] != "") {continue;}
                else if(syst_list[isyst].Contains("thu_shape") || syst_list[isyst].Contains("Clos") ) {continue;} //these weights are computed within this func

				if(syst_list[isyst] != "") {tree->SetBranchStatus(syst_list[isyst], 1); tree->SetBranchAddress(syst_list[isyst], &v_float_systWeights[isyst]);} //Nominal weight already set, don't redo it
			}

			//Reserve memory for 1 TH1F* per category, per systematic
			vector<vector<vector<TH1F*>>> v3_histo_chan_syst_var(channel_list.size());
			vector<vector<vector<TH2F*>>> v3_histo_chan_syst_var2D(channel_list.size());

			for(int ichan=0; ichan<channel_list.size(); ichan++)
			{
				// if((channel_list.size() > 1 && channel_list[ichan] == "") || sample_list[isample] == "DATA" || sample_list[isample] == "QFlip" || systTree_list[itree] != "") {v3_histo_chan_syst_var[ichan].resize(1);} //Cases for which we only need to store the nominal weight
				if(sample_list[isample] == "DATA" || sample_list[isample] == "QFlip" || systTree_list[itree] != "")
				{
					v3_histo_chan_syst_var[ichan].resize(1); //Cases for which we only need to store the nominal weight
					if(template_name == "2D") {v3_histo_chan_syst_var2D[ichan].resize(1);}
				}
				else //Subcategories -> 1 histo for nominal + 1 histo per systematic
				{
					v3_histo_chan_syst_var[ichan].resize(syst_list.size());
					if(template_name == "2D") {v3_histo_chan_syst_var2D[ichan].resize(syst_list.size());}
				}

				//Init histos
				for(int isyst=0; isyst<v3_histo_chan_syst_var[ichan].size(); isyst++)
				{
					v3_histo_chan_syst_var[ichan][isyst].resize(total_var_list.size());

					if(template_name == "2D")
					{
						v3_histo_chan_syst_var2D[ichan][isyst].resize(1);
					}

					for(int ivar=0; ivar<total_var_list.size(); ivar++)
					{
						// if(makeHisto_inputVars && !Get_Variable_Range(total_var_list[ivar], nbins, xmin, xmax)) {cout<<FRED("Unknown variable name : "<<total_var_list[ivar]<<"! (add in function 'Get_Variable_Range()')")<<endl; continue;} //Get binning for this variable (if not template) //FIXME

						v3_histo_chan_syst_var[ichan][isyst][ivar] = new TH1F("", "", nbins, xmin, xmax);
						if(template_name == "2D") {v3_histo_chan_syst_var2D[ichan][isyst][0] = new TH2F("", "", 10, -1, 1, 10, -1, 1);}
					}
				} //syst
			} //chan

			// cout<<endl<< "--- "<<sample_list[isample]<<" : Processing: " << tree->GetEntries() << " events" << std::endl;


// ###### #    # ###### #    # #####    #       ####   ####  #####
// #      #    # #      ##   #   #      #      #    # #    # #    #
// #####  #    # #####  # #  #   #      #      #    # #    # #    #
// #      #    # #      #  # #   #      #      #    # #    # #####
// #       #  #  #      #   ##   #      #      #    # #    # #
// ######   ##   ###### #    #   #      ######  ####   ####  #

			cout<<"* Tree '"<<systTree_list[itree]<<"' :"<<endl;

			// int nentries = 10;
			int nentries = tree->GetEntries();

			// if(writeTemplate_forAllCouplingPoints) {nentries = 100;}

			float total_nentries = total_var_list.size()*nentries;
			cout<<"Will process : "<<total_var_list.size()<<" vars * "<<nentries<<" = "<<setprecision(9)<<total_nentries<<" events...."<<endl<<endl;

			//Draw progress bar
			// Int_t ibar = 0; //progress bar
			// TMVA::Timer timer(total_nentries, "", true);
			// TMVA::gConfig().SetDrawProgressBar(1);

			for(int ientry=0; ientry<nentries; ientry++)
			{
				// cout<<"ientry "<<ientry<<endl;

				if(!makeHisto_inputVars && ientry%20000==0) {cout<<ITAL(""<<ientry<<" / "<<nentries<<"")<<endl;}
				// else if(makeHisto_inputVars)
				// {
				// 	ibar++;
				// 	if(ibar%20000==0) {timer.DrawProgressBar(ibar, "test");}
				// }

				weight_SF = 1;
				SMcoupling_SF = 1;

				//Reset all vectors reading inputs to 0
				// std::fill(var_list_floats.begin(), var_list_floats.end(), 0);
				// std::fill(var_list_floats_ttbar.begin(), var_list_floats_ttbar.end(), 0);
				// std::fill(var_list_floats_ttV.begin(), var_list_floats_ttV.end(), 0);

				tree->GetEntry(ientry);

				// if(sample_list[isample] == "tZq") {cout<<"weight "<<weight<<endl;}
				if(isnan(weight) || isinf(weight))
				{
					cout<<BOLD(FRED("* Found event with weight = "<<weight<<" ; remove it..."))<<endl; continue;
				}

				//--- Cut on category value
				if(!is_goodCategory) {continue;}

//---- APPLY CUTS HERE (defined in main)  ----
				bool pass_all_cuts = true;
				for(int icut=0; icut<v_cut_name.size(); icut++)
				{
					if(v_cut_def[icut] == "") {continue;}

					//Categories are encoded into Char_t. Convert them to float for code automation
					if(v_cut_name[icut].Contains("is_") ) {v_cut_float[icut] = (float) v_cut_char[icut];}
					// cout<<"Cut : name="<<v_cut_name[icut]<<" / def="<<v_cut_def[icut]<<" / value="<<v_cut_float[icut]<<" / pass ? "<<Is_Event_Passing_Cut(v_cut_def[icut], v_cut_float[icut])<<endl;
					if(!Is_Event_Passing_Cut(v_cut_def[icut], v_cut_float[icut])) {pass_all_cuts = false; break;}
				}
				if(!pass_all_cuts) {continue;}
	//--------------------------------------------

				//Get MVA value to make template
				double mva_value1 = -9, mva_value2 = -9;

                if(!makeHisto_inputVars)
                {
                    mva_value1 = reader->EvaluateMVA(MVA_method_name1);
                }
                else
                {

                }

				//Get relevant binning
				float xValue_tmp = -1;

				// cout<<"//--------------------------------------------"<<endl;
				// cout<<"xValue_tmp = "<<xValue_tmp<<endl;
				// cout<<"mva_value1 = "<<mva_value1<<endl;
				// cout<<"mva_value2 = "<<mva_value2<<endl;

				//-- Fill histos for all subcategories
				for(int ichan=0; ichan<channel_list.size(); ichan++)
				{
					for(int isyst=0; isyst<syst_list.size(); isyst++)
					{
						double weight_tmp = 0; //Fill histo with this weight ; manipulate differently depending on syst

						// cout<<"-- Channel "<<channel_list[ichan]<<" / syst "<<syst_list[isyst]<<endl;

						if(syst_list[isyst] != "")
						{
							weight_tmp = v_float_systWeights[isyst];
						}
						else {weight_tmp = weight;} //Nominal (no syst)

						// cout<<"v_float_systWeights[isyst] "<<v_float_systWeights[isyst]<<endl;

						if(isnan(weight_tmp) || isinf(weight_tmp))
						{
							cout<<BOLD(FRED("* Found event with syst. weight = "<<weight_tmp<<" ; remove it..."))<<endl;
							cout<<"(Channel "<<channel_list[ichan]<<" / syst "<<syst_list[isyst]<<")"<<endl;
							continue;
						}

						//Printout weight
                        // if(sample_list[isample] == "ZZ" && syst_list[isyst].Contains("PU") && channel_list[ichan] == "")
						// cout<<"v3_histo_chan_syst_var["<<channel_list[ichan]<<"]["<<syst_list[isyst]<<"]+= "<<weight_tmp<<" => Integral "<<v3_histo_chan_syst_var[ichan][isyst]->Integral()<<endl;

						if(makeHisto_inputVars)
						{
							for(int ivar=0; ivar<total_var_list.size(); ivar++)
							{
								//Special variables, adress already used for other purpose
								if(total_var_list[ivar] == "nMediumBJets") {total_var_floats[ivar] = nMediumBJets;}
								else if(total_var_list[ivar] == "lepCharge") {total_var_floats[ivar] = lepCharge;}
								else if(total_var_list[ivar] == "channel") {total_var_floats[ivar] = channel;}

								Fill_TH1F_UnderOverflow(v3_histo_chan_syst_var[ichan][isyst][ivar], total_var_floats[ivar], weight_tmp);
							}
						}
						else
						{
                            if(template_name == "2D") {v3_histo_chan_syst_var2D[ichan][isyst][0]->Fill(mva_value1, mva_value2, weight_tmp);}
                            else {Fill_TH1F_UnderOverflow(v3_histo_chan_syst_var[ichan][isyst][0], xValue_tmp, weight_tmp);}
						}
					} //syst loop

					if(channel_list[ichan] != "") {break;} //subcategories are orthogonal ; if already found, can break subcat. loop
				} //subcat/chan loop

			} //end TTree entries loop
//--------------------------------------------

// #    # #  ####  #####  ####     #    #   ##   #    # # #####
// #    # # #        #   #    #    ##  ##  #  #  ##   # # #    #
// ###### #  ####    #   #    #    # ## # #    # # #  # # #    #
// #    # #      #   #   #    #    #    # ###### #  # # # #####
// #    # # #    #   #   #    #    #    # #    # #   ## # #
// #    # #  ####    #    ####     #    # #    # #    # # #

			for(int ichan=0; ichan<channel_list.size(); ichan++)
			{
				for(int isyst=0; isyst<syst_list.size(); isyst++)
				{
					for(int ivar=0; ivar<total_var_list.size(); ivar++)
					{
						// cout<<"chan "<<channel_list[ichan]<<"syst "<<syst_list[isyst]<<endl;

						if(sample_list[isample] != "DATA" && sample_list[isample] != "Fakes" && sample_list[isample] != "QFlip")
						{
							//Luminosity rescaling
							if(template_name == "2D") {v3_histo_chan_syst_var2D[ichan][isyst][ivar]->Scale(luminosity_rescale);}
							else {v3_histo_chan_syst_var[ichan][isyst][ivar]->Scale(luminosity_rescale);}
						} //MC
					} //Var
				} //syst
			} //ichan


// #    # #####  # ##### ######
// #    # #    # #   #   #
// #    # #    # #   #   #####
// # ## # #####  #   #   #
// ##  ## #   #  #   #   #
// #    # #    # #   #   ######

			// --- Write histograms
			TString samplename = sample_list[isample];
			if(sample_list[isample] == "DATA") {samplename = "data_obs";}

			for(int ichan=0; ichan<channel_list.size(); ichan++)
			{
				// cout<<"channel "<<channel_list[ichan]<<endl;

				for(int isyst=0; isyst<syst_list.size(); isyst++)
				{
					// cout<<"isyst "<<isyst<<endl;

					for(int ivar=0; ivar<total_var_list.size(); ivar++)
					{
						TString output_histo_name;
						if(makeHisto_inputVars)
						{
							output_histo_name = total_var_list[ivar] + "_" + region;
							if(channel_list[ichan] != "") {output_histo_name+= "_" + channel_list[ichan];}
							output_histo_name+= "__" + samplename;
							if(syst_list[isyst] != "") {output_histo_name+= "__" + syst_list[isyst];}
							else if(systTree_list[itree] != "") {output_histo_name+= "__" + systTree_list[itree];}
						}
						else
						{
							output_histo_name = classifier_name + template_name + "__" + region;
							if(channel_list[ichan] != "") {output_histo_name+= "_" + channel_list[ichan];}
							output_histo_name+= "__" + samplename;
							if(syst_list[isyst] != "") {output_histo_name+= "__" + syst_list[isyst];}
							else if(systTree_list[itree] != "") {output_histo_name+= "__" + systTree_list[itree];}
						}

						file_output->cd();

						if(template_name == "2D") {v3_histo_chan_syst_var2D[ichan][isyst][ivar]->Write(output_histo_name);}
						else {v3_histo_chan_syst_var[ichan][isyst][ivar]->Write(output_histo_name);}

						delete v3_histo_chan_syst_var[ichan][isyst][ivar]; v3_histo_chan_syst_var[ichan][isyst][ivar] = NULL;
						if(template_name == "2D") {delete v3_histo_chan_syst_var[ichan][isyst][ivar]; v3_histo_chan_syst_var[ichan][isyst][ivar] = NULL;}
					} //var loop
				} //syst loop
			} //chan loop

			// cout<<"Done with "<<sample_list[isample]<<" sample"<<endl;

			tree->ResetBranchAddresses(); //Detach tree from local variables (safe)
			delete tree; tree = NULL;
		} //end tree loop

		file_input->Close(); file_input = NULL;
	} //end sample loop


 //  ####  #       ####   ####  ######
 // #    # #      #    # #      #
 // #      #      #    #  ####  #####
 // #      #      #    #      # #
 // #    # #      #    # #    # #
 //  ####  ######  ####   ####  ######

	cout<<endl<<FYEL("==> Created root file: ")<<file_output->GetName()<<endl;
	cout<<FYEL("containing the "<<classifier_name<<" templates as histograms for : all samples / all channels")<<endl<<endl;

	file_output->Close(); file_output = NULL;

    if(template_name == "2Dlin" || template_name == "2D")
	{
		delete reader1; reader1 = NULL;
		delete reader2; reader2 = NULL;
	}
    else {delete reader; reader = NULL;}

	return;
}
