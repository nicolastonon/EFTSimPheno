//by Nicolas Tonon (IPHC)

//--- LIST OF FUNCTIONS (for quick search) :
//--------------------------------------------
// Train_BDT

// Produce_Templates

// Merge_Templates_ByProcess()
// Modify_Template_Histograms()
// Copy_Templates_withCERNconventions()

// Draw_Templates

// Produce_KerasDNN_Histos_For_ROC

// Draw_ColZ_Templates

// Make_ScaleVariations_Histograms()
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
TopEFT_analysis::TopEFT_analysis(vector<TString> thesamplelist, vector<TString> thesamplegroups, vector<TString> thesystlist, vector<TString> thesystTreelist, vector<TString> thechannellist, vector<TString> thevarlist/*, vector<int> thecolorlist*/, vector<TString> set_v_cut_name, vector<TString> set_v_cut_def, vector<bool> set_v_cut_IsUsedForBDT, vector<TString> set_v_add_var_names, TString theplotextension, double lumi, bool show_pulls, TString region, TString signal_process, TString classifier_name, TString DNN_type)
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

	t_name = "Tree";

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

	/**
	 * NB : REMINDER -- CAN NOT SET TWICE THE ADDRESS OF A VARIABLE
	 * Since some variables are listed both in 'var_list_ttV' and 'var_list_ttbar', whenever the 2 lists are used at same time (e.g. BDT ttbar vs ttV),
	 * must make sure that address is set only to 1 float ; arbitrarily decide that it will be the float of the ttV-list !
	 */
	if(analysis_type == "FCNC")
	{
		//FCNC analysis : BDT-ttV variables
		for(int i=0; i<thevarlist_ttV.size(); i++) //TMVA vars
		{
			var_list_ttV.push_back(thevarlist_ttV[i]);
			var_list_floats_ttV.push_back(-999);

			//--> This 'default' list of inputs will not be used for templates
			//Still, called in the code e.g. for CR plots, etc --> Set it as the ttV input list!
			var_list.push_back(thevarlist_ttV[i]);
			var_list_floats.push_back(-999);

			for(int ivar=0; ivar<set_v_add_var_names.size(); ivar++)
			{
				if(set_v_add_var_names[ivar] == thevarlist_ttV[i])
				{
					cout<<FGRN("** Constructor")<<" : erased variable "<<set_v_add_var_names[ivar]<<" from vector set_v_add_var_names (possible conflict) !"<<endl;
					set_v_add_var_names.erase(set_v_add_var_names.begin() + ivar);
					ivar--; //modify index accordingly
				}
			}
		}
		//FCNC analysis : BDT-ttbar variables
		for(int i=0; i<thevarlist_ttbar.size(); i++) //TMVA vars
		{
			var_list_ttbar.push_back(thevarlist_ttbar[i]);
			var_list_floats_ttbar.push_back(-999);

			for(int ivar=0; ivar<set_v_add_var_names.size(); ivar++)
			{
				if(set_v_add_var_names[ivar] == thevarlist_ttbar[i])
				{
					cout<<FGRN("** Constructor")<<" : erased variable "<<set_v_add_var_names[ivar]<<" from vector set_v_add_var_names (possible conflict) !"<<endl;
					set_v_add_var_names.erase(set_v_add_var_names.begin() + ivar);
					ivar--; //modify index accordingly
				}
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

	cout<<endl<<endl<<BOLD(FBLU("[Region : "<<nLep_cat<<" / "<<region<<"]"))<<endl<<endl<<endl;

	//--------------------------------------------
	// #####  #####   ####  ##### ######  ####  ##### #  ####  #    #  ####
	// #    # #    # #    #   #   #      #    #   #   # #    # ##   # #
	// #    # #    # #    #   #   #####  #        #   # #    # # #  #  ####
	// #####  #####  #    #   #   #      #        #   # #    # #  # #      #
	// #      #   #  #    #   #   #      #    #   #   # #    # #   ## #    #
	// #      #    #  ####    #   ######  ####    #   #  ####  #    #  ####
	//--------------------------------------------

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

/*
* Train, test and evaluate the BDT with signal and bkg MC
* (Use a DataLoader ; seems that it is required in newer root versions)
* @param channel
* @param type           [ttbar or ttV or ttH]
* @param write_ranking_info [Save variable ranking info in file or not (must set to false if optimizing BDT variable list)]
 */
void TopEFT_analysis::Train_BDT(TString channel, TString type, bool write_ranking_info)
{
	//--- Options
	bool use_relative_weights = true; //if false, will use fabs(weight)
	bool use_LO_samples = true; //if true, use LO madgraph samples for training
    bool use_list_specificSamples = true; //Use a list of training samples hard-coded within this function
	// bool use_different_regions_TrainTest = false; //NEW, idea is to train using "training sel" but to test on "SR/CR sel". Pb1 : SR is a subset of training sel, so not correct. Pb2 : no ttbar stat in SR

    bool train_onSMcoupling = false; //true <-> train on tHq-SM instead of tHq-ITC //PROBLEM : DOES NOT WORK ! KEEP TO FALSE

	//--- TMVA functions -- Very long ! NB : not implemented for DNN !
	bool do_HPO = false; //Perform HyperParameter Optimisation
	bool do_VI = false; //Assess input variables importances
	bool do_XVAL = false; //Perform cross-validation
	//NB : there is also the function "TMVA::Factory::OptimizeAllMethodsForClassification"...

//--------------------------------------------
	cout<<endl<<BOLD(FYEL("##################################"))<<endl;
	cout<<FYEL("--- TRAINING ---")<<endl;
	cout<<BOLD(FYEL("##################################"))<<endl<<endl;

	if(type == "ttbar"){ cout<<endl<<"-- NB : USE ONLY "<<BOLD(FGRN(""<<signal_process<<" / TTbar "))<<"events"<<endl;}
	else if(type == "ttV") {cout<<endl<<"-- NB : USE ONLY "<<BOLD(FGRN(""<<signal_process<<" / ttZ / ttW"))<<" events"<<endl;}
	else if(type == "ttH") {cout<<endl<<"-- NB : USE ONLY "<<BOLD(FGRN(""<<signal_process<<" / ttH"))<<" events"<<endl;}
	else if(type == "2Dlin") {cout<<FRED("Error : must train ttV/ttbar BDTs separately first ! Run the training with 'ttV' or 'ttbar' as argument !")<<endl; return;}
	else if(type != "") {cout<<FRED("Error : wrong background name for training !")<<endl; return;}
	cout<<endl<<endl;

	if(use_relative_weights) {cout<<"-- Using "<<BOLD(FGRN("*RELATIVE weights*"))<<" --"<<endl<<endl<<endl;}
	if(use_list_specificSamples) {cout<<"-- Using "<<BOLD(FGRN("*specific traning sample -- defined in training function*"))<<" --"<<endl<<endl<<endl;}
    else if(use_LO_samples) {cout<<"-- Using "<<BOLD(FGRN("*LO samples*"))<<" --"<<endl<<endl<<endl;}
	else {cout<<"-- Using "<<BOLD(FGRN("*ABSOLUTE weights*"))<<" --"<<endl<<endl<<endl;}

	if(include_MEM_variables && type == "ttbar" && analysis_type == "tHq") {cout<<FBLU("Setting 'include_MEM_variables' to false ! (for ttV only)")<<endl; include_MEM_variables = false;}

	if(type == "" && train_with_trainingSel) {cout<<FBLU("Setting 'train_with_trainingSel' to false ! (for ttV only)")<<endl; train_with_trainingSel = false;}

	if(classifier_name == "DNN" && DNN_type != "TMVA") {MYDEBUG("Wrong arguments -- Trying to train a non-TMVA DNN within TMVA !");}

	mkdir("weights", 0777);
	mkdir("weights/BDT", 0777);
	mkdir("weights/DNN", 0777);
	mkdir("weights/DNN/TMVA", 0777);
	mkdir("weights/DNN/PyKeras", 0777);
	mkdir("weights/DNN/Keras", 0777);

	usleep(1000000); //Pause for 1s (in microsec)

    if(analysis_type == "ttH" && signal_process != "ttH")
    {
        cout<<endl<<endl<<BOLD(FRED("*** Using ttH ntuples but you chose "<<signal_process<<" as signal ! Are you sure ?"))<<endl<<endl<<endl;
        usleep(2000000);
    }
	else if(analysis_type == "tHq" && signal_process != "tHq" && !signal_process.Contains("VLQ"))
    {
        cout<<endl<<endl<<BOLD(FRED("*** Using tHq ntuples but you chose "<<signal_process<<" as signal ! Are you sure ?"))<<endl<<endl<<endl;
        usleep(2000000);
    }
	else if(analysis_type == "FCNC" && !signal_process.Contains("FCNC"))
    {
        cout<<endl<<endl<<BOLD(FRED("*** Using FCNC ntuples but you chose "<<signal_process<<" as signal ! Are you sure ?"))<<endl<<endl<<endl;
        usleep(2000000);
    }

//--------------------------------------------
// ##### #####    ##   # #    # # #    #  ####      ####    ##   #    # #####  #      ######  ####
//   #   #    #  #  #  # ##   # # ##   # #    #    #       #  #  ##  ## #    # #      #      #
//   #   #    # #    # # # #  # # # #  # #          ####  #    # # ## # #    # #      #####   ####
//   #   #####  ###### # #  # # # #  # # #  ###         # ###### #    # #####  #      #           #
//   #   #   #  #    # # #   ## # #   ## #    #    #    # #    # #    # #      #      #      #    #
//   #   #    # #    # # #    # # #    #  ####      ####  #    # #    # #      ###### ######  ####
//--------------------------------------------
    vector<TString> v_trainingSamples;
    if(use_list_specificSamples)
    {
        //Signal samples
		if(signal_process == "tHq")
		{
			v_trainingSamples.push_back("tHq");
		}
		else if(signal_process == "tWZ")
		{
			v_trainingSamples.push_back("tWZ");
		}
		else if(signal_process == "ttH")
		{
			v_trainingSamples.push_back("ttH");
			// v_trainingSamples.push_back("ttH_LO");
		}
		else if(signal_process == "ST_hut_FCNC")
		{
			v_trainingSamples.push_back("tH_ST_hut_FCNC");
		}
        else if(signal_process == "TT_hut_FCNC")
		{
			v_trainingSamples.push_back("tH_TT_hut_FCNC");
		}
		else if(signal_process == "ST_hct_FCNC")
		{
			v_trainingSamples.push_back("tH_ST_hct_FCNC");
		}
		else if(signal_process == "TT_hct_FCNC")
		{
			v_trainingSamples.push_back("tH_TT_hct_FCNC");
		}
		else if(signal_process == "ST_FCNC")
		{
			v_trainingSamples.push_back("tH_ST_hut_FCNC");
			v_trainingSamples.push_back("tH_ST_hct_FCNC");
		}
		else if(signal_process == "TT_FCNC")
		{
			v_trainingSamples.push_back("tH_TT_hut_FCNC");
			v_trainingSamples.push_back("tH_TT_hct_FCNC");
		}
		else if(signal_process.Contains("hut_FCNC"))
		{
			v_trainingSamples.push_back("tH_ST_hut_FCNC");
			v_trainingSamples.push_back("tH_TT_hut_FCNC");
		}
        else if(signal_process.Contains("hct_FCNC"))
		{
			v_trainingSamples.push_back("tH_ST_hct_FCNC");
			v_trainingSamples.push_back("tH_TT_hct_FCNC");
		}
        else if(signal_process.Contains("VLQ"))
		{
            cout<<BOLD(FRED("You should set the boolean 'use_list_specificSamples' to FALSE in the code ! You must select yourself in the main the VLQ sample to use for training ! Abort"))<<endl;
            return;
			// v_trainingSamples.push_back("VLQ_M600");
		}
		else {v_trainingSamples.push_back(signal_process);}

        //Background samples
        if(type == "ttV")
        {
            // v_trainingSamples.push_back("ttZ");
            v_trainingSamples.push_back("ttZ_LO");
            // v_trainingSamples.push_back("ttW_PSweights");
            v_trainingSamples.push_back("ttW_LO");
        }
        else if(type == "ttbar") // -- choose ttbar samples
        {
			if(nLep_cat == "2l")
			{
				v_trainingSamples.push_back("TTbar_DiLep_PSweights");
				v_trainingSamples.push_back("TTbar_SemiLep_PSweights");
				v_trainingSamples.push_back("TTbar_DiLep");
	            v_trainingSamples.push_back("TTbar_SemiLep");
	            v_trainingSamples.push_back("TTJets_DiLep_MLM");
				v_trainingSamples.push_back("TTJets_SemiLep_MLM");

				// if(analysis_type == "FCNC") {v_trainingSamples.push_back("TTJets");} //-- Should not use in tHq/ttH : too many negative weights (too long), and worse perf with abs. weights
			}
			else if(nLep_cat == "3l")
			{
				v_trainingSamples.push_back("TTbar_DiLep_PSweights");
	            v_trainingSamples.push_back("TTbar_DiLep");
	            v_trainingSamples.push_back("TTJets_DiLep_MLM");
	            v_trainingSamples.push_back("TTJets");
			}
        }
		else if(type == "") //Merge many bkgs
		{
			// if(nLep_cat == "2l")
			{
				v_trainingSamples.push_back("ttZ_LO");
				v_trainingSamples.push_back("ttW_LO");
				v_trainingSamples.push_back("ttH");
				v_trainingSamples.push_back("tZq");
				v_trainingSamples.push_back("WZ");

				// v_trainingSamples.push_back("TTbar_DiLep_PSweights");
				// v_trainingSamples.push_back("TTbar_SemiLep_PSweights");
				// v_trainingSamples.push_back("TTbar_DiLep");
	            // v_trainingSamples.push_back("TTbar_SemiLep");
	            // v_trainingSamples.push_back("TTJets_DiLep_MLM");
				// v_trainingSamples.push_back("TTJets_SemiLep_MLM");
			}
			// else if(nLep_cat == "3l")
			// {
			// 	v_trainingSamples.push_back("TTbar_DiLep_PSweights");
	        //     v_trainingSamples.push_back("TTbar_DiLep");
	        //     v_trainingSamples.push_back("TTJets_DiLep_MLM");
	        //     v_trainingSamples.push_back("TTJets");
			// }
		}
    }
    else {v_trainingSamples = sample_list;} //Else, run on the sample list defined in main, and guess which ones are used for training


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
	if(train_with_trainingSel) {cat_tmp = Get_Category_Boolean_Name(nLep_cat, "Training", analysis_type, "", scheme);}
	else {cat_tmp = Get_Category_Boolean_Name(nLep_cat, region, analysis_type, "", scheme);}

	//Even if ask templates in the SR, need to use training (looser) category for training !
	if(cat_tmp.Contains("_SR") )
	{
		int i = cat_tmp.Index("_SR"); //Find index of substring
		cat_tmp.Remove(i); //Remove substring
	}

	// if(nLep_cat == "2l") {cat_tmp = "is_tHqFCNC_ATLAS_2lSS_SR";}
	// else {cat_tmp = "is_tHqFCNC_ATLAS_3l_SR";}

    tmp+= cat_tmp + "==1";

    // if(!train_with_trainingSel || !use_different_regions_TrainTest) {tmp+= cat_tmp + "==1";} //Else, use different files for train/test rather than different categ.

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

	//Should no include 2l ee events for training ?
	// if(nLep_cat == "2l")
	// {
	// 	mycuts = "Channel==0 || Channel==1";
	// 	mycutb = mycuts;
	// }

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

	TString output_file_name = "outputs/" + classifier_name + type + "_" + nLep_cat;
	if(channel != "") {output_file_name+= "_" + channel;}
	// if(!train_with_trainingSel || use_different_regions_TrainTest) {output_file_name+= "_" + region;}
	if(classifier_name == "DNN") {output_file_name+= "_" + DNN_type;}
	output_file_name+= "__" + signal_process;
	output_file_name+= this->filename_suffix + ".root";

	TFile* output_file = TFile::Open( output_file_name, "RECREATE" );

	// Create the factory object
	// TMVA::Factory* factory = new TMVA::Factory(type.Data(), output_file, "!V:!Silent:Color:DrawProgressBar:AnalysisType=Classification" );
	TString weights_dir = "weights";
	if(use_2016_ntuples) {weights_dir+= "/2016";}
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
	if(analysis_type == "FCNC") //use specific vectors for FCNC inputs
	{
		if(type == "ttV")
		{
			for(int i=0; i<var_list_ttV.size(); i++)
			{
				dataloader->AddVariable(var_list_ttV[i].Data(), 'F');
			}
		}
		else if(type == "ttbar")
		{
			for(int i=0; i<var_list_ttbar.size(); i++)
			{
				dataloader->AddVariable(var_list_ttbar[i].Data(), 'F');
			}
		}
		else {cout<<"Error : wrong type !"<<endl; return;}
	}
	else //use same input vectors as usual
	{
		for(int i=0; i<var_list.size(); i++)
		{
			dataloader->AddVariable(var_list[i].Data(), 'F');
		}
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
	for(int isample=0; isample<v_trainingSamples.size(); isample++)
    {
		cout<<"-- Sample : "<<v_trainingSamples[isample]<<endl;

		//Make sure the ttW & ttZ MadgraphMLM samples are used for training
		TString samplename_tmp = v_trainingSamples[isample];

		if(type == "ttbar")
		{
			if(!samplename_tmp.Contains(signal_process) && !samplename_tmp.Contains("TTbar") && !samplename_tmp.Contains("TTJet") ) {continue;}

			// if(use_3l_events && samplename_tmp.Contains("Semi") ) {continue;} //only use ttbar_dilep in 3l region
		}
		else if(type == "ttV")
		{
            if(!samplename_tmp.Contains(signal_process) && !samplename_tmp.Contains("ttZ") && !samplename_tmp.Contains("ttW") ) {continue;}
		}
		else if(type == "ttH")
		{
			if(!samplename_tmp.Contains(signal_process) && !samplename_tmp.Contains("ttH") ) {continue;}
		}

		//-- Naming conventions
		if(use_2016_ntuples)
		{
			if(samplename_tmp == "ttZ") {samplename_tmp = "ttZ_MadgraphMLM";}
			else if(samplename_tmp == "ttW") {samplename_tmp = "ttW_MadgraphMLM";}
		}
		else if(use_LO_samples) //-- Use LO madgraph samples
		{
			//Replace ttV & ttH samples by Madgraph LO samples for training (no neg weights, ...)
			if(samplename_tmp == "ttZ") {samplename_tmp = "ttZ_LO";}
			else if(samplename_tmp.Contains("ttW") ) {samplename_tmp = "ttW_LO";}
			// else if(samplename_tmp == "ttH") {samplename_tmp = "ttH_LO";} //missing for now

			//Replace ttbar samples by TTJets.root
			// else if(samplename_tmp.Contains("TTbar_DiLep") ) {samplename_tmp = "TTJets_DiLep_MLM";}
			// else if(samplename_tmp.Contains("TTbar_Semi") ) {samplename_tmp = "TTJets_SemiLep_MLM";}
		}


        // --- Register the training and test trees
        TString inputfile = "";
        TString inputfile_train = "";
        TString inputfile_test = "";

		if(train_with_trainingSel) //Training selection
        {
            // inputfile = "./input_ntuples/FCNC/"+nLep_cat+"/SR_ATLAS/"+samplename_tmp+".root";

            inputfile = dir_ntuples_noRegion + samplename_tmp + ".root";

            // if(!use_different_regions_TrainTest) {inputfile = dir_ntuples_noRegion + samplename_tmp + ".root";}
            // else {inputfile_train = dir_ntuples_noRegion + samplename_tmp + ".root"; inputfile_test = dir_ntuples + v_trainingSamples[isample] + ".root";}
        }
		else {inputfile = dir_ntuples + samplename_tmp + ".root";} //SR/CR selection

		if(include_MEM_variables)
		{
			bool ntuples_withMEM = false; //Hardcoded -- find samples from first MEM run

			TString sample_tmp = v_trainingSamples[isample];

//For old MEM ntuples (first MEM run) only -- OBSOLETE
//--------------------------------------------
			if(ntuples_withMEM)
			{
				//Naming convention
				if(v_trainingSamples[isample].Contains("ttZ")) {sample_tmp = "ttZ";}
				if(v_trainingSamples[isample].Contains("ttH")) {sample_tmp = "ttH";}
				if(v_trainingSamples[isample].Contains("ttW")) {sample_tmp = "ttW";}
				if(v_trainingSamples[isample].Contains("tHq")) {sample_tmp = "tHq";}
				if(v_trainingSamples[isample].Contains("TTbar_DiLep")) {sample_tmp = "ttbar_dilep";}
				if(v_trainingSamples[isample].Contains("TTbar_SemiLep")) {sample_tmp = "ttbar_semilep";}
				if(v_trainingSamples[isample].Contains("TTJets_DiLep")) {sample_tmp = "ttjet_dilep";}
				if(v_trainingSamples[isample].Contains("TTJets_SemiLep")) {sample_tmp = "ttjet_semilep";}

				inputfile = "./MEM/ntuples/afterMEM/output_"+sample_tmp+ + "_"+nLep_cat+"_Training2land3l_all.root"; //Different ntuples for MEM
			}
			else {inputfile = "./MEM/ntuples/beforeMEM/"+nLep_cat+"/"+sample_tmp+".root";}
//--------------------------------------------

			//Replaced by new ntuples
			if(analysis_type == "tHq")
			{
				inputfile = "./input_ntuples/tHq2017/MEM/3l/"+sample_tmp+".root";
			}
			else if(analysis_type == "FCNC")
			{
				inputfile = "./input_ntuples/FCNC/MEM/3l/"+sample_tmp+".root";
			}

			cout<<"inputfile = "<<inputfile<<endl;
		}

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

        // if(train_with_trainingSel && use_different_regions_TrainTest)
        // {
        //     file_input_train = TFile::Open(inputfile_train);
    	// 	if(!file_input_train) {cout<<BOLD(FRED(<<inputfile_train<<" not found!"))<<endl; continue;}
    	// 	files_to_close.push_back(file_input_train);
        //     tree_train = (TTree*) file_input_train->Get(t_name);
    	// 	if(tree_train==0) {cout<<BOLD(FRED("ERROR :"))<<" file "<<inputfile_train<<" --> *tree = 0 !"<<endl; continue;}
    	// 	else {cout<<endl<<FMAG("=== Opened file : ")<<inputfile_train<<endl<<endl;}
        //
        //     file_input_test = TFile::Open(inputfile_test);
    	// 	if(!file_input_test) {cout<<BOLD(FRED(<<inputfile_test<<" not found!"))<<endl; continue;}
    	// 	files_to_close.push_back(file_input_test);
        //     tree_test = (TTree*) file_input_test->Get(t_name);
    	// 	if(tree_test==0) {cout<<BOLD(FRED("ERROR :"))<<" file "<<inputfile_test<<" --> *tree = 0 !"<<endl; continue;}
    	// 	else {cout<<endl<<FMAG("=== Opened file : ")<<inputfile_test<<endl<<endl;}
        // }
        // else
        {
            file_input = TFile::Open(inputfile);
            if(!file_input) {cout<<BOLD(FRED(<<inputfile<<" not found!"))<<endl; continue;}
            files_to_close.push_back(file_input);
            tree = (TTree*) file_input->Get(t_name);
            if(tree==0) {cout<<BOLD(FRED("ERROR :"))<<" file "<<inputfile<<" --> *tree = 0 !"<<endl; continue;}
            else {cout<<endl<<FMAG("=== Opened file : ")<<inputfile<<endl<<endl;}
        }

        // global event weights per tree (see below for setting event-wise weights)
		//NB : in tHq2016, different global weights were attributed to each sample !
		// ( see : https://github.com/stiegerb/cmgtools-lite/blob/80X_M17_tHqJan30/TTHAnalysis/python/plotter/tHq-multilepton/signal_mva/trainTHQMVA.py#L78-L98)
        Double_t signalWeight     = 1.0;
        Double_t backgroundWeight = 1.0;

    //-- Choose between absolute/relative weights for training
		if(samplename_tmp.Contains(signal_process) )
		{
            // if(train_with_trainingSel && use_different_regions_TrainTest)
            // {
            //     dataloader->AddTree(tree_train, "Signal", 1.0, mycuts, "train");
            //     dataloader->AddTree(tree_test, "Signal", 1.0, mycutb, "test");
            // }
            // else
            {
                nEvents_sig+= tree->GetEntries(mycuts); dataloader->AddSignalTree(tree, signalWeight);
            }

			if(use_relative_weights)
			{
				TString weightExp = "weight";
				if(train_onSMcoupling && signal_process.Contains("tHq")) {weightExp = "weight * SMcoupling_SF";}

				dataloader->SetSignalWeightExpression(weightExp);
				cout<<"Signal sample : "<<samplename_tmp<<" / Weight expression : "<<weightExp<<endl;
			}
			else
			{
				TString weightExp = "fabs(weight)";
				if(train_onSMcoupling && signal_process.Contains("tHq")) {weightExp = "fabs(weight * SMcoupling_SF)";}

				dataloader->SetSignalWeightExpression(weightExp);
				cout<<"Signal sample : "<<samplename_tmp<<" / Weight expression : "<<weightExp<<endl;
			}
		}
		else
		{
            // if(train_with_trainingSel && use_different_regions_TrainTest)
            // {
            //     dataloader->AddTree(tree_train, "Background", 1.0, mycuts, "train");
            //     dataloader->AddTree(tree_test, "Background", 1.0, mycutb, "test");
            // }
            // else
            {
                nEvents_bkg+= tree->GetEntries(mycutb); dataloader->AddBackgroundTree(tree, backgroundWeight);
            }

            if(use_relative_weights)
			{
				TString weightExp = "weight";
				dataloader->SetBackgroundWeightExpression(weightExp);
				cout<<"Background sample : "<<samplename_tmp<<" / Weight expression : "<<weightExp<<endl;
			}
			else
			{
				TString weightExp = "fabs(weight)";
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

	float trainingEv_proportion = 0.5;
	// float trainingEv_proportion = 0.7;

	//-- Choose dataset splitting
	TString nTraining_Events_sig = "", nTraining_Events_bkg = "", nTesting_Events_sig = "", nTesting_Events_bkg = "";

	// if(!use_different_regions_TrainTest) //--- ttH2017
	{
        int nmaxEv = 150000; //max nof events for train or test
        int nTrainEvSig = (nEvents_sig * trainingEv_proportion < nmaxEv) ? nEvents_sig * trainingEv_proportion : nmaxEv;
        int nTrainEvBkg = (nEvents_bkg * trainingEv_proportion < nmaxEv) ? nEvents_bkg * trainingEv_proportion : nmaxEv;
        int nTestEvSig = (nEvents_sig * (1-trainingEv_proportion) < nmaxEv) ? nEvents_sig * (1-trainingEv_proportion) : nmaxEv;
        int nTestEvBkg = (nEvents_bkg * (1-trainingEv_proportion) < nmaxEv) ? nEvents_bkg * (1-trainingEv_proportion) : nmaxEv;

		nTraining_Events_sig = Convert_Number_To_TString(nTrainEvSig, 12);
        nTraining_Events_bkg = Convert_Number_To_TString(nTrainEvBkg, 12);
		nTesting_Events_sig = Convert_Number_To_TString(nTestEvSig, 12);
        nTesting_Events_bkg = Convert_Number_To_TString(nTestEvBkg, 12);
	}
	// else //Split in halves
	// {
		// nTraining_Events_sig = "0", nTraining_Events_bkg = "0";
		// nTesting_Events_sig = "0", nTesting_Events_bkg = "0";
	// }

	cout<<endl<<endl<<FBLU("===================================")<<endl;
	cout<<FBLU("-- Requesting "<<nTraining_Events_sig<<" Training events [SIGNAL]")<<endl;
	cout<<FBLU("-- Requesting "<<nTesting_Events_sig<<" Testing events [SIGNAL]")<<endl;
	cout<<FBLU("-- Requesting "<<nTraining_Events_bkg<<" Training events [BACKGROUND]")<<endl;
	cout<<FBLU("-- Requesting "<<nTesting_Events_bkg<<" Testing events [BACKGROUND]")<<endl;
	cout<<FBLU("===================================")<<endl<<endl<<endl;


	// For these particular tests, need to reduce the stat (CPU-intensive)
	// HPO with 1k events : ~ 5min
	if(do_HPO || do_VI)
	{
		dataloader->PrepareTrainingAndTestTree(mycuts, mycutb, "nTrain_Signal=1000:nTrain_Background=1000:nTest_Signal=1000:nTest_Background=1000:SplitMode=Random:NormMode=NumEvents:!V");
	}
	else if(do_XVAL)
	{
		dataloader->PrepareTrainingAndTestTree(mycuts, mycutb, "SplitMode=Random:NormMode=NumEvents:V");
		// dataloader->PrepareTrainingAndTestTree(mycuts, mycutb, "nTrain_Signal=10:nTrain_Background=10:SplitMode=Random:NormMode=NumEvents:V"); //For quick xcheck

	}
	else //Use full stat. //CHANGED : removed 'NormMode=NumEvents'
	{
		dataloader->PrepareTrainingAndTestTree(mycuts, mycutb, "nTrain_Signal="+nTraining_Events_sig+":nTrain_Background="+nTraining_Events_bkg+":nTest_Signal="+nTesting_Events_sig+":nTest_Background="+nTesting_Events_bkg+":SplitMode=Random:!V");
		// dataloader->PrepareTrainingAndTestTree(mycuts, mycutb, "nTrain_Signal=0:nTrain_Background=0:nTest_Signal=0:nTest_Background=0:SplitMode=Random:NormMode=NumEvents:!V");
	}

	//-- for quick testing
	// dataloader->PrepareTrainingAndTestTree(mycuts, mycutb, "nTrain_Signal=10:nTrain_Background=10:nTest_Signal=10:nTest_Background=10:SplitMode=Random:NormMode=NumEvents:!V");


	//--- Boosted Decision Trees -- Choose method
	TMVA::Factory *factory = new TMVA::Factory(classifier_name, output_file, "V:!Silent:Color:DrawProgressBar:Correlations=True:AnalysisType=Classification");

	// TString method_title = channel + this->filename_suffix; //So that the output weights are labelled differently for each channel
	TString method_title = type + "_" + nLep_cat; //So that the output weights are labelled differently for each channel
	if(channel != "") {method_title += "_" + channel;}
	else {method_title += "_all";}
	if(!train_with_trainingSel) {method_title+= "_" + region;}
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
TString method_options_1 = "";
TString method_options_2 = "";
TString method_options_3 = "";

//---- REFERENCES :
//tHq2016
// method_options = "!H:!V:NTrees=800:nCuts=50:MaxDepth=3:BoostType=Grad:Shrinkage=0.10:!UseBaggedGrad:NegWeightTreatment=PairNegWeightsGlobal:CreateMVAPdfs";

//ttH2017
// method_options= "!H:!V:NTrees=200:BoostType=Grad:Shrinkage=0.10:!UseBaggedGrad:nCuts=200:nEventsMin=100:NNodesMax=5:MaxDepth=8:NegWeightTreatment=PairNegWeightsGlobal:CreateMVAPdfs:DoBoostMonitor=True";

//tZq2017
// method_options = "!H:!V:NTrees=1000:nCuts=200:MaxDepth=4:MinNodeSize=5%:UseBaggedGrad=True:BaggedSampleFraction=0.5:BoostType=Grad:Shrinkage=0.10:!UseBaggedGrad:NegWeightTreatment=PairNegWeightsGlobal:CreateMVAPdfs";

	if(analysis_type == "ttH") //--- ttH2017
	{
		method_options= "!H:!V:NTrees=200:BoostType=Grad:Shrinkage=0.10:!UseBaggedGrad:nCuts=200:nEventsMin=100:NNodesMax=5:MaxDepth=8:NegWeightTreatment=PairNegWeightsGlobal:CreateMVAPdfs:DoBoostMonitor=True";
	}
	else if(analysis_type == "tHq")
	{
		if(classifier_name == "BDT") //--- AN tHq 2016
		{
			//tHq2017
            method_options = "!H:!V:NTrees=200:nCuts=40:MaxDepth=4:BoostType=Grad:Shrinkage=0.10:!UseBaggedGrad:NegWeightTreatment=PairNegWeightsGlobal:CreateMVAPdfs";

			//For method TESTING
			// method_options = "!H:!V:NTrees=400:nCuts=20:MaxDepth=3:BoostType=Grad:Shrinkage=0.05:!UseBaggedGrad:NegWeightTreatment=PairNegWeightsGlobal:CreateMVAPdfs";
		}
	}
	else if(analysis_type == "FCNC")
	{
		//tHq2017 //-- USE NEGWEIGHTS
		method_options = "!H:!V:NTrees=200:nCuts=40:MaxDepth=4:BoostType=Grad:Shrinkage=0.10:!UseBaggedGrad:NegWeightTreatment=PairNegWeightsGlobal:CreateMVAPdfs";

		// method_options = "!H:!V:NTrees=200:nCuts=40:MaxDepth=4:BoostType=Grad:Shrinkage=0.10:!UseBaggedGrad:NegWeightTreatment=IgnoreNegWeightsInTraining:CreateMVAPdfs";

		//--------------------------------------------
		// method_options = "!H:!V:NTrees=2"; //-- QUICK TESTING OF INPUTS
	}

	cout<<endl<<endl<<FBLU("[Method options : "<<method_options<<"]")<<endl<<endl;

	TMVA::HyperParameterOptimisation* hpo = NULL;
	TMVA::VariableImportance* vi = NULL;
	TMVA::CrossValidation* cv = NULL; //REMOVABLE
	TStopwatch tw;
	TCanvas* c1 = NULL;
	TMultiGraph* graph = NULL;

//--------------------------------------------
//  ####  #####  ##### # #    #        #####    ##   #####
// #    # #    #   #   # ##  ##        #    #  #  #  #    #
// #    # #    #   #   # # ## #        #    # #    # #    #
// #    # #####    #   # #    # ###    #####  ###### #####  ###
// #    # #        #   # #    # ###    #      #    # #   #  ###
//  ####  #        #   # #    # ###    #      #    # #    # ###
//--------------------------------------------
	/**
	 * -- Run HyperParameter Optimisation algorithm from TMVA (for BDT/SVM only)
	 * Runs OptimiseTuningParameters for each combination of folds
	 * Returns 1 set of HP per fold
	 * 'Fold' = sub-dataset
	 */
	if(do_HPO == true)
	{
		cout<<endl<<endl<<FYEL("===== Optimising MVA Hyper Parameters")<< endl;
		hpo = new TMVA::HyperParameterOptimisation(dataloader);

		hpo->BookMethod(TMVA::Types::kBDT, method_title, method_options);

		hpo->SetNumFolds(3);
		// hpo->SetFitter("Scan");
		hpo->SetFitter("Minuit");
		hpo->SetFOMType("Separation");

		cout<<endl;
		tw.Start();
		hpo->Evaluate();
		tw.Print();
		cout<<endl;


		TMVA::HyperParameterOptimisationResult& results = (TMVA::HyperParameterOptimisationResult&) hpo->GetResults();
		results.Print();

		// for(auto& opt : HPOResult.fFoldParameters.at(0))
		// {
		// 	optionsString += ":";
		// 	optionsString += opt.first;
		// 	optionsString += "=";
		// 	optionsString += opt.second;
		// }

		graph = results.GetROCCurves();
		c1 = new TCanvas("c1","c1", 1000, 800);
		graph->Draw();
		c1->SaveAs("BDT_hpo.png");
	}


//--------------------------------------------
// #    #   ##   #####         # #    # #####
// #    #  #  #  #    #        # ##  ## #    #
// #    # #    # #    #        # # ## # #    #
// #    # ###### #####  ###    # #    # #####  ###
//  #  #  #    # #   #  ###    # #    # #      ###
//   ##   #    # #    # ###    # #    # #      ###
//--------------------------------------------

	/**
	 * Run the TMVA algorithm to assess the relative importance of each variable
	 *
	 */
	else if(do_VI == true)
	{
		cout<<endl<<endl<<FYEL("===== Assessing Input variables Importances")<< endl;

		vi = new TMVA::VariableImportance(dataloader);
		vi->BookMethod(TMVA::Types::kBDT, method_title, method_options);

		// vi->SetType(TMVA::kRandom); //?
		vi->SetType(TMVA::kShort); //Very fast, but can be trusted ? Weird results

		cout<<endl;
		tw.Start();
		vi->Evaluate();
		tw.Print();
		cout<<endl;

		c1 = new TCanvas("c1","c1", 1000, 800);
		TMVA::VariableImportanceResult& results = (TMVA::VariableImportanceResult&) vi->GetResults();
		results.Print();
		results.Draw();
		c1->SaveAs("BDT_vi.png");
	}

//--------------------------------------------
// #    #       #    #   ##   #
//  #  #        #    #  #  #  #
//   ##   ##### #    # #    # #
//   ##         #    # ###### #
//  #  #         #  #  #    # #
// #    #         ##   #    # ######
//--------------------------------------------
	/**
	 * --- Perform x-validation of the BDT
	 * Several metrics available (ROC, separation, signif, ...)
	 * NB : "2-fold cross-validation" <-> dataset split in 2. First use f1 for train and f2 for test, then invert and average results
	 */

	else if(do_XVAL == true) //REMOVABLE
	{
		cout<<endl<<endl<<FYEL("===== Performing Cross-Validation")<< endl;

		// ModelPersistence = to save the trained model in xml or serialized files
		cv = new TMVA::CrossValidation("TMVACrossValidation",dataloader,output_file,"V:!Silent:ModelPersistence:AnalysisType=Classification:NumFolds=5");

		cv->BookMethod(TMVA::Types::kBDT, method_title, method_options);

		// cv->SetNumFolds(3);

		cout<<endl;
		tw.Start();
		cv->Evaluate();
		tw.Print();
		cout<<endl;

		// TMVA::CrossValidationResult& cvresult = (TMVA::CrossValidationResult&) cv->GetResults()[0];
		TMVA::CrossValidationResult cvresult = cv->GetResults()[0];
		cvresult.Print();
		c1 = cvresult.Draw();
		c1->SaveAs("BDT_xval.png"); //Need to delete c1 ?

		// cout<<endl<<endl<<BOLD("==> Average ROC Integral = " << cvresult.GetROCAverage() << " +/- " << cvresult.GetROCStandardDeviation()/sqrt(cv->GetNumFolds())<<"")<<endl;
		cout<<endl<<endl<<BOLD("==> Average ROC Integral = " << cvresult.GetROCAverage() << " +/- " <<cvresult.GetROCStandardDeviation()<<"")<<endl;
	}



//--------------------------------------------
 // ##### #####    ##   # #    #       ##### ######  ####  #####       ###### #    #   ##   #
 //   #   #    #  #  #  # ##   #         #   #      #        #         #      #    #  #  #  #
 //   #   #    # #    # # # #  #         #   #####   ####    #         #####  #    # #    # #
 //   #   #####  ###### # #  # #         #   #           #   #         #      #    # ###### #
 //   #   #   #  #    # # #   ##         #   #      #    #   #         #       #  #  #    # #
 //   #   #    # #    # # #    #         #   ######  ####    #         ######   ##   #    # ######
//--------------------------------------------

	if(!do_HPO && !do_VI && !do_XVAL)
	{
		if(classifier_name == "BDT") {factory->BookMethod(dataloader, TMVA::Types::kBDT, method_title, method_options);} //Book BDT
		else if(DNN_type == "TMVA")
		{
			factory->BookMethod(dataloader, TMVA::Types::kDNN, method_title, method_options);

			// factory->BookMethod(dataloader, TMVA::Types::kDNN, "DNN 1", method_options_1);
			// factory->BookMethod(dataloader, TMVA::Types::kDNN, "DNN 2", method_options_2);
			// factory->BookMethod(dataloader, TMVA::Types::kDNN, "DNN 3", method_options_3);
		} //Book TMVA's DNN

		output_file->cd();

		mkdir("outputs/Rankings", 0777); //Dir. containing variable ranking infos
		mkdir("outputs/ROCS", 0777); //Dir. containing variable ranking infos

		TString ranking_file_path = "outputs/Rankings/rank_"+classifier_name+type;
		ranking_file_path+= "_" + nLep_cat;
		if(!train_with_trainingSel) {ranking_file_path+= "_" + region;}
		ranking_file_path+= ".txt";

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
			Extract_Ranking_Info(ranking_file_path, type, channel); //Extract only ranking info from TMVA output
		}
		else {system("rm ./ranking_info_tmp.txt");} //Else remove the temporary ranking file
	}

	for(unsigned int i=0; i<files_to_close.size(); i++) {files_to_close[i]->Close(); delete files_to_close[i];}

	if(do_HPO || do_VI || do_XVAL) {delete c1; c1 = NULL;}
	if(do_HPO) {delete hpo; hpo = NULL;}
	if(do_VI) {delete vi, vi = NULL;}
	if(do_VI) {delete cv; cv = NULL;} //REMOVABLE

	delete dataloader; dataloader = NULL; //Also deletes hpo/vi/xval objects ?
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





/////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////

/**
 * NEW function to produce templates, adapted for ttH-tH analysis (e.g. need to produce templates in many subcategories, with large number of systematics, etc.)
 * Improved the computing-time, logic, adaptability
 * NB : Do not support DNNs anymore. Could easily re-implement TMVA/pyKeras DNNs, same as done with BDT ; for Keras DNNs, should look what was done in the past, and make cleaner implementation
 * NB : 2D not supported anymore (would be useful to visualize 2D distribution, optimize binning, etc.). Could re-implement within this function, or in a new dedicated/simplified function
 *
 * @param template_name  'ttbar' / 'ttV' / '2Dlin' (ttbar vs ttV, rebinned into 1D => used for limits) / 'categ' (yields per subcategory)
 */
void TopEFT_analysis::Produce_Templates(TString template_name, bool makeHisto_inputVars, int bin_choice)
{
//--------------------------------------------
	bool read_preComputed_MVA = false; //true <-> read MVA variable computed at NTAnalyzer level, instead of reading BDT xml file

	bool use_rebinning = true; //true <-> use 2D->1D binning as defined in ttH2017/tHq2016 analysis
	bool use_binning_compPallabi = false;

	bool force_normTemplate_positive = true; //true <-> if histo's norm is <=0, set to >~0 (flat) to avoid crash in Combine //NB : necessary ?

	bool rescale_ITC_to_SM_xsec = false; //true <-> rescale ITC templates to SM xsec (for combine models)

	bool copy_templates_forCERN = false; //true -> copy output file in new file following CERN's naming conventions
		bool copy_forAllCouplings = false; //true -> copy default file to serve as skeleton for all kT/kV files

	bool writeTemplate_forAllCouplingPoints = false; //true <-> for each coupling point in member vector, will reproduce signal samples and overwrite templates in dedicated template rootfile (must have been created beforehand with call to 'Copy_TemplateFile_allCouplings()')

	bool unblind_lowBDT = false; //true <-> in FCNC, only look at events below some BDT treshold ; different output name

//HARDCODED FOR NOW IN THQ ANALYSIS : USE PALLABI'S OLD MVA FOR 2LSS (closer to what she has)
//--------------------------------------------
	if(analysis_type == "tHq" && !makeHisto_inputVars)
	{
		if(nLep_cat == "3l" && read_preComputed_MVA)
		{
			cout<<"Use my own templates in 3l region for tHq analysis. Setting 'read_preComputed_MVA' to false !"<<endl;
			read_preComputed_MVA = false;
		}
		if(nLep_cat == "2l" && !read_preComputed_MVA)
		{
			cout<<"Using Pallabi's old MVA in 2lSS region for tHq analysis. Setting 'read_preComputed_MVA' to true !"<<endl;
			read_preComputed_MVA = true;
		}
	}

	if(read_preComputed_MVA && analysis_type == "FCNC") {cout<<"No pre-computed MVA for FCNC analysis ! Setting 'read_preComputed_MVA' to false !"<<endl; read_preComputed_MVA = false;}

//--------------------------------------------
	cout<<endl<<BOLD(FYEL("##################################"))<<endl;
	if(makeHisto_inputVars) {cout<<FYEL("--- Producing Input variables histograms ---")<<endl;}
	else if(template_name == "ttbar" || template_name == "ttV" || template_name == "2Dlin" || template_name == "categ" || template_name == "2D") {cout<<FYEL("--- Producing "<<template_name<<" Templates ---")<<endl;}
	else {cout<<BOLD(FRED("--- ERROR : invalid arguments ! Exit !"))<<endl; cout<<"Valid template names are : ttbar / ttV / 2D / 2Dlin !"<<endl; return;}
	cout<<BOLD(FYEL("##################################"))<<endl<<endl;

	if(classifier_name != "BDT") {cout<<BOLD(FRED("Error : DNNs are not supported anymore !"))<<endl; return;}

	//Don't make systematics shifted histos for input vars (too long)
	if(makeHisto_inputVars)
	{
		classifier_name = ""; //For naming conventions
		// syst_list.resize(1); syst_list[0] = ""; //Force Remove systematics
	}

	if(use_binning_compPallabi && (analysis_type != "tHq" || !use_rebinning)) {use_binning_compPallabi = false;}

	if(writeTemplate_forAllCouplingPoints)
	{
		if(makeHisto_inputVars) {cout<<"Can not set 'writeTemplate_forAllCouplingPoints' and 'makeHisto_inputVars' both to true ! Abort"<<endl; return;}
		if(copy_templates_forCERN) {cout<<"Can not set 'writeTemplate_forAllCouplingPoints' and 'Copy_Templates_withCERNconventions' both to true ! Abort"<<endl; return;}
		// if(!use_SM_coupling) {cout<<"If 'writeTemplate_forAllCouplingPoints' is true, must set 'use_SM_coupling' to true ! Setting it to true..."<<endl; use_SM_coupling = true;} //
	}

	if(template_name == "categ") {classifier_name = ""; syst_list.resize(1); syst_list[0] = "";} //For naming conventions //Remove systematics
	else if(template_name == "2D") {syst_list.resize(1); syst_list[0] = "";} //Don't process systematics for 2D

	if(include_MEM_variables && analysis_type == "tHq" &&( template_name == "ttbar" || nLep_cat == "2l")) {cout<<"NB : in tHq analysis, MEM variables are only available in 3l for ttbar BDT ! Setting 'include_MEM_variables' to false !"<<endl; include_MEM_variables = false;}

	if(!makeHisto_inputVars)
	{
		if(read_preComputed_MVA) {cout<<BOLD(FMAG("-- Using pre-computed MVA distribution (from ttH2017 or Pallabi) !"))<<endl;}
		if(use_rebinning && template_name == "2Dlin") {cout<<BOLD(FMAG("-- Using 2D->1D rebinning !"))<<endl;}
		usleep(1000000); //Pause
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
	if(use_3l_events) output_file_name+= "_3l";
	else output_file_name+= "_2l";
	output_file_name+= "_" + region + filename_suffix;
    if(unblind_lowBDT) {output_file_name+= "_lowBDT";}
    output_file_name+= ".root";

	//-- For input vars
	if(makeHisto_inputVars)
	{
		output_file_name = "outputs/ControlHistograms";
		if(use_3l_events) output_file_name+= "_3l";
		else output_file_name+= "_2l";
		output_file_name+= "_" + region + filename_suffix +".root";
	}

	//Create output file
	TFile* file_output = 0;
	if(!writeTemplate_forAllCouplingPoints)
	{
		if(bin_choice != -1)
		{
			mkdir("./outputs/tests/", 0777);

			output_file_name = "outputs/tests/Templates_" + classifier_name + template_name;
			if(use_3l_events) output_file_name+= "_3l";
			else output_file_name+= "_2l";
			output_file_name+= "_" + region + filename_suffix +"_bin"+Convert_Number_To_TString(bin_choice)+".root";
		}
		file_output = TFile::Open(output_file_name, "RECREATE");
	}
	else {file_output = TFile::Open(output_file_name, "READ");} //Don't really need this file, but avoid segfault when filling histos...?

	if(!read_preComputed_MVA && !makeHisto_inputVars) //Need to book user-trained BDTs, and do the application in this function
	{
		if(template_name == "2Dlin" || template_name == "2D")
		{
			reader1 = new TMVA::Reader( "!Color:!Silent" );
			reader2 = new TMVA::Reader( "!Color:!Silent" );
		}
		else
		{
			reader = new TMVA::Reader( "!Color:!Silent" );
		}

		// Name & adress of local variables which carry the updated input values during the event loop
		// NB : the variable names MUST corresponds in name and type to those given in the weight file(s) used -- same order
		// NB : if booking 2 BDTs (e.g. template_name=="2Dlin", must make sure that they use the same input variables... or else, find some way to make it work in the code)


		if(analysis_type == "FCNC")
		{
			if(template_name == "ttbar")
			{
				for(int i=0; i<var_list_ttbar.size(); i++)
				{
					reader->AddVariable(var_list_ttbar[i], &var_list_floats_ttbar[i]);
				}
			}
			else if(template_name == "ttV")
			{
				for(int i=0; i<var_list_ttV.size(); i++)
				{
					// cout<<"var_list_ttV[i] "<<var_list_ttV[i]<<endl;
					reader->AddVariable(var_list_ttV[i], &var_list_floats_ttV[i]);
				}
			}

			/**
			* NB : REMINDER -- CAN NOT SET TWICE THE ADDRESS OF A VARIABLE
			* Since some variables are listed both in 'var_list_ttV' and 'var_list_ttbar', whenever the 2 lists are used at same time (e.g. BDT ttbar vs ttV),
			* must make sure that address is set only to 1 float ; arbitrarily decide that it will be the float of the ttV-list !
			*/
			if(template_name == "2Dlin" || template_name == "2D")
			{
				//First set BDT-ttbar addresses
				for(int i=0; i<var_list_ttbar.size(); i++)
				{
					// cout<<"var_list_ttbar[i] "<<var_list_ttbar[i]<<endl;

					int index_sameVar_in_ttV_list = -1;
					for(int j=0; j<var_list_ttV.size(); j++)
					{
						if(var_list_ttbar[i] == var_list_ttV[j]) {index_sameVar_in_ttV_list = j; break;}
					}

					// cout<<"-> index_sameVar_in_ttV_list "<<index_sameVar_in_ttV_list<<endl;

					//If found same var in ttV vector, point to ttV float ; else point to ttbar float
					if(index_sameVar_in_ttV_list >= 0) {reader1->AddVariable(var_list_ttbar[i].Data(), &var_list_floats_ttV[index_sameVar_in_ttV_list]);}
					else {reader1->AddVariable(var_list_ttbar[i].Data(), &var_list_floats_ttbar[i]);}
				}
				//Then set BDT-ttV addresses
				for(int i=0; i<var_list_ttV.size(); i++)
				{
					// cout<<"var_list_ttV[i] "<<var_list_ttV[i]<<endl;

					reader2->AddVariable(var_list_ttV[i].Data(), &var_list_floats_ttV[i]);
				}
			}
		}

		//tHq, ttH analysis : 1 single vector of inputs for ttV/ttbar
		else
		{
			//HARDCODED for tHq analysis! Use same vector for ttV, ttbar, but differ for MEM vars (ttV only)
			for(int i=0; i<var_list.size(); i++)
			{
				if(template_name == "2Dlin" || template_name == "2D")
				{
						// cout<<"var "<<var_list[i]<<endl;
					//-- tHq analysis : same vars for ttV/ttbar, except MEM (now include resHTT/sumjetpt also in ttbar)
					// if(analysis_type != "tHq" || (!var_list[i].Contains("mem") && !var_list[i].Contains("kin") && var_list[i] != "resHTT" && var_list[i] != "sum_jetPt")) //for BDT-ttV only
					if(analysis_type != "tHq" || (!var_list[i].Contains("mem") && !var_list[i].Contains("kin") ) ) //for BDT-ttV only
					{
						reader1->AddVariable(var_list[i].Data(), &var_list_floats[i]);
						// cout<<"Added variable "<<var_list[i]<<endl;
					}

					reader2->AddVariable(var_list[i].Data(), &var_list_floats[i]);
					// cout<<"Added variable "<<var_list[i]<<endl;
				}
				else
				{
					reader->AddVariable(var_list[i].Data(), &var_list_floats[i]);
					//cout<<"Added variable "<<var_list[i]<<endl;
				}
			}
		}

		for(int i=0; i<v_cut_name.size(); i++)
		{
			if(v_cut_IsUsedForBDT[i] && !v_cut_def[i].Contains("=="))
			{

				if(template_name == "2Dlin" || template_name == "2D")
				{
					reader1->AddVariable(v_cut_name[i].Data(), &v_cut_float[i]);
					reader2->AddVariable(v_cut_name[i].Data(), &v_cut_float[i]);
				}
				else
				{
					reader->AddVariable(v_cut_name[i].Data(), &v_cut_float[i]);
				}
			}
		}
	}

	// --- Book the MVA methods (1 or 2, depending on template)
	TString dir = "weights/";
	if(use_2016_ntuples) {dir+= "2016/";}
	dir+= classifier_name;

	TString MVA_method_name1 = "", MVA_method_name2 = "";
	TString weightfile = "";
	TString template_name_MVA = "";
	if(!read_preComputed_MVA && !makeHisto_inputVars)
	{
		if(template_name == "ttbar" || template_name == "ttV") //Book only 1 BDT
		{
			template_name_MVA = template_name + "_" + nLep_cat + "_all";

			MVA_method_name1 = template_name_MVA + " method";
			weightfile = dir + "/" + classifier_name + "_" + template_name_MVA;
			if(!train_with_trainingSel) {weightfile+= "_" + region;}
			weightfile+= "__" + signal_process + ".weights.xml";

			if(!Check_File_Existence(weightfile) ) {cout<<BOLD(FRED("Weight file "<<weightfile<<" not found ! Abort"))<<endl; return;}

			reader->BookMVA(MVA_method_name1, weightfile);
		}
		else if((template_name == "2Dlin" || template_name == "2D") && !read_preComputed_MVA && !makeHisto_inputVars) //Need to book 2 BDTs
		{
			template_name_MVA = "ttbar_" + nLep_cat + "_all";
			MVA_method_name1 = template_name_MVA + " method";
			weightfile = dir + "/" + classifier_name + "_" + template_name_MVA + "__" + signal_process + ".weights.xml";
			if(!Check_File_Existence(weightfile) ) {cout<<BOLD(FRED("Weight file "<<weightfile<<" not found ! Abort"))<<endl; return;}
			reader1->BookMVA(MVA_method_name1, weightfile);

			template_name_MVA = "ttV_" + nLep_cat + "_all";
			MVA_method_name2 = template_name_MVA + " method";
			weightfile = dir + "/" + classifier_name + "_" + template_name_MVA + "__" + signal_process + ".weights.xml";
			if(!Check_File_Existence(weightfile) ) {cout<<BOLD(FRED("Weight file "<<weightfile<<" not found ! Abort"))<<endl; return;}
			reader2->BookMVA(MVA_method_name2, weightfile);
		}

		// cout<<"MVA_method_name1 "<<MVA_method_name1<<endl;
		// cout<<"MVA_method_name2 "<<MVA_method_name2<<endl;
	}

	//Must book the 'Fakes_MC' sample if we want to substract it to data-driven (removed from list at the end)
	if(substract_MCPrompt_from_DD)
	{
		sample_list.push_back("Fakes_MC"); sample_groups.push_back("Fakes");
	}

	//Input TFile and TTree, called for each sample
	TFile* file_input;
	TTree* tree(0);

	//Template binning
	double xmin = -1, xmax = 1;
	nbins = 10;

	if(template_name == "2Dlin" && !unblind_lowBDT)
	{
		if(use_rebinning)
		{
			if(analysis_type == "ttH") //ttH2017 binning
			{
				if(nLep_cat == "2l") {xmin = 0; xmax = 11; nbins = 11;} //11 bins
				else {xmin = 0; xmax = 6; nbins = 6;} //6 bins
			}
			else //Binning
			{
				if(analysis_type == "tHq")
				{
					if(use_binning_compPallabi)
					{
						xmin = 0; xmax = 10; nbins = 10;
					}
					else
					{
						if(nLep_cat == "3l") {xmin = 0; xmax = 9; nbins = 9;}
						else {xmin = 0; xmax = 8; nbins = 8;}

						// xmin = 0; xmax = 10; nbins = 10;
					}
				}
				else
				{
					xmin = 0; xmax = 10; nbins = 10;
				}
			}
		}
		else
		{
			xmin = -1; xmax = 1; nbins = 10;
			// xmin = -2; xmax = 2; nbins = 10;  //If we simply transform the MVA as (BDT_tt + BDT_ttV), the output goes from -2 to +2
		}
	}
	else if(template_name == "categ")
	{
		int nCateg_toPlot = (channel_list.size()-1) / 2; //don't show nominal, don't split by charge
		xmin = 0; xmax = nCateg_toPlot; nbins = nCateg_toPlot;
	}

	if(bin_choice != -1) {xmin = 0; xmax = 9; nbins = 9;}

	//Read binning file from ttH2017 (in Func_other.cxx > Get_ttH2017_Binning())
	TString binning_filepath = "./weights/other/ttH2017_binning/binning_" + nLep_cat + ".root";
	TFile* f_binning = 0;
	TH2F* h_binning = 0;
	if(use_rebinning && analysis_type == "ttH" && !Check_File_Existence(binning_filepath) )
	{
		cout<<endl<<"File "<<binning_filepath<<FRED(" not found! Can not use ttH binning, will use a simple linear combination of the 2 BDTs instead!")<<endl;
		use_rebinning = false;
	}
	else if(use_rebinning && analysis_type == "ttH")
	{
		f_binning = TFile::Open(binning_filepath);
		h_binning = (TH2F*) f_binning->Get("hTargetBinning");
		if(!h_binning) {cout<<"Null binning histo ! Can not use ttH binning, will use a simple linear combination of the 2 BDTs instead!"<<endl; use_rebinning = false;}
	}

	//Want to plot ALL selected variables
	vector<TString> total_var_list;
	if(makeHisto_inputVars)
	{
		// for(int i=0; i<v_cut_name.size(); i++)
		// {
		// 	if(v_cut_name[i].Contains("is_")) {continue;} //Don't care about plotting the categories
		//
		// 	total_var_list.push_back(v_cut_name[i]);
		// }
		for(int i=0; i<var_list.size(); i++)
		{
			// if(!var_list[i].Contains("mem") || var_list[i].Contains("e-")) {continue;}

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
		// if(!sample_list[isample].Contains("Fakes_MC")) {continue;}

		float SWE_nominal; //nominal sum of weight
		vector<float> v_SWE_LHE; //sums of weights for all couplings
		if(writeTemplate_forAllCouplingPoints)
		{
			if(!sample_list[isample].Contains("tHq_h") && !sample_list[isample].Contains("tHW_h") && !sample_list[isample].Contains("ttH_ctcvcp_h")) {continue;}

			Get_SumWeights_allCouplings(sample_list[isample], v_SWE_LHE, SWE_nominal); //Read sums of weights

			// cout<<"SWE nominal = "<<SWE_nominal<<endl;
			// for(int ilhe=0; ilhe<v_SWE_LHE.size(); ilhe++)
			// {
			// 	cout<<"SWE "<<ilhe<<" = "<<v_SWE_LHE[ilhe]<<endl;
			// }
		}

		//Protections
		if(nLep_cat == "3l" && sample_list[isample] == "QFlip") {continue;}

		cout<<endl<<endl<<UNDL(FBLU("Sample : "<<sample_list[isample]<<""))<<endl;

		//Open input TFile
		TString inputfile = dir_ntuples + sample_list[isample] + ".root";

		//If we are not using ntuple split based on category, Flip & Fake are included in DATA sample !
		//NB : Fakes_MC is generated as a separate sample !
		if((sample_list[isample] == "QFlip" || sample_list[isample] == "Fakes" ) && dir_ntuples.Contains("allRegionsEvents") )
		{
			inputfile = dir_ntuples + "DATA.root";
		}
		else if(analysis_type == "tHq" && sample_list[isample] == "ttH")
		{
			inputfile = dir_ntuples + "ttH_ctcvcp.root"; //Hardcoded : read ttH_ctcvcp sample instead of fxfx sample for tHq analysis!
		}

		//MEM ntuples //Hardcoded path
		if(include_MEM_variables && nLep_cat == "3l")
		{
			if(analysis_type == "tHq") {inputfile = "./input_ntuples/tHq2017/MEM/3l/SR/" + sample_list[isample] + ".root";}
			if(analysis_type == "FCNC") {inputfile = "./input_ntuples/FCNC/MEM/3l/SR/" + sample_list[isample] + ".root";}

			if(analysis_type == "tHq")
			{
				//-- Look for ttH_ctcvcp sample instead of ttH_fxfx
				if(sample_list[isample] == "ttH") {inputfile = "./input_ntuples/tHq2017/MEM/3l/SR/ttH_ctcvcp.root";}
				if(sample_list[isample] == "ttH_hww") {inputfile = "./input_ntuples/tHq2017/MEM/3l/SR/ttH_ctcvcp_hww.root";}
				if(sample_list[isample] == "ttH_hzz") {inputfile = "./input_ntuples/tHq2017/MEM/3l/SR/ttH_ctcvcp_hzz.root";}
				if(sample_list[isample] == "ttH_htt") {inputfile = "./input_ntuples/tHq2017/MEM/3l/SR/ttH_ctcvcp_htt.root";}
				if(sample_list[isample] == "ttH_hzg") {inputfile = "./input_ntuples/tHq2017/MEM/3l/SR/ttH_ctcvcp_hzg.root";}
				if(sample_list[isample] == "ttH_hmm") {inputfile = "./input_ntuples/tHq2017/MEM/3l/SR/ttH_ctcvcp_hmm.root";}
			}
		}

		// cout<<"inputfile "<<inputfile<<endl;

        //NEW : some processes don't have entries in 3l SR, so did not run MEM on them -> File is missing !
        //This can cause problems, e.g. when merging histograms per group : expect to find all histograms (even if set to ~0)
        //--> In case file is not found, will write *all* the histograms associated to the files here, and set them to ~0 !
		if(!Check_File_Existence(inputfile))
		{
			cout<<endl<<"File "<<inputfile<<FRED(" not found!")<<endl;

			if(template_name == "2D") {continue;} //with 2D templates, don't want to write TH1Fs

			for(int itree=0; itree<systTree_list.size(); itree++)
			{
				// cout<<"tree "<<systTree_list[itree]<<endl;

				vector<vector<vector<TH1F*>>> vvv_histo_chan_syst_var(channel_list.size());
				vector<vector<vector<TH1F*>>> vvv_histo_chan_syst_LHE(channel_list.size()); //To get tH templates for all LHE points

				for(int ichan=0; ichan<channel_list.size(); ichan++)
				{
					if(writeTemplate_forAllCouplingPoints && nLep_cat == "3l" && ichan != 0) {continue;} //don't care about splitting in 3l
					else if(writeTemplate_forAllCouplingPoints && nLep_cat == "2l" && ichan == 0) {continue;} //only care about split channels in 2l

					// cout<<"chan "<<channel_list[ichan]<<endl;

					if((sample_list[isample].Contains("Flip") || sample_list[isample].Contains("GammaConv")) && (channel_list[ichan] == "uuu" || channel_list[ichan] == "uu" || channel_list[ichan].Contains("mm_")) ) {continue;} //Only in channels with ele
					else if(analysis_type == "ttH" && (region == "CR_WZ" || region == "CR_ZZ" || nLep_cat == "4l") && channel_list[ichan] != "") {continue;} //no subcat for WZ/ZZ CRs, 4l SR

					if(sample_list[isample] == "DATA" || sample_list[isample] == "QFlip" || systTree_list[itree] != "")
					{
						vvv_histo_chan_syst_var[ichan].resize(1); //Cases for which we only need to store the nominal weight
						vvv_histo_chan_syst_LHE[ichan].resize(1);
					}
					else //Subcategories -> 1 histo for nominal + 1 histo per systematic
					{
						vvv_histo_chan_syst_var[ichan].resize(syst_list.size());
						vvv_histo_chan_syst_LHE[ichan].resize(syst_list.size());
					}

					for(int isyst=0; isyst<syst_list.size(); isyst++)
					{
						//-- Protections : not all syst weights apply to all samples, etc.
						if((sample_list[isample] == "DATA" || sample_list[isample] == "QFlip" || systTree_list[itree] != "") && syst_list[isyst] != "") {break;} //nominal only
						// else if(channel_list.size() > 1 && channel_list[ichan] == "" && (systTree_list[itree] != "" || syst_list[isyst] != "")) {break;} //nominal only
						else if((syst_list[isyst].Contains("FR_") || syst_list[isyst].Contains("Clos")) && !sample_list[isample].Contains("Fake") ) {continue;}
						else if(sample_list[isample].Contains("Fake") && !syst_list[isyst].Contains("FR_") && !syst_list[isyst].Contains("Clos") && syst_list[isyst] != "") {continue;}
						else if(syst_list[isyst].Contains("Clos") && nLep_cat == "3l") {continue;}
						else if(syst_list[isyst].Contains("ttH") && !sample_list[isample].Contains("ttH")) {continue;}
						else if(syst_list[isyst].Contains("ttW") && !sample_list[isample].Contains("ttW")) {continue;}
						else if(syst_list[isyst].Contains("ttZ") && !sample_list[isample].Contains("ttZ") && !sample_list[isample].Contains("TTJets")) {continue;}
						else if(syst_list[isyst].Contains("_e_") && (channel_list[ichan].Contains("mm_") || channel_list[ichan] == "uu" || channel_list[ichan] == "uuu")) {continue;}
						else if(syst_list[isyst].Contains("_m_") && (channel_list[ichan].Contains("ee_") || channel_list[ichan] == "ee" || channel_list[ichan] == "eee")) {continue;}

						// cout<<"syst "<<syst_list[isyst]<<endl;

						if(!writeTemplate_forAllCouplingPoints) {vvv_histo_chan_syst_var[ichan][isyst].resize(total_var_list.size());}

						for(int ivar=0; ivar<total_var_list.size(); ivar++)
						{
							for(int ilhe=0; ilhe<v_couplingPoints.size(); ilhe++)
							{
								if(!writeTemplate_forAllCouplingPoints && ilhe != 0) {continue;} //ilhe loop only for all couplings

								// cout<<"var "<<total_var_list[ivar]<<endl;

								// cout<<"vvv_histo_chan_syst_var.size() "<<vvv_histo_chan_syst_var.size()<<endl;
								// cout<<"vvv_histo_chan_syst_var[ilhe].size() "<<vvv_histo_chan_syst_var[ilhe].size()<<endl;

								if(!writeTemplate_forAllCouplingPoints) {vvv_histo_chan_syst_var[ichan][isyst][ivar] = new TH1F("", "", nbins, xmin, xmax);}
								else {vvv_histo_chan_syst_LHE[ichan][isyst][ilhe] = new TH1F("", "", nbins, xmin, xmax);}

								if(!writeTemplate_forAllCouplingPoints)
								{
									Set_Histogram_FlatZero(vvv_histo_chan_syst_var[ichan][isyst][ivar], true, "");
									cout<<"(Sample "<<sample_list[isample]<<" / tree "<<systTree_list[itree]<<" / channel "<<channel_list[ichan]<<" / syst "<<syst_list[isyst]<<")"<<endl<<endl;
								}
								else
								{
									Set_Histogram_FlatZero(vvv_histo_chan_syst_LHE[ichan][isyst][ilhe], true, "");
									cout<<"(Sample "<<sample_list[isample]<<" / tree "<<systTree_list[itree]<<" / coupling "<<v_couplingPoints[ilhe]<<" / syst "<<syst_list[isyst]<<")"<<endl<<endl;
								}

								TString samplename = sample_list[isample];
								if(sample_list[isample] == "DATA") {samplename = "data_obs";}

								TString output_histo_name;
								output_histo_name = classifier_name + template_name + "_" + nLep_cat + "_" + region;
								if(channel_list[ichan] != "") {output_histo_name+= "_" + channel_list[ichan];}
								output_histo_name+= "__" + samplename;
								if(syst_list[isyst] != "") {output_histo_name+= "__" + syst_list[isyst];}
								else if(systTree_list[itree] != "") {output_histo_name+= "__" + systTree_list[itree];}

								file_output->cd();

								if(!writeTemplate_forAllCouplingPoints)
								{
									//ST_hut_FCNC 3l region has low stat => merge bins 2 by 2 ?
									if(signal_process == "ST_hut_FCNC" && nLep_cat == "3l")
									{
										vvv_histo_chan_syst_var[ichan][isyst][ivar]->Rebin(2);
									}

									vvv_histo_chan_syst_var[ichan][isyst][ivar]->Write(output_histo_name);
									delete vvv_histo_chan_syst_var[ichan][isyst][ivar]; vvv_histo_chan_syst_var[ichan][isyst][ivar] = NULL;
								}
								else
								{
									vvv_histo_chan_syst_LHE[ichan][isyst][ilhe]->Write(output_histo_name);
									delete vvv_histo_chan_syst_LHE[ichan][isyst][ilhe]; vvv_histo_chan_syst_LHE[ichan][isyst][ilhe] = NULL;
								}

								// cout<<"-- Write histo "<<output_histo_name<<" (integral = "<<vvv_histo_chan_syst_var[ichan][isyst]->Integral()<<", "<<vvv_histo_chan_syst_var[ichan][isyst]->GetEntries()<<" entries)"<<endl;
							} //ilhe
						} //var
					} //syst
				} //chan
			} //itree

			continue;
		} //MISSING FILE --> Write histos nonetheless

		file_input = TFile::Open(inputfile, "READ");

		vector<float> v_integralsRescale(channel_list.size()); //Can use this vector to store (e.g. nominal) integrals, and rescale some systematic

		//-- Loop on TTrees : first empty element corresponds to nominal TTree ; additional TTrees may correspond to JES/JER TTrees (defined in main)
		//NB : only nominal TTree contains systematic weights ; others only contain the nominal weight (but variables have different values)
		for(int itree=0; itree<systTree_list.size(); itree++)
		{
			if(systTree_list[itree] != "" && (sample_list[isample] == "DATA" || sample_list[isample] == "QFlip" || sample_list[isample].Contains("Fakes")) ) {continue;} //For MC only
			if(template_name == "2D" && systTree_list[itree] != "") {continue;}

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

			//-- Set adresses of input variables
			Float_t signal_TT_MVA, signal_TTV_MVA; //Store value of precomputed MVA (stored as branch at NTAnalyzer level)

			if(makeHisto_inputVars)
			{
				for(int i=0; i<total_var_list.size(); i++)
				{
					if(!include_MEM_variables || (!total_var_list[i].Contains("mem") && !total_var_list[i].Contains("kin")) ) //transform MEM variables directly in event loop
					{
						tree->SetBranchStatus(total_var_list[i], 1);
						tree->SetBranchAddress(total_var_list[i], &total_var_floats[i]);
					}
				}
			}
			else if(!read_preComputed_MVA) //Book input variables in same order as trained BDT
			{
				if(analysis_type == "FCNC") //NB : don't set same address twice!
				{
					//First set BDT-ttbar addresses
					for(int i=0; i<var_list_ttbar.size(); i++)
					{
						int index_sameVar_in_ttV_list = -1;
						for(int j=0; j<var_list_ttV.size(); j++)
						{
							if(var_list_ttbar[i] == var_list_ttV[j]) {index_sameVar_in_ttV_list = j; break;}
						}

						//Only need to set address of ttbar variable if it is not already present in ttV vector ; or if only calling BDT-ttbar (set all addresses here)
						if(template_name == "ttbar" || index_sameVar_in_ttV_list == -1)
						{
							if(!include_MEM_variables || (!var_list_ttbar[i].Contains("_mem_") && !var_list_ttbar[i].Contains("_kin_")) ) //transform MEM variables directly in event loop
							{
								// cout<<"var_list_ttbar[i] "<<var_list_ttbar[i]<<endl;

								tree->SetBranchStatus(var_list_ttbar[i], 1);
								tree->SetBranchAddress(var_list_ttbar[i], &var_list_floats_ttbar[i]);
							}
						}
					}

					//Then set BDT-ttV addresses
					if(template_name == "ttV" || template_name.Contains("2D"))
					{
						for(int i=0; i<var_list_ttV.size(); i++)
						{
							if(!include_MEM_variables || (!var_list_ttV[i].Contains("_mem_") && !var_list_ttV[i].Contains("_kin_")) ) //transform MEM variables directly in event loop
							{
								// cout<<"var_list_ttV[i] "<<var_list_ttV[i]<<endl;

								tree->SetBranchStatus(var_list_ttV[i], 1);
								tree->SetBranchAddress(var_list_ttV[i], &var_list_floats_ttV[i]);
							}
						}
					}
				}

				else //not FCNC analysis
				{
					for(int i=0; i<var_list.size(); i++)
					{
						if(!include_MEM_variables || (!var_list[i].Contains("mem") && !var_list[i].Contains("kin")) ) //transform MEM variables directly in event loop
						{
							tree->SetBranchStatus(var_list[i], 1);
							tree->SetBranchAddress(var_list[i], &var_list_floats[i]);
							// cout<<"Activate var '"<<var_list[i]<<"'"<<endl;
						}
					}
				}
			}
			else //Set address of MVA variables
			{
				if(template_name == "ttbar" || template_name == "2Dlin" || template_name == "2D")
				{
					TString var_tmp = "signal_";
					if(nLep_cat == "2l") {var_tmp+= "2lss_TT_MVA";}
					else {var_tmp+= "3l_TT_MVA";}
					tree->SetBranchStatus(var_tmp, 1);
					tree->SetBranchAddress(var_tmp, &signal_TT_MVA);
				}

				if(template_name == "ttV" || template_name == "2Dlin" || template_name == "2D")
				{
					TString var_tmp = "signal_";
					if(nLep_cat == "2l") {var_tmp+= "2lss_TTV_MVA";}
					else {var_tmp+= "3l_TTV_MVA";}
					tree->SetBranchStatus(var_tmp, 1);
					tree->SetBranchAddress(var_tmp, &signal_TTV_MVA);
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
			Char_t is_goodCategory2; //for debug, checks, might need second cat.
			TString cat_name = Get_Category_Boolean_Name(nLep_cat, region, analysis_type, sample_list[isample], scheme);

			tree->SetBranchStatus(cat_name, 1);
			tree->SetBranchAddress(cat_name, &is_goodCategory);
			// cout<<"Categ <=> "<<cat_name<<endl;

			//--- Cut on relevant categorization (lepton flavour, btagging, charge)
			float channel, nMediumBJets, lepCharge;
			tree->SetBranchStatus("channel", 1);
			tree->SetBranchAddress("channel", &channel);
			tree->SetBranchStatus("nMediumBJets", 1);
			tree->SetBranchAddress("nMediumBJets", &nMediumBJets);
			tree->SetBranchStatus("lepCharge", 1);
			tree->SetBranchAddress("lepCharge", &lepCharge);

			//NEW : MEM variables
			Double_t mc_mem_thj_weight, mc_mem_thj_weight_log, mc_mem_ttw_weight, mc_mem_ttw_weight_log, mc_mem_ttz_weight, mc_mem_ttz_weight_log;
			Double_t mc_mem_ttbar_weight, mc_mem_ttbarsl_weight, mc_mem_ttbarfl_weight, mc_mem_ttbar_weight_log, mc_mem_ttbarsl_weight_log, mc_mem_ttbarfl_weight_log;
			if(include_MEM_variables)
			{
				tree->SetBranchStatus("mc_mem_thj_weight", 1);
				tree->SetBranchAddress("mc_mem_thj_weight", &mc_mem_thj_weight);
				tree->SetBranchStatus("mc_mem_thj_weight_log", 1);
				tree->SetBranchAddress("mc_mem_thj_weight_log", &mc_mem_thj_weight_log);
				tree->SetBranchStatus("mc_mem_ttw_weight", 1);
				tree->SetBranchAddress("mc_mem_ttw_weight", &mc_mem_ttw_weight);
				tree->SetBranchStatus("mc_mem_ttz_weight", 1);
				tree->SetBranchAddress("mc_mem_ttz_weight", &mc_mem_ttz_weight);

				//-- added 21/05 (was not set..!)
				tree->SetBranchStatus("mc_mem_ttw_weight_log", 1);
				tree->SetBranchAddress("mc_mem_ttw_weight_log", &mc_mem_ttw_weight_log);
				tree->SetBranchStatus("mc_mem_ttz_weight_log", 1);
				tree->SetBranchAddress("mc_mem_ttz_weight_log", &mc_mem_ttz_weight_log);

				if(analysis_type == "FCNC") //Also ttbar hypotheses
				{
					tree->SetBranchStatus("mc_mem_ttbar_weight", 1);
					tree->SetBranchAddress("mc_mem_ttbar_weight", &mc_mem_ttbar_weight);
					tree->SetBranchStatus("mc_mem_ttbarsl_weight", 1);
					tree->SetBranchAddress("mc_mem_ttbarsl_weight", &mc_mem_ttbarsl_weight);
					tree->SetBranchStatus("mc_mem_ttbarfl_weight", 1);
					tree->SetBranchAddress("mc_mem_ttbarfl_weight", &mc_mem_ttbarfl_weight);

					tree->SetBranchStatus("mc_mem_ttbar_weight_log", 1);
					tree->SetBranchAddress("mc_mem_ttbar_weight_log", &mc_mem_ttbar_weight_log);
					tree->SetBranchStatus("mc_mem_ttbarsl_weight_log", 1);
					tree->SetBranchAddress("mc_mem_ttbarsl_weight_log", &mc_mem_ttbarsl_weight_log);
					tree->SetBranchStatus("mc_mem_ttbarfl_weight_log", 1);
					tree->SetBranchAddress("mc_mem_ttbarfl_weight_log", &mc_mem_ttbarfl_weight_log);
				}
			}

			//--- Event weights
			float weight; float weight_SF; //Stored separately
			float SMcoupling_SF, SMcoupling_weight, mc_weight_originalValue;
            float chargeLeadingLep;
			tree->SetBranchStatus("weight", 1);
			tree->SetBranchAddress("weight", &weight);
			tree->SetBranchStatus("mc_weight_originalValue", 1);
			tree->SetBranchAddress("mc_weight_originalValue", &mc_weight_originalValue);

			if(use_SM_coupling && (sample_list[isample].Contains("tHq") || sample_list[isample].Contains("tHW")))
			{
				tree->SetBranchStatus("SMcoupling_SF", 1);
				tree->SetBranchAddress("SMcoupling_SF", &SMcoupling_SF);
			}

			if(sample_list[isample].Contains("Fakes") )
			{
				tree->SetBranchStatus("weightfake", 1); tree->SetBranchAddress("weightfake", &weight_SF); //Special weight for DD Fakes events
			}
			else if(sample_list[isample].Contains("Flip") )
			{
                tree->SetBranchStatus("weightflip", 1); tree->SetBranchAddress("weightflip", &weight_SF); //Special weight for DD QFlip events

				tree->SetBranchStatus("chargeLeadingLep", 1); tree->SetBranchAddress("chargeLeadingLep", &chargeLeadingLep); //Special case : QFlip events are +-. Look at charge of leading lepton to decide if goes into "pos" or "neg" categ.
			}

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

			//For tHq analysis and tH/ttH signal : get all LHE weights for reweighting
			vector<Float_t>* LHEweights;
			if(writeTemplate_forAllCouplingPoints && systTree_list[itree] == "") //LHEweights not yet stored in JES TTrees --> Keep using nominal one
			{
				tree->SetBranchStatus("LHEweights", 1);
				tree->SetBranchAddress("LHEweights", &LHEweights);
			}

			// cout<<"LHEweights->size() "<<LHEweights->size()<<endl;
			// for(int ilhe=0; ilhe<LHEweights->size(); ilhe++)
			// {
			// 	cout<<"LHEweights["<<ilhe<<"] "<<LHEweights->at(ilhe)<<endl;
			// }

			//Reserve memory for 1 TH1F* per category, per systematic
			vector<vector<vector<TH1F*>>> vvv_histo_chan_syst_var(channel_list.size());
			vector<vector<vector<TH2F*>>> vvv_histo_chan_syst_var2D(channel_list.size());
			vector<vector<vector<TH1F*>>> vvv_histo_chan_syst_LHE(channel_list.size()); //To get tH templates for all LHE points (not split per channel)

			for(int ichan=0; ichan<channel_list.size(); ichan++)
			{
				// if((channel_list.size() > 1 && channel_list[ichan] == "") || sample_list[isample] == "DATA" || sample_list[isample] == "QFlip" || systTree_list[itree] != "") {vvv_histo_chan_syst_var[ichan].resize(1);} //Cases for which we only need to store the nominal weight
				if(sample_list[isample] == "DATA" || sample_list[isample] == "QFlip" || systTree_list[itree] != "")
				{
					vvv_histo_chan_syst_var[ichan].resize(1); //Cases for which we only need to store the nominal weight
					if(template_name == "2D") {vvv_histo_chan_syst_var2D[ichan].resize(1);}

					vvv_histo_chan_syst_LHE[ichan].resize(1);
				}
				else //Subcategories -> 1 histo for nominal + 1 histo per systematic
				{
					vvv_histo_chan_syst_var[ichan].resize(syst_list.size());
					if(template_name == "2D") {vvv_histo_chan_syst_var2D[ichan].resize(syst_list.size());}

					vvv_histo_chan_syst_LHE[ichan].resize(syst_list.size());
				}

				//Init histos
				if(!writeTemplate_forAllCouplingPoints)
				{
					for(int isyst=0; isyst<vvv_histo_chan_syst_var[ichan].size(); isyst++)
					{
						vvv_histo_chan_syst_var[ichan][isyst].resize(total_var_list.size());

						if(template_name == "2D")
						{
							vvv_histo_chan_syst_var2D[ichan][isyst].resize(1);
						}

						for(int ivar=0; ivar<total_var_list.size(); ivar++)
						{
							if(makeHisto_inputVars && !Get_Variable_Range(total_var_list[ivar], nbins, xmin, xmax)) {cout<<FRED("Unknown variable name : "<<total_var_list[ivar]<<"! (add in function 'Get_Variable_Range()')")<<endl; continue;} //Get binning for this variable (if not template)

							vvv_histo_chan_syst_var[ichan][isyst][ivar] = new TH1F("", "", nbins, xmin, xmax);
							if(template_name == "2D") {vvv_histo_chan_syst_var2D[ichan][isyst][0] = new TH2F("", "", 10, -1, 1, 10, -1, 1);}
						}
					} //syst
				}
				else
				{
					for(int isyst=0; isyst<vvv_histo_chan_syst_LHE[ichan].size(); isyst++)
					{
						// cout<<"isyst "<<isyst<<endl;

						vvv_histo_chan_syst_LHE[ichan][isyst].resize(v_couplingPoints.size());

						for(int ilhe=0; ilhe<v_couplingPoints.size(); ilhe++)
						{
							// cout<<"ilhe "<<ilhe<<endl;

							vvv_histo_chan_syst_LHE[ichan][isyst][ilhe] = new TH1F("", "", nbins, xmin, xmax);
						}
					} //syst
				}
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

				if(template_name == "2D" && analysis_type == "tHq")
				{
					if(nLep_cat == "2l" && channel == 2) {continue;} //don't want ee in tHq 2D plot
				}

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

				//added 21/05 -- WAS BUGGED BEFORE : lepCharge address was overriden, not used as variable!!
				if(analysis_type == "tHq")
				{
					for(int ivar=0; ivar<var_list.size(); ivar++)
					{
						if(var_list[ivar] == "nMediumBJets") {var_list_floats[ivar] = nMediumBJets;}
						if(var_list[ivar] == "lepCharge") {var_list_floats[ivar] = lepCharge;}
					}
				}
				else if(analysis_type == "FCNC")
				{
					for(int ivar=0; ivar<var_list_ttbar.size(); ivar++)
					{
						if(var_list_ttbar[ivar] == "nMediumBJets") {var_list_floats_ttbar[ivar] = nMediumBJets;}
						if(var_list_ttbar[ivar] == "lepCharge") {var_list_floats_ttbar[ivar] = lepCharge;}
					}
					for(int ivar=0; ivar<var_list_ttV.size(); ivar++)
					{
						if(var_list_ttV[ivar] == "nMediumBJets") {var_list_floats_ttV[ivar] = nMediumBJets;}
						if(var_list_ttV[ivar] == "lepCharge") {var_list_floats_ttV[ivar] = lepCharge;}
					}
				}

				//Fakes, QFlip --> Scale factors
				if(sample_list[isample].Contains("Fakes") || sample_list[isample].Contains("QFlip") )
				{
                    if(weight_SF == 1) {continue;} //Tmp bug in FCNC ntuples : some Fakes_MC entries with weightfake==1, should be much less... remove them
					weight*= weight_SF; //Used to fill nominal
				}

				if(use_SM_coupling && (sample_list[isample].Contains("tHq") || sample_list[isample].Contains("tHW")))
				{
					weight*= SMcoupling_SF;

					//STILL NEEDED - still using 'bugged' ntuples [SMcoupling off by cst factor] (JES MEM variations already corrected)
					if(analysis_type == "tHq" && include_MEM_variables && nLep_cat == "3l" && itree == 0 && (template_name == "2Dlin" || template_name == "ttV")) //MEM is used
					{
						if(sample_list[isample].Contains("tHq")) {weight*= 13.956002;}
						else if(sample_list[isample].Contains("tHW") ) {weight*= 8.9769040;}
					}
				}

                if(sample_list[isample].Contains("Flip") ) {lepCharge = chargeLeadingLep;} //For +- events, look at charge of leading lepton

				if(include_MEM_variables) //-- must define here all MEM variables by hand !
				{
					//Trick 1 : depending on case, must check/modify different vectors ; use pointer to point to relevant vectors
					vector<TString>* v_var_tmp;
					vector<float>* v_floats_tmp;

					//Trick 2 : if reading 2 BDTs with 2 different lists of variables, must loop twice to modify vector for each BDT -> use loop index
					int n_lists = 1;
					if(analysis_type == "FCNC" && template_name.Contains("2D")) {n_lists = 2;} //ttV+ttbar lists

					for(int iloop=0; iloop<n_lists; iloop++)
					{
						if(analysis_type == "FCNC")
						{
							if(template_name == "ttV") {v_var_tmp = &var_list_ttV; v_floats_tmp = &var_list_floats_ttV;}
							else if(template_name == "ttbar") {v_var_tmp = &var_list_ttbar; v_floats_tmp = &var_list_floats_ttbar;}
							else if(template_name.Contains("2D"))
							{
								if(iloop == 0) {v_var_tmp = &var_list_ttbar; v_floats_tmp = &var_list_floats_ttbar;}
								else if(iloop == 0) {v_var_tmp = &var_list_ttV; v_floats_tmp = &var_list_floats_ttV;}
							}
						}
						else {v_var_tmp = &var_list; v_floats_tmp = &var_list_floats;} //single list

						for(int i=0; i<v_var_tmp->size(); i++)
						{
							if(v_var_tmp->at(i) == "log((mc_mem_thj_weight+1e-3*mc_mem_ttz_weight+1e-13*mc_mem_ttw_weight) / (1e-3*mc_mem_ttz_weight+1e-13*mc_mem_ttw_weight))")
							{
								v_floats_tmp->at(i) = log( (mc_mem_thj_weight + 1e-3*mc_mem_ttz_weight + 1e-13*mc_mem_ttw_weight) / (1e-13*mc_mem_ttw_weight + 1e-3*mc_mem_ttz_weight));
							}
							else if(v_var_tmp->at(i) == "log((mc_mem_thj_weight)/(1e-13*mc_mem_ttw_weight))")
							{
								v_floats_tmp->at(i) = log(mc_mem_thj_weight / 1e-13*mc_mem_ttw_weight);
							}
							else if(v_var_tmp->at(i) == "log((mc_mem_thj_weight)/(1e-3*mc_mem_ttz_weight))")
							{
								v_floats_tmp->at(i) = log(mc_mem_thj_weight / 1e-3*mc_mem_ttz_weight);
							}
							else if(v_var_tmp->at(i) == "mc_mem_thj_weight_log")
							{
								v_floats_tmp->at(i) = mc_mem_thj_weight_log;
							}
							else if(v_var_tmp->at(i) == "mc_mem_ttw_weight")
							{
								v_floats_tmp->at(i) = mc_mem_ttw_weight;
							}
							else if(v_var_tmp->at(i) == "mc_mem_ttz_weight")
							{
								v_floats_tmp->at(i) = mc_mem_ttz_weight;
							}
							//-- ADDED 21/05 (was bugged : ttW/ttZ_log never set)
							else if(v_var_tmp->at(i) == "mc_mem_ttw_weight_log")
							{
								v_floats_tmp->at(i) = mc_mem_ttw_weight_log;
							}
							else if(v_var_tmp->at(i) == "mc_mem_ttz_weight_log")
							{
								v_floats_tmp->at(i) = mc_mem_ttz_weight_log;
							}
							else if(v_var_tmp->at(i) == "mc_mem_ttbar_weight_log")
							{
								v_floats_tmp->at(i) = mc_mem_ttbar_weight_log;
							}
							else if(v_var_tmp->at(i) == "mc_mem_ttbarsl_weight_log")
							{
								v_floats_tmp->at(i) = mc_mem_ttbarsl_weight_log;
							}
							else if(v_var_tmp->at(i) == "mc_mem_ttbarfl_weight_log")
							{
								v_floats_tmp->at(i) = mc_mem_ttbarfl_weight_log;
							}
							else if(v_var_tmp->at(i) == "log((mc_mem_thj_weight+1e-13*mc_mem_ttbar_weight) / (1e-13*mc_mem_ttbar_weight))")
							{
								v_floats_tmp->at(i) = log((mc_mem_thj_weight+1e-13*mc_mem_ttbar_weight) / (1e-13*mc_mem_ttbar_weight));
							}
							else if(v_var_tmp->at(i) == "log((mc_mem_thj_weight+1e-13*mc_mem_ttbarsl_weight) / (1e-13*mc_mem_ttbarsl_weight))")
							{
								v_floats_tmp->at(i) = log((mc_mem_thj_weight+1e-13*mc_mem_ttbarsl_weight) / (1e-13*mc_mem_ttbarsl_weight));
							}
							else if(v_var_tmp->at(i) == "log((mc_mem_thj_weight+1e-13*mc_mem_ttbarfl_weight) / (1e-13*mc_mem_ttbarfl_weight))")
							{
								v_floats_tmp->at(i) = log((mc_mem_thj_weight+1e-13*mc_mem_ttbarfl_weight) / (1e-13*mc_mem_ttbarfl_weight));
							}
							else if(v_var_tmp->at(i).Contains("mem")) {cout<<"ERROR ! MEM variable not recognized : "<<v_var_tmp->at(i)<<endl;}

							// cout<<endl<<"var "<<v_var_tmp->at(i)<<" : "<<v_floats_tmp->at(i)<<endl;
						}
					}

					if(makeHisto_inputVars) //Different vectors for input variables, must copy values
					{
						for(int ivar=0; ivar<total_var_list.size(); ivar++)
						{
							if(total_var_list[ivar].Contains("mem"))
							{
								for(int i=0; i<var_list.size(); i++)
								{
									if(total_var_list[ivar] == var_list[i]) {total_var_floats[ivar] = v_floats_tmp->at(i);}
								}
							}
						}

					}
				}

				bool debug_printout_values = false;
				if(debug_printout_values)
				{
					cout<<endl<<"//--------------------------------------------"<<endl;

					if(analysis_type == "FCNC")
					{
						if(template_name.Contains("2D"))
						{
							//First set BDT-ttbar addresses
							for(int ivar=0; ivar<var_list_ttbar.size(); ivar++)
							{
								cout<<"ivar "<<var_list_ttbar[ivar]<<" : ";
								int index_sameVar_in_ttV_list = -1;
								for(int j=0; j<var_list_ttV.size(); j++)
								{
									if(var_list_ttbar[ivar] == var_list_ttV[j]) {index_sameVar_in_ttV_list = j; break;}
								}

								//If found same var in ttV vector, point to ttV float ; else point to ttbar float
								if(index_sameVar_in_ttV_list >= 0) {cout<<var_list_floats_ttV[index_sameVar_in_ttV_list]<<endl;}
								else {cout<<var_list_floats_ttbar[ivar]<<endl;}
							}
							for(int ivar=0; ivar<var_list_ttV.size(); ivar++)
							{
								cout<<"ivar "<<var_list_ttV[ivar]<<" : "<<var_list_floats_ttV[ivar]<<endl;
							}

							// cout<<"ivar "<<var_list[ivar]<<" : "<<var_list_floats_ttbar[ivar]<<" / "<<var_list_floats_ttV[ivar]<<endl;
						}
						else if(template_name.Contains("ttbar"))
						{
							for(int ivar=0; ivar<var_list_ttbar.size(); ivar++)
							{
								cout<<"ivar "<<var_list_ttbar[ivar]<<" : "<<var_list_floats_ttbar[ivar]<<endl;
							}
						}
						else if(template_name.Contains("ttV"))
						{
							for(int ivar=0; ivar<var_list_ttbar.size(); ivar++)
							{
								cout<<"ivar "<<var_list_ttV[ivar]<<" : "<<var_list_floats_ttV[ivar]<<endl;
							}
						}
					}

					else //only 1 reader in other analyses
					{
						for(int ivar=0; ivar<var_list.size(); ivar++)
						{
							cout<<"ivar "<<var_list[ivar]<<" : "<<var_list_floats[ivar]<<endl;
						}
					}
				} //DEBUG

				//Get MVA value to make template
				double mva_value1 = -9, mva_value2 = -9;

				//-- Fill first MVA value
				if(!makeHisto_inputVars && !read_preComputed_MVA)
				{
					if(template_name == "2Dlin" || template_name == "2D")
					{
						// for(int ivar=0; ivar<var_list.size(); ivar++)
						// {
						// 	cout<<var_list[ivar]<<" "<<var_list_floats[ivar]<<endl;
						// }

						mva_value1 = reader1->EvaluateMVA(MVA_method_name1);
					}
					else {mva_value1 = reader->EvaluateMVA(MVA_method_name1);}
				}
				else
				{
					if(template_name == "ttbar" || template_name == "2Dlin" || template_name == "2D") {mva_value1 = signal_TT_MVA;}
					else if(template_name == "ttV") {mva_value1 = signal_TTV_MVA;}
				}

				//-- If necessary, fill second MVA value
				if(!makeHisto_inputVars && (template_name == "2Dlin" || template_name == "2D"))
				{
					if(!read_preComputed_MVA) {mva_value2 = reader2->EvaluateMVA(MVA_method_name2);}
					else {mva_value2 = signal_TTV_MVA;}
				}

				// if(sample_list[isample] == "tHq_hww")
				// {
					// cout<<"mva_value1 = "<<mva_value1<<endl;
					// cout<<"mva_value2 = "<<mva_value2<<endl;
				// }

				//Get relevant binning
				float xValue_tmp = -1;

				if(!makeHisto_inputVars)
				{
					if(template_name == "ttbar" || template_name == "ttV") {xValue_tmp = mva_value1;} //Simply fill MVA value
					else if(template_name == "2Dlin")
					{
						if(bin_choice != -1)
						{
							xValue_tmp = Test_Binnings(bin_choice, mva_value1, mva_value2, false) - 0.5;
							if(xValue_tmp == -1) {return;} //wrong value
							// cout<<"==> "<<xValue_tmp<<endl;
						}
						else if(use_rebinning)
						{
							if(analysis_type == "ttH") //Get ttH2017 binning
							{
								xValue_tmp = Get_ttH2017_Binning(h_binning, mva_value1, mva_value2) - 0.5; //Since bin width=1, bin center = (bin number - 0.5)
							}
							else if(analysis_type == "tHq") //Get tHq2017 binning
							{
								if(use_binning_compPallabi) {xValue_tmp = Get_tHq2017_Binning_compPallabi(mva_value1, mva_value2, nLep_cat) - 0.5;}
								else {xValue_tmp = Get_tHq2017_Binning(mva_value1, mva_value2, nLep_cat) - 0.5;} //Since bin width=1, bin center = (bin number - 0.5)
							}
							else if(analysis_type == "FCNC")
							{
								xValue_tmp = Get_FCNC_Binning(mva_value1, mva_value2, nLep_cat, signal_process) - 0.5; //Since bin width=1, bin center = (bin number - 0.5)
							}
						}
						// else {xValue_tmp = mva_value1+mva_value2;} //Simply add up BDT_ttbar + BDT_ttV values
                        else {xValue_tmp = mva_value2;} //Simply use BDT_ttV values
                        // else {xValue_tmp = mva_value1;} //Simply use BDT_ttbar values
					}
				}

				// cout<<"//--------------------------------------------"<<endl;
				// cout<<"xValue_tmp = "<<xValue_tmp<<endl;
				// cout<<"mva_value1 = "<<mva_value1<<endl;
				// cout<<"mva_value2 = "<<mva_value2<<endl;

				//-- UNBLIND LOW BDT
				if(unblind_lowBDT && analysis_type == "FCNC")
				{
					if(mva_value1 > 0) {continue;} //Keep events below 0 for BDT-ttbar
					xValue_tmp = mva_value1; //BDT-ttbar value, for fit
				}

				//-- Fill histos for all subcategories
				for(int ichan=0; ichan<channel_list.size(); ichan++)
				{
					if(writeTemplate_forAllCouplingPoints && nLep_cat == "3l" && ichan != 0) {continue;} //don't care about splitting in 3l
					else if(writeTemplate_forAllCouplingPoints && nLep_cat == "2l" && ichan == 0) {continue;} //only care about split channels in 2l

					if((sample_list[isample].Contains("Flip") || sample_list[isample].Contains("GammaConv")) && (channel_list[ichan] == "uuu" || channel_list[ichan] == "uu" || channel_list[ichan].Contains("mm_")) ) {continue;} //Only in channels with ele
					else if(analysis_type == "ttH" && (region == "CR_WZ" || region == "CR_ZZ" || nLep_cat == "4l") && channel_list[ichan] != "") {continue;} //no subcat for WZ/ZZ CRs, 4l SR

					if(channel_list[ichan] != "" && !Check_isEvent_passSubCategory(channel_list[ichan], channel, nMediumBJets, lepCharge) ) {continue;} //First element of vector corresponds to full selection (no subcateg) -> Fill it will all events passing cuts, nominal 'weight' only

					//If we just want to get the yields per subcategory, only care about nominal weight
					if(template_name == "categ")
					{
						xValue_tmp = Get_Binning_SubCategory(nLep_cat, channel_list[ichan]) - 0.5; //Get bin associated with category //bin width is 1, compute bin center
						Fill_TH1F_UnderOverflow(vvv_histo_chan_syst_var[0][0][0], xValue_tmp, weight); //Fill histo containing sum of channels with nominal weight, for plot
						// Fill_TH1F_UnderOverflow(vvv_histo_chan_syst_var[0][0], xValue_tmp, weight); //Fill histo containing sum of channels with nominal weight, for plot
						break; //fill 1 channel / central value only
					}

					for(int isyst=0; isyst<syst_list.size(); isyst++)
					{
						//-- Protections : not all syst weights apply to all samples, etc.
						if(syst_list[isyst] != "" && (sample_list[isample] == "DATA" || sample_list[isample] == "QFlip" || systTree_list[itree] != "")) {break;} //only nominal
						// else if(syst_list[isyst] != "" && channel_list.size() > 1 && channel_list[ichan] == "") {break;} //Fill this histo with nominal weight only
						else if((syst_list[isyst].Contains("FR_") || syst_list[isyst].Contains("Clos")) && !sample_list[isample].Contains("Fake") ) {continue;}
						else if(sample_list[isample].Contains("Fake") && !syst_list[isyst].Contains("FR_") && !syst_list[isyst].Contains("Clos") && syst_list[isyst] != "") {continue;}
						else if(syst_list[isyst].Contains("Clos") && nLep_cat == "3l") {continue;}
						else if(syst_list[isyst].Contains("ttH") && !sample_list[isample].Contains("ttH")) {continue;}
						else if(syst_list[isyst].Contains("ttW") && !sample_list[isample].Contains("ttW")) {continue;}
						else if(syst_list[isyst].Contains("ttZ") && !sample_list[isample].Contains("ttZ") && !sample_list[isample].Contains("TTJets")) {continue;}
						else if(syst_list[isyst].Contains("_e_") && (channel_list[ichan].Contains("mm_") || channel_list[ichan] == "uu" || channel_list[ichan] == "uuu")) {continue;}
						else if(syst_list[isyst].Contains("_m_") && (channel_list[ichan].Contains("ee_") || channel_list[ichan] == "ee" || channel_list[ichan] == "eee")) {continue;}

						double weight_tmp = 0; //Fill histo with this weight ; manipulate differently depending on syst

						// cout<<"-- Channel "<<channel_list[ichan]<<" / syst "<<syst_list[isyst]<<endl;

						if(syst_list[isyst] != "")
						{
							//For 'thu_shape' and Fakes closure shape syst, need to manipulate the weight // see : https://github.com/peruzzim/cmgtools-lite/blob/94X_dev_ttH/TTHAnalysis/python/plotter/ttH-multilepton/systsUnc.txt#L74
							if(syst_list[isyst].Contains("thu_shape") || syst_list[isyst].Contains("Clos") )
							{
								float add_weight = Get_lnN1D_Syst_Weight(syst_list[isyst], sample_list[isample], channel_list[ichan], nLep_cat, mva_value1, mva_value2, analysis_type);
								weight_tmp = weight * add_weight;

								// cout<<"! weight "<<weight<<endl;
								// cout<<"! add_weight "<<add_weight<<endl;

								// cout<<"vvv_histo_chan_syst_var["<<syst_list[isyst]<<"]["<<channel_list[ichan]<<"]+= "<<weight<<endl;
							}
							else if(sample_list[isample].Contains("Fakes") )
							{
								// if(syst_list[isyst].Contains("FR_be_el") && channel==1)
								// {
								// 	cout<<endl;
								// 	cout<<"channel_list[ichan] "<<channel_list[ichan]<<endl;
								// 	cout<<"channel "<<channel<<endl;
								// 	cout<<"syst_list[isyst] "<<syst_list[isyst]<<endl;
								// 	cout<<"weight "<<weight<<endl;
								// 	cout<<"weight_SF "<<weight_SF<<endl;
								// 	cout<<"v_float_systWeights[isyst] "<<v_float_systWeights[isyst]<<endl;
								// }

								weight_tmp = v_float_systWeights[isyst] * weight / weight_SF; //For Data-driven fakes, weight=1. But for MC Fakes contribution, must make sure to multiply by MC weight //BUT, 'weight' was already multiply by nominal 'weightfake' -> need to divide again !
							}
							else
							{
								if(use_SM_coupling && (sample_list[isample].Contains("tHq") || sample_list[isample].Contains("tHW")))
								{
									weight_tmp = v_float_systWeights[isyst] * SMcoupling_SF;

									//STILL NEEDED - still using ntuples with SMcoupling off by a constant factor (JES MEM variations already corrected)
									if(include_MEM_variables && nLep_cat == "3l" && itree == 0 && (template_name == "2Dlin" || template_name == "ttV")) //MEM is used
									{
										if(sample_list[isample].Contains("tHq")) {weight_tmp*= 13.956002;}
										else if(sample_list[isample].Contains("tHW") ) {weight_tmp*= 8.9769040;}
									}
								}
								else {weight_tmp = v_float_systWeights[isyst];}
							}
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
						// cout<<"vvv_histo_chan_syst_var["<<channel_list[ichan]<<"]["<<syst_list[isyst]<<"]+= "<<weight_tmp<<" => Integral "<<vvv_histo_chan_syst_var[ichan][isyst]->Integral()<<endl;

						if(makeHisto_inputVars)
						{
							for(int ivar=0; ivar<total_var_list.size(); ivar++)
							{
								//Special variables, adress already used for other purpose
								if(total_var_list[ivar] == "nMediumBJets") {total_var_floats[ivar] = nMediumBJets;}
								else if(total_var_list[ivar] == "lepCharge") {total_var_floats[ivar] = lepCharge;}
								else if(total_var_list[ivar] == "channel") {total_var_floats[ivar] = channel;}

								Fill_TH1F_UnderOverflow(vvv_histo_chan_syst_var[ichan][isyst][ivar], total_var_floats[ivar], weight_tmp);
							}
						}
						else
						{
							if(writeTemplate_forAllCouplingPoints)
							{
								for(int ilhe=0; ilhe<v_couplingPoints.size(); ilhe++)
								{
									// cout<<endl<<"ichan "<<ichan<<" / ilhe "<<ilhe<<" / v_couplingPoints[ilhe] "<<v_couplingPoints[ilhe]<<" / isyst "<<isyst<<endl;

									float weight_modifCoupling = weight_tmp * Get_SF_DifferentCoupling(sample_list[isample], v_couplingPoints[ilhe], LHEweights, mc_weight_originalValue, SWE_nominal, v_SWE_LHE[ilhe]);
									Fill_TH1F_UnderOverflow(vvv_histo_chan_syst_LHE[ichan][isyst][ilhe], xValue_tmp, weight_modifCoupling);
									// cout<<"vvv_histo_chan_syst_LHE[ichan][isyst][ilhe]->Integral() "<<vvv_histo_chan_syst_LHE[ichan][isyst][ilhe]->Integral()<<endl;
								}
							}
							else
							{
								if(template_name == "2D") {vvv_histo_chan_syst_var2D[ichan][isyst][0]->Fill(mva_value1, mva_value2, weight_tmp);}
								else {Fill_TH1F_UnderOverflow(vvv_histo_chan_syst_var[ichan][isyst][0], xValue_tmp, weight_tmp);}
							}
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
				if(writeTemplate_forAllCouplingPoints && nLep_cat == "3l" && ichan != 0) {continue;} //don't care about splitting in 3l
				else if(writeTemplate_forAllCouplingPoints && nLep_cat == "2l" && ichan == 0) {continue;} //only care about split channels in 2l

				if((sample_list[isample].Contains("Flip") || sample_list[isample].Contains("GammaConv")) && (channel_list[ichan] == "uuu" || channel_list[ichan] == "uu" || channel_list[ichan].Contains("mm_")) ) {continue;} //Only in channels with ele
				else if(analysis_type == "ttH" && (region == "CR_WZ" || region == "CR_ZZ" || nLep_cat == "4l") && channel_list[ichan] != "") {continue;} //no subcat for WZ/ZZ CRs, 4l SR

				for(int isyst=0; isyst<syst_list.size(); isyst++)
				{
					for(int ivar=0; ivar<total_var_list.size(); ivar++)
					{
						// cout<<"chan "<<channel_list[ichan]<<"syst "<<syst_list[isyst]<<endl;

						//-- Protections : not all syst weights apply to all samples, etc.
						if((sample_list[isample] == "DATA" || sample_list[isample] == "QFlip" || systTree_list[itree] != "") && syst_list[isyst] != "") {break;} //nominal only
						// if(channel_list.size() > 1 && channel_list[ichan] == "" && (systTree_list[itree] != "" || syst_list[isyst] != "")) {break;} //only nominal for full selection
						if(systTree_list[itree] != "" && syst_list[isyst] != "") {break;}
						else if((syst_list[isyst].Contains("FR_") || syst_list[isyst].Contains("Clos")) && !sample_list[isample].Contains("Fake") ) {continue;}
						else if(sample_list[isample].Contains("Fake") && !syst_list[isyst].Contains("FR_") && !syst_list[isyst].Contains("Clos") && syst_list[isyst] != "") {continue;}
						else if(syst_list[isyst].Contains("Clos") && nLep_cat == "3l") {continue;}
						else if(syst_list[isyst].Contains("ttH") && !sample_list[isample].Contains("ttH")) {continue;}
						else if(syst_list[isyst].Contains("ttW") && !sample_list[isample].Contains("ttW")) {continue;}
						else if(syst_list[isyst].Contains("ttZ") && !sample_list[isample].Contains("ttZ") && !sample_list[isample].Contains("TTJets")) {continue;}
						else if(syst_list[isyst].Contains("_e_") && (channel_list[ichan].Contains("mm_") || channel_list[ichan] == "uu" || channel_list[ichan] == "uuu")) {continue;}
						else if(syst_list[isyst].Contains("_m_") && (channel_list[ichan].Contains("ee_") || channel_list[ichan] == "ee" || channel_list[ichan] == "eee")) {continue;}

						//ST_hut_FCNC 3l region has low stat => merge bins 2 by 2 ?
						if(signal_process == "ST_hut_FCNC" && nLep_cat == "3l")
						{
							vvv_histo_chan_syst_var[ichan][isyst][ivar]->Rebin(2);
						}

						if(sample_list[isample] != "DATA" && sample_list[isample] != "Fakes" && sample_list[isample] != "QFlip")
						{
							//Luminosity rescaling
							if(writeTemplate_forAllCouplingPoints)
							{
								for(int ilhe=0; ilhe<v_couplingPoints.size(); ilhe++)
								{
									vvv_histo_chan_syst_LHE[ichan][isyst][ilhe]->Scale(luminosity_rescale);
								}
							}
							else
							{
								if(template_name == "2D") {vvv_histo_chan_syst_var2D[ichan][isyst][ivar]->Scale(luminosity_rescale);}
								else {vvv_histo_chan_syst_var[ichan][isyst][ivar]->Scale(luminosity_rescale);}

								//ITC rescaling to SM xsec (for Combine)
								if(!use_SM_coupling && rescale_ITC_to_SM_xsec) //If using SM weights, already scaled to SM xsec! (xsec ratio included in SM reweights)
								{
									if(sample_list[isample].Contains("tHq")) {vvv_histo_chan_syst_var[ichan][isyst][ivar]->Scale(0.07096 / 0.7927);}
									else if(sample_list[isample].Contains("tHW")) {vvv_histo_chan_syst_var[ichan][isyst][ivar]->Scale(0.01561 / 0.1472);}
								}

								//tH-FCNC rescaling
								if(sample_list[isample] == "tH_ST_hut_FCNC") {vvv_histo_chan_syst_var[ichan][isyst][ivar]->Scale(SF_ST_hut_FCNC);}
								else if(sample_list[isample] == "tH_ST_hct_FCNC") {vvv_histo_chan_syst_var[ichan][isyst][ivar]->Scale(SF_ST_hct_FCNC);}
								else if(sample_list[isample].Contains("tH_TT") && sample_list[isample].Contains("FCNC")) {vvv_histo_chan_syst_var[ichan][isyst][ivar]->Scale(SF_TT_FCNC);}
							}
						} //MC
					} //Var
				} //syst
			} //ichan

			//Check if sample is to be merged with others ! If yes, then don't force it positive (will modify the merged histo afterwards if still needed)
			bool merge_this_sample = false;
			if(!isample && sample_groups.size() > 1 && sample_groups[isample+1] == sample_groups[isample]) {merge_this_sample = true;}
			else if(isample == sample_list.size()-1 && sample_groups[isample-1] == sample_groups[isample]) {merge_this_sample = true;}
			else if(isample > 0 && isample < sample_list.size()-1 && (sample_groups[isample+1] == sample_groups[isample] || sample_groups[isample-1] == sample_groups[isample])) {merge_this_sample = true;}

			//-- If integral of histo is negative, set to 0 (else COMBINE crashes) -- must mean that norm is close to 0 anyway
			if(force_normTemplate_positive && !merge_this_sample)
			{
				for(int ichan=0; ichan<channel_list.size(); ichan++)
				{
					if(writeTemplate_forAllCouplingPoints && nLep_cat == "3l" && ichan != 0) {continue;} //don't care about splitting in 3l
					else if(writeTemplate_forAllCouplingPoints && nLep_cat == "2l" && ichan == 0) {continue;} //only care about split channels in 2l

					if((sample_list[isample].Contains("Flip") || sample_list[isample].Contains("GammaConv")) && (channel_list[ichan] == "uuu" || channel_list[ichan] == "uu" || channel_list[ichan].Contains("mm_")) ) {continue;} //Only in channels with ele
					else if(analysis_type == "ttH" && (region == "CR_WZ" || region == "CR_ZZ" || nLep_cat == "4l") && channel_list[ichan] != "") {continue;} //no subcat for WZ/ZZ CRs, 4l SR

					for(int isyst=0; isyst<syst_list.size(); isyst++)
					{
						for(int ivar=0; ivar<total_var_list.size(); ivar++)
						{
							//-- Protections : not all syst weights apply to all samples, etc.
							if((sample_list[isample] == "DATA" || sample_list[isample] == "QFlip" || systTree_list[itree] != "") && syst_list[isyst] != "") {break;} //nominal only
							// else if(channel_list.size() > 1 && channel_list[ichan] == "" && (systTree_list[itree] != "" || syst_list[isyst] != "")) {break;} //nominal only
							else if((syst_list[isyst].Contains("FR_") || syst_list[isyst].Contains("Clos")) && !sample_list[isample].Contains("Fake") ) {continue;}
							else if(sample_list[isample].Contains("Fake") && !syst_list[isyst].Contains("FR_") && !syst_list[isyst].Contains("Clos") && syst_list[isyst] != "") {continue;}
							else if(syst_list[isyst].Contains("Clos") && nLep_cat == "3l") {continue;}
							else if(syst_list[isyst].Contains("ttH") && !sample_list[isample].Contains("ttH")) {continue;}
							else if(syst_list[isyst].Contains("ttW") && !sample_list[isample].Contains("ttW")) {continue;}
							else if(syst_list[isyst].Contains("ttZ") && !sample_list[isample].Contains("ttZ") && !sample_list[isample].Contains("TTJets")) {continue;}
							else if(syst_list[isyst].Contains("_e_") && (channel_list[ichan].Contains("mm_") || channel_list[ichan] == "uu" || channel_list[ichan] == "uuu")) {continue;}
							else if(syst_list[isyst].Contains("_m_") && (channel_list[ichan].Contains("ee_") || channel_list[ichan] == "ee" || channel_list[ichan] == "eee")) {continue;}

							if(writeTemplate_forAllCouplingPoints)
							{
								for(int ilhe=0; ilhe<v_couplingPoints.size(); ilhe++)
								{
									if(vvv_histo_chan_syst_LHE[ichan][isyst][ilhe]->Integral() <= 0)
									{
										Set_Histogram_FlatZero(vvv_histo_chan_syst_LHE[ichan][isyst][ilhe], true, "");
										cout<<"(Sample "<<sample_list[isample]<<" / tree "<<systTree_list[itree]<<" / syst "<<syst_list[isyst]<<")"<<endl;
									}
								}
							}
							else if(vvv_histo_chan_syst_var[ichan][isyst][ivar]->Integral() <= 0)
							{
								// cout<<endl<<FRED("Integral = "<<vvv_histo_chan_syst_var[ichan][isyst][ivar]->Integral()<<" (<= 0) ! Distribution set to ~>0 (flat) to avoid crashes in COMBINE")<<endl;

								Set_Histogram_FlatZero(vvv_histo_chan_syst_var[ichan][isyst][ivar], true, "");
								cout<<"(Sample "<<sample_list[isample]<<" / tree "<<systTree_list[itree]<<" / channel "<<channel_list[ichan]<<" / syst "<<syst_list[isyst]<<")"<<endl;
							}
						}
					}
				}
			}


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

				if(writeTemplate_forAllCouplingPoints && nLep_cat == "3l" && ichan != 0) {continue;} //don't care about splitting in 3l
				if(writeTemplate_forAllCouplingPoints && nLep_cat == "2l" && ichan == 0) {continue;} //only care about split channels in 2l

				if((sample_list[isample].Contains("Flip") || sample_list[isample].Contains("GammaConv")) && (channel_list[ichan] == "uuu" || channel_list[ichan] == "uu" || channel_list[ichan].Contains("mm_")) ) {continue;} //Only in channels with ele
				else if(analysis_type == "ttH" && (region == "CR_WZ" || region == "CR_ZZ" || nLep_cat == "4l") && channel_list[ichan] != "") {continue;} //no subcat for WZ/ZZ CRs, 4l SR

				for(int isyst=0; isyst<syst_list.size(); isyst++)
				{
					// cout<<"isyst "<<isyst<<endl;

					for(int ivar=0; ivar<total_var_list.size(); ivar++)
					{
						//-- Protections : not all syst weights apply to all samples, etc.
						if((sample_list[isample] == "DATA" || sample_list[isample] == "QFlip" || systTree_list[itree] != "") && syst_list[isyst] != "") {break;} //nominal only
						// else if(channel_list.size() > 1 && channel_list[ichan] == "" && (systTree_list[itree] != "" || syst_list[isyst] != "")) {break;} //nominal only
						else if((syst_list[isyst].Contains("FR_") || syst_list[isyst].Contains("Clos")) && !sample_list[isample].Contains("Fake") ) {continue;}
						else if(syst_list[isyst].Contains("Clos") && nLep_cat == "3l") {continue;}
						else if(sample_list[isample].Contains("Fake") && !syst_list[isyst].Contains("FR_") && !syst_list[isyst].Contains("Clos") && syst_list[isyst] != "") {continue;}
						else if(syst_list[isyst].Contains("ttH") && !sample_list[isample].Contains("ttH")) {continue;}
						else if(syst_list[isyst].Contains("ttW") && !sample_list[isample].Contains("ttW")) {continue;}
						else if(syst_list[isyst].Contains("ttZ") && !sample_list[isample].Contains("ttZ") && !sample_list[isample].Contains("TTJets")) {continue;}
						else if(syst_list[isyst].Contains("_e_") && (channel_list[ichan].Contains("mm_") || channel_list[ichan] == "uu" || channel_list[ichan] == "uuu")) {continue;}
						else if(syst_list[isyst].Contains("_m_") && (channel_list[ichan].Contains("ee_") || channel_list[ichan] == "ee" || channel_list[ichan] == "eee")) {continue;}

						// cout<<"Sample "<<sample_list[isample]<<" / Channel "<<channel_list[ichan]<<" / Syst "<<syst_list[isyst]<<endl;;

						//-- Rescale JES to nominal (shape-only)
						// if(systTree_list[itree] == "" && syst_list[isyst] == "")
						// {
						// 	v_integralsRescale[ichan] = vvv_histo_chan_syst_var[ichan][isyst]->Integral();
						// }
						// else if(systTree_list[itree].Contains("JES"))
						// {
						// 	if(vvv_histo_chan_syst_var[ichan][isyst]->Integral() != 0) {vvv_histo_chan_syst_var[ichan][isyst]->Scale(v_integralsRescale[ichan] / vvv_histo_chan_syst_var[ichan][isyst]->Integral());}
						// }

						TString output_histo_name;
						if(makeHisto_inputVars)
						{
							output_histo_name = total_var_list[ivar] + "_" + nLep_cat + "_" + region;
							if(channel_list[ichan] != "") {output_histo_name+= "_" + channel_list[ichan];}
							output_histo_name+= "__" + samplename;
							if(syst_list[isyst] != "") {output_histo_name+= "__" + syst_list[isyst];}
							else if(systTree_list[itree] != "") {output_histo_name+= "__" + systTree_list[itree];}
						}
						else
						{
							output_histo_name = classifier_name + template_name + "_" + nLep_cat + "_" + region;
							if(channel_list[ichan] != "") {output_histo_name+= "_" + channel_list[ichan];}
							output_histo_name+= "__" + samplename;
							if(syst_list[isyst] != "") {output_histo_name+= "__" + syst_list[isyst];}
							else if(systTree_list[itree] != "") {output_histo_name+= "__" + systTree_list[itree];}
						}

						if(writeTemplate_forAllCouplingPoints)
						{
							for(int ilhe=0; ilhe<v_couplingPoints.size(); ilhe++)
							{
								// cout<<"ilhe "<<ilhe<<endl;

								output_file_name = "./outputs/templates_allCouplingPoints/tHq_";
								if(nLep_cat == "2l")
								{
									if(channel_list[ichan] == "uu" || channel_list[ichan] == "mm") {output_file_name+= "2lss_mm";}
									else if(channel_list[ichan] == "eu" || channel_list[ichan] == "ue" || channel_list[ichan] == "em" || channel_list[ichan] == "me") {output_file_name+= "2lss_em";}
								}
								else {output_file_name+= "3l";}
								output_file_name+= "_"+v_couplingPoints[ilhe]+".input.root";

								TFile* file_output_tmp = TFile::Open(output_file_name, "UPDATE");
								file_output_tmp->cd();
								// cout<<"-- Opened file : "<<output_file_name<<endl;

								//CERN wants all templates rescaled to SM xsec
								float SF_xsec = Get_scalingFactor_toSMxsec(v_couplingPoints[ilhe], sample_list[isample]);
								vvv_histo_chan_syst_LHE[ichan][isyst][ilhe]->Scale(SF_xsec);

								TString outname = Get_CERN_CombineNamingConvention(nLep_cat, sample_list[isample], syst_list[isyst]);
								vvv_histo_chan_syst_LHE[ichan][isyst][ilhe]->Write(outname, TObject::kOverwrite);
								// cout<<"--> Written histo '"<<outname<<"'"<<endl;

								delete vvv_histo_chan_syst_LHE[ichan][isyst][ilhe]; vvv_histo_chan_syst_LHE[ichan][isyst][ilhe] = NULL;

								file_output_tmp->Close();

								// cout<<"... have overwritten "<<v_couplingPoints.size()<<"histograms !"<<endl;
							}
						}
						else
						{
							file_output->cd();

							if(template_name == "2D") {vvv_histo_chan_syst_var2D[ichan][isyst][ivar]->Write(output_histo_name);}
							else {vvv_histo_chan_syst_var[ichan][isyst][ivar]->Write(output_histo_name);}

							// if(sample_list[isample].Contains("Conv") )
							// {
								// cout<<"-- Write histo "<<output_histo_name<<" (integral = "<<vvv_histo_chan_syst_var[ichan][isyst][ivar]->Integral()<<", "<<vvv_histo_chan_syst_var[ichan][isyst][ivar]->GetEntries()<<" entries)"<<endl;
							// }

							delete vvv_histo_chan_syst_var[ichan][isyst][ivar]; vvv_histo_chan_syst_var[ichan][isyst][ivar] = NULL;
							if(template_name == "2D") {delete vvv_histo_chan_syst_var[ichan][isyst][ivar]; vvv_histo_chan_syst_var[ichan][isyst][ivar] = NULL;}
						}

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
	if(!read_preComputed_MVA)
	{
		if(template_name == "2Dlin" || template_name == "2D")
		{
			delete reader1; reader1 = NULL;
			delete reader2; reader2 = NULL;
		}

		delete reader1; reader2 = NULL;
	}



//   ##   ###### ##### ###### #####
//  #  #  #        #   #      #    #
// #    # #####    #   #####  #    #
// ###### #        #   #      #####
// #    # #        #   #      #   #
// #    # #        #   ###### #    #

//--------------------------------------------
	//-- Merge some samples together for fit, like ttH analysis
	if(!makeHisto_inputVars) //Needed for fit only
	{
		if(!writeTemplate_forAllCouplingPoints)
		{
			Merge_Templates_ByProcess(output_file_name, template_name, force_normTemplate_positive);
		}

		//-- Modify histograms after they have been produced in this function (e.g. rescale, mirror templates, etc.)
		for(int ilhe=0; ilhe<v_couplingPoints.size(); ilhe++)
		{
			if(!writeTemplate_forAllCouplingPoints && ilhe != 0) {continue;} //ilhe loop only for all couplings

			for(int ichan=0; ichan<channel_list.size(); ichan++)
			{
				if(writeTemplate_forAllCouplingPoints)
				{
					if(nLep_cat == "3l" && channel_list[ichan] != "") {continue;} //don't care about splitting in 3l
					else if(nLep_cat == "2l" && channel_list[ichan] == "") {continue;} //only care about split channels in 2l

					output_file_name = "./outputs/templates_allCouplingPoints/tHq_";
					if(nLep_cat == "2l")
					{
						if(channel_list[ichan] == "uu" || channel_list[ichan] == "mm") {output_file_name+= "2lss_mm";}
						else if(channel_list[ichan] == "eu" || channel_list[ichan] == "ue" || channel_list[ichan] == "em" || channel_list[ichan] == "me") {output_file_name+= "2lss_em";}
					}
					else {output_file_name+= "3l";}
					output_file_name+= "_"+v_couplingPoints[ilhe]+".input.root";
				}

				Modify_Template_Histograms(output_file_name, template_name, force_normTemplate_positive, writeTemplate_forAllCouplingPoints);
			}
		}
	}

	//The 'Fakes_MC' sample was added to list only for creation/substraction, need to remove it now
	if(substract_MCPrompt_from_DD)
	{
		sample_list.pop_back(); sample_groups.pop_back();
	}

	if(copy_templates_forCERN)
	{
		if(nLep_cat == "2l")
		{
			//Copies every template that was just saved, but in a different file using the CERN combine conventions
			Copy_Templates_withCERNconventions(template_name, rescale_ITC_to_SM_xsec, "uu");
			Copy_Templates_withCERNconventions(template_name, rescale_ITC_to_SM_xsec, "ue");

			//Copies template file (produced above) for all kt/kv points (nominal is the same for all)
			if(copy_forAllCouplings)
			{
				Copy_TemplateFile_allCouplings(v_couplingPoints, nLep_cat, "uu");
				Copy_TemplateFile_allCouplings(v_couplingPoints, nLep_cat, "eu");
			}
		}
		else
		{
			Copy_Templates_withCERNconventions(template_name, rescale_ITC_to_SM_xsec, "");

			if(copy_forAllCouplings)
			{
				Copy_TemplateFile_allCouplings(v_couplingPoints, nLep_cat, "");
			}
		}
	}

	return;
}
























//--------------------------------------------
 // #    # ###### #####   ####  ######    ##### ###### #    # #####  #        ##   ##### ######  ####
 // ##  ## #      #    # #    # #           #   #      ##  ## #    # #       #  #    #   #      #
 // # ## # #####  #    # #      #####       #   #####  # ## # #    # #      #    #   #   #####   ####
 // #    # #      #####  #  ### #           #   #      #    # #####  #      ######   #   #           #
 // #    # #      #   #  #    # #           #   #      #    # #      #      #    #   #   #      #    #
 // #    # ###### #    #  ####  ######      #   ###### #    # #      ###### #    #   #   ######  ####
//--------------------------------------------

/**
 * This code is producing/plotting histos for each subprocess independently
 * However ttH is plotting/fitting groups of processes (e.g. "Rares", "EWK", ...)
 * Therefore this is a possible fix : continue to store and plot individual subprocesses...
 * ... BUT, on top, merge the relevant subprocesses together, so that these merged templates can be used for the Combine fit
 * NB : don't merge EWK anymore (only merged in plots)
 * NB : here the order of loops is important because we sum histograms recursively, and the 'sample_list' loop must be the most nested one !
 */
void TopEFT_analysis::Merge_Templates_ByProcess(TString filename, TString template_name, bool force_normTemplate_positive)
{
	if(template_name != "2Dlin" && template_name != "ttV" && template_name != "ttbar") {return;}

	cout<<endl<<FYEL("==> Merging 'Rares'/'ttZ'/'Fakes'/etc. templates together...")<<endl;

	if(!Check_File_Existence(filename) ) {cout<<endl<<FRED("File "<<filename<<" not found! Abort template merging !")<<endl; return;}
	TFile* f = TFile::Open(filename, "UPDATE");

	cout<<__LINE__<<endl;

	//NB :here the order of loops is important because we sum histograms recursively ! The 'sample_list' loop must be the most nested one !
	for(int ichan=0; ichan<channel_list.size(); ichan++)
	{
		if(analysis_type == "ttH" && (region == "CR_WZ" || region == "CR_ZZ" || nLep_cat == "4l") && channel_list[ichan] != "") {continue;} //no subcat for WZ/ZZ CRs, 4l SR

		for(int itree=0; itree<systTree_list.size(); itree++)
		{
			// if(systTree_list[itree] != "" && channel_list.size() > 1 && channel_list[ichan] == "") {continue;}

			for(int isyst=0; isyst<syst_list.size(); isyst++)
			{
				// if(((channel_list.size() > 1 && channel_list[ichan] == "") || systTree_list[itree] != "") && syst_list[isyst] != "") {continue;}
				if(systTree_list[itree] != "" && syst_list[isyst] != "") {continue;}

				TH1F* h_merging = 0;

				for(int isample=0; isample<sample_list.size(); isample++)
				{
					//-- Protections : not all syst weights apply to all samples, etc.
					if((sample_list[isample] == "DATA" || sample_list[isample] == "QFlip") && syst_list[isyst] != "") {continue;} //nominal only
					else if(systTree_list[itree] != "" && (sample_list[isample] == "DATA" || sample_list[isample] == "QFlip" || sample_list[isample].Contains("Fakes") )) {continue;}
					else if((sample_list[isample].Contains("Flip") || sample_list[isample].Contains("GammaConv")) && (channel_list[ichan] == "uuu" || channel_list[ichan] == "uu" || channel_list[ichan].Contains("mm_")) ) {continue;} //Only in channels with ele
					else if(nLep_cat == "3l" && sample_list[isample] == "QFlip") {continue;}
					else if((syst_list[isyst].Contains("FR_") || syst_list[isyst].Contains("Clos")) && !sample_list[isample].Contains("Fake") ) {continue;}
					else if(sample_list[isample].Contains("Fake") && !syst_list[isyst].Contains("FR_") && !syst_list[isyst].Contains("Clos") && syst_list[isyst] != "") {continue;}
					else if(syst_list[isyst].Contains("Clos") && (nLep_cat == "3l" || channel_list[ichan] == "")) {continue;} //No clos shape syst in 3l, and not implemented for inclusive channel
					else if(syst_list[isyst].Contains("ttH") && !sample_list[isample].Contains("ttH")) {continue;}
					else if(syst_list[isyst].Contains("ttW") && !sample_list[isample].Contains("ttW")) {continue;}
					else if(syst_list[isyst].Contains("ttZ") && !sample_list[isample].Contains("ttZ") && !sample_list[isample].Contains("TTJets")) {continue;}
					else if(syst_list[isyst].Contains("_e_") && (channel_list[ichan].Contains("mm_") || channel_list[ichan] == "uu" || channel_list[ichan] == "uuu")) {continue;}
					else if(syst_list[isyst].Contains("_m_") && (channel_list[ichan].Contains("ee_") || channel_list[ichan] == "ee" || channel_list[ichan] == "eee")) {continue;}

					// cout<<endl<<"Syst "<<syst_list[isyst]<<systTree_list[itree]<<" / chan "<<channel_list[ichan]<<" / sample "<<sample_list[isample]<<endl;

					//Check if this sample needs to be merged, i.e. if the samples before/after belong to the same "group of samples"
					bool merge_this_sample = false;
					if(!isample && sample_groups.size() > 1 && sample_groups[isample+1] == sample_groups[isample]) {merge_this_sample = true;}
					else if(isample == sample_list.size()-1 && sample_groups[isample-1] == sample_groups[isample]) {merge_this_sample = true;}
					else if(isample > 0 && isample < sample_list.size()-1 && (sample_groups[isample+1] == sample_groups[isample] || sample_groups[isample-1] == sample_groups[isample])) {merge_this_sample = true;}

					// cout<<"merge_this_sample "<<merge_this_sample<<endl;
					if(!merge_this_sample) {continue;} //Only care about samples to merge : others are already stored in file

					TString samplename = sample_list[isample];
					if(samplename == "DATA") {samplename = "data_obs";}

					TString histoname = classifier_name + template_name + "_" + nLep_cat + "_" + region;
					if(channel_list[ichan] != "") {histoname+= "_" + channel_list[ichan];}
					histoname+= "__" + samplename;
					if(syst_list[isyst] != "") {histoname+= "__" + syst_list[isyst];}
					else if(systTree_list[itree] != "") {histoname+= "__" + systTree_list[itree];}

					// cout<<"histoname "<<histoname<<endl;

					if(!f->GetListOfKeys()->Contains(histoname) )
					{
						cout<<FRED("Histo "<<histoname<<" not found in file "<<filename<<" !")<<endl;
						// cout<<"Can not produce merged histo !"<<endl;
					 	continue;
					}

					TH1F* h_tmp = (TH1F*) f->Get(histoname);
					// cout<<"h_tmp->Integral() = "<<h_tmp->Integral()<<endl;

					int factor = +1; //Addition
					if(sample_list[isample] == "Fakes_MC") {factor = -1;} //Substraction of 'MC Fakes' (prompt contribution to fakes)

					if(h_tmp != 0)
					{
						if(!h_merging) {h_merging = (TH1F*) h_tmp->Clone();}
						else {h_merging->Add(h_tmp, factor);}
					}
					else {cout<<"h_tmp null !"<<endl;}

					// cout<<"h_merging->Integral() = "<<h_merging->Integral()<<endl;

					delete h_tmp; h_tmp = 0;
					if(!h_merging) {cout<<"h_merging is null ! Fix this first"<<endl; return;}

					//Check if next sample will be merged with this one, or else if must write the histogram
					if(isample < sample_list.size()-1 && sample_groups[isample+1] == sample_groups[isample]) {continue;}
					else
					{
						if(force_normTemplate_positive)
						{
							//If integral of histo is negative, set to 0 (else COMBINE crashes) -- must mean that norm is close to 0 anyway
							if(h_merging->Integral() <= 0)
							{
								// cout<<endl<<"While merging processes by groups ('Rares'/...) :"<<endl<<FRED(" h_merging->Integral() = "<<h_merging->Integral()<<" (<= 0) ! Distribution set to ~>0 (flat), to avoid crashes in COMBINE !")<<endl;

								Set_Histogram_FlatZero(h_merging, true, "h_merging");
								cout<<"(Syst "<<syst_list[isyst]<<systTree_list[itree]<<" / chan "<<channel_list[ichan]<<" / sample "<<sample_list[isample]<<")"<<endl;
							}
						}
						// cout<<"h_merging->Integral() = "<<h_merging->Integral()<<endl;

						TString histoname_new = classifier_name + template_name + "_" + nLep_cat + "_" + region;
						if(channel_list[ichan] != "") {histoname_new+="_"  + channel_list[ichan];}
						histoname_new+= "__" + sample_groups[isample];
						if(syst_list[isyst] != "") {histoname_new+= "__" + syst_list[isyst];}
						else if(systTree_list[itree] != "") {histoname_new+= "__" + systTree_list[itree];}

						f->cd();
						h_merging->Write(histoname_new, TObject::kOverwrite);

						// if(sample_groups[isample] == "Fakes")
						// cout<<"- Writing merged histo "<<histoname_new<<" with integral "<<h_merging->Integral()<<endl;

						delete h_merging; h_merging = 0;
					} //write histo
				} //sample loop
			} //syst loop
		} //tree loop
	} //channel loop

	f->Close();

	cout<<endl<<FYEL("... Done")<<endl<<endl<<endl;

	return;
}



















//--------------------------------------------
// ##     ##  #######  ########  #### ######## ##    ##    ##     ## ####  ######  ########
// ###   ### ##     ## ##     ##  ##  ##        ##  ##     ##     ##  ##  ##    ##    ##
// #### #### ##     ## ##     ##  ##  ##         ####      ##     ##  ##  ##          ##
// ## ### ## ##     ## ##     ##  ##  ######      ##       #########  ##   ######     ##
// ##     ## ##     ## ##     ##  ##  ##          ##       ##     ##  ##        ##    ##
// ##     ## ##     ## ##     ##  ##  ##          ##       ##     ##  ##  ##    ##    ##
// ##     ##  #######  ########  #### ##          ##       ##     ## ####  ######     ##
//--------------------------------------------

/**
 * Some templates need to be created or modified after producing the 'main bunch' of templates
 * E.g : forward jet syst is mirrored -> Create down from central and variation, and rescale to nominal
 * E.g : Fakes Closure and Theory shape syst are mirrored -> Create down from central and variation
 * E.g : Scale syst is shape-only => Need to rescale to nominal
 * ...etc...
 *
 * NB : this function is (a priori) only used to manipulate templates in order to obtain correct limits
 * Therefore, we care only about the final processes used in Combine, not the individual processes
 * -> E.g. will only manipulate the 'Rares' merged templates, and not all the individual templates of each processes (TTTT, WZZ, etc.) ; same for TTZ=ttZ+TTJets, etc.
 * 'use_CERN_namings' <-> when producing templates for all coupling points, naming conventions are different -> don't want to print warnings. Also, need to call different histograms...
 */
void TopEFT_analysis::Modify_Template_Histograms(TString filename, TString template_name, bool force_normTemplate_positive, bool use_CERN_namings)
{
	if(template_name != "2Dlin" && template_name != "ttV" && template_name != "ttbar") {return;}

	cout<<endl<<FYEL("==> Creating mirror templates, rescale some histos to nominal, ...")<<endl;
	cout<<endl<<"(File : "<<filename<<")"<<endl;

	if(!Check_File_Existence(filename) ) {cout<<endl<<FRED("File "<<filename<<" not found! Abort template merging !")<<endl; return;}
	TFile* f = TFile::Open(filename, "UPDATE");

	//NB : as explained in func description, only care about the merged samples used by Combine, not the individual processes (if merged)
	//Therefore, run on the whole "groups of processes" instead of individual processes
	for(int isample=0; isample<sample_groups.size(); isample++)
	{
		if(isample > 0 && sample_groups[isample] == sample_groups[isample-1]) {continue;} //If many processes belong to same group, process group only once !
		if(sample_groups[isample] == "DATA") {continue;}

		for(int ichan=0; ichan<channel_list.size(); ichan++)
		{
			if((sample_list[isample].Contains("Flip") || sample_list[isample].Contains("GammaConv")) && (channel_list[ichan] == "uuu" || channel_list[ichan] == "uu" || channel_list[ichan].Contains("mm_")) ) {continue;} //Only in channels with ele
			else if(analysis_type == "ttH" && (region == "CR_WZ" || region == "CR_ZZ" || nLep_cat == "4l") && channel_list[ichan] != "") {continue;} //no subcat for WZ/ZZ CRs, 4l SR

			for(int itree=0; itree<systTree_list.size(); itree++)
			{
				// cout<<"Tree "<<systTree_list[itree]<<endl;

				if(systTree_list[itree] != "" && (sample_groups[isample] == "DATA" || sample_groups[isample] == "QFlip" || sample_groups[isample].Contains("Fakes")) ) {continue;} //For MC only

				for(int isyst=0; isyst<syst_list.size(); isyst++)
				{
					//-- Protections : not all syst weights apply to all samples, etc.
					if((sample_groups[isample] == "DATA" || sample_groups[isample] == "QFlip" || systTree_list[itree] != "") && syst_list[isyst] != "") {break;} //nominal only
					// else if(channel_list.size() > 1 && channel_list[ichan] == "" && (systTree_list[itree] != "" || syst_list[isyst] != "")) {continue;}
					else if(nLep_cat == "3l" && sample_groups[isample] == "QFlip") {continue;}
					else if((syst_list[isyst].Contains("FR_") || syst_list[isyst].Contains("Clos")) && !sample_groups[isample].Contains("Fake") ) {continue;}
					else if(sample_groups[isample].Contains("Fake") && !syst_list[isyst].Contains("FR_") && !syst_list[isyst].Contains("Clos") && syst_list[isyst] != "") {continue;}
					else if(syst_list[isyst].Contains("Clos") && nLep_cat == "3l") {continue;}
					else if(syst_list[isyst].Contains("ttH") && !sample_groups[isample].Contains("ttH")) {continue;}
					else if(syst_list[isyst].Contains("ttW") && !sample_groups[isample].Contains("ttW")) {continue;}
					else if(syst_list[isyst].Contains("ttZ") && !sample_groups[isample].Contains("ttZ", TString::kIgnoreCase) && !sample_groups[isample].Contains("TTJets")) {continue;}
					else if(syst_list[isyst].Contains("_e_") && (channel_list[ichan].Contains("mm_") || channel_list[ichan] == "uu" || channel_list[ichan] == "uuu")) {continue;}
					else if(syst_list[isyst].Contains("_m_") && (channel_list[ichan].Contains("ee_") || channel_list[ichan] == "ee" || channel_list[ichan] == "eee")) {continue;}

					if(!syst_list[isyst].Contains("fwdJet") && !syst_list[isyst].Contains("Clos") && !syst_list[isyst].Contains("thu_shape")) {continue;} //only affect these syst
					if(syst_list[isyst] == "fwdJetDown") {continue;} //this variation is initiallity set to nominal ; generate correct variation within this func from 'Up' variation

					if(use_CERN_namings && !sample_groups[isample].Contains("_h")) {continue;} //When making templates for all couplings, only change the signals (not other processes)

					// cout<<endl<<"Syst "<<syst_list[isyst]<<systTree_list[itree]<<" / chan "<<channel_list[ichan]<<" / sample "<<sample_list[isample]<<endl;

					TH1F* h_nominal = 0; //Nominal template
					TH1F* h_Up = 0; //Taken as the varied histo computed in Produce_Templates()
					TH1F* h_Down = 0; //Created by this function

					TString samplename = sample_groups[isample];
					TString histoname = classifier_name + template_name + "_" + nLep_cat + "_" + region;
					if(channel_list[ichan] != "") {histoname+= "_" + channel_list[ichan];}
					histoname+= "__" + samplename;
					// cout<<endl<<"histoname "<<histoname<<endl;
					TString histoname_withSyst = histoname + "__" + syst_list[isyst];

					if(use_CERN_namings)
					{
						histoname = Get_CERN_CombineNamingConvention(nLep_cat, sample_groups[isample], "");
						histoname_withSyst = Get_CERN_CombineNamingConvention(nLep_cat, sample_groups[isample], syst_list[isyst]);
					}

					if(!f->GetListOfKeys()->Contains(histoname) ) {cout<<FRED("Histo "<<histoname<<" not found in file "<<filename<<" ! Can not produce mirror variation for histo : "<<histoname<<"__"<<syst_list[isyst]<<"")<<endl; continue;}
					if(!f->GetListOfKeys()->Contains(histoname_withSyst) ) {cout<<FRED("Histo "<<histoname<<"__"<<syst_list[isyst]<<" not found in file "<<filename<<" ! Can not produce mirror variation for histo : "<<histoname<<"__"<<syst_list[isyst]<<"")<<endl; continue;}

					// cout<<"histoname "<<histoname<<endl;
					// cout<<"histoname_withSyst "<<histoname_withSyst<<endl;

					h_nominal = (TH1F*) f->Get(histoname)->Clone();
					h_Up = (TH1F*) f->Get(histoname_withSyst)->Clone(); //Computed 1 varied histo in Produce_Templates <-> associate it arbitrarily with "Up" variation

					//Create 'Down' histo //-- shape only ?
					if(syst_list[isyst].Contains("Clos"))
					{
						//In FCNC analysis, we want to inflate fake clos shape systematics by 20% (done bin by bin)
                        // if(analysis_type == "FCNC") {Inflate_Syst_inShapeTemplate(h_Up, h_nominal, 0.2);}
                        if(analysis_type == "FCNC") {Inflate_Syst_inShapeTemplate(h_Up, h_nominal, 1);} //100%

						Get_TemplateSymm_Histogram(h_nominal, h_Up, h_Down, false);
					}
					else if(syst_list[isyst].Contains("thu_shape")) {Get_TemplateSymm_Histogram(h_nominal, h_Up, h_Down, false);}
					else if(syst_list[isyst].Contains("fwdJet")) {Get_Mirror_Histogram(h_nominal, h_Up, h_Down, true);} //Mirror histo, shape-only

                    if(!h_Down && !use_CERN_namings)
					{
						cout<<endl<<"Syst "<<syst_list[isyst]<<systTree_list[itree]<<" / chan "<<channel_list[ichan]<<" / sample "<<sample_list[isample]<<endl;
						cout<<FRED("Error : h_Down mirror template is null ! Skip...")<<endl;
						continue;
					}

                    if(force_normTemplate_positive)
                    {
                        //If integral of histo is negative, set to 0 (else COMBINE crashes) -- must mean that norm is close to 0 anyway
                        if(h_Down->Integral() <= 0 && !use_CERN_namings)
                        {
							// cout<<endl<<FRED("h_Down->Integral() = "<<h_Down->Integral()<<" <= 0 ! Distribution set to ~>0 (flat), to avoid crashes in COMBINE !")<<endl;
							cout<<endl<<"(Syst "<<syst_list[isyst]<<systTree_list[itree]<<" / chan "<<channel_list[ichan]<<" / sample "<<sample_list[isample]<<")"<<endl;

							Set_Histogram_FlatZero(h_Down, true, "h_Down");
                        }
                    }

					f->cd();

					TString outname_up = histoname + "__" + syst_list[isyst] + "Up";
					if(use_CERN_namings) {outname_up = Get_CERN_CombineNamingConvention(nLep_cat, sample_groups[isample], syst_list[isyst]+"Up");}
					if(syst_list[isyst] == "fwdJetUp") {outname_up = histoname + "__fwdJetUp";}
					h_Up->Write(outname_up, TObject::kOverwrite); //may have changed norm
					// cout<<"-> Wrote "<<outname_up<<", Integral = "<<h_Up->Integral()<<endl;

					TString outname_down = histoname + "__" + syst_list[isyst] + "Down";
					if(use_CERN_namings) {outname_down = Get_CERN_CombineNamingConvention(nLep_cat, sample_groups[isample], syst_list[isyst]+"Down");}
					if(syst_list[isyst] == "fwdJetUp") {outname_down = histoname + "__fwdJetDown";}
					h_Down->Write(outname_down, TObject::kOverwrite); //new histo
					// cout<<"-> Wrote "<<outname_down<<", Integral = "<<h_Down->Integral()<<endl;

					delete h_nominal; h_nominal = 0;
					delete h_Up; h_Up = 0;
					delete h_Down; h_Down = 0;
				} //syst loop
			} //tree loop
		} //channel loop
	} //sample loop

	f->Close();

	cout<<endl<<FYEL("... Done")<<endl<<endl<<endl;

	return;
}








//--------------------------------------------
//  ######  ######## ########  ##    ##     ######   #######  ##    ## ##     ## ######## ##    ## ######## ####  #######  ##    ##  ######
// ##    ## ##       ##     ## ###   ##    ##    ## ##     ## ###   ## ##     ## ##       ###   ##    ##     ##  ##     ## ###   ## ##    ##
// ##       ##       ##     ## ####  ##    ##       ##     ## ####  ## ##     ## ##       ####  ##    ##     ##  ##     ## ####  ## ##
// ##       ######   ########  ## ## ##    ##       ##     ## ## ## ## ##     ## ######   ## ## ##    ##     ##  ##     ## ## ## ##  ######
// ##       ##       ##   ##   ##  ####    ##       ##     ## ##  ####  ##   ##  ##       ##  ####    ##     ##  ##     ## ##  ####       ##
// ##    ## ##       ##    ##  ##   ###    ##    ## ##     ## ##   ###   ## ##   ##       ##   ###    ##     ##  ##     ## ##   ### ##    ##
//  ######  ######## ##     ## ##    ##     ######   #######  ##    ##    ###    ######## ##    ##    ##    ####  #######  ##    ##  ######
//--------------------------------------------

/**
 * Copy all templates into a separate file, using the same naming convention as CERN
 */
void TopEFT_analysis::Copy_Templates_withCERNconventions(TString template_name, bool rescale_ITC_to_SM_xsec, TString channel)
{
	// if(nLep_cat != "3l") {return;}

	cout<<endl<<FYEL("==> Copying templates to new separate file, using same naming conventions as CERN ...")<<endl;

	TString input_name = "./outputs/Templates_"+classifier_name+template_name+"_"+nLep_cat+"_"+region+".root";
	if(!Check_File_Existence(input_name) ) {cout<<BOLD(FRED("Input file "<<input_name<<" not found ! Abort"))<<endl; return;}
	TFile* f_input = TFile::Open(input_name, "READ");

	TString outputname = "./outputs/tHq_";
	if(nLep_cat == "2l")
	{
		if(channel == "uu" || channel == "mm") {outputname+= "2lss_mm";}
		else if(channel == "eu" || channel == "ue" || channel == "em" || channel == "me") {outputname+= "2lss_em";}
	}
	else {outputname+= "3l";}
	outputname+= ".input.root";

	TFile* f_output = TFile::Open(outputname, "RECREATE");

	//Pallabi needs all templates to be scaled to SM => in case SM rescaling was not done by main function, do it here !
	if(!use_SM_coupling && !rescale_ITC_to_SM_xsec) {rescale_ITC_to_SM_xsec = true;}
	else {rescale_ITC_to_SM_xsec = false;}

	for(int isample=0; isample<sample_groups.size(); isample++)
	{
		if(sample_groups[isample] == "QFlip" && nLep_cat == "3l") {continue;}
		if((sample_groups[isample].Contains("tHq") || sample_groups[isample].Contains("tHW") || sample_groups[isample].Contains("ttH")) && !sample_groups[isample].Contains("_h")) {continue;} //Don't care about un-split signals
		if(isample > 0 && sample_groups[isample] == sample_groups[isample-1]) {continue;} //already written

		// cout<<"sample group : "<<sample_groups[isample]<<endl;

		TString samplename = sample_groups[isample];
		if(sample_groups[isample] == "DATA") {samplename = "data_obs";}

		for(int itree=0; itree<systTree_list.size(); itree++)
		{
			if(systTree_list[itree] != "" && (sample_groups[isample] == "DATA" || sample_groups[isample] == "QFlip" || sample_groups[isample].Contains("Fakes")) ) {continue;} //For MC only

			// cout<<"tree "<<systTree_list[itree]<<endl;

			for(int ichan=0; ichan<channel_list.size(); ichan++)
			{
				if((sample_groups[isample].Contains("Flip") || sample_groups[isample].Contains("GammaConv")) && (channel_list[ichan] == "uuu" || channel_list[ichan] == "uu" || channel_list[ichan].Contains("mm_")) ) {continue;} //Only in channels with ele
				else if(analysis_type == "ttH" && (region == "CR_WZ" || region == "CR_ZZ" || nLep_cat == "4l") && channel_list[ichan] != "") {continue;} //no subcat for WZ/ZZ CRs, 4l SR

				if(nLep_cat == "3l" && channel_list[ichan] != "") {continue;} //no splitting in 3l
				else if(nLep_cat == "2l" && channel_list[ichan] != channel) {continue;} //only split samples in 2lss

				// cout<<"chan "<<channel_list[ichan]<<endl;

				for(int isyst=0; isyst<syst_list.size(); isyst++)
				{
					// cout<<"syst "<<syst_list[isyst]<<endl;

					for(int ivariation=0; ivariation<2; ivariation++) //Only use this dummy loop because for some systematics, the "up" and "down" variations are not listed (only 1, for practical reasons) => can simulate 2 variations of same syst with this loop
					{
						if(ivariation != 0 && (systTree_list[itree] != "" || syst_list[isyst] == "" || (syst_list[isyst] != "" && (syst_list[isyst].Contains("Down") || syst_list[isyst].Contains("Up")))) ) {continue;} //only if missing "up" or "down"

						// cout<<"ivariation "<<ivariation<<endl;

						//-- Protections : not all syst weights apply to all samples, etc.
						if((sample_groups[isample] == "DATA" || sample_groups[isample] == "QFlip" || systTree_list[itree] != "") && syst_list[isyst] != "") {break;} //nominal only
						else if((syst_list[isyst].Contains("FR_") || syst_list[isyst].Contains("Clos")) && !sample_groups[isample].Contains("Fake") ) {continue;}
						else if(syst_list[isyst].Contains("Clos") && nLep_cat == "3l") {continue;}
						else if(sample_groups[isample].Contains("Fake") && !syst_list[isyst].Contains("FR_") && !syst_list[isyst].Contains("Clos") && syst_list[isyst] != "") {continue;}
						else if(syst_list[isyst].Contains("ttH") && !sample_groups[isample].Contains("ttH")) {continue;}
						else if(syst_list[isyst].Contains("ttW") && !sample_groups[isample].Contains("ttW")) {continue;}
						else if(syst_list[isyst].Contains("ttZ") && !sample_groups[isample].Contains("ttZ", TString::kIgnoreCase) && !sample_groups[isample].Contains("TTJets")) {continue;}
						else if(syst_list[isyst].Contains("_e_") && (channel_list[ichan].Contains("mm_") || channel_list[ichan] == "uu" || channel_list[ichan] == "uuu")) {continue;}
						else if(syst_list[isyst].Contains("_m_") && (channel_list[ichan].Contains("ee_") || channel_list[ichan] == "ee" || channel_list[ichan] == "eee")) {continue;}

						TString output_histo_name = classifier_name + template_name + "_" + nLep_cat + "_" + region;
						if(channel_list[ichan] != "") {output_histo_name+= "_" + channel_list[ichan];}
						output_histo_name+= "__" + samplename;
						if(syst_list[isyst] != "") {output_histo_name+= "__" + syst_list[isyst];}
						else if(systTree_list[itree] != "") {output_histo_name+= "__" + systTree_list[itree];}

						if(!f_input->GetListOfKeys()->Contains(output_histo_name) ) {cout<<FRED("Histo "<<output_histo_name<<" not found in file "<<input_name<<" !")<<endl; continue;}

						TString syst_tmp = syst_list[isyst];
						if(syst_list[isyst] == "" && systTree_list[itree] != "") {syst_tmp = systTree_list[itree];}

						//Can simulate "up"/"down" variations here
						if(syst_tmp != "" && !syst_tmp.Contains("Down") && !syst_tmp.Contains("Up"))
						{
							if(ivariation == 0) {syst_tmp+= "Up";}
							else
							{
								syst_tmp+= "Down";
							}
						}

						TString outputname_new = Get_CERN_CombineNamingConvention(nLep_cat, sample_groups[isample], syst_tmp);

						TH1F* h_tmp = (TH1F*) f_input->Get(output_histo_name);

						// cout<<"Histo "<<output_histo_name<<" / integral : "<<h_tmp->Integral()<<endl;

						//ITC rescaling to SM xsec (for Combine)
						if(rescale_ITC_to_SM_xsec) //If using SM weights, already scaled to SM xsec!
						{
							if(sample_list[isample].Contains("tHq")) {h_tmp->Scale(0.07096 / 0.7927);}
							else if(sample_list[isample].Contains("tHW")) {h_tmp->Scale(0.01561 / 0.1472);}
						}

						f_output->cd();
						if(outputname_new != "")
						{
							h_tmp->Write(outputname_new);
							// cout<<"... Written as '"<<outputname_new<<"' !"<<endl;
						}

						delete h_tmp; h_tmp = 0;
					}
				}
			}
		}
	}

	f_input->Close();
	f_output->Close();

	cout<<endl<<FYEL("==> Created root file: ")<<f_output->GetName()<<endl;
	cout<<FYEL("containing the "<<classifier_name<<" templates with CERN naming conventions !")<<endl;

	cout<<endl<<FYEL("... Done")<<endl<<endl<<endl;

	return;
}
















//-----------------------------------------------------------------------------------------
// ########  ########     ###    ##      ##
// ##     ## ##     ##   ## ##   ##  ##  ##
// ##     ## ##     ##  ##   ##  ##  ##  ##
// ##     ## ########  ##     ## ##  ##  ##
// ##     ## ##   ##   ######### ##  ##  ##
// ##     ## ##    ##  ##     ## ##  ##  ##
// ########  ##     ## ##     ##  ###  ###

// ######## ######## ##     ## ########  ##          ###    ######## ########  ######
//    ##    ##       ###   ### ##     ## ##         ## ##      ##    ##       ##    ##
//    ##    ##       #### #### ##     ## ##        ##   ##     ##    ##       ##
//    ##    ######   ## ### ## ########  ##       ##     ##    ##    ######    ######
//    ##    ##       ##     ## ##        ##       #########    ##    ##             ##
//    ##    ##       ##     ## ##        ##       ##     ##    ##    ##       ##    ##
//    ##    ######## ##     ## ##        ######## ##     ##    ##    ########  ######
//-----------------------------------------------------------------------------------------




/**
 * New template plotting function, improved and adapted to new templates
 * @param template_name 'ttbar' / 'ttV' / '2Dlin' (used for limits) / 'categ' (show yield in subcategories)
 * @param channel       empty <-> sum of all subchannels ; else, plot particular channel
 * @param prefit        true <-> use templates created by this code ; else, look for COMBINE MLF output file
 *
 * NB : in my template files, I produce templates for channel == "" which correspond to sum of all subcategories -> use this for plotting
 * But in Combine output, all prefit/postfit histograms are given per subcategory only... Need to sum all of them !
 */
void TopEFT_analysis::Draw_Templates(bool drawInputVars, TString channel, TString template_name, bool prefit, bool use_combine_file)
{
//--------------------------------------------
	bool doNot_stack_signal = true; //true <-> draw tHq/tHW as lines, not on top of stack (since not SM process)

	bool draw_errors = true; //true <-> superimpose error bands on plot/ratio plot

	bool draw_logarithm = false;

    bool unblind_lowBDT = false; //true <-> read specific file, don't show FCNC signal
//--------------------------------------------
	if(analysis_type == "ttH" && (region == "CR_WZ" || region == "CR_ZZ" || nLep_cat == "4l") && channel != "") {cout<<"Error ! No subcategories in this region !"<<endl; return;} //no subcat for WZ/ZZ CRs, 4l SR

	cout<<endl<<BOLD(FYEL("##################################"))<<endl;
	if(drawInputVars) {cout<<FYEL("--- Producing Input Variables Plots / channel : "<<channel<<" ---")<<endl;}
	else if(template_name == "ttbar" || template_name == "ttV" || template_name == "2D" || template_name == "2Dlin" || template_name == "categ") {cout<<FYEL("--- Producing "<<template_name<<" Template Plots / channel : "<<channel<<" ---")<<endl;}
	else {cout<<FRED("--- ERROR : invalid args !")<<endl;}
	cout<<BOLD(FYEL("##################################"))<<endl<<endl;

	if(drawInputVars || template_name == "categ")
	{
		classifier_name = ""; //For naming conventions
		use_combine_file = false;
		if(drawInputVars && !prefit) {cout<<"Error ! Can not draw postfit input vars yet !"<<endl; return;}
		if(template_name == "categ" && !prefit) {cout<<"Can not plot yields per subcategory using the Combine output file ! Will plot [PREFIT] instead of [POSTFIT] !"<<endl; prefit = true;}
	}

	if(channel == "eu") {channel = "ue";} //convention

//  ####  ###### ##### #    # #####
// #      #        #   #    # #    #
//  ####  #####    #   #    # #    #
//      # #        #   #    # #####
// #    # #        #   #    # #
//  ####  ######   #    ####  #

	//Can use 2 different files :
	//- the files containing the template histograms, produced with this code (-> only prefit plots)
	//- or, better, the file produced by Combine from the templates : contains the prefit distributions with total errors, and the postfit distribution
	//If want postfit plots, must use the Combine file. If want prefit plots, can use both of them (NB : errors will be different)


	//Get input TFile
	if(!prefit)
	{
		use_combine_file = true;
	}
	if(drawInputVars && use_combine_file)
	{
		cout<<"-- Setting 'use_combine_file = false' !"<<endl;
		use_combine_file = false;
	}

	TString input_name = "";

	if(use_combine_file)
	{
		input_name = "./outputs/fitDiagnostics_";
		input_name+= classifier_name + template_name + "_" + nLep_cat + "_" + region + filename_suffix;
		if(classifier_name == "DNN") {input_name+= "_" + DNN_type;}
		input_name+= ".root";

		if(!Check_File_Existence(input_name))
		{
			input_name = "./outputs/fitDiagnostics.root"; //Try another name
			if(!Check_File_Existence(input_name))
			{
				cout<<FBLU("-- NB : File ")<<input_name<<FBLU(" (<-> file produced by COMBINE) not found !")<<endl;
				if(!prefit) {cout<<FRED("=> Can not produce postfit plots ! Abort !")<<endl; return;}

				use_combine_file = false;

				if(drawInputVars) //Input vars
				{
					input_name = "outputs/ControlHistograms_" + nLep_cat + "_" + region + filename_suffix + ".root";
				}
				else //Templates
				{
					input_name = "outputs/Templates_" + classifier_name + template_name + "_" + nLep_cat + "_" + region + filename_suffix;
					if(classifier_name == "DNN") {input_name+= "_" + DNN_type;}
					input_name+= ".root";
				}

				if(!Check_File_Existence(input_name))
				{
					cout<<FRED("File "<<input_name<<" (<-> file containing prefit templates) not found ! Did you specify the region/background ? Abort")<<endl;
					return;
				}
				else {cout<<FBLU("--> Using file ")<<input_name<<FBLU(" instead (NB : only stat. error will be included)")<<endl;}
			}
			else {cout<<FBLU("--> Using Combine output file : ")<<input_name<<FBLU(" (NB : total error included)")<<endl; use_combine_file = true;}
		}
		else {cout<<FBLU("--> Using Combine output file : ")<<input_name<<FBLU(" (NB : total error included)")<<endl; use_combine_file = true;}
	}
	else
	{
		if(drawInputVars) //Input vars
		{
			input_name = "outputs/ControlHistograms_" + nLep_cat + "_" + region + filename_suffix + ".root";
		}
		else //Templates
		{
			input_name = "outputs/Templates_" + classifier_name + template_name + "_" + nLep_cat + "_" + region + filename_suffix;
			if(classifier_name == "DNN") {input_name+= "_" + DNN_type;}
			if(unblind_lowBDT) {input_name+= "_lowBDT";}
			input_name+= ".root";
		}

		if(!Check_File_Existence(input_name))
		{
			cout<<FRED("File "<<input_name<<" (<-> file containing prefit templates) not found ! Did you specify the region/background ? Abort")<<endl;
			return;
		}
	}

	cout<<endl<<endl<<endl;
	usleep(1000000); //Pause for 1s (in microsec)

	//Input file containing histos
	TFile* file_input = 0;
	file_input = TFile::Open(input_name);

	//Need to rescale signal to fitted signal strength manually, and add its error in quadrature in each bin (verify)
	double sigStrength = 0;
	double sigStrength_Error = 0;
	if(!prefit)
	{
		TTree* t_postfit = (TTree*) file_input->Get("tree_fit_sb");
		t_postfit->SetBranchAddress("r", &sigStrength);
		t_postfit->SetBranchAddress("rErr", &sigStrength_Error);
		t_postfit->GetEntry(0); //Only 1 entry = fit results
		delete t_postfit; t_postfit = NULL;
	}

	//Want to plot ALL selected variables
	vector<TString> total_var_list;
	if(drawInputVars)
	{
		// for(int i=0; i<v_cut_name.size(); i++)
		// {
		// 	if(v_cut_name[i].Contains("is_")) {continue;} //Don't care about plotting the categories
		//
		// 	total_var_list.push_back(v_cut_name[i]);
		// }
		for(int i=0; i<var_list.size(); i++)
		{
			// if(!v_var_tmp->at(i).Contains("mem") || var_list[i].Contains("e-")) {continue;}

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


// #       ####   ####  #####   ####
// #      #    # #    # #    # #
// #      #    # #    # #    #  ####
// #      #    # #    # #####       #
// #      #    # #    # #      #    #
// ######  ####   ####  #       ####

	for(int ivar=0; ivar<total_var_list.size(); ivar++)
	{
		cout<<endl<<FBLU("* Variable : "<<total_var_list[ivar]<<" ")<<endl<<endl;

		//TH1F* to retrieve distributions
		TH1F* h_tmp = 0; //Tmp storing histo

		TH1F* h_thq = 0; //Store tHq shape
		TH1F* h_thw = 0; //Store tHW shape
		TH1F* h_fcnc = 0; //Store FCNC shape
		TH1F* h_fcnc2 = 0; //Store FCNC shape
		TH1F* h_sum_data = 0; //Will store data histogram
		vector<TH1F*> v_MC_histo; //Will store all MC histograms (1 TH1F* per MC sample)

		TGraphAsymmErrors* g_data = 0; //If using Combine file, data are stored in TGAE
		TGraphAsymmErrors* g_tmp = 0; //Tmp storing graph

		vector<TString> MC_samples_legend; //List the MC samples which are actually used (to get correct legend)

		//-- Init error vectors
		double x, y, errory_low, errory_high;

		vector<double> v_eyl, v_eyh, v_exl, v_exh, v_x, v_y; //Contain the systematic errors (used to create the TGraphError)
		int nofbins=-1;

		vector<float> v_yield_sig, v_yield_bkg;

		//Combine output : all histos are given for subcategories --> Need to sum them all
		for(int ichan=0; ichan<channel_list.size(); ichan++)
		{
			//If using my own template file, there is already a "summed categories" versions of the histograms
			if(channel_list[ichan] != channel)
			{
				if(use_combine_file) {if(channel != "") {continue;} } //In combine file, to get inclusive plot, must sum all subcategories
				else {continue;}
			}

			//Combine file : histos stored in subdirs -- define dir name
			TString dir_hist = "";
			if(prefit) {dir_hist = "shapes_prefit/";}
			else {dir_hist = "shapes_fit_s/";}
			dir_hist+= classifier_name + template_name + "_" + nLep_cat + "_" + region;
			if(channel_list[ichan] != "") {dir_hist+= "_" + channel_list[ichan];} //for combine file
			dir_hist+= "/";
			if(use_combine_file && !file_input->GetDirectory(dir_hist)) {cout<<FRED("Directory "<<dir_hist<<" not found ! Skip !")<<endl; continue;}

// #    #  ####
// ##  ## #    #
// # ## # #
// #    # #
// #    # #    #
// #    #  ####

			//--- Retrieve all MC samples
			int nof_skipped_samples = 0; //Get sample index right

			vector<bool> v_isSkippedSample(sample_list.size()); //Get sample index right (some samples are skipped)

			for(int isample = 0; isample < sample_list.size(); isample++)
			{
				int index_MC_sample = isample - nof_skipped_samples; //Sample index, but not counting data/skipped sample

				//In Combine, some individual contributions are merged as "Rares"/"EWK", etc.
				//If using Combine file, change the names of the samples we look for, and look only once for histogram of each "group"
				TString samplename = sample_list[isample];
				if(use_combine_file)
				{
					if(isample > 0 && sample_groups[isample] == sample_groups[isample-1]) {v_isSkippedSample[isample] = true; nof_skipped_samples++; continue;} //if same group as previous sample
					else {samplename = sample_groups[isample];}
				}

				//Protections, special cases
				if(sample_list[isample].Contains("DATA") ) {v_isSkippedSample[isample] = true; nof_skipped_samples++; continue;}

				//Skip these, but still need to reserve slots in vectors
				if((nLep_cat == "3l" || channel_list[ichan] == "uu") && sample_list[isample] == "QFlip")
				{
					if(channel != "") {v_isSkippedSample[isample] = true; nof_skipped_samples++;}
					else if(v_MC_histo.size() <=  index_MC_sample) {v_MC_histo.push_back(0); MC_samples_legend.push_back(samplename);}

					continue;
				}
				if((channel_list[ichan] == "uuu" || channel_list[ichan] == "uu") && sample_list[isample].Contains("GammaConv"))
				{
					if(channel != "") {v_isSkippedSample[isample] = true; nof_skipped_samples++;}
					else if(v_MC_histo.size() <=  index_MC_sample) {v_MC_histo.push_back(0); MC_samples_legend.push_back(samplename);}

					continue;
				}

	            if(!use_combine_file && (sample_list[isample].Contains("tHq_") || sample_list[isample].Contains("tHW_") || sample_list[isample].Contains("ttH_")) ) {v_isSkippedSample[isample] = true; nof_skipped_samples++; continue;} //Don't use split signals for plot //unless in combine file (must use what was used for fit)

				// cout<<endl<<UNDL(FBLU("-- Sample : "<<sample_list[isample]<<" : "))<<endl;

				h_tmp = 0;

				TString histo_name = "";
				if(use_combine_file) {histo_name = dir_hist + samplename;}
				else
				{
					histo_name = classifier_name + total_var_list[ivar] + "_" + nLep_cat + "_" + region;
					if(channel != "") {histo_name+= "_" + channel;}
					histo_name+= + "__" + samplename;
				}

				if(use_combine_file && !file_input->GetDirectory(dir_hist)->GetListOfKeys()->Contains(samplename) ) {cout<<ITAL("Histogram '"<<histo_name<<"' : not found ! Skip...")<<endl; v_isSkippedSample[isample] = true; nof_skipped_samples++; continue;}
				else if(!use_combine_file && !file_input->GetListOfKeys()->Contains(histo_name) ) {cout<<ITAL("Histogram '"<<histo_name<<"' : not found ! Skip...")<<endl; v_isSkippedSample[isample] = true; nof_skipped_samples++; continue;}

				h_tmp = (TH1F*) file_input->Get(histo_name);
				// cout<<"histo_name "<<histo_name<<endl;
	            // cout<<"h_tmp->Integral() = "<<h_tmp->Integral()<<endl;

				if(draw_errors)
				{
					//Initialize error vectors (only once at start)
					if(nofbins == -1) //if not yet init, get histo parameters
					{
						nofbins = h_tmp->GetNbinsX();
						for(int ibin=0; ibin<nofbins; ibin++)
						{
							v_eyl.push_back(0); v_eyh.push_back(0);
							v_exl.push_back(h_tmp->GetXaxis()->GetBinWidth(ibin+1) / 2); v_exh.push_back(h_tmp->GetXaxis()->GetBinWidth(ibin+1) / 2);
							v_x.push_back( (h_tmp->GetXaxis()->GetBinLowEdge(nofbins+1) - h_tmp->GetXaxis()->GetBinLowEdge(1) ) * ((ibin+1 - 0.5)/nofbins) + h_tmp->GetXaxis()->GetBinLowEdge(1));
							v_y.push_back(0);
						}
					}

					//Increment errors
					for(int ibin=0; ibin<nofbins; ibin++) //Start at bin 1
					{
						//For the processes which will not be stacked (FCNC, ...), don't increment the y-values of the errors
						if(doNot_stack_signal && (samplename.Contains("tHq") || samplename.Contains("tHW")) ) {break;} //Don't stack signal
						if(samplename.Contains("FCNC") ) {break;} //Don't stack FCNC signal

						// NOTE : for postfit, the bin error accounts for all systematics !
						//If using Combine output file (from MLF), bin error contains total error. Else if using template file directly, just stat. error
						v_eyl[ibin]+= pow(h_tmp->GetBinError(ibin+1), 2);
						v_eyh[ibin]+= pow(h_tmp->GetBinError(ibin+1), 2);

						v_y[ibin]+= h_tmp->GetBinContent(ibin+1); //This vector is used to know where to draw the error zone on plot (= on top of stack)

						// if(ibin != 4) {continue;} //cout only 1 bin
						// cout<<"x = "<<v_x[ibin]<<endl;    cout<<", y = "<<v_y[ibin]<<endl;    cout<<", eyl = "<<v_eyl[ibin]<<endl;    cout<<", eyh = "<<v_eyh[ibin]<<endl; //cout<<", exl = "<<v_exl[ibin]<<endl;    cout<<", exh = "<<v_exh[ibin]<<endl;
					} //loop on bins

					//-- NEW, draw all errors
					//--------------------------------------------
					if(!use_combine_file) //In Combine file, already accounted in binError
					{
						for(int itree=0; itree<systTree_list.size(); itree++)
						{
							for(int isyst=0; isyst<syst_list.size(); isyst++)
							{
								//-- Protections : not all syst weights apply to all samples, etc.
								if(syst_list[isyst] != "" && systTree_list[itree] != "") {break;} //only nominal
								if(sample_list[isample] == "DATA" || sample_list[isample] == "QFlip") {break;}
								if(syst_list[isyst] == "") {continue;}
								if((syst_list[isyst].Contains("FR_") || syst_list[isyst].Contains("Clos")) && !sample_list[isample].Contains("Fake") ) {continue;}
								if(sample_list[isample].Contains("Fake") && !syst_list[isyst].Contains("FR_") && !syst_list[isyst].Contains("Clos") && syst_list[isyst] != "") {continue;}
								if(syst_list[isyst].Contains("Clos") && nLep_cat == "3l") {continue;}
								if(syst_list[isyst].Contains("ttH") && !sample_list[isample].Contains("ttH")) {continue;}
								if(syst_list[isyst].Contains("ttW") && !sample_list[isample].Contains("ttW")) {continue;}
								if(syst_list[isyst].Contains("ttZ") && !sample_list[isample].Contains("ttZ") && !sample_list[isample].Contains("TTJets")) {continue;}
								if(syst_list[isyst].Contains("_e_") && (channel_list[ichan].Contains("mm_") || channel_list[ichan] == "uu" || channel_list[ichan] == "uuu")) {continue;}
								if(syst_list[isyst].Contains("_m_") && (channel_list[ichan].Contains("ee_") || channel_list[ichan] == "ee" || channel_list[ichan] == "eee")) {continue;}

								// cout<<"sample "<<sample_list[isample]<<" / channel "<<channel_list[ichan]<<" / syst "<<syst_list[isyst]<<endl;

								TH1F* histo_syst = 0; //Store the "systematic histograms"

								TString histo_name_syst = histo_name + "__" + syst_list[isyst];

								if(!file_input->GetListOfKeys()->Contains(histo_name_syst)) {continue;} //No error messages if systematics histos not found

								histo_syst = (TH1F*) file_input->Get(histo_name_syst);

								//Add up here the different errors (quadratically), for each bin separately
								for(int ibin=0; ibin<nofbins; ibin++)
								{
									if(histo_syst->GetBinContent(ibin+1) == 0) {continue;} //Some syst may be null, don't compute diff

									double tmp = 0;

									//For each systematic, compute (shifted-nominal), check the sign, and add quadratically to the corresponding bin error
									tmp = histo_syst->GetBinContent(ibin+1) - h_tmp->GetBinContent(ibin+1);

									if(tmp>0) {v_eyh[ibin]+= pow(tmp,2);}
									else if(tmp<0) {v_eyl[ibin]+= pow(tmp,2);}

									if(ibin > 0) {continue;} //cout only first bin
									// cout<<"//--------------------------------------------"<<endl;
									// cout<<"Sample "<<sample_list[isample]<<" / Syst "<<syst_list[isyst]<< " / chan "<<channel_list[ichan]<<endl;
									// cout<<"x = "<<v_x[ibin]<<endl;    cout<<", y = "<<v_y[ibin]<<endl;    cout<<", eyl = "<<v_eyl[ibin]<<endl;    cout<<", eyh = "<<v_eyh[ibin]<<endl; //cout<<", exl = "<<v_exl[ibin]<<endl;    cout<<", exh = "<<v_exh[ibin]<<endl;
									// cout<<"(nominal value = "<<h_tmp->GetBinContent(ibin+1)<<" - shifted value = "<<histo_syst->GetBinContent(ibin+1)<<") = "<<h_tmp->GetBinContent(ibin+1)-histo_syst->GetBinContent(ibin+1)<<endl;
								}

								delete histo_syst;
							} //end syst loop
						} //systTree_list loop
					} //--- systematics error loop
					//--------------------------------------------
				} //error condition

				if(!samplename.Contains("DATA") )
				{
					if(v_MC_histo.size() <=  index_MC_sample) {MC_samples_legend.push_back(samplename);}
					// MC_samples_legend.push_back(samplename); //Fill vector containing existing MC samples names
					// cout<<"ADD samplename "<<samplename<<endl;
					// cout<<"MC_samples_legend.size() "<<MC_samples_legend.size()<<endl;
				}

 //  ####   ####  #       ####  #####   ####
 // #    # #    # #      #    # #    # #
 // #      #    # #      #    # #    #  ####
 // #      #    # #      #    # #####       #
 // #    # #    # #      #    # #   #  #    #
 //  ####   ####  ######  ####  #    #  ####

				//Use color vector filled in main()
				h_tmp->SetFillStyle(1001);
				if(samplename == "Fakes") {h_tmp->SetFillStyle(3005);}
		        else if(samplename == "QFlip" ) {h_tmp->SetFillStyle(3006);}
		        else if(samplename.Contains("TTbar") || samplename.Contains("TTJet") )
				{
					// h_tmp->SetFillStyle(3005);
					if(samplename.Contains("Semi") ) {h_tmp->SetLineWidth(0);}
				}

				h_tmp->SetFillColor(color_list[isample]);
				h_tmp->SetLineColor(kBlack);

				if((doNot_stack_signal && (samplename.Contains("tHq") || samplename.Contains("tHW")) ) || samplename.Contains("FCNC")) //Superimpose BSM signal
				{
					h_tmp->SetFillColor(0);
					h_tmp->SetLineColor(color_list[isample]);
				}

				//Check color of previous *used* sample (up to 6)
				for(int k=1; k<6; k++)
				{
					if(isample - k >= 0)
					{
						if(v_isSkippedSample[isample-k]) {continue;}
						else if(color_list[isample] == color_list[isample-k]) {h_tmp->SetLineColor(color_list[isample]); break;}
					}
					else {break;}
				}

				// v_MC_histo.push_back((TH1F*) h_tmp->Clone());
				if(v_MC_histo.size() <=  index_MC_sample) {v_MC_histo.push_back((TH1F*) h_tmp->Clone());}
				else if(!v_MC_histo[index_MC_sample]) {v_MC_histo[index_MC_sample] = (TH1F*) h_tmp->Clone();} //For FakeEle and FakeMu
				else {v_MC_histo[index_MC_sample]->Add((TH1F*) h_tmp->Clone());}

				if(channel_list[ichan] == "")
				{
					if(v_yield_sig.size() == 0) {v_yield_sig.resize(h_tmp->GetNbinsX()); v_yield_bkg.resize(h_tmp->GetNbinsX());}

					for(int ibin=0; ibin<h_tmp->GetNbinsX(); ibin++)
					{
						if(sample_list[isample].Contains(signal_process) ) {v_yield_sig[ibin]+= h_tmp->GetBinContent(ibin+1);}
						else {v_yield_bkg[ibin]+= h_tmp->GetBinContent(ibin+1);}
					}
				}

				// cout<<"sample : "<<sample_list[isample]<<" / color = "<<color_list[isample]<<" fillstyle = "<<h_tmp->GetFillStyle()<<endl;
				// cout<<"index_MC_sample "<<index_MC_sample<<endl;
				// cout<<"v_MC_histo.size() "<<v_MC_histo.size()<<endl;
				// cout<<"MC_samples_legend.size() "<<MC_samples_legend.size()<<endl<<endl;

				delete h_tmp; h_tmp = 0;
			} //end sample loop
		} //subcat loop

		//Printout S/B for each bin
		// for(int ibin=0; ibin<v_yield_sig.size(); ibin++)
		// {
		// 	cout<<endl<<"ibin : S/B = "<<v_yield_sig[ibin]/v_yield_bkg[ibin]<<" // bkg = "<<v_yield_bkg[ibin]<<" // sig = "<<v_yield_sig[ibin]<<endl;
		// }

// #####    ##   #####   ##
// #    #  #  #    #    #  #
// #    # #    #   #   #    #
// #    # ######   #   ######
// #    # #    #   #   #    #
// #####  #    #   #   #    #

		//--- Retrieve DATA histo
		h_tmp = 0;
		TString histo_name = "";
		if(use_combine_file) {histo_name = "data";}
		else
		{
			histo_name = classifier_name + total_var_list[ivar] + "_" + nLep_cat + "_" + region;
			if(channel != "") {histo_name+= "_" + channel;}
			histo_name+= "__data_obs";
		}

		if(use_combine_file)
		{
			for(int ichan=0; ichan<channel_list.size(); ichan++)
			{
				if(channel != "" && channel_list[ichan] != channel) {continue;}

				//Combine file : histos stored in subdirs -- define dir name
				TString dir_hist = "";
				if(prefit) {dir_hist = "shapes_prefit/";}
				else {dir_hist = "shapes_fit_s/";}
				dir_hist+= classifier_name + template_name + "_" + nLep_cat + "_" + region;
				if(channel_list[ichan] != "") {dir_hist+= "_" + channel_list[ichan] + "/";} //for combine file
				if(!file_input->GetDirectory(dir_hist)) {cout<<ITAL("Directory "<<dir_hist<<" not found ! Skip !")<<endl; continue;}

				if(!file_input->GetDirectory(dir_hist)->GetListOfKeys()->Contains("data")) {cout<<FRED(""<<dir_hist<<"data : not found ! Skip...")<<endl; continue;}

				histo_name = dir_hist + "/data";
				// cout<<"histo_name "<<histo_name<<endl;
				g_tmp = (TGraphAsymmErrors*) file_input->Get(histo_name); //stored as TGraph

				//Remove X-axis error bars, not needed for plot
				for(int ipt=0; ipt<g_tmp->GetN(); ipt++)
				{
					g_tmp->SetPointEXhigh(ipt, 0);
					g_tmp->SetPointEXlow(ipt, 0);
				}

				if(!g_data) {g_data = (TGraphAsymmErrors*) g_tmp->Clone();}
				else //Need to sum TGraphs content by hand //not anymore, 1 channel only !
				{
					double x_tmp,y_tmp,errory_low_tmp,errory_high_tmp;
					for(int ipt=0; ipt<g_data->GetN(); ipt++)
					{
						g_data->GetPoint(ipt, x, y);
						errory_low = g_data->GetErrorYlow(ipt);
						errory_high = g_data->GetErrorYhigh(ipt);

						g_tmp->GetPoint(ipt, x_tmp, y_tmp);
						errory_low_tmp = g_tmp->GetErrorYlow(ipt);
						errory_high_tmp = g_tmp->GetErrorYhigh(ipt);

						double new_error_low = sqrt(errory_low*errory_low+errory_low_tmp*errory_low_tmp);
						double new_error_high = sqrt(errory_high_tmp*errory_high_tmp+errory_high_tmp*errory_high_tmp);
						g_data->SetPoint(ipt, x, y+y_tmp);
						g_data->SetPointError(ipt,0,0, new_error_low, new_error_high); //ok to add errors in quadrature ?

						// cout<<"ipt "<<ipt<<" / x1 "<<x<<" / y1 "<<y<<" / error1 "<<errory_low<<", "<<errory_high<<endl;
						// cout<<"ipt "<<ipt<<" / x2 "<<x_tmp<<" / y2 "<<y_tmp<<" / error2 "<<errory_low_tmp<<", "<<errory_high_tmp<<endl;
						// cout<<"=> y1+y2 = "<<y+y_tmp<<" / error = "<<new_error_low<<", "<<new_error_high<<endl;
					}
				}
			} //chan loop
		}
		else //If using template file made from this code
		{
			if(!file_input->GetListOfKeys()->Contains(histo_name)) {cout<<histo_name<<" : not found"<<endl;}
			else
			{
				h_tmp = (TH1F*) file_input->Get(histo_name);
				if(h_sum_data == 0) {h_sum_data = (TH1F*) h_tmp->Clone();}
				else {h_sum_data->Add((TH1F*) h_tmp->Clone());} //not needed anymore (1 channel only)

				delete h_tmp; h_tmp = NULL;
			}
		}

		bool data_notEmpty = true;
		// if(use_combine_file && !g_data) {cout<<endl<<BOLD(FRED("--- Empty data TGraph ! Exit !"))<<endl<<endl; return;}
		// if(!use_combine_file && !h_sum_data) {cout<<endl<<BOLD(FRED("--- Empty data histogram ! Exit !"))<<endl<<endl; return;}
		if(use_combine_file && !g_data) {cout<<endl<<BOLD(FRED("--- Empty data TGraph !"))<<endl<<endl; data_notEmpty = false;}
		if(!use_combine_file && !h_sum_data) {cout<<endl<<BOLD(FRED("--- Empty data histogram "<<histo_name<<" !"))<<endl<<endl; data_notEmpty = false;}

		//Make sure there are no negative bins
		if(data_notEmpty)
		{
			if(use_combine_file)
			{
				for(int ipt=0; ipt<g_data->GetN(); ipt++)
				{
					g_data->GetPoint(ipt, x, y);
					if(y<0) {g_data->SetPoint(ipt, x, 0); g_data->SetPointError(ipt,0,0,0,0);}



					//Blind tHq SRs for now -- set data to 0
                    // if(is_blind && (analysis_type == "tHq" || analysis_type == "FCNC") && (region.Contains("SR") || region == "FCNC") ) {g_data->SetPoint(ipt, x, 0); g_data->SetPointError(ipt,0,0,0,0);}
                    if(is_blind && analysis_type == "FCNC" && (region.Contains("SR") || region == "FCNC") ) {g_data->SetPoint(ipt, x, 0); g_data->SetPointError(ipt,0,0,0,0);}
				}
			}
			else
			{
				for(int ibin = 1; ibin<h_sum_data->GetNbinsX()+1; ibin++)
				{
					if(h_sum_data->GetBinContent(ibin) < 0) {h_sum_data->SetBinContent(ibin, 0);}

					//Blind tHq SRs for now -- set data to 0
					if(is_blind && analysis_type == "FCNC" && (region.Contains("SR") || region == "FCNC") ) {h_sum_data->SetBinContent(ibin, 0);}
				}
			}
			for(int k=0; k<v_MC_histo.size(); k++)
			{
				if(!v_MC_histo[k]) {continue;} //Fakes templates can be null
				for(int ibin=0; ibin<v_MC_histo[k]->GetNbinsX(); ibin++)
				{
					if(v_MC_histo[k]->GetBinContent(ibin) < 0) {v_MC_histo[k]->SetBinContent(ibin, 0);}
				}
			}
		}


 // # #    # #####  ###### #    #
 // # ##   # #    # #       #  #
 // # # #  # #    # #####    ##
 // # #  # # #    # #        ##
 // # #   ## #    # #       #  #
 // # #    # #####  ###### #    #

	//-- Get indices of particular samples, sum the others into 1 single histo (used for ratio subplot)
		TH1F* histo_total_MC = 0; //Sum of all MC samples

		//Indices of important samples, for specific treatment
		int index_tHq_sample = -99;
		int index_tHW_sample = -99;
		int index_NPL_sample = -99;
		int index_FCNC_sample = -99;
		int index_FCNC_sample2 = -99;

		// cout<<"v_MC_histo.size() "<<v_MC_histo.size()<<endl;
		// cout<<"MC_samples_legend.size() "<<MC_samples_legend.size()<<endl;

		//Merge all the MC nominal histograms (contained in v_MC_histo)
		for(int i=0; i<v_MC_histo.size(); i++)
		{
			if(!v_MC_histo[i]) {continue;} //Fakes templates may be null

			// cout<<"MC_samples_legend[i] "<<MC_samples_legend[i]<<endl;

			if(MC_samples_legend[i].Contains("tHq") )
			{
				if(index_tHq_sample<0) {index_tHq_sample = i;}
				if(!h_thq) {h_thq = (TH1F*) v_MC_histo[i]->Clone();}
				else {h_thq->Add((TH1F*) v_MC_histo[i]->Clone());}
				if(doNot_stack_signal) continue; //don't stack
			}
			else if(MC_samples_legend[i].Contains("tHW") )
			{
				if(index_tHW_sample<0) {index_tHW_sample = i;}
				if(!h_thw) {h_thw = (TH1F*) v_MC_histo[i]->Clone();}
				else {h_thw->Add((TH1F*) v_MC_histo[i]->Clone());}
				if(doNot_stack_signal) continue; //don't stack
			}
			else if(MC_samples_legend[i].Contains("FCNC") )
			{
				if(MC_samples_legend[i].Contains("tH_TT"))
				{
					if(index_FCNC_sample2<0) {index_FCNC_sample2 = i;}
					h_fcnc2 = (TH1F*) v_MC_histo[i]->Clone();
					continue; //don't stack FCNC signal
				}
				else if(MC_samples_legend[i].Contains("tH_ST"))
				{
					if(index_FCNC_sample<0) {index_FCNC_sample = i;}
					h_fcnc = (TH1F*) v_MC_histo[i]->Clone();
					continue; //don't stack FCNC signal
				}
			}

			// cout<<"Adding sample "<<MC_samples_legend[i]<<" to histo_total_MC"<<endl;

			if(!histo_total_MC) {histo_total_MC = (TH1F*) v_MC_histo[i]->Clone();}
			else {histo_total_MC->Add((TH1F*) v_MC_histo[i]->Clone());}
		}


// ####### #
//    #    #       ######  ####  ###### #    # #####
//    #    #       #      #    # #      ##   # #    #
//    #    #       #####  #      #####  # #  # #    #
//    #    #       #      #  ### #      #  # # #    #
//    #    #       #      #    # #      #   ## #    #
//    #    ####### ######  ####  ###### #    # #####

		bool use_diff_legend_layout = true;
		TLegend* qw = 0;
		if(use_diff_legend_layout)
		{
			// if(doNot_stack_signal) {qw = new TLegend(0.50,.70,0.99,0.88);}
			if(draw_logarithm && nLep_cat == "2l")
			{
				qw = new TLegend(0.20,0.28,0.65,0.50);
			}
			else
			{
				if(analysis_type == "FCNC" && nLep_cat == "2l") //slightly wider, because rescale TT (bigger legend entry)
				{
					qw = new TLegend(0.51,.75,1.,0.90);
				}
				else
				{
					if(doNot_stack_signal) {qw = new TLegend(0.53,.75,1.,0.90);}
					else {qw = new TLegend(0.62,.72,1.,0.90);}
				}
			}
			qw->SetNColumns(3);
		}
		else
		{
			if(doNot_stack_signal) {qw = new TLegend(0.79,.60,1.,1.);}
			else
			{
				if(use_combine_file && data_notEmpty)
				{
					double x_tmp, y_tmp;
					g_data->GetPoint(0, x_tmp, y_tmp);
					g_data->GetPoint(g_data->GetN()-1, x, y);
					if(y > y_tmp) {qw = new TLegend(0.20,0.48,0.20+0.16,0.48+0.39);}
				}
				else if(!use_combine_file && data_notEmpty && h_sum_data->GetBinContent(h_sum_data->GetNbinsX()) > h_sum_data->GetBinContent(1)) {qw = new TLegend(0.20,0.48,0.20+0.16,0.48+0.39);}
				else {qw = new TLegend(.83,.60,0.99,0.99);}
			}
		}

		qw->SetLineColor(1);
		qw->SetTextSize(0.03);

		//--Data on top of legend
		if( (!is_blind || analysis_type == "ttH" || analysis_type == "tHq" || (!region.Contains("SR") && region != "FCNC") ) )
		{
			if(use_combine_file && g_data != 0) {qw->AddEntry(g_data, "Data" , "ep");}
			else if(!use_combine_file && h_sum_data != 0) {qw->AddEntry(h_sum_data, "Data" , "ep");}
			else {cout<<__LINE__<<BOLD(FRED(" : null data !"))<<endl;}
		}

		for(int i=0; i<v_MC_histo.size(); i++)
		{
			// cout<<"MC_samples_legend[i] "<<MC_samples_legend[i]<<endl;

			if((nLep_cat == "3l" || channel == "uu") && MC_samples_legend[i] == "QFlip")
			{
				continue;
			}
			if((channel == "uuu" || channel == "uu") && MC_samples_legend[i].Contains("GammaConv"))
			{
				continue;
			}

			if(!v_MC_histo[i]) {continue;} //Fakes templates can be null

			//Merged with other samples in legend -- don't add these
			if(MC_samples_legend[i].Contains("WW")
	        || MC_samples_legend[i] == "TTTT"
	        || MC_samples_legend[i] == "ZZZ"
	        || MC_samples_legend[i] == "WWZ"
	        || MC_samples_legend[i] == "WZZ"
	        || MC_samples_legend[i].Contains("TTbar")
	        || MC_samples_legend[i].Contains("TTJets")
	        || MC_samples_legend[i] == "WWTo2LNu"
	        || MC_samples_legend[i] == "ST"
	        // || MC_samples_legend[i] == "tWZ"
	        || MC_samples_legend[i].Contains("WW")
	        || MC_samples_legend[i] == "tGJets"
	        || MC_samples_legend[i].Contains("ZZ")
	        || MC_samples_legend[i].Contains("DY")
	        // || MC_samples_legend[i].Contains("GammaConv")
	        || MC_samples_legend[i] == "ttWW"
	        || MC_samples_legend[i] == "tttW"
	        || MC_samples_legend[i] == "ttWH"
	        || MC_samples_legend[i] == "WZG"
	        || MC_samples_legend[i] == "GGHZZ4L"
	        || MC_samples_legend[i] == "WW_DPS"
	        || MC_samples_legend[i] == "WpWp"
	        || MC_samples_legend[i] == "ttG"
	        || MC_samples_legend[i] == "tGJets"
			|| MC_samples_legend[i] == "WG"
			|| MC_samples_legend[i] == "VHToNonbb"
			|| MC_samples_legend[i] == "WJets"
			|| MC_samples_legend[i] == "WZ_c"
			|| MC_samples_legend[i] == "WZ_l"
			|| MC_samples_legend[i] == "tHq_hzz"
	        || MC_samples_legend[i] == "tHq_htt"
	        || MC_samples_legend[i] == "tHq_hmm"
	        || MC_samples_legend[i] == "tHq_hzg"
			|| MC_samples_legend[i] == "tHW_hzz"
	        || MC_samples_legend[i] == "tHW_htt"
	        || MC_samples_legend[i] == "tHW_hmm"
	        || MC_samples_legend[i] == "tHW_hzg"
			|| MC_samples_legend[i] == "ttH_hzz"
	        || MC_samples_legend[i] == "ttH_htt"
	        || MC_samples_legend[i] == "ttH_hmm"
	        || MC_samples_legend[i] == "ttH_hzg"
	        ) {continue;}

			if(MC_samples_legend[i] == "tHq" || MC_samples_legend[i] == "tHq_hww")
			{
				if(doNot_stack_signal)
				{
					h_thq->SetLineColor(color_list[i]);
					if(sample_list[0] == "DATA") {h_thq->SetLineColor(color_list[i+1]);}
					h_thq->SetFillColor(0);
					h_thq->SetLineWidth(4);
					// h_thq->SetLineStyle(3);
					// h_thq->Scale(histo_total_MC->Integral()*1./h_thq->Integral() );

					// h_thq->Scale(10);
					// qw->AddEntry(h_thq, "tHq(#kappa_{t}=-1) x10", "L");

					if(!use_SM_coupling)
					{
						// qw->AddEntry(h_thq, "tHq(#kappa_{t}=-1)", "L");

						h_thq->Scale(10);
						qw->AddEntry(h_thq, "tHq(#kappa_{t}=-1)x10", "L");
					}
					else
					{
						h_thq->Scale(100);
						qw->AddEntry(h_thq, "tHq(x100)", "L");
					}
				}
				else
				{
					if(use_SM_coupling)
					{
						qw->AddEntry(v_MC_histo[i], "tHq(x10)", "f");
						v_MC_histo[i]->Scale(10);
					}
					else {qw->AddEntry(v_MC_histo[i], "tHq(#kappa_{t}=-1)", "f");}
				}
			}
			else if(MC_samples_legend[i] == "tHW" || MC_samples_legend[i] == "tHW_hww")
			{
				if(doNot_stack_signal)
				{
					h_thw->SetLineColor(color_list[i]);
					if(sample_list[0] == "DATA") {h_thw->SetLineColor(color_list[i+1]);}
					h_thw->SetFillColor(0);
					h_thw->SetLineWidth(4);
					h_thw->SetLineStyle(2);

					if(!use_SM_coupling)
					{
						// qw->AddEntry(h_thw, "tHW(#kappa_{t}=-1)", "L");

						h_thw->Scale(10);
						qw->AddEntry(h_thw, "tHW(#kappa_{t}=-1)x10", "L");
					}
					else
					{
						h_thw->Scale(100);
						qw->AddEntry(h_thw, "tHW(x100)", "L");
					}
				}
				else
				{
					if(use_SM_coupling)
					{
						qw->AddEntry(v_MC_histo[i], "tHW(x10)", "f");
						v_MC_histo[i]->Scale(10);
					}
					else {qw->AddEntry(v_MC_histo[i], "tHW(#kappa_{t}=-1)", "f");}
				}
			}
			else if(MC_samples_legend[i].Contains("tH_TT") && !unblind_lowBDT) //FCNC TT
			{
				h_fcnc2->SetLineColor(color_list[i]);
				if(sample_list[0] == "DATA") {h_fcnc2->SetLineColor(color_list[i+1]);}
				h_fcnc2->SetFillColor(0);
				h_fcnc2->SetLineWidth(5);
				// h_fcnc2->SetLineStyle(7);
				TString leg_name = "tH-TT";
				if(signal_process.Contains("hut")) {leg_name+="(hut)";}
				else {leg_name+="(hct)";}
				if(nLep_cat == "2l")  //Need to divide TT signal, else too large in 2l
				{
					h_fcnc2->Scale(0.5);
					leg_name+= "x0.5";
				}
				qw->AddEntry(h_fcnc2, leg_name, "L");
				// h_fcnc2->Scale(10);
				// qw->AddEntry(h_fcnc2, "tH-TT x10", "L");
			}
			else if(MC_samples_legend[i].Contains("tH_ST") && !unblind_lowBDT) //FCNC ST
			{
				h_fcnc->SetLineColor(color_list[i]);
				if(sample_list[0] == "DATA") {h_fcnc->SetLineColor(color_list[i+1]);}
				h_fcnc->SetFillColor(0);
				h_fcnc->SetLineWidth(5);
				h_fcnc->SetLineStyle(2);
				TString leg_name = "tH-ST";
				if(signal_process.Contains("hut")) {leg_name+="(hut)";}
				else {leg_name+="(hct)";}
				if(MC_samples_legend[i].Contains("tH_ST")) {qw->AddEntry(h_fcnc, leg_name, "L");}
				else {qw->AddEntry(h_fcnc, leg_name, "L");}
				// h_fcnc->Scale(10);
				// if(MC_samples_legend[i].Contains("tH_ST")) {qw->AddEntry(h_fcnc, "tH-ST x10", "L");}
				// else {qw->AddEntry(h_fcnc, "tH FCNC x10", "L");}
			}

			else if(MC_samples_legend[i].Contains("ttH")) {qw->AddEntry(v_MC_histo[i], "ttH", "f");}
			else if(MC_samples_legend[i] == "tZq" && analysis_type == "ttH") {qw->AddEntry(v_MC_histo[i], "Rare", "f");}
			else if(MC_samples_legend[i] == "tZq" && analysis_type != "ttH") {qw->AddEntry(v_MC_histo[i], "tZq", "f");}
			else if(MC_samples_legend[i] == "tWZ" && analysis_type != "ttH") {qw->AddEntry(v_MC_histo[i], "Rares", "f");}
			else if(MC_samples_legend[i].Contains("ttW") ) {qw->AddEntry(v_MC_histo[i], "t#bar{t}W", "f");}
			else if(MC_samples_legend[i] == "ttZ") {qw->AddEntry(v_MC_histo[i], "t#bar{t}Z", "f");}
			else if(MC_samples_legend[i] == "WZ" || MC_samples_legend[i] == "WZ_b") {qw->AddEntry(v_MC_histo[i], "EWK", "f");}
			else if(MC_samples_legend[i] == "Fakes") {qw->AddEntry(v_MC_histo[i], "Non-prompt", "f");}
			else if(MC_samples_legend[i] == "QFlip") {qw->AddEntry(v_MC_histo[i], "Flip", "f");}
			else if(MC_samples_legend[i].Contains("GammaConv") ) {qw->AddEntry(v_MC_histo[i], "#gamma-conv.", "f");}
			// else if(MC_samples_legend[i].Contains("TTbar") || MC_samples_legend[i].Contains("TTJet") ) {qw->AddEntry(v_MC_histo[i], "t#bar{t} (MC)", "f");}
			else if(!MC_samples_legend[i].Contains("FCNC") ) {qw->AddEntry(v_MC_histo[i], MC_samples_legend[i], "f");}
		}


// ##### #    #  ####  #####   ##    ####  #    #
//   #   #    # #        #    #  #  #    # #   #
//   #   ######  ####    #   #    # #      ####
//   #   #    #      #   #   ###### #      #  #
//   #   #    # #    #   #   #    # #    # #   #
//   #   #    #  ####    #   #    #  ####  #    #

		THStack* stack_MC = new THStack;

		//Add legend entries -- iterate backwards, so that last histo stacked is on top of legend
		//Also add MC histograms to the THStack
		for(int i=v_MC_histo.size()-1; i>=0; i--)
		{
			if(!v_MC_histo[i]) {continue;} //Some templates may be null

			if(doNot_stack_signal)
			{
				//-- If want to use split signal samples
				// if(split_signals_byHiggsDecay && ((i>=index_tHq_sample && i<=index_tHq_sample+2) || ((i>=index_tHW_sample && i<=index_tHW_sample+2)) ) ) {continue;}
				// else if(!split_signals_byHiggsDecay && (i==index_tHq_sample || i==index_tHW_sample) ) {continue;} //don't stack signal

				//-- If want to use full signal samples
				if(i==index_tHq_sample || i==index_tHW_sample) {continue;}
				if(MC_samples_legend[i].Contains("tHq") || MC_samples_legend[i].Contains("tHW")) {continue;}
			}
			if(i==index_FCNC_sample || i==index_FCNC_sample2) {continue;} //don't stack FCNC signal

			stack_MC->Add(v_MC_histo[i]);
			// cout<<"Stacking sample "<<MC_samples_legend[i]<<" / integral "<<v_MC_histo[i]->Integral()<<endl;
		}

		//Set Yaxis maximum & minimum
		if(use_combine_file && data_notEmpty)
		{
			Long64_t locmax = TMath::LocMax(g_data->GetN(), g_data->GetY()); //the corresponding x value can be obtained with double xmax = gr->GetX()[locmax];
			double ymax = g_data->GetY()[locmax];

			if(ymax > stack_MC->GetMaximum() ) {stack_MC->SetMaximum(ymax*1.3);}
			else stack_MC->SetMaximum(stack_MC->GetMaximum()*1.5);
		}
		else if(!use_combine_file && data_notEmpty)
		{
			if(h_sum_data->GetMaximum() > stack_MC->GetMaximum() ) {stack_MC->SetMaximum(h_sum_data->GetMaximum()+0.3*h_sum_data->GetMaximum());}
			else {stack_MC->SetMaximum(stack_MC->GetMaximum()*1.5);}
		}

		stack_MC->SetMinimum(0.0001); //Remove '0' label

		if(draw_logarithm)
		{
			stack_MC->SetMinimum(0.5);
			stack_MC->SetMaximum(stack_MC->GetMaximum()*6);
		}


// #####  #####    ##   #    #
// #    # #    #  #  #  #    #
// #    # #    # #    # #    #
// #    # #####  ###### # ## #
// #    # #   #  #    # ##  ##
// #####  #    # #    # #    #

		//Canvas definition
		Load_Canvas_Style();
		TCanvas* c1 = new TCanvas("c1","c1", 1000, 800);
		// TCanvas* c1 = new TCanvas("c1","c1", 600, 800);
		c1->SetTopMargin(0.1);
		c1->SetBottomMargin(0.25);
		// c1->SetRightMargin(0.15);

		if(draw_logarithm) {c1->SetLogy();}

		//Draw stack
		stack_MC->Draw("hist");

		//Draw data
		if(data_notEmpty)
		{
			if(use_combine_file)
			{
				g_data->SetMarkerStyle(20);
				g_data->Draw("e0psame");
			}
			else
			{
				h_sum_data->SetMarkerStyle(20);
				h_sum_data->SetMinimum(0.) ;
				h_sum_data->Draw("e0psame");
			}
		}

		//Superimpose shape of signal
		if(doNot_stack_signal)
		{
			if(h_thq != 0) {h_thq->Draw("same hist");}
			if(h_thw != 0) {h_thw->Draw("same hist");}
		}

		if(!unblind_lowBDT)
		{
			if(h_fcnc != 0) {h_fcnc->Draw("same hist");}
			if(h_fcnc2 != 0) {h_fcnc2->Draw("same hist");}
		}

		qw->Draw("same"); //Draw legend


// ###### #####  #####   ####  #####   ####      ####  #####   ##    ####  #    #
// #      #    # #    # #    # #    # #         #        #    #  #  #    # #   #
// #####  #    # #    # #    # #    #  ####      ####    #   #    # #      ####
// #      #####  #####  #    # #####       #         #   #   ###### #      #  #
// #      #   #  #   #  #    # #   #  #    #    #    #   #   #    # #    # #   #
// ###### #    # #    #  ####  #    #  ####      ####    #   #    #  ####  #    #

		//-- Compute sqrt of quadratic errors
		if(draw_errors)
		{
			for(int ibin=0; ibin<nofbins; ibin++)
			{
				v_eyh[ibin] = pow(v_eyh[ibin], 0.5);
				v_eyl[ibin] = pow(v_eyl[ibin], 0.5);

				// if(ibin > 0) {continue;} //cout only first bin
				// cout<<"x = "<<v_x[ibin]<<endl;    cout<<", y = "<<v_y[ibin]<<endl;    cout<<", eyl = "<<v_eyl[ibin]<<endl;    cout<<", eyh = "<<v_eyh[ibin]<<endl; //cout<<", exl = "<<v_exl[ibin]<<endl;    cout<<", exh = "<<v_exh[ibin]<<endl;
			}
		}

		//Use pointers to vectors : need to give the adress of first element (all other elements can then be accessed iteratively)
		double* eyl = &v_eyl[0];
		double* eyh = &v_eyh[0];
		double* exl = &v_exl[0];
		double* exh = &v_exh[0];
		double* xx = &v_x[0];
		double* yy = &v_y[0];

		//Create TGraphAsymmErrors with the error vectors / (x,y) coordinates --> Can superimpose it on plot
		TGraphAsymmErrors* gr_error = 0;

		gr_error = new TGraphAsymmErrors(nofbins,xx,yy,exl,exh,eyl,eyh);
		gr_error->SetFillStyle(3002);
		gr_error->SetFillColor(kBlack);
		gr_error->Draw("e2 same"); //Superimposes the uncertainties on stack


// #####    ##   ##### #  ####
// #    #  #  #    #   # #    #
// #    # #    #   #   # #    #
// #####  ######   #   # #    #
// #   #  #    #   #   # #    #
// #    # #    #   #   #  ####

// #####  #       ####  #####
// #    # #      #    #   #
// #    # #      #    #   #
// #####  #      #    #   #
// #      #      #    #   #
// #      ######  ####    #

		//-- create subpad to plot ratio
		TPad *pad_ratio = new TPad("pad_ratio", "pad_ratio", 0.0, 0.0, 1.0, 1.0);
		pad_ratio->SetTopMargin(0.75);
		pad_ratio->SetFillColor(0);
		pad_ratio->SetFillStyle(0);
		pad_ratio->SetGridy(1);
		pad_ratio->Draw();
		pad_ratio->cd(0);

		if(use_combine_file && data_notEmpty) //Copy the content of the data graph into a TH1F (NB : symmetric errors...?)
		{
			if(!v_MC_histo[0]) {cout<<__LINE__<<FRED("Error : v_MC_histo[0] is null ! Abort")<<endl; return;}

	        h_sum_data = (TH1F*) v_MC_histo[0]->Clone(); //To clone binning of the MC histos
			h_sum_data->SetFillColor(kBlack);
			h_sum_data->SetLineColor(kBlack);
			// cout<<"h_sum_data->GetNbinsX() "<<h_sum_data->GetNbinsX()<<endl;

	        for(int ipt=0; ipt<g_data->GetN(); ipt++)
	        {
	            g_data->GetPoint(ipt, x, y);
	            double error = g_data->GetErrorY(ipt);

	            h_sum_data->SetBinContent(ipt+1, y);
	            h_sum_data->SetBinError(ipt+1, error);
	        }
		}

		TH1F* histo_ratio_data = 0;
		if(data_notEmpty)
		{
			histo_ratio_data = (TH1F*) h_sum_data->Clone();

			//debug printout
			// cout<<"h_sum_data->GetBinContent(5) "<<h_sum_data->GetBinContent(5)<<endl;
			// cout<<"h_sum_data->GetBinError(5) "<<h_sum_data->GetBinError(5)<<endl;
			// cout<<"histo_total_MC->GetBinContent(5) "<<histo_total_MC->GetBinContent(5)<<endl;
			// cout<<"histo_total_MC->GetBinError(5) "<<histo_total_MC->GetBinError(5)<<endl;

			if(!show_pulls_ratio)
			{
				//To get error bars correct in ratio plot, must only account for errors from data, not MC ! (MC error shown as gray bad)
				for(int ibin=1; ibin<histo_total_MC->GetNbinsX()+1; ibin++)
				{
					histo_total_MC->SetBinError(ibin, 0);
				}

				histo_ratio_data->Divide(histo_total_MC);
			} //Ratio
		 	else //--- Compute pull distrib
			{
				for(int ibin=1; ibin<histo_ratio_data->GetNbinsX()+1; ibin++)
				{
					//Add error on signal strength (since we rescale signal manually)
					// double bin_error_mu = v_MC_histo.at(index_tZq_sample)->GetBinError(ibin) * sig_strength_err;
					// cout<<"bin_error_mu = "<<bin_error_mu<<endl;

					double bin_error_mu = 0; //No sig strength uncert. for prefit ! //-- postfit -> ?

					//Quadratic sum of systs, stat error, and sig strength error
					double bin_error = pow(pow(histo_total_MC->GetBinError(ibin), 2) + pow(histo_ratio_data->GetBinError(ibin), 2) + pow(bin_error_mu, 2), 0.5);

					// if(ibin==1) {cout<<"Data = "<<histo_ratio_data->GetBinContent(1)<<" / Total MC = "<<histo_total_MC->GetBinContent(1)<<" / error = "<<bin_error<<endl;}

					if(!histo_total_MC->GetBinError(ibin)) {histo_ratio_data->SetBinContent(ibin,-99);} //Don't draw null markers
					else{histo_ratio_data->SetBinContent(ibin, (histo_ratio_data->GetBinContent(ibin) - histo_total_MC->GetBinContent(ibin)) / bin_error );}
				}
			}

			//debug printout
			// cout<<"histo_ratio_data->GetBinContent(5) "<<histo_ratio_data->GetBinContent(5)<<endl;
			// cout<<"histo_ratio_data->GetBinError(5) "<<histo_ratio_data->GetBinError(5)<<endl;

			//Don't draw null data
			for(int ibin=1; ibin<histo_ratio_data->GetNbinsX()+1; ibin++)
			{
				// cout<<"histo_ratio_data["<<ibin<<"] = "<<histo_ratio_data->GetBinContent(ibin)<<endl;

				if(std::isnan(histo_ratio_data->GetBinContent(ibin)) || std::isinf(histo_ratio_data->GetBinContent(ibin)) || histo_ratio_data->GetBinContent(ibin) == 0) {histo_ratio_data->SetBinContent(ibin, -99);}
			}
		}
		else {histo_ratio_data = (TH1F*) histo_total_MC->Clone();}

		if(show_pulls_ratio) {histo_ratio_data->GetYaxis()->SetTitle("Pulls");}
		else {histo_ratio_data->GetYaxis()->SetTitle("Data/MC");}
		histo_ratio_data->GetYaxis()->SetTickLength(0.);
		histo_ratio_data->GetXaxis()->SetTitleOffset(1);
		histo_ratio_data->GetYaxis()->SetTitleOffset(1.2);
		histo_ratio_data->GetYaxis()->SetLabelSize(0.048);
		histo_ratio_data->GetXaxis()->SetLabelFont(42);
		histo_ratio_data->GetYaxis()->SetLabelFont(42);
		histo_ratio_data->GetXaxis()->SetTitleFont(42);
		histo_ratio_data->GetYaxis()->SetTitleFont(42);
		histo_ratio_data->GetYaxis()->SetNdivisions(503); //grid draw on primary tick marks only
		histo_ratio_data->GetXaxis()->SetNdivisions(505);
		histo_ratio_data->GetYaxis()->SetTitleSize(0.06);
		histo_ratio_data->GetXaxis()->SetTickLength(0.04);
		histo_ratio_data->SetMarkerStyle(20);
		histo_ratio_data->SetMarkerSize(1.2); //changed from 1.4

		//NB : when using SetMaximum(), points above threshold are simply not drawn
		//So for ratio plot, even if point is above max but error compatible with 1, point/error bar not represented!
		if(show_pulls_ratio)
		{
			histo_ratio_data->SetMinimum(-2.99);
			histo_ratio_data->SetMaximum(2.99);
		}
		else
		{
			histo_ratio_data->SetMinimum(-0.2);
			histo_ratio_data->SetMaximum(2.2);
		}

		if(drawInputVars)
		{
			histo_ratio_data->GetXaxis()->SetTitle(total_var_list[ivar]);

            //Hardcode some names for manuscript plots
            if(analysis_type == "tHq")
            {
                if(total_var_list[ivar] == "inv_mll") {histo_ratio_data->GetXaxis()->SetTitle("m_{ll} (OSSF pair)");}
                else if(total_var_list[ivar] == "nMediumBJets") {histo_ratio_data->GetXaxis()->SetTitle("B-tagged jet multiplicity");}
                else if(total_var_list[ivar] == "metpt") {histo_ratio_data->GetXaxis()->SetTitle("Missing transverse energy");}
            }
            else if(analysis_type == "FCNC")
            {
                if(total_var_list[ivar] == "inv_mll") {histo_ratio_data->GetXaxis()->SetTitle("m_{ll} (OSSF pair)");}
                else if(total_var_list[ivar] == "minDRll") {histo_ratio_data->GetXaxis()->SetTitle("Min. #DeltaR between 2 leptons");}
            }
		}
		else
		{
			if(template_name=="2D") {histo_ratio_data->GetXaxis()->SetTitle("BDT2D bin");}
			else if(template_name=="2Dlin") {histo_ratio_data->GetXaxis()->SetTitle("BDT bin");}
			else {histo_ratio_data->GetXaxis()->SetTitle(classifier_name+" (vs "+template_name + ")");}

			if(template_name == "categ") //Vertical text X labels (categories names)
			{
				histo_ratio_data->GetXaxis()->SetTitle("Categ.");
				histo_ratio_data->GetXaxis()->SetLabelSize(0.08);
				histo_ratio_data->GetXaxis()->SetLabelOffset(0.02);

				int nx = 2;
				if(nLep_cat == "2l")
				{
					const char *labels[5]  = {"ee", "e#mu bl", "e#mu bt", "#mu#mu bl", "#mu#mu bt"};
					for(int i=1;i<=5;i++) {histo_ratio_data->GetXaxis()->SetBinLabel(i,labels[i-1]);}
				}
				else if(nLep_cat == "3l")
				{
					const char *labels[2]  = {"bl", "bt",};
					for(int i=1;i<=2;i++) {histo_ratio_data->GetXaxis()->SetBinLabel(i,labels[i-1]);}
				}
			}
		}

		pad_ratio->cd(0);
		if(show_pulls_ratio) {histo_ratio_data->Draw("HIST P");} //Draw ratio points
		else {histo_ratio_data->Draw("E1X0 P");} //Draw ratio points ; E1 : perpendicular lines at end ; X0 : suppress x errors

// ###### #####  #####   ####  #####   ####     #####    ##   ##### #  ####
// #      #    # #    # #    # #    # #         #    #  #  #    #   # #    #
// #####  #    # #    # #    # #    #  ####     #    # #    #   #   # #    #
// #      #####  #####  #    # #####       #    #####  ######   #   # #    #
// #      #   #  #   #  #    # #   #  #    #    #   #  #    #   #   # #    #
// ###### #    # #    #  ####  #    #  ####     #    # #    #   #   #  ####

		TGraphAsymmErrors* gr_ratio_error = 0;
		if(draw_errors)
		{
			//Copy previous TGraphAsymmErrors, then modify it -> error TGraph for ratio plot
			TGraphAsymmErrors *thegraph_tmp;
			double *theErrorX_h;
			double *theErrorY_h;
			double *theErrorX_l;
			double *theErrorY_l;
			double *theY;
			double *theX;

			thegraph_tmp = (TGraphAsymmErrors*) gr_error->Clone();
			theErrorX_h = thegraph_tmp->GetEXhigh();
			theErrorY_h = thegraph_tmp->GetEYhigh();
			theErrorX_l = thegraph_tmp->GetEXlow();
			theErrorY_l = thegraph_tmp->GetEYlow();
			theY        = thegraph_tmp->GetY() ;
			theX        = thegraph_tmp->GetX() ;

			//Divide error --> ratio
			for(int i=0; i<thegraph_tmp->GetN(); i++)
			{
				theErrorY_l[i] = theErrorY_l[i]/theY[i];
				theErrorY_h[i] = theErrorY_h[i]/theY[i];
				theY[i]=1; //To center the filled area around "1"
			}

			gr_ratio_error = new TGraphAsymmErrors(thegraph_tmp->GetN(), theX , theY ,  theErrorX_l, theErrorX_h, theErrorY_l, theErrorY_h);
			gr_ratio_error->SetFillStyle(3002);
			gr_ratio_error->SetFillColor(kBlack);
			// gr_ratio_error->SetFillColor(kCyan);

			pad_ratio->cd(0);
			if(!show_pulls_ratio) {gr_ratio_error->Draw("e2 same");} //Draw error bands in ratio plot
		} //draw errors


//  ####   ####   ####  #    # ###### ##### #  ####   ####
// #    # #    # #      ##  ## #        #   # #    # #
// #      #    #  ####  # ## # #####    #   # #       ####
// #      #    #      # #    # #        #   # #           #
// #    # #    # #    # #    # #        #   # #    # #    #
//  ####   ####   ####  #    # ######   #   #  ####   ####

		//-- Draw ratio y-lines manually
		TH1F *h_line1 = 0;
		TH1F *h_line2 = 0;
		if(data_notEmpty)
		{
			h_line1 = new TH1F("","",this->nbins, h_sum_data->GetXaxis()->GetXmin(), h_sum_data->GetXaxis()->GetXmax());
			h_line2 = new TH1F("","",this->nbins, h_sum_data->GetXaxis()->GetXmin(), h_sum_data->GetXaxis()->GetXmax());
			// TH1F *h_line3 = new TH1F("","",this->nbins, h_sum_data->GetXaxis()->GetXmin(), h_sum_data->GetXaxis()->GetXmax());
			for(int ibin=1; ibin<this->nbins +1; ibin++)
			{
				if(show_pulls_ratio)
				{
					h_line1->SetBinContent(ibin, -1);
					h_line2->SetBinContent(ibin, 1);
				}
				else
				{
					h_line1->SetBinContent(ibin, 0.5);
					h_line2->SetBinContent(ibin, 1.5);
				}
			}
			h_line1->SetLineStyle(6);
			h_line2->SetLineStyle(6);
			h_line1->Draw("hist same");
			h_line2->Draw("hist same");
		}

		double xmax_stack = stack_MC->GetXaxis()->GetXmax();
		double xmin_stack = stack_MC->GetXaxis()->GetXmin();
		TString Y_label = "Events";
		if(data_notEmpty)
		{
			double xmax_data = h_sum_data->GetXaxis()->GetXmax();
			double xmin_data = h_sum_data->GetXaxis()->GetXmin();
			Y_label = "Events / " + Convert_Number_To_TString( (xmax_data - xmin_data) / h_sum_data->GetNbinsX(), 2); //Automatically get the Y label depending on binning
		}

		if(stack_MC!= 0)
		{
			stack_MC->GetXaxis()->SetLabelFont(42);
			stack_MC->GetYaxis()->SetLabelFont(42);
			stack_MC->GetYaxis()->SetTitleFont(42);
			stack_MC->GetYaxis()->SetTitleSize(0.06);
			stack_MC->GetYaxis()->SetTickLength(0.04);
			stack_MC->GetXaxis()->SetLabelSize(0.0);
			stack_MC->GetYaxis()->SetLabelSize(0.048);
			stack_MC->GetXaxis()->SetNdivisions(505);
			stack_MC->GetYaxis()->SetNdivisions(506);
			stack_MC->GetYaxis()->SetTitleOffset(1.2);
			stack_MC->GetYaxis()->SetTitle(Y_label);
		}

	//----------------
	// CAPTIONS //
	//----------------
	// -- using https://twiki.cern.ch/twiki/pub/CMS/Internal/FigGuidelines

        bool draw_cms_prelim_label = false;

		float l = c1->GetLeftMargin();
		float t = c1->GetTopMargin();

		TString cmsText = "CMS";
		TLatex latex;
		latex.SetNDC();
		latex.SetTextColor(kBlack);
		latex.SetTextFont(61);
		latex.SetTextAlign(11);
		latex.SetTextSize(0.06);
		if(draw_cms_prelim_label) {latex.DrawLatex(l + 0.01, 0.92, cmsText);}

		TString extraText = "Preliminary";
		latex.SetTextFont(52);
		latex.SetTextSize(0.05);
		if(draw_cms_prelim_label)
		{
            if(analysis_type != "tHq")
            {
                latex.DrawLatex(l + 0.12, 0.92, extraText);
            }
            else {latex.DrawLatex(l, 0.92, extraText);}
		}

		float lumi = ref_luminosity * luminosity_rescale;
		TString lumi_13TeV = Convert_Number_To_TString(lumi);
		lumi_13TeV += " fb^{-1} (13 TeV)";

		latex.SetTextFont(42);
		latex.SetTextAlign(31);
		latex.SetTextSize(0.04);

		if(use_diff_legend_layout) {latex.DrawLatex(0.95, 0.92,lumi_13TeV);}
		else
		{
			// if(h_sum_data->GetBinContent(h_sum_data->GetNbinsX() ) > h_sum_data->GetBinContent(1) ) {latex.DrawLatex(0.95, 0.92,lumi_13TeV);}
			// else {latex.DrawLatex(0.78, 0.92,lumi_13TeV);}
			latex.DrawLatex(0.78, 0.92,lumi_13TeV);
		}

		//------------------
		//-- Channel info
		TLatex text2 ;
		text2.SetNDC();
		text2.SetTextAlign(13);
		text2.SetTextSize(0.045);
		text2.SetTextFont(42);

		TString info_data;
		if(nLep_cat == "2l")
		{
			info_data = "2lss l^{#pm}l^{#pm}";

			if(channel == "eu" || channel == "ue" || channel == "em" || channel == "me") {info_data = "2lss e^{#pm}#mu^{#pm}";}
			else if(channel == "uu" || channel == "mm") {info_data = "2lss #mu^{#pm}#mu^{#pm}";}
			if(channel == "ee") {info_data = "2lss e^{#pm}e^{#pm}";}
		}
		else if(nLep_cat == "3l")
		{
			// info_data = "3l";
			info_data = "l^{#pm}l^{#pm}l^{#pm}";
		}

		// if(h_sum_data->GetBinContent(h_sum_data->GetNbinsX() ) > h_sum_data->GetBinContent(1) ) {text2.DrawLatex(0.55,0.87,info_data);}
		// else {text2.DrawLatex(0.20,0.87,info_data);}
		text2.DrawLatex(0.23,0.86,info_data);

		//Hardcoded range : looks like, when given input histos with different binnings, the combine output returns histos with the largest range used... so may need to change the range manually ?
		if(use_combine_file)
		{
			if(analysis_type == "ttH" && nLep_cat == "3l") //for ttH binning only ! tHq use 10 bins for both SRs
			{
				stack_MC->GetXaxis()->SetRangeUser(0, 6);
				histo_ratio_data->GetXaxis()->SetRangeUser(0, 6);
			}
			else if(analysis_type == "tHq" && nLep_cat == "2l") //pallabi uses 8 bins, I use 9 for now
			{
				stack_MC->GetXaxis()->SetRangeUser(0, 8);
				histo_ratio_data->GetXaxis()->SetRangeUser(0, 8);
			}
		}



// #    # #####  # ##### ######     ####  #    # ##### #####  #    # #####
// #    # #    # #   #   #         #    # #    #   #   #    # #    #   #
// #    # #    # #   #   #####     #    # #    #   #   #    # #    #   #
// # ## # #####  #   #   #         #    # #    #   #   #####  #    #   #
// ##  ## #   #  #   #   #         #    # #    #   #   #      #    #   #
// #    # #    # #   #   ######     ####   ####    #   #       ####    #

		if(drawInputVars)
		{
			mkdir("plots", 0777);
			mkdir("plots/input_vars", 0777);
			mkdir( ("plots/input_vars/"+nLep_cat).Data(), 0777);
			mkdir( ("plots/input_vars/"+nLep_cat+"/"+region).Data(), 0777);
		}
		else
		{
			mkdir("plots", 0777);
			mkdir("plots/templates", 0777);
			mkdir( ("plots/templates/"+nLep_cat).Data(), 0777);
			mkdir( ("plots/templates/"+nLep_cat+"/"+region).Data(), 0777);
			if(prefit) {mkdir( ("plots/templates/"+nLep_cat+"/"+region+"/prefit").Data(), 0777);}
			else {mkdir( ("plots/templates/"+nLep_cat+"/"+region+"/postfit").Data(), 0777);}
		}

		//Output
		TString output_plot_name;

		if(drawInputVars)
		{
			output_plot_name = "plots/input_vars/" + nLep_cat + "/" + region + "/" + total_var_list[ivar];
		}
		else
		{
			output_plot_name = "plots/templates/" + nLep_cat + "/" + region;
			if(prefit) {output_plot_name+= "/prefit/";}
			else {output_plot_name+= "/postfit/";}
			output_plot_name+= classifier_name + template_name + "_" + nLep_cat +"_template";
		}
		if(channel != "") {output_plot_name+= "_" + channel;}
		output_plot_name+= this->filename_suffix;
		if(signal_process.Contains("hut") ) {output_plot_name+= "_hut";}
		if(signal_process.Contains("hct") ) {output_plot_name+= "_hct";}
		if(draw_logarithm) {output_plot_name+= "_log";}
		output_plot_name+= this->plot_extension;

		c1->SaveAs(output_plot_name);

		if(data_notEmpty)
		{
			delete h_line1; h_line1 = NULL;
			delete h_line2; h_line2 = NULL;
		}

		if(draw_errors) {delete gr_error; delete gr_ratio_error; gr_error = NULL; gr_ratio_error = NULL;}
		delete pad_ratio; pad_ratio = NULL;

		delete c1; c1 = NULL;
		delete qw; qw = NULL;
		delete stack_MC; stack_MC = NULL;

		if(use_combine_file) {delete g_data; g_data = NULL;}
	} //Var loop

	file_input->Close();

	return;
}































//--------------------------------------------
// ##     ## ######## ##       ########  ######## ########     ######## ##     ## ##    ##  ######   ######
// ##     ## ##       ##       ##     ## ##       ##     ##    ##       ##     ## ###   ## ##    ## ##    ##
// ##     ## ##       ##       ##     ## ##       ##     ##    ##       ##     ## ####  ## ##       ##
// ######### ######   ##       ########  ######   ########     ######   ##     ## ## ## ## ##        ######
// ##     ## ##       ##       ##        ##       ##   ##      ##       ##     ## ##  #### ##             ##
// ##     ## ##       ##       ##        ##       ##    ##     ##       ##     ## ##   ### ##    ## ##    ##
// ##     ## ######## ######## ##        ######## ##     ##    ##        #######  ##    ##  ######   ######
//--------------------------------------------











//--------------------------------------------
 // #    # #  ####  #####  ####     #####   ####   ####
 // #    # # #        #   #    #    #    # #    # #    #
 // ###### #  ####    #   #    #    #    # #    # #
 // #    # #      #   #   #    #    #####  #    # #
 // #    # # #    #   #   #    #    #   #  #    # #    #
 // #    # #  ####    #    ####     #    #  ####   ####

 // #    # ###### #####    ##    ####     #####  #    # #    #
 // #   #  #      #    #  #  #  #         #    # ##   # ##   #
 // ####   #####  #    # #    #  ####     #    # # #  # # #  #
 // #  #   #      #####  ######      #    #    # #  # # #  # #
 // #   #  #      #   #  #    # #    #    #    # #   ## #   ##
 // #    # ###### #    # #    #  ####     #####  #    # #    #
//--------------------------------------------


/**
 * Produce output file containing DNN-Keras templates, in a correct format (binning, ...) for plotting ROC curves from histograms
 * Same as calling directly the function Produce_Templates(), but here we make sure that all the class members are set correctly first.
 * @param nbins : choose the binning used for plotting ROC (e.g. 100)
 */
void TopEFT_analysis::Produce_KerasDNN_Histos_For_ROC(TString template_name, int nbins/*=100*/)
{
	//Save the current members' values
	int nbins_tmp = nbins;
	TString classifier_name_tmp = classifier_name;
	TString DNN_type_tmp = DNN_type;
	TString region_tmp = region;
	TString dir_ntuples_tmp = dir_ntuples;
	vector<TString> sample_list_tmp = sample_list;

	//Modify temporarily the members' values
	nbins = nbins;
	classifier_name = "DNN";
	DNN_type = "Keras";
	if(train_with_trainingSel)  //Will use training selection events for application (instead in CR/SR)
	{
		region = "Training";
		dir_ntuples = dir_ntuples_noRegion; //Training ntuples stored in parent dir.
	}

	//Define list of samples to use
	sample_list.resize(0);
	for(int isample=0; isample<sample_list_tmp.size(); isample++)
	{
		if(sample_list_tmp[isample].Contains(signal_process) ) {sample_list.push_back(sample_list_tmp[isample]);}
	}

	if(template_name == "ttV")
	{
		sample_list.push_back("ttZ");
		sample_list.push_back("ttW_PSweights");
	}
	else if(template_name == "ttbar")
	{
		sample_list.push_back("TTbar_DiLep_PSweights");
		sample_list.push_back("TTbar_SemiLep_PSweights");
	}

	//Rename output, to make it different than regular templates
	TString oldname = "outputs/Templates_" + classifier_name + template_name + "_" + nLep_cat + "_" + region + filename_suffix + "_Keras.root";
	TString newname = "outputs/HistoForROC_DNN" + template_name;
	if(use_3l_events) newname+= "_3l";
	else newname+= "_2l";
	newname+= "_" + region + filename_suffix + "_Keras.root";
	// newname+= filename_suffix + "_Keras.root";

	Produce_Templates(template_name, false); //Produce histos //Use LO samples (same used for training)
	MoveFile(oldname, newname); //Rename output

	cout<<endl<<FMAG("==> Renamed as such : ")<<newname<<endl;

	Plot_KerasDNN_Outputs(newname, sample_list, template_name);

	//Restore the previous members' values
	nbins = nbins_tmp;
	classifier_name = classifier_name_tmp;
	DNN_type = DNN_type_tmp;
	region = region_tmp;
	dir_ntuples = dir_ntuples_tmp;
	sample_list = sample_list_tmp;

	return;
}


void TopEFT_analysis::Plot_KerasDNN_Outputs(TString filepath, vector<TString> sample_list, TString template_name)
{
	Load_Canvas_Style();

	TFile* f = TFile::Open(filepath);
	TH1F *h_sig = 0, *h_bkg = 0;

	for(int isample=0; isample<sample_list.size(); isample++)
	{
		TString histo_name = "DNN" + template_name + "_" + nLep_cat + "_all__" + sample_list[isample];
		TH1F* h_tmp = (TH1F*) f->Get(histo_name);

		if(!h_sig) {h_sig = (TH1F*) h_tmp->Clone();} //sig is first
		else if(!h_bkg) {h_bkg = (TH1F*) h_tmp->Clone();} //then first bkg
		else {h_bkg->Add(h_tmp);} //second bkg

		delete h_tmp; h_tmp = NULL;
	}

	TCanvas* c = new TCanvas("", "", 1000, 800);
	c->SetTopMargin(0.1);

	TLegend* qw = new TLegend(.40,.68,0.65,0.85);
	qw->SetTextSize(0.04);
	qw->SetBorderSize(0.);

	h_sig->Scale(1./h_sig->Integral());
	h_bkg->Scale(1./h_bkg->Integral());

	h_sig->SetLineColor(kBlue);
	h_bkg->SetLineColor(kRed);
	h_sig->SetLineWidth(2);
	h_bkg->SetLineWidth(2);

	h_sig->Rebin(2);
	h_bkg->Rebin(2);

	//Set Yaxis maximum & minimum
	if(h_sig->GetMaximum() > h_bkg->GetMaximum() ) {h_sig->SetMaximum(h_sig->GetMaximum()*1.2);}
	else {h_sig->SetMaximum(h_bkg->GetMaximum()*1.2);}

	qw->AddEntry(h_sig, "Signal", "L");
	qw->AddEntry(h_bkg, "Background", "L");

	h_sig->GetXaxis()->SetTitle("DNN"+template_name + " " + nLep_cat + " output");
	h_sig->GetYaxis()->SetTitle("PDF (Entire training samples)");
	h_sig->GetYaxis()->SetLabelSize(0.02);

	h_sig->Draw("hist");
	h_bkg->Draw("hist same");
	qw->Draw("same");

	//--- Captions
	float l = c->GetLeftMargin();
	float t = c->GetTopMargin();
	TString cmsText = "CMS";
	TLatex latex;
	latex.SetNDC();
	latex.SetTextAngle(0);
	latex.SetTextColor(kBlack);
	latex.SetTextFont(61);
	latex.SetTextAlign(11);
	latex.SetTextSize(0.06);
	latex.DrawLatex(l + 0.01, 0.92, cmsText);
	TString extraText = "Preliminary";
	latex.SetTextFont(52);
	latex.SetTextSize(0.05);
	latex.DrawLatex(l + 0.12, 0.92, extraText);
	float lumi = ref_luminosity * luminosity_rescale;
	TString lumi_13TeV = Convert_Number_To_TString(lumi);
	lumi_13TeV += " fb^{-1} (13 TeV)";
	latex.SetTextFont(42);
	latex.SetTextAlign(31);
	latex.SetTextSize(0.04);
	latex.DrawLatex(0.95, 0.92,lumi_13TeV);

	TString outname = "plots/DNN" + template_name + "_" + nLep_cat + ".png";
	c->SaveAs(outname);

	delete c; c = NULL;
	delete h_sig; h_sig = NULL;
	delete h_bkg; h_bkg = NULL;
	f->Close();

	return;
}

















//--------------------------------------------
 // #####  #####    ##   #    #     ####   ####  #      ######
 // #    # #    #  #  #  #    #    #    # #    # #          #
 // #    # #    # #    # #    #    #      #    # #         #
 // #    # #####  ###### # ## #    #      #    # #        #
 // #    # #   #  #    # ##  ##    #    # #    # #       #
 // #####  #    # #    # #    #     ####   ####  ###### ######
//--------------------------------------------


void TopEFT_analysis::Draw_ColZ_Templates(TString type)
{
	cout<<endl<<BOLD(FYEL("##################################"))<<endl;
	cout<<FYEL("--- Producing ColZ Template Plots / Type : "<<type<<" ---")<<endl;
	cout<<BOLD(FYEL("##################################"))<<endl<<endl;

	// TString input_name = "outputs/Reader_" + template_name + this->filename_suffix + ".root";
	TString input_name = "outputs/Templates_BDT2D_" + nLep_cat + "_" + region;
	input_name+= filename_suffix + ".root";

	TFile* file_input = 0;
	file_input = TFile::Open(input_name);
	if(file_input == 0) {cout<<endl<<BOLD(FRED("--- File not found ! Exit !"))<<endl<<endl; return;}

	TH2F* h_sig = 0;
	TH2F* h_bkg = 0;
	TH2F* h_tmp = 0;
	TH2F* ptr_hist = 0;

	//---------------------------
	//RETRIEVE & SUM HISTOGRAMS
	//---------------------------

	// for(int ichan=0; ichan<channel_list.size(); ichan++)
	{
		//--- All MC samples
		for(int isample = 0; isample < sample_list.size(); isample++)
		{
			bool is_signalSample = false;
			bool is_bkgSample = false;

			if((sample_list[isample].Contains("tHq") || sample_list[isample].Contains("tHW") || sample_list[isample].Contains("ttH")) && sample_list[isample].Contains("_h")) {continue;} //only care about full signal samples, not split by H decay

			//Protections
			if(nLep_cat == "3l" && sample_list[isample] == "QFlip") {continue;}
			if(use_MC_fakes && sample_list[isample].Contains("Fakes") ) {continue;} //Use MC fakes sample
			else if(!use_MC_fakes && sample_list[isample].Contains("TTbar") ) {continue;} //Use DD Fakes sample

			if(sample_list[isample].Contains("DATA") ) {continue;}

			if(analysis_type == "tHq")
			{
				if(sample_list[isample].Contains("tHq") || sample_list[isample].Contains("tHW") || sample_list[isample].Contains("ttH") ) {is_signalSample = true;}
				else {is_bkgSample = true;}

				// if(type == "signal" && sample_list[isample] != "tHq") {continue;}
				// else if(type == "bkg" && (sample_list[isample] == "tHq" || sample_list[isample] == "tHW")) {continue;}
			}
			else if(analysis_type == "FCNC") //depends on signal
			{
				// cout<<"Sample : "<<sample_list[isample]<<endl;
				// cout<<"signal_process "<<signal_process<<endl;

				if(signal_process == "hut_FCNC" && sample_list[isample].Contains("hut") ) {cout<<__LINE__<<endl; is_signalSample = true;}
				else if(signal_process == "hct_FCNC" && sample_list[isample].Contains("hct") ) {is_signalSample = true;}
				else if(signal_process == "ST_hut_FCNC" && sample_list[isample] == "ST_hut_FCNC") {is_signalSample = true;}
				else if(signal_process == "TT_hut_FCNC" && sample_list[isample] == "TT_hut_FCNC") {is_signalSample = true;}
				else if(signal_process == "ST_hct_FCNC" && sample_list[isample] == "ST_hct_FCNC") {is_signalSample = true;}
				else if(signal_process == "TT_hct_FCNC" && sample_list[isample] == "TT_hct_FCNC") {is_signalSample = true;}
				else {is_bkgSample = true;}

				// if(type == "signal" && signal_process == "hut_FCNC" && !sample_list[isample].Contains("hut") ) {continue;}
				// else if(type == "signal" && signal_process == "hct_FCNC" && !sample_list[isample].Contains("hct") ) {continue;}
				// else if(type == "signal" && signal_process == "ST_hut_FCNC" && sample_list[isample] != "ST_hut_FCNC") {continue;}
				// else if(type == "signal" && signal_process == "TT_hut_FCNC" && sample_list[isample] != "TT_hut_FCNC") {continue;}
				// else if(type == "signal" && signal_process == "ST_hct_FCNC" && sample_list[isample] != "ST_hct_FCNC") {continue;}
				// else if(type == "signal" && signal_process == "TT_hct_FCNC" && sample_list[isample] != "TT_hct_FCNC") {continue;}
				// else if(type == "bkg" && (sample_list[isample].Contains("hut") || sample_list[isample].Contains("hct"))) {continue;}
			}
			else {cout<<"ERROR "<<endl; return;}

			if(type == "signal" && !is_signalSample) {continue;}
			if(type == "bkg" && !is_bkgSample) {continue;}

			cout<<"//--------------------------------------------"<<endl;
			cout<<"Sample : "<<sample_list[isample]<<endl;
			cout<<"is_signalSample "<<is_signalSample<<endl;
			cout<<"is_bkgSample "<<is_bkgSample<<endl;

			h_tmp = 0;

			TString histo_name = "BDT2D_" + nLep_cat + "_" + region + "__" + sample_list[isample];

			if(!file_input->GetListOfKeys()->Contains(histo_name) ) {cout<<histo_name<<" : not found"<<endl; continue;}

			h_tmp = (TH2F*) file_input->Get(histo_name)->Clone();

			// if(type == "sb")
			// {
			// 	h_tmp->Rebin2D(2, 2);
			// }

			// cout<<"Sample  : "<<sample_list[isample]<<endl;
			// for(int ibin=1; ibin<21; ibin++)
			// {
			// 	for(int jbin=1; jbin<21; jbin++)
			// 	{
			// 		if(type != "sb") {continue;}
			//
			// 		if(h_tmp->GetBinContent(ibin, jbin) < 0)
			// 		{
			// 			// cout<<"ibin "<<ibin<<" / jbin "<<jbin<<endl;
			//
			// 			cout<<"content : "<<h_tmp->GetBinContent(ibin, jbin)<<endl;
			// 		}
			// 	}
			// }

			if(type == "signal")
			{
				if(!h_sig) {h_sig = (TH2F*) h_tmp->Clone();}
				else {h_sig->Add(h_tmp);}
			}
			else if(type == "bkg")
			{
				if(!h_bkg) {h_bkg = (TH2F*) h_tmp->Clone();}
				else {h_bkg->Add(h_tmp);}
			}
			else if(type == "sb")
			{
				// if(sample_list[isample] == "tHq")
				if(is_signalSample) //as tHq2016
				{
					if(!h_sig) {h_sig = (TH2F*) h_tmp->Clone();}
					else {h_sig->Add(h_tmp);}
				}
				else if(is_bkgSample)
				{
					if(!h_bkg) {h_bkg = (TH2F*) h_tmp->Clone();}
					else {h_bkg->Add(h_tmp);}
				}
			}

			delete h_tmp; h_tmp = NULL;
		} //end sample loop
	} // end chanl loop

	cout<<"h_sig "<<h_sig<<endl;
	cout<<"h_bkg "<<h_bkg<<endl;
	if(h_sig) {cout<<"h_sig->Integral() "<<h_sig->Integral()<<endl;}
	if(h_bkg) {cout<<"h_bkg->Integral() "<<h_bkg->Integral()<<endl;}

	//Genric pointer, depending on histo we're interested in
	if(type == "signal" || type == "sb") {ptr_hist = h_sig;}
	else if(type == "bkg") {ptr_hist = h_bkg;}

	for(int ibin=1; ibin<ptr_hist->GetNbinsX()+1; ibin++)
	{
		// cout<<"ibin "<<ibin<<endl;

		for(int jbin=1; jbin<ptr_hist->GetNbinsY()+1; jbin++)
		{
			// cout<<"jbin "<<jbin<<endl;

			if(type == "signal" && h_sig->GetBinContent(ibin, jbin) < 0) {h_sig->SetBinContent(ibin, jbin, 0);}
			else if(type == "bkg" && h_bkg->GetBinContent(ibin, jbin) < 0) {h_bkg->SetBinContent(ibin, jbin, 0);}
			else if(type == "sb")
			{
				if(h_sig->GetBinContent(ibin, jbin) <= 0) h_sig->SetBinContent(ibin, jbin, 0);
				if(h_bkg->GetBinContent(ibin, jbin) <= 0) h_bkg->SetBinContent(ibin, jbin, 0);
			}

			// if(h_sig->GetBinContent(ibin, jbin) < 0 || h_bkg->GetBinContent(ibin, jbin) < 0)
			// {
			// 	cout<<"sig content : "<<h_sig->GetBinContent(ibin, jbin)<<endl;
			// 	cout<<"bkg content : "<<h_bkg->GetBinContent(ibin, jbin)<<endl;
			// }
		}
	}

	// cout<<"Integral : "<<ptr_hist->Integral()<<endl;

	ptr_hist->GetXaxis()->SetLabelFont(42);
	ptr_hist->GetYaxis()->SetLabelFont(42);
	ptr_hist->GetYaxis()->SetTitleFont(42);
	ptr_hist->GetYaxis()->SetTitleSize(0.06);
	ptr_hist->GetYaxis()->SetTickLength(0.04);
	ptr_hist->GetYaxis()->SetLabelSize(0.048);
	ptr_hist->GetYaxis()->SetNdivisions(505);
	ptr_hist->GetXaxis()->SetTitleFont(42);
	ptr_hist->GetXaxis()->SetTitleSize(0.06);
	ptr_hist->GetXaxis()->SetTickLength(0.04);
	ptr_hist->GetXaxis()->SetLabelSize(0.048);
	ptr_hist->GetXaxis()->SetNdivisions(505);
	ptr_hist->GetYaxis()->SetTitleOffset(1.2);
	ptr_hist->GetYaxis()->SetTitle("BDT-ttV");
	ptr_hist->GetXaxis()->SetTitle("BDT-ttbar");

	//Canvas definition
	Load_Canvas_Style();
	TCanvas* c1 = new TCanvas("c1","c1", 1200, 800);
	// c1->SetTopMargin(0.1);
	// c1->SetBottomMargin(0.25);
	c1->SetRightMargin(0.15);

	if(type == "signal") {h_sig->Draw("colz");}
	else if(type == "bkg") {h_bkg->Draw("colz");}
	else if(type == "sb")
	{
		if(!h_sig || !h_bkg) {cout<<FRED("Error ! Null histo")<<endl;}

		// h_sig->Scale(1./h_sig->Integral());
		// h_bkg->Scale(1./h_bkg->Integral());

		// cout<<"h_sig->GetBinContent(10, 10) "<<h_sig->GetBinContent(10, 10)<<endl;
		h_sig->Divide(h_bkg);
		// cout<<"h_sig->GetBinContent(10, 10) "<<h_sig->GetBinContent(10, 10)<<endl;

        // vector<float> v_sb_frac;
        // vector<float> v_bkg_yield;

		//Printout
		for(int ibin=0; ibin<10; ibin++)
		{
            cout<<"//--------------------------------------------"<<endl;

			for(int jbin=0; jbin<10; jbin++)
			{
                float bkg_yield = h_bkg->GetBinContent(ibin+1, jbin+1);
                float sig_yield = h_sig->GetBinContent(ibin+1, jbin+1);
				float fraction = 100 * sig_yield; // /!\ h_sig already divided by h_bkg !
				if(fraction != 0) cout<<"Xbin="<<ibin+1<<" / Ybin="<<jbin+1<<" --> S/B = "<<fraction<<" % // Bkg = "<<bkg_yield<<endl;
				// cout<<"h_sig->GetBinContent() "<<h_sig->GetBinContent(ibin+1, jbin+1)<<endl;
				// cout<<"h_bkg->GetBinContent() "<<h_bkg->GetBinContent(ibin+1, jbin+1)<<endl;

                // v_sb_frac.push_back(fraction);
                // v_bkg_yield.push_back(bkg_yield);
            }
		}

		// h_sig->Scale(1./h_sig->Integral());

		if(analysis_type == "tHq")
		{
			h_sig->SetMaximum(1.);
		}
		else
		{
			h_sig->SetMaximum(5);
		}

		h_sig->Draw("colz");
	}

	mkdir("plots", 0777);
	mkdir("plots/templates", 0777);
	mkdir("plots/templates/3l", 0777);
	mkdir("plots/templates/2l", 0777);

	//Output
	TString output_plot_name = "plots/templates/" + nLep_cat + "/BDT2D_";
	output_plot_name+= nLep_cat +"_template_all_" + type;
	output_plot_name+= this->filename_suffix + this->plot_extension;

	c1->SaveAs(output_plot_name);

	delete file_input; file_input = NULL;

	delete c1; c1 = NULL;

	return;
}






void TopEFT_analysis::Draw_ColZ_All()
{
	Draw_ColZ_Templates("signal");
	Draw_ColZ_Templates("bkg");
	Draw_ColZ_Templates("sb");

	return;
}




void TopEFT_analysis::Make_Overlap_Plots()
{
	vector<TString> v_samples;
    v_samples.push_back("tHq");
    v_samples.push_back("tHW");
    v_samples.push_back("ttH");

	ofstream ofile("overlap.txt");
	ofile<<"### OVERLAP BETWEEN 'THQ' AND 'TTH' CATEGORIES ###"<<endl<<endl<<endl;

	for(int isample=0; isample<v_samples.size(); isample++)
	{
		TString filename = dir_ntuples_allEvents + v_samples[isample] + ".root";

		if(!Check_File_Existence(filename) ) {cout<<"File "<<filename<<" not found !"<<endl; return;}

		TFile* f = TFile::Open(filename);

		ofile<<"* Sample "<<v_samples[isample]<<" :"<<endl<<endl;

		Plot_Overlap_Categories(f, ofile, v_samples[isample]);

		f->Close();
	}

	return;
}


































//--------------------------------------------
//  ######   ######     ###    ##       ########    ##     ##    ###    ########  ####    ###    ######## ####  #######  ##    ##
// ##    ## ##    ##   ## ##   ##       ##          ##     ##   ## ##   ##     ##  ##    ## ##      ##     ##  ##     ## ###   ##
// ##       ##        ##   ##  ##       ##          ##     ##  ##   ##  ##     ##  ##   ##   ##     ##     ##  ##     ## ####  ##
//  ######  ##       ##     ## ##       ######      ##     ## ##     ## ########   ##  ##     ##    ##     ##  ##     ## ## ## ##
//       ## ##       ######### ##       ##           ##   ##  ######### ##   ##    ##  #########    ##     ##  ##     ## ##  ####
// ##    ## ##    ## ##     ## ##       ##            ## ##   ##     ## ##    ##   ##  ##     ##    ##     ##  ##     ## ##   ###
//  ######   ######  ##     ## ######## ########       ###    ##     ## ##     ## #### ##     ##    ##    ####  #######  ##    ##
//--------------------------------------------

/**
 * Compare the effect of the different scale variations on the BDT/input var shape
 * Produce the histograms, and call func to plot them
 * Extended to also deal with PDF variations (produce histos)
 */
void TopEFT_analysis::Make_ScaleVariations_Histograms(bool plotOnly, bool split_by_channel, bool normalize_all_histograms)
{
	bool use_TTbar_MC = false; //true <-> replace DD fakes by TTbar

	bool use_rebinning = false; //use ttH-2017 or tHq-2016 binning
//--------------------------------------------
	cout<<endl<<BOLD(FYEL("##################################"))<<endl;
	cout<<FYEL("-- Producing Scale Variations templates and plots --")<<endl;
	cout<<endl<<BOLD(FYEL("##################################"))<<endl;

//--------------------------------------------
	//--- All variables that we we want to plot (e.g. : "nJet25", "BDT_ttV", ...)
    vector<TString> v_variables;
	// v_variables.push_back("nJet25");
	// v_variables.push_back("metpt");
	v_variables.push_back("signal_2lss_TTV_MVA");
	v_variables.push_back("signal_2lss_TT_MVA");
	v_variables.push_back("signal_3l_TTV_MVA");
	v_variables.push_back("signal_3l_TT_MVA");

	//NB : Make sure to execute the code with proper "ttV" or "ttbar" command-line arg ! Else wrong variables, crash
	// v_variables.push_back("BDT_ttV");
	// v_variables.push_back("BDT_ttbar");
    // v_variables.push_back("BDT_2Dlin");

	//--- All scale variations that we want to produce
	vector<TString> v_scaleVariations; vector<Int_t> v_colors;
	v_scaleVariations.push_back(""); v_colors.push_back(kBlack);

	//-- Scale variations
	v_scaleVariations.push_back("muR0p5"); v_colors.push_back(kOrange+1);
	v_scaleVariations.push_back("muF0p5"); v_colors.push_back(kMagenta);
	v_scaleVariations.push_back("muR0p5muF0p5"); v_colors.push_back(kOrange+9);
	v_scaleVariations.push_back("muR2"); v_colors.push_back(kSpring+3);
	v_scaleVariations.push_back("muF2"); v_colors.push_back(kGreen+4);
	v_scaleVariations.push_back("muR2muF2"); v_colors.push_back(kBlue);

	//-- PDF variations
	// v_scaleVariations.push_back("NNPDF31_central"); v_colors.push_back(kOrange);
	// v_scaleVariations.push_back("NNPDF31_down"); v_colors.push_back(kPink);
	// v_scaleVariations.push_back("NNPDF31_up"); v_colors.push_back(kRed);

	// v_scaleVariations.push_back("NNPDF30_central"); v_colors.push_back(kBlue);
	// v_scaleVariations.push_back("NNPDF30_down"); v_colors.push_back(kCyan);
	// v_scaleVariations.push_back("NNPDF30_up"); v_colors.push_back(kViolet);

	// v_scaleVariations.push_back("CT14_central"); v_colors.push_back(kBlue);
	// v_scaleVariations.push_back("CT14_down"); v_colors.push_back(kCyan);
	// v_scaleVariations.push_back("CT14_up"); v_colors.push_back(kViolet);


//--------------------------------------------
    //Can decide from arg to only produce plots (not reproduce all histos, if already there)
	if(plotOnly)
	{
		Plot_ScaleVariations_Histograms(v_variables, v_scaleVariations, v_colors, split_by_channel, normalize_all_histograms, use_TTbar_MC);
		return;
	}

//--------------------------------------------
	if(v_scaleVariations.size()<2 || !v_variables.size()) {cout<<"Empty variable/variation ?"<<endl; return;}

	cout<<endl<<BOLD(FYEL("##################################"))<<endl;
    cout<<FYEL("--- Producing Scale Variation Plots ---")<<endl;
	cout<<BOLD(FYEL("##################################"))<<endl<<endl;

	TString output_file_name = "outputs/ScaleVariations_";
	if(use_3l_events) output_file_name+= "_3l";
	else output_file_name+= "_2l";
	output_file_name+= "_" + region + filename_suffix + ".root";

	TFile* file_output = TFile::Open( output_file_name, "RECREATE" );

	int nchan = channel_list.size();
	if(!split_by_channel) {nchan = 1;}

	TH1::SetDefaultSumw2();

	TFile* file_input;
	TTree* tree(0);

	//-- replace DD fakes by MC fakes //should remove DD here or in loop
	if(use_TTbar_MC)
	{
		sample_list.push_back("TTbar_DiLep_PSweights");
		sample_list.push_back("TTbar_SemiLep_PSweights");
		sample_list.push_back("TTJets");
	}

	// cout<<"sample_list.size() "<<sample_list.size()<<endl;

	// --- Variable loop
	for(int ivar=0; ivar<v_variables.size(); ivar++)
	{
		if(v_variables[ivar].Contains("2l") && nLep_cat == "3l") {continue;}
		if(v_variables[ivar].Contains("3l") && nLep_cat == "2l") {continue;}

		cout<<endl<<endl<<"Variable "<<v_variables[ivar]<<endl;

		TString MVA_method_name1 = "", MVA_method_name2 = "";
		TString weightfile = "";
		TString template_name_MVA = "";
		if(v_variables[ivar].Contains("BDT")) //Book only 1 BDT
		{
			// --- Book the MVA methods (1 or 2, depending on template)
			TString dir = "weights/";
			if(use_2016_ntuples) {dir+= "2016/";}
			dir+= classifier_name;
			if(classifier_name == "DNN") {dir+= "/" + DNN_type;}

			if(use_rebinning)
			{
				if(analysis_type == "ttH") {dir+= "/ttH";}
				else if(analysis_type == "tHq") {dir+= "/tHq";}
				else {cout<<"Error ! No precomputed MVA for FCNC analysis ? Check here ! l."<<__LINE__<<endl; return;}
			}

			reader = new TMVA::Reader( "!Color:!Silent" );

			// Name & adress of local variables which carry the updated input values during the event loop
			// - the variable names MUST corresponds in name and type to those given in the weight file(s) used -- same order
			for(int i=0; i<var_list.size(); i++)
			{
				// cout<<"var_list[ivar] : "<<var_list[i]<<endl;
				reader->AddVariable(var_list[i].Data(), &var_list_floats[i]); //cout<<"Added variable "<<var_list[i]<<endl;
			}

			for(int i=0; i<v_cut_name.size(); i++)
			{
				// cout<<"v_cut_name[i] : "<<v_cut_name[i]<<endl;
				if(v_cut_IsUsedForBDT[i] && !v_cut_def[i].Contains("==")) {reader->AddVariable(v_cut_name[i].Data(), &v_cut_float[i]);}
			}

			if(v_variables[ivar].Contains("2Dlin") ) //Need to book 2 BDTs
			{
				template_name_MVA = "BDT_ttbar_" + nLep_cat + "_all";
				MVA_method_name1 = template_name_MVA + " method";
				weightfile = dir + "/" + template_name_MVA + "__" + signal_process + ".weights.xml";
				if(!Check_File_Existence(weightfile) ) {cout<<BOLD(FRED("Weight file "<<weightfile<<" not found ! Abort"))<<endl; return;}
				reader->BookMVA(MVA_method_name1, weightfile);

				template_name_MVA = "BDT_ttV_" + nLep_cat + "_all";
				MVA_method_name2 = template_name_MVA + " method";
				weightfile = dir + "/" + template_name_MVA + "__" + signal_process + ".weights.xml";
				if(!Check_File_Existence(weightfile) ) {cout<<BOLD(FRED("Weight file "<<weightfile<<" not found ! Abort"))<<endl; return;}
				reader->BookMVA(MVA_method_name2, weightfile);
			}
			else //ttV or ttbar or ...
			{
				template_name_MVA = v_variables[ivar] + "_" + nLep_cat + "_all";
				// for(int ichan=0; ichan<channel_list.size(); ichan++) //Book the method for each channel (separate BDTs)
				{
					MVA_method_name1 = template_name_MVA + " method";
					weightfile = dir + "/" + template_name_MVA;
					if(!train_with_trainingSel) {weightfile+= "_" + region;}
					weightfile+= "__" + signal_process + ".weights.xml";

					if(!Check_File_Existence(weightfile) ) {cout<<BOLD(FRED("Weight file "<<weightfile<<" not found ! Abort"))<<endl; return;}

					reader->BookMVA(MVA_method_name1, weightfile);
				}
			}
		}

		TH1F* hSumWeights = 0; //Read SumOfWeight histogram from ntuple, read nominal/scale variation sumOfWeights

//--------------------------------------------
// #       ####   ####  #####   ####
// #      #    # #    # #    # #
// #      #    # #    # #    #  ####
// #      #    # #    # #####       #
// #      #    # #    # #      #    #
// ######  ####   ####  #       ####
//--------------------------------------------

		for(int iscale=0; iscale<v_scaleVariations.size(); iscale++)
		{
			cout<<endl<<endl<<endl<<UNDL("== Variation : "<<v_scaleVariations[iscale]<<"")<<endl<<endl;

			//Loop on samples, syst., events ---> Fill histogram/channel --> Write()
			for(int isample=0; isample<sample_list.size(); isample++)
			{
				bool is_nominalScale_only = true;

				if(analysis_type == "FCNC" && sample_list[isample] != "TTJets") {continue;}

				// if(use_TTbar_MC && !sample_list[isample].Contains("TTbar")) {continue;} //ttbar only
				// else {is_nominalScale_only = false;}

				// if(debug && sample_list[isample] != "tHq" && sample_list[isample] != "tHW" && sample_list[isample] != "ttH" && sample_list[isample] != "ttZ" && sample_list[isample] != "ttW_PSweights" && sample_list[isample] != "WZ") {continue;} //Skip secondary samples -- make sure this condition is the same in the plotting func !

				//-- New ntuples, many are missing scale variations yet
				if(sample_list[isample] == "DATA" || sample_list[isample] == "QFlip" || sample_list[isample].Contains("GammaConv") || sample_list[isample].Contains("Fakes") ) {is_nominalScale_only = true;} //Don't consider these samples for scale variations

				if(sample_list[isample]=="tHq" || sample_list[isample]=="tHW" || sample_list[isample]=="ttH" || sample_list[isample]=="ttZ" || sample_list[isample]=="ttW_PSweights" || sample_list[isample]=="tZq" || sample_list[isample]=="WZ" || sample_list[isample]=="ZZ" )
				{
					is_nominalScale_only = false;
				}

				cout<<endl<<endl<<"SAMPLE "<<sample_list[isample]<<" / is_nominalScale_only "<<is_nominalScale_only<<endl;

				if(v_scaleVariations[iscale].Contains("NNPDF31") ) //Samples containing this PDF set
				{
					if(sample_list[isample]=="tHq" || sample_list[isample]=="tHW" || sample_list[isample]=="ttH" || sample_list[isample]=="ttZ" || sample_list[isample]=="ttW_PSweights" || sample_list[isample]=="tZq" || sample_list[isample]=="WZ" || sample_list[isample]=="ZZ" || sample_list[isample]=="WpWp" || sample_list[isample]=="ttWW" || sample_list[isample]=="ttWH" || sample_list[isample]== "tttW" || sample_list[isample]== "TTTT" || sample_list[isample]== "GGHZZ4L")
					{
						is_nominalScale_only = false;
					}
				}
				else if(v_scaleVariations[iscale].Contains("NNPDF30") ) //Samples containing this PDF set
				{
					if(sample_list[isample]=="tHq" || sample_list[isample]=="tHW" || sample_list[isample]=="ttH" || sample_list[isample]=="ttZ" || sample_list[isample]=="ttW_PSweights" || sample_list[isample]=="tZq" || sample_list[isample]=="WZ" || sample_list[isample]=="ZZ" || sample_list[isample]=="ZZZ" || sample_list[isample]=="WZZ" || sample_list[isample]=="WWZ" || sample_list[isample]=="WWW" || sample_list[isample]=="WpWp" || sample_list[isample]=="ttWW" || sample_list[isample]=="ttWH" || sample_list[isample]=="tttW" || sample_list[isample]=="TTTT" || sample_list[isample]=="GGHZZ4L")
					{
						is_nominalScale_only = false;
					}
				}
				else if(v_scaleVariations[iscale].Contains("CT14") ) //Samples containing this PDF set
				{
					if(sample_list[isample]=="tHW" || sample_list[isample]=="ttH" || sample_list[isample]=="ttZ" || sample_list[isample]=="ttW_PSweights" || sample_list[isample]=="WpWp" || sample_list[isample]=="ttWW" || sample_list[isample]=="ttWH" || sample_list[isample]=="tttW" || sample_list[isample]=="ZZ")
					{
						is_nominalScale_only = false;
					}
				}

	            //Protections
				if(sample_list[isample] == "DATA") {continue;} //Only care about MC?
				if(nLep_cat == "3l" && sample_list[isample] == "QFlip") {continue;}
				if(v_scaleVariations[iscale] != "" && (sample_list[isample] == "QFlip" || sample_list[isample] == "Fakes" || sample_list[isample] == "DATA")) {continue;}

	            int nbins; double xmin, xmax;
	            if( !Get_Variable_Range(v_variables[ivar], nbins, xmin, xmax) ) {cout<<FRED("Unknown variable name : "<<v_variables[ivar]<<"!")<<endl; return;}

				if(v_variables[ivar].Contains("2Dlin") )
				{
					if(use_rebinning)
					{
						if(analysis_type == "ttH") //ttH2017 binning
						{
							if(nLep_cat == "2l") {xmin = 0; xmax = 11; nbins = 11;} //11 bins
							else {xmin = 0; xmax = 6; nbins = 6;} //6 bins
						}
						else //tHq2016 binning, to be updated
						{
							xmin = 0; xmax = 10; nbins = 10;
						}
					}
					else {xmin = -2; xmax = 2; nbins = 10;} //If we simply transform the MVA as (BDT_tt + BDT_ttV), the output goes from -2 to +2
				}

	            vector<TH1F*> v_histo_channels(channel_list.size() );
	            if(split_by_channel) //Reserve memory for 1 TH1F* per channel
	            {
	                for(int ichan=0; ichan<channel_list.size(); ichan++)
	                {
	                    v_histo_channels[ichan] = new TH1F(v_variables[ivar]+"_"+channel_list[ichan], v_variables[ivar]+"_"+channel_list[ichan], nbins, xmin, xmax);
	                }
	            }
	            else //Summing all channels together
	            {
	                v_histo_channels.resize(1);
	                v_histo_channels[0] = new TH1F(v_variables[ivar]+"_all", v_variables[ivar]+"_all", nbins, xmin, xmax);
	            }

				if(analysis_type == "FCNC") {dir_ntuples = "./input_ntuples/FCNC/"+nLep_cat+"/";} //No ttbar stat in SR
				TString inputfile = dir_ntuples + sample_list[isample] + ".root";

				//If we are not using ntuple split based on category, Flip & Fake are included in DATA sample !
				if( (sample_list[isample] == "QFlip" || sample_list[isample].Contains("Fake") ) && dir_ntuples.Contains("allRegionsEvents") )
				{
					inputfile = dir_ntuples + "DATA.root";
				}

				cout<<"inputfile "<<inputfile<<endl;

				//Get TTree (read variable, weight -- nominal or variation)
				if(!Check_File_Existence(inputfile) ) {cout<<endl<<"File "<<inputfile<<FRED(" not found!")<<endl; continue;}
				file_input = TFile::Open(inputfile);
				tree = 0;
	            tree = (TTree*) file_input->Get(t_name);

				//Read hSumWeights histograms
				if(!is_nominalScale_only)
				{
					if(!file_input->GetListOfKeys()->Contains("hSumWeights") ) {cout<<FRED("Histogram hSumWeights not found in file "<<inputfile<<" !")<<endl; continue;}
					hSumWeights = (TH1F*) file_input->Get("hSumWeights");
				}

				for(int i=0; i<var_list.size(); i++)
				{
					tree->SetBranchAddress(var_list[i], &var_list_floats[i]);
				}

				for(int i=0; i<v_cut_name.size(); i++)
				{
					if(v_cut_name[i].Contains("is_") ) //Categories are encoded into Char_t, not float
					{
						tree->SetBranchAddress(v_cut_name[i], &v_cut_char[i]);
					}
					else //All others are floats
					{
						tree->SetBranchAddress(v_cut_name[i], &v_cut_float[i]);
					}
				}

				//--- Cut on relevant event categories
				Char_t is_goodCategory = 0; //Categ. of event
				tree->SetBranchAddress(Get_Category_Boolean_Name(nLep_cat, region, analysis_type, sample_list[isample], scheme), &is_goodCategory);

				//-- nominal
				Float_t mc_weight_originalValue = 1; //Nominal mc_weight
				Float_t sumWeights_nominal = 1; //Nominal sum of weights

				//-- scale var
				Float_t weight_variation = 1; //Scale variation weight
				Float_t sumWeights_variation = 1; //Sum of variation weights

				//PDF var
				vector<Float_t>* v_PDFset_reweights = new vector<Float_t>; //PDF replicas reweights
				vector<Float_t>* v_PDFset_sumWeights = new vector<Float_t>; //PDF replicas seums of weights

				if(!is_nominalScale_only)
				{
					tree->SetBranchAddress("mc_weight_originalValue", &mc_weight_originalValue);

					//NB : either read *all* sums of weights from hSumWeights histo, or from branch ; can not do both (problem for merged samples like ttZ)
					// sumWeights_nominal = hSumWeights->GetBinContent(2); //"mc_weight_originalValue"
					tree->SetBranchAddress("sumWeights_mc_weight_originalValue", &sumWeights_nominal); //Stored per event

					if(v_scaleVariations[iscale] == "") {sumWeights_variation = hSumWeights->GetBinContent(2);}
					else if(v_scaleVariations[iscale].Contains("muR") || v_scaleVariations[iscale].Contains("muF"))
					{
						tree->SetBranchAddress("weight_scale_"+v_scaleVariations[iscale], &weight_variation);
						tree->SetBranchAddress("sumWeights_scale_"+v_scaleVariations[iscale], &sumWeights_variation);
					}

	                else if(v_scaleVariations[iscale].Contains("NNPDF31") )
	                {
	                    tree->SetBranchAddress("v_PDF_weights_1", &v_PDFset_reweights);
	                    tree->SetBranchAddress("v_PDF_SumWeights_1", &v_PDFset_sumWeights);
	                }
					else if(v_scaleVariations[iscale].Contains("NNPDF30") )
	                {
	                    tree->SetBranchAddress("v_PDF_weights_2", &v_PDFset_reweights);
	                    tree->SetBranchAddress("v_PDF_SumWeights_2", &v_PDFset_sumWeights);
	                }
					else if(v_scaleVariations[iscale].Contains("CT14") )
	                {
	                    tree->SetBranchAddress("v_PDF_weights_3", &v_PDFset_reweights);
	                    tree->SetBranchAddress("v_PDF_SumWeights_3", &v_PDFset_sumWeights);
	                }

					// else {cout<<"Error ! Unknown variation "<<v_scaleVariations[iscale]<<" ! Abort !"<<endl; return;}
				}

				//Dedicated variables, easier to access in event loop
				// float theVar = -666;
				// tree->SetBranchAddress("catJets", &theVar);
				float i_channel = -1; tree->SetBranchAddress("channel", &i_channel);

				float weight; float weight_SF;
	            tree->SetBranchAddress("weight", &weight);

	            float var_value = 0;
				if(!v_variables[ivar].Contains("BDT")) {tree->SetBranchAddress(v_variables[ivar], &var_value);}

				if(sample_list[isample].Contains("Fakes") || sample_list[isample].Contains("TTbar") ) {tree->SetBranchAddress("weightfake", &weight_SF);} //Special SF for DD fakes events
				else if(sample_list[isample].Contains("Flip") ) {tree->SetBranchAddress("weightflip", &weight_SF);} //Special SF for DD QFlip events


// ###### #    # ###### #    # #####    #       ####   ####  #####
// #      #    # #      ##   #   #      #      #    # #    # #    #
// #####  #    # #####  # #  #   #      #      #    # #    # #    #
// #      #    # #      #  # #   #      #      #    # #    # #####
// #       #  #  #      #   ##   #      #      #    # #    # #
// ######   ##   ###### #    #   #      ######  ####   ####  #

				// int nentries = 100;
				int nentries = tree->GetEntries();

				cout<<endl<< "--- "<<sample_list[isample]<<" : Processing: " << nentries << " events" << std::endl;

				for(int ientry=0; ientry<nentries; ientry++)
				{
					if(ientry%20000==0) cout<<ientry<<" / "<<nentries<<endl;

					// cout<<"ientry "<<ientry<<endl;

					weight = 0;
					weight_SF = 0;
	                var_value = 0;
					is_goodCategory = 0;
					i_channel = -1;
					weight_variation = 1;
					mc_weight_originalValue = 1;
					v_PDFset_reweights->clear();
					v_PDFset_sumWeights->clear();

					tree->GetEntry(ientry);

					// if(isnan(weight) || isinf(weight))
					// {
					// 	cout<<FRED("* Found event with weight = "<<weight<<" ; remove it...")<<endl;
					// }

					if(sample_list[isample].Contains("Fakes") || sample_list[isample].Contains("TTbar") || sample_list[isample].Contains("Flip") ) {weight*= weight_SF;}

					//--- Cut on category value
					// if(!is_goodCategory) {continue;}

					if(!split_by_channel) {i_channel = 0;} //If only 1 summed channel, vector index is 0
					if(i_channel < 0 || i_channel >= channel_list.size() ) {continue;}


//---- APPLY CUTS HERE  ----
					bool pass_all_cuts = true;
					for(int icut=0; icut<v_cut_name.size(); icut++)
					{
						if(v_cut_def[icut] == "") {continue;}

						//Categories are encoded into Char_t. Convert them to float for code automation
						if(v_cut_name[icut].Contains("is_") ) {v_cut_float[icut] = (float) v_cut_char[icut];}

						// cout<<"Cut : name="<<v_cut_name[icut]<<" / def="<<v_cut_def[icut]<<" / value="<<v_cut_float[icut]<<" / pass ? "<<Is_Event_Passing_Cut(v_cut_def[icut], v_cut_float[icut])<<endl;

						if( !Is_Event_Passing_Cut(v_cut_def[icut], v_cut_float[icut]) ) {pass_all_cuts = false; break;}
					}
					if(!pass_all_cuts) {continue;}

					//APPLY THE SCALE VARIATION WEIGHT
					float weight_new = 1;

					if(v_scaleVariations[iscale] == "" || is_nominalScale_only) //Nominal, or samples without scale variations
					{
						weight_new = weight;
					}
					else if(v_scaleVariations[iscale].Contains("muR") || v_scaleVariations[iscale].Contains("muF"))
					{
						weight_new = weight *  (weight_variation / mc_weight_originalValue) * (sumWeights_nominal / sumWeights_variation); //previous
						// weight_new = weight *  (weight_variation / mc_weight_originalValue) * pow((sumWeights_nominal / sumWeights_variation), 2);
					}
					else //must first run Make_PDFvariations_Plot() to get indices of min/max variations
					{
						//Hardcoded index of min/max variations for each set/region
						int ipdf = -1;

						//NNPDF31
						if(v_scaleVariations[iscale] == "NNPDF31_up")
						{
							if(nLep_cat == "2l") {ipdf = 30;}
							else if(nLep_cat == "3l") {ipdf = 30;}
						}
						else if(v_scaleVariations[iscale] == "NNPDF31_down" )
						{
							if(nLep_cat == "2l") {ipdf = 25;}
							else if(nLep_cat == "3l") {ipdf = 39;}
						}

						//NNPDF30
						else if(v_scaleVariations[iscale] == "NNPDF30_up" )
						{
							if(nLep_cat == "2l") {ipdf = 47;}
							else if(nLep_cat == "3l") {ipdf = 47;}
						}
						else if(v_scaleVariations[iscale] == "NNPDF30_down" )
						{
							if(nLep_cat == "2l") {ipdf = 71;}
							else if(nLep_cat == "3l") {ipdf = 28;}
						}

						else if(v_scaleVariations[iscale] == "CT14_up" )
						{
							ipdf = 14;
						}
						else if(v_scaleVariations[iscale] == "CT14_down" )
						{
							ipdf = 38;
						}

						//Central
						else if(v_scaleVariations[iscale].Contains("central") )
						{
							if(v_PDFset_reweights->size() == 103) {ipdf = 1;}
						}

						// cout<<"ipdf = "<<ipdf<<endl;

						if(ipdf != -1 && v_PDFset_reweights->size() != 0) {weight_new = weight * (v_PDFset_reweights->at(ipdf-1) / mc_weight_originalValue) * (sumWeights_nominal / v_PDFset_sumWeights->at(ipdf-1));}
						else {weight_new = weight;} //In some samples, central PDF set member = nominal weight
					}

					//Debug printout
					if(iscale != 0 && sample_list[isample].Contains("tHq") )
					{
						cout<<endl<<"//--------------------------------------------"<<endl;
						cout<<"- Weight_MC nominal = "<<mc_weight_originalValue<<endl;
						cout<<"Sum weights nominal = "<<sumWeights_nominal<<endl;
						cout<<"- Weight_MC variation = "<<weight_variation<<endl;
						cout<<"Sum weights variation = "<<sumWeights_variation<<endl;
						cout<<"  Weight : "<<weight<<" => "<<weight_new<<endl;
						cout<<"//--------------------------------------------"<<endl;
					}

	                if(v_variables[ivar].Contains("BDT"))
	                {
	    				if(classifier_name == "BDT" || DNN_type != "Keras") //TMVA application
	    				{
	    					var_value = reader->EvaluateMVA(MVA_method_name1);
	    					// cout<<"mva_value1 = "<<mva_value1<<endl;
	    				}

	    				if(v_variables[ivar].Contains("2Dlin") ) //Check values of both BDT, fill 1D BDT accordingly (special binning)
	    				{
	    					if(classifier_name == "DNN") {cout<<BOLD(FRED("DNN not supported yet for template '2Dlin' !"))<<endl; break;}

	    					double value2 = reader->EvaluateMVA(MVA_method_name2);

	    					Fill_TH1F_UnderOverflow( v_histo_channels[(int) i_channel], var_value+value2, weight_new);
	    					// Fill_BDT_specialBinning(v_histo_channels[(int) i_channel], mva_value1, mva_value2, weight, nLep_cat);
	    				}
	    				else //Can directly fill BDT template
	    				{
	    					Fill_TH1F_UnderOverflow( v_histo_channels[(int) i_channel], var_value, weight_new);
	    				}
	                }
	                else {Fill_TH1F_UnderOverflow( v_histo_channels[(int) i_channel], var_value, weight_new);}

					// cout<<"v_histo_channels[(int) i_channel]->GetEntries() "<<v_histo_channels[(int) i_channel]->GetEntries()<<endl;
				} //end entries loop

				//Re-scale to desired luminosity, unless it's data
				if(sample_list[isample] != "DATA")
				{
					for(int ichan=0; ichan<nchan; ichan++)
					{
						//Luminosity rescaling
						v_histo_channels[ichan]->Scale(luminosity_rescale);
					}
				}

				// --- Write histograms
				file_output->cd();

				TString samplename = sample_list[isample];
				if(sample_list[isample] == "DATA") {samplename = "data_obs";}
				for(int ichan=0; ichan<nchan; ichan++)
				{
					TString output_histo_name = v_variables[ivar] + "_" + nLep_cat + "_" + region;
					if(channel_list[ichan] != "" && split_by_channel) {output_histo_name+= "_"  + channel_list[ichan];}
					else{output_histo_name+= "_all";}
					if(v_scaleVariations[iscale] != "") {output_histo_name+= "_" + v_scaleVariations[iscale];}
					output_histo_name+= "__" + samplename;
					cout<<"Writing histo : "<<output_histo_name<<endl; //" ("<<v_histo_channels[ichan]->GetEntries()<<")"<<endl;

					v_histo_channels[ichan]->Write(output_histo_name);
				}

				for(int ichan=0; ichan<nchan; ichan++)
				{
					delete v_histo_channels[ichan]; v_histo_channels[ichan] = NULL;
				}

				cout<<"Done with "<<sample_list[isample]<<" sample"<<endl;

				tree->ResetBranchAddresses(); //Detach tree from local variables (safe)
				delete tree; tree = NULL;
				file_input->Close(); file_input = NULL;

				delete v_PDFset_reweights; delete v_PDFset_sumWeights;
			} //end sample loop
		} //end scale var loop
	} 	//end variable loop


 // #    # ###### #####   ####  ######
 // ##  ## #      #    # #    # #
 // # ## # #####  #    # #      #####
 // #    # #      #####  #  ### #
 // #    # #      #   #  #    # #
 // #    # ###### #    #  ####  ######

//--------------------------------------------
//Substract MCprompt contributions from data-driven samples

	//Add MC prompt samples so they could be substracted from the data-driven histograms
	//Will remove them from sammple list at end of function
	if(substract_MCPrompt_from_DD)
	{
		sample_list.push_back("Fakes_MC");
	}

	TH1F* h_merge = 0;
	if(substract_MCPrompt_from_DD)
	{
		for(int ivar=0; ivar<v_variables.size(); ivar++)
		{
			if(v_variables[ivar].Contains("2l") && nLep_cat == "3l") {continue;}
			if(v_variables[ivar].Contains("3l") && nLep_cat == "2l") {continue;}

			for(int ichan=0; ichan<nchan; ichan++)
			{
				TString output_histo_prefix = v_variables[ivar] + "_" + nLep_cat + "_" + region;
				if(channel_list[ichan] != "" && split_by_channel) {output_histo_prefix+= "_"  + channel_list[ichan];}
				else{output_histo_prefix+= "_all";}

				//Substract (h_DD - h_MCPrompt) for Fakes
				TString output_histo_name = output_histo_prefix + "__Fakes_MC";
				if(!file_output->GetListOfKeys()->Contains(output_histo_name) ) {cout<<FRED("Histogram "<<output_histo_name<<" not found in file "<<output_file_name<<" ! Continue !")<<endl; continue;}
				// cout<<"output_histo_name "<<output_histo_name<<endl;
				TH1F* h_prompt = (TH1F*) file_output->Get(output_histo_name);
				h_merge = (TH1F*) h_prompt->Clone();
				h_merge->Scale(-1); //substract this contribution
				file_output->Delete(output_histo_name+";1");
				output_histo_name = output_histo_prefix + "__Fakes";
				if(file_output->GetListOfKeys()->Contains(output_histo_name) )
				{
					TH1F* h_dd = (TH1F*) file_output->Get(output_histo_name);
					h_merge->Add(h_dd);
					h_merge->Write(output_histo_name, TObject::kOverwrite);
					delete h_dd; h_dd = NULL;
				}
				delete h_merge; h_merge = NULL;
			}
		}

        //Remove the MCPrompt samples from list, were used only for substraction from data-driven
		sample_list.pop_back();
	}

	cout<<endl<<FYEL("==> Created root file: ")<<file_output->GetName()<<endl;
	cout<<FYEL("containing the "<<classifier_name<<" templates as histograms for : all samples / all channels")<<endl<<endl;
	delete file_output; file_output = NULL;
	delete reader; reader = NULL;

	Plot_ScaleVariations_Histograms(v_variables, v_scaleVariations, v_colors, split_by_channel, normalize_all_histograms, use_TTbar_MC);

	//-- remove samples we added
	if(use_TTbar_MC)
	{
		sample_list.pop_back();
		sample_list.pop_back();
		sample_list.pop_back();
	}

	return;
}













//--------------------------------------------
 // #####  #       ####  #####
 // #    # #      #    #   #
 // #    # #      #    #   #
 // #####  #      #    #   #
 // #      #      #    #   #
 // #      ######  ####    #
//--------------------------------------------

/**
 * Corresponding plotting function
 */
void TopEFT_analysis::Plot_ScaleVariations_Histograms(vector<TString> v_variables, vector<TString> v_scaleVariations, vector<Int_t> v_colors, bool split_by_channel, bool normalize_all_histograms, bool use_TTbar_MC)
{
	cout<<endl<<BOLD(FYEL("##################################"))<<endl;
	cout<<FYEL("-- Producing Scale Variations plots --")<<endl;
	cout<<endl<<BOLD(FYEL("##################################"))<<endl;

	TString input_file_name = "outputs/ScaleVariations_";
	if(use_3l_events) input_file_name+= "_3l";
	else input_file_name+= "_2l";
	input_file_name+= "_" + region + filename_suffix + ".root";
	if(!Check_File_Existence(input_file_name) ) {cout<<FRED("File "<<input_file_name<<" not found ! Abort !")<<endl; return;}
	TFile* file_input = TFile::Open(input_file_name);

	mkdir("plots/scaleVariations", 0777);

	//Canvas definition
	Load_Canvas_Style();
	TH1::SetDefaultSumw2();

	int nchan = channel_list.size();
	if(!split_by_channel) {nchan = 1;}

	//Need to store 1 histo per scale variation (to superimpose all)
	vector<TH1F*> v_scaleVar_histos; v_scaleVar_histos.resize(v_scaleVariations.size());

	//Also store 1 histo for the subplot (variation divided by nominal)
	vector<TH1F*> v_scaleVar_histos_subplot; v_scaleVar_histos_subplot.resize(v_scaleVariations.size());

	// cout<<"v_scaleVar_histos.size() "<<v_scaleVar_histos.size()<<endl;

	//Store integral for each sample and each variation
	vector<vector<float>> v_v_integral(sample_list.size());
	for(int isample=0; isample<sample_list.size(); isample++)
	{
		for(int iscale=0; iscale<v_scaleVariations.size(); iscale++)
		{
			v_v_integral[isample].push_back(0); //Init to 0
		}
	}

	//---------------------------
	//ERROR VECTORS INITIALIZATION
	//---------------------------
	bool draw_errors = false;

	vector<double> v_eyl, v_eyh, v_exl, v_exh, v_x, v_y; //Contain the systematic errors (used to create the TGraphError)
	int nofbins=-1;

	// --- Variable loop
	for(int ivar=0; ivar<v_variables.size(); ivar++)
	{
		if(v_variables[ivar].Contains("2l") && nLep_cat == "3l") {continue;}
		if(v_variables[ivar].Contains("3l") && nLep_cat == "2l") {continue;}

		// cout<<"-- Var "<<v_variables[ivar]<<endl<<endl;

		for(int ichan=0; ichan<nchan; ichan++)
		{
			TCanvas* c1 = new TCanvas("c1","c1", 1000, 800);
			c1->SetTopMargin(0.1);
			// c1->SetBottomMargin(0.25);
			c1->SetBottomMargin(0.50);

			TLegend* qw = new TLegend(0.78,.73,1.,1.);
			qw->SetTextSize(0.03);

			for(int iscale=0; iscale<v_scaleVariations.size(); iscale++)
			{
				// cout<<endl<<endl<<"-- VARIATION "<<v_scaleVariations[iscale]<<endl<<endl;

				v_scaleVar_histos[iscale] = 0;

				for(int isample=0; isample<sample_list.size(); isample++)
				{
					// cout<<"* isample "<<isample<<" / SAMPLE "<<sample_list[isample]<<endl;

					// if(sample_list[isample] != "tHq" && sample_list[isample] != "tHW" && sample_list[isample] != "ttH" && sample_list[isample] != "ttZ" && sample_list[isample] != "ttW_PSweights" && sample_list[isample] != "WZ") {continue;} //Skip secondary samples -- make sure this condition is the same in the main func !

					if(use_TTbar_MC && sample_list[isample].Contains("Fakes")) {continue;}

					if(analysis_type == "FCNC" && sample_list[isample] != "TTJets") {continue;}

					bool is_nominalScale_only = false;
					if(sample_list[isample] == "DATA" || sample_list[isample] == "QFlip" || sample_list[isample].Contains("GammaConv") || sample_list[isample].Contains("Fakes") ) {is_nominalScale_only = true;} //Don't consider these samples for scale variations
					if(is_nominalScale_only) {continue;}

					//Protections
					if(sample_list[isample] == "DATA") {continue;} //Only care about MC?
					if(nLep_cat == "3l" && sample_list[isample] == "QFlip") {continue;}

					TString samplename_tmp = sample_list[isample];
					if(sample_list[isample] == "DATA") {samplename_tmp = "data_obs";}

					//If data/data-driven sample, must still add (nominal) contribution !
					TString scalevariation_tmp = v_scaleVariations[iscale];
					if(v_scaleVariations[iscale] != "" && (sample_list[isample] == "QFlip" || sample_list[isample] == "Fakes" || sample_list[isample] == "DATA"))
					{
						scalevariation_tmp = "";
					}

					TString input_histo_name = v_variables[ivar] + "_" + nLep_cat + "_" + region;
					if(channel_list[ichan] != "" && split_by_channel) {input_histo_name+= "_"  + channel_list[ichan];}
					else{input_histo_name+= "_all";}
					if(scalevariation_tmp != "") {input_histo_name+= "_" + scalevariation_tmp;}
					input_histo_name+= "__" + samplename_tmp;
					// cout<<"Opening histo : "<<input_histo_name<<endl;

					if(!file_input->GetListOfKeys()->Contains(input_histo_name) ) {cout<<FRED("Histogram "<<input_histo_name<<" not found in file "<<input_file_name<<" ! Continue !")<<endl; continue;}
					TH1F* h_tmp = (TH1F*) file_input->Get(input_histo_name);

					if(draw_errors)
					{
						//Initialize error vectors (once)
						if(nofbins == -1)
						{
							nofbins = h_tmp->GetNbinsX();
							for(int ibin=0; ibin<nofbins; ibin++)
							{
								v_eyl.push_back(0); v_eyh.push_back(0);
								v_exl.push_back(h_tmp->GetXaxis()->GetBinWidth(ibin+1) / 2); v_exh.push_back(h_tmp->GetXaxis()->GetBinWidth(ibin+1) / 2);
								v_x.push_back( (h_tmp->GetXaxis()->GetBinLowEdge(nofbins+1) - h_tmp->GetXaxis()->GetBinLowEdge(1) ) * ((ibin+1 - 0.5)/nofbins) + h_tmp->GetXaxis()->GetBinLowEdge(1));
								v_y.push_back(0);
							}
						}

						//Increment errors
						for(int ibin=0; ibin<nofbins; ibin++) //Start at bin 1
						{
							// NOTE : for postfit, the bin error accounts for all systematics !
							//If using Combine output file (from MLF), bin error contains total error. Else if using template file directly, just stat. error
							v_eyl[ibin]+= pow(h_tmp->GetBinError(ibin+1), 2);
							v_eyh[ibin]+= pow(h_tmp->GetBinError(ibin+1), 2);

							v_y[ibin]+= h_tmp->GetBinContent(ibin+1); //This vector is used to know where to draw the error zone on plot (= on top of stack)

							// if(ibin > 0) {continue;} //cout only first bin
							//cout<<"x = "<<v_x[ibin]<<endl;    cout<<", y = "<<v_y[ibin]<<endl;    cout<<", eyl = "<<v_eyl[ibin]<<endl;    cout<<", eyh = "<<v_eyh[ibin]<<endl; //cout<<", exl = "<<v_exl[ibin]<<endl;    cout<<", exh = "<<v_exh[ibin]<<endl;
						} //loop on bins
					} //error condition

					if(!v_scaleVar_histos[iscale])
					{
						v_scaleVar_histos[iscale] = (TH1F*) h_tmp->Clone();
					}
					else
					{
						v_scaleVar_histos[iscale]->Add(h_tmp);
					}

					// cout<<"h_tmp->Integral() = "<<h_tmp->Integral()<<endl;
					// cout<<"v_scaleVar_histos[iscale]->Integral() = "<<v_scaleVar_histos[iscale]->Integral()<<endl;

					//Increment integral (computed for each sample/scale) -- only for first var
					if(!ichan)
					{
						v_v_integral[isample][iscale]+= h_tmp->Integral();
					}

					delete h_tmp;
				} //sample loop

				//Printout bin per bin content, for cross check of plot
				// if(iscale == 0 || iscale == v_scaleVariations.size()-1)
				// {
				// 	cout<<endl<<endl;
				// 	for(int ibin=0; ibin<v_scaleVar_histos[iscale]->GetNbinsX(); ibin++)
				// 	{
				// 		cout<<"Bin "<<ibin<<" --> "<<v_scaleVar_histos[iscale]->GetBinContent(ibin+1)<<endl;
				// 	}
				// 	cout<<endl<<endl;
				// }

				//Cosmetics
				v_scaleVar_histos[iscale]->SetLineColor(v_colors[iscale]);

				//Largest variations (supposedly) are emphasized
				if(v_scaleVariations[iscale] == "muR0p5muF0p5" || v_scaleVariations[iscale] == "muR2muF2")
				{
					v_scaleVar_histos[iscale]->SetLineWidth(2);
					v_scaleVar_histos[iscale]->SetLineStyle(1);
				}
				else
				{
					v_scaleVar_histos[iscale]->SetLineWidth(2);
					v_scaleVar_histos[iscale]->SetLineStyle(2);
				}

				if(!iscale) //only needed for first histo
				{
					// v_scaleVar_histos[iscale]->GetXaxis()->SetTitle(v_variables[ivar]);
					v_scaleVar_histos[iscale]->GetYaxis()->SetTitle("Weighted events");
					if(normalize_all_histograms) {v_scaleVar_histos[iscale]->GetYaxis()->SetTitle("Normalized");}
					v_scaleVar_histos[iscale]->SetLineColor(v_colors[iscale]);
					v_scaleVar_histos[iscale]->SetLineStyle(1);
					v_scaleVar_histos[iscale]->SetLineWidth(3);

					v_scaleVar_histos[iscale]->GetXaxis()->SetLabelFont(42);
					v_scaleVar_histos[iscale]->GetYaxis()->SetLabelFont(42);
					v_scaleVar_histos[iscale]->GetYaxis()->SetTitleFont(42);
					v_scaleVar_histos[iscale]->GetYaxis()->SetTitleSize(0.06);
					v_scaleVar_histos[iscale]->GetXaxis()->SetTitleSize(0.06);
					v_scaleVar_histos[iscale]->GetYaxis()->SetTickLength(0.04);
					v_scaleVar_histos[iscale]->GetYaxis()->SetLabelSize(0.048);
					v_scaleVar_histos[iscale]->GetXaxis()->SetNdivisions(505);
					v_scaleVar_histos[iscale]->GetYaxis()->SetNdivisions(506);
					v_scaleVar_histos[iscale]->GetYaxis()->SetTitleOffset(1.2);
					v_scaleVar_histos[iscale]->GetXaxis()->SetLabelSize(0.0); //subplot axis instead
				}

				if(normalize_all_histograms) {v_scaleVar_histos[iscale]->Scale(1./v_scaleVar_histos[iscale]->Integral());}

				v_scaleVar_histos[iscale]->SetMaximum(v_scaleVar_histos[iscale]->GetMaximum()*1.3);
				v_scaleVar_histos[iscale]->SetMinimum(0.001);

				// v_scaleVar_histos[iscale]->Rebin(2);

				v_scaleVar_histos[iscale]->Draw("same hist");

				if(v_scaleVariations[iscale] == "") {qw->AddEntry(v_scaleVar_histos[iscale], "Nominal", "L");}
				else
				{
					// qw->AddEntry(v_scaleVar_histos[iscale], v_scaleVariations[iscale], "L");

					TString ts_leg = v_scaleVariations[iscale];
					if(v_scaleVariations[iscale] == "muR0p5") {ts_leg = "#mu_{R}=0.5";}
					else if(v_scaleVariations[iscale] == "muF0p5") {ts_leg = "#mu_{F}=0.5";}
					else if(v_scaleVariations[iscale] == "muF0p5") {ts_leg = "#mu_{F}=0.5";}
					else if(v_scaleVariations[iscale] == "muR0p5muF0p5") {ts_leg = "#mu_{R}=0.5, #mu_{F}=0.5";}
					else if(v_scaleVariations[iscale] == "muR2") {ts_leg = "#mu_{R}=2";}
					else if(v_scaleVariations[iscale] == "muF2") {ts_leg = "#mu_{F}=2";}
					else if(v_scaleVariations[iscale] == "muR2muF2") {ts_leg = "#mu_{R}=2, #mu_{F}=2";}
					else if(v_scaleVariations[iscale] == "NNPDF31_central") {ts_leg = "NNPDF31";}
					else if(v_scaleVariations[iscale] == "NNPDF31_up") {ts_leg = "Up";}
					else if(v_scaleVariations[iscale] == "NNPDF31_down") {ts_leg = "Down";}
					else if(v_scaleVariations[iscale] == "NNPDF30_central") {ts_leg = "NNPDF30";}
					else if(v_scaleVariations[iscale] == "NNPDF30_up") {ts_leg = "Up";}
					else if(v_scaleVariations[iscale] == "NNPDF30_down") {ts_leg = "Down";}
					else if(v_scaleVariations[iscale] == "CT14_central") {ts_leg = "CT14";}
					else if(v_scaleVariations[iscale] == "CT14_up") {ts_leg = "Up";}
					else if(v_scaleVariations[iscale] == "CT14_down") {ts_leg = "Down";}
					qw->AddEntry(v_scaleVar_histos[iscale], ts_leg, "L");
				}
			} //scale loop

			qw->Draw("same");

			//Subplot
			TPad *pad_ratio = new TPad("pad_ratio", "pad_ratio", 0.0, 0.0, 1.0, 1.0);
			// pad_ratio->SetTopMargin(0.75);
			pad_ratio->SetTopMargin(0.50);
			pad_ratio->SetFillColor(0);
			pad_ratio->SetFillStyle(0);
			pad_ratio->SetGridy(1);
			pad_ratio->Draw();
			pad_ratio->cd(0);

			for(int iscale=0; iscale<v_scaleVariations.size(); iscale++)
			{
				if(!iscale) //first, nominal subhisto only used to create the subcanvas
				{
					v_scaleVar_histos_subplot[0] = (TH1F*) v_scaleVar_histos[0]->Clone(); //Copy variation histo

					// cout<<"v_scaleVar_histos_subplot[0]->Integral() "<<v_scaleVar_histos_subplot[0]->Integral()<<endl;

					v_scaleVar_histos_subplot[0]->Add(v_scaleVar_histos[0], -1.); //Substract nominal
					v_scaleVar_histos_subplot[0]->Divide(v_scaleVar_histos[0]); //Divide by nominal

					//Cosmetics
					v_scaleVar_histos_subplot[0]->SetLineColor(v_colors[0]);
					v_scaleVar_histos_subplot[0]->SetLineWidth(3);

					v_scaleVar_histos_subplot[0]->GetXaxis()->SetTitle(v_variables[ivar]);
					if(v_variables[ivar] == "signal_3l_TT_MVA") {v_scaleVar_histos_subplot[0]->GetXaxis()->SetTitle("BDT 3l t#bar{t}");}
					if(v_variables[ivar] == "signal_2lss_TT_MVA") {v_scaleVar_histos_subplot[0]->GetXaxis()->SetTitle("BDT 2lSS t#bar{t}");}
					if(v_variables[ivar] == "signal_3l_TTV_MVA") {v_scaleVar_histos_subplot[0]->GetXaxis()->SetTitle("BDT 3l t#bar{t}V");}
					if(v_variables[ivar] == "signal_2lss_TTV_MVA") {v_scaleVar_histos_subplot[0]->GetXaxis()->SetTitle("BDT 2lSS t#bar{t}V");}
					v_scaleVar_histos_subplot[0]->GetYaxis()->CenterTitle();
					v_scaleVar_histos_subplot[0]->GetYaxis()->SetTitle("#frac{(X-#mu)}{#mu} [%]");
					v_scaleVar_histos_subplot[0]->GetYaxis()->SetTitleOffset(1.3);
					v_scaleVar_histos_subplot[0]->GetYaxis()->SetTickLength(0.);
					v_scaleVar_histos_subplot[0]->GetXaxis()->SetTitleOffset(0.5);
					v_scaleVar_histos_subplot[0]->GetYaxis()->SetLabelSize(0.048);
					v_scaleVar_histos_subplot[0]->GetXaxis()->SetLabelFont(42);
					v_scaleVar_histos_subplot[0]->GetYaxis()->SetLabelFont(42);
					v_scaleVar_histos_subplot[0]->GetXaxis()->SetTitleFont(42);
					v_scaleVar_histos_subplot[0]->GetYaxis()->SetTitleFont(42);
					v_scaleVar_histos_subplot[0]->GetYaxis()->SetNdivisions(507); //grid draw on primary tick marks only
					v_scaleVar_histos_subplot[0]->GetXaxis()->SetNdivisions(505);
					v_scaleVar_histos_subplot[0]->GetYaxis()->SetTitleSize(0.05);
					v_scaleVar_histos_subplot[0]->GetXaxis()->SetTickLength(0.04);

					v_scaleVar_histos_subplot[iscale]->Scale(100.); //express in %

					v_scaleVar_histos_subplot[0]->SetMinimum(-6.99); //%
					v_scaleVar_histos_subplot[0]->SetMaximum(+6.99); //%
					v_scaleVar_histos_subplot[0]->Draw("hist");
				}
				else //scale variations histos are plotted normally
				{
					v_scaleVar_histos_subplot[iscale] = (TH1F*) v_scaleVar_histos[iscale]->Clone(); //Copy variation histo

					// cout<<"v_scaleVar_histos_subplot[iscale]->Integral() "<<v_scaleVar_histos_subplot[iscale]->Integral()<<endl;

					// if(iscale==v_scaleVariations.size()-1)
					// {
					// 	cout<<endl<<"v_scaleVar_histos[0]->GetBinContent(2) = "<<v_scaleVar_histos[0]->GetBinContent(2)<<endl;
					// 	cout<<"v_scaleVar_histos[0]->GetBinError(2) = "<<v_scaleVar_histos[0]->GetBinError(2)<<endl;
					//
					// 	cout<<endl<<"v_scaleVar_histos_subplot[iscale]->GetBinContent(2) = "<<v_scaleVar_histos_subplot[iscale]->GetBinContent(2)<<endl;
					// 	cout<<"v_scaleVar_histos_subplot[iscale]->GetBinError(2) = "<<v_scaleVar_histos_subplot[iscale]->GetBinError(2)<<endl;
					// }

					v_scaleVar_histos_subplot[iscale]->Add(v_scaleVar_histos[0], -1.); //Substract nominal
					v_scaleVar_histos_subplot[iscale]->Divide(v_scaleVar_histos[0]); //Divide by nominal

					// if(iscale==v_scaleVariations.size()-1)
					// {
					// 	cout<<"v_scaleVar_histos_subplot[iscale]->GetBinContent(2) = "<<v_scaleVar_histos_subplot[iscale]->GetBinContent(2)<<endl;
					// 	cout<<"v_scaleVar_histos_subplot[iscale]->GetBinError(2) = "<<v_scaleVar_histos_subplot[iscale]->GetBinError(2)<<endl<<endl;
					// }

					//Cosmetics
					v_scaleVar_histos_subplot[iscale]->SetLineColor(v_colors[iscale]);

					//Largest variations (supposedly) are emphasized
					if(v_scaleVariations[iscale] == "muR0p5muF0p5" || v_scaleVariations[iscale] == "muR2muF2")
					{
						v_scaleVar_histos_subplot[iscale]->SetLineWidth(2);
						v_scaleVar_histos_subplot[iscale]->SetLineStyle(1);

						// v_scaleVar_histos_subplot[iscale]->Draw("hist E1 same");
					}
					else
					{
						v_scaleVar_histos_subplot[iscale]->SetLineWidth(2);
						v_scaleVar_histos_subplot[iscale]->SetLineStyle(2);

						// v_scaleVar_histos_subplot[iscale]->Draw("hist same");
					}

					v_scaleVar_histos_subplot[iscale]->Scale(100.); //express in %

					v_scaleVar_histos_subplot[iscale]->Draw("hist same");
				}
			} //scale loop

			//Printout bin per bin content, for cross check of plot
			// {
			// 	cout<<endl<<"Sub plot muR2muF2 (bin contents) :"<<endl;
			// 	for(int ibin=0; ibin<v_scaleVar_histos_subplot[v_scaleVariations.size()-1]->GetNbinsX(); ibin++)
			// 	{
			// 		cout<<"Bin "<<ibin<<" --> "<<v_scaleVar_histos_subplot[v_scaleVariations.size()-1]->GetBinContent(ibin+1)<<endl;
			// 	}
			// 	cout<<endl<<endl;
			// }

			TString outputname = "plots/scaleVariations/scaleVar_" + v_variables[ivar];
			if(normalize_all_histograms)
			{
				mkdir("plots/scaleVariations/norm", 0777);
				outputname = "plots/scaleVariations/norm/scaleVar_" + v_variables[ivar];
			}
			if(use_3l_events) outputname+= "_3l";
			else outputname+= "_2l";
			outputname+= "_" + region + filename_suffix;
			outputname+= ".png";
			c1->SaveAs(outputname);

			for(int iscale=0; iscale<v_scaleVariations.size(); iscale++)
			{
				delete v_scaleVar_histos[iscale];
				delete v_scaleVar_histos_subplot[iscale];
			}

			delete pad_ratio; pad_ratio = NULL;
			delete c1;
			delete qw;
		} //ichan loop
	} //var loop


//--------------------------------------------
	//Printout of integrals for each sample / scale
	cout<<endl<<endl<<"//--------------------------------------------"<<endl;
	cout<<"[Integral for each sample & scale variation]"<<endl<<endl;

	for(int isample=0; isample<sample_list.size(); isample++)
	{
		if(sample_list[isample] == "DATA" || sample_list[isample] == "QFlip" || sample_list[isample].Contains("GammaConv") || sample_list[isample].Contains("Fakes") ) {continue;} //Don't consider these samples for scale variations

		if(use_TTbar_MC && sample_list[isample].Contains("Fakes")) {continue;}

		if(analysis_type == "FCNC" && sample_list[isample] == "TTJets") {continue;}

		cout<<endl<<endl<<UNDL(FYEL("* SAMPLE : "<<sample_list[isample]<<""))<<endl<<endl;

		for(int iscale=0; iscale<v_scaleVariations.size(); iscale++)
		{
			cout<<"-- Scale : "<<v_scaleVariations[iscale];
			if(v_scaleVariations[iscale] == "") {cout<<"Nominal";}
			cout<<" --> "<<std::setprecision(4)<<v_v_integral[isample][iscale];
			if(!iscale) {cout<<endl;}
			else
			{
				double variation = (v_v_integral[isample][iscale] / v_v_integral[isample][0]-1) * 100.;
				cout<<" ("; if(variation>0) {cout<<"+";}
				cout<<std::setprecision(2)<<variation<<" %)"<<endl;
			}
		}
	}
	cout<<"//--------------------------------------------"<<endl<<endl<<endl;

	//Printout of integral for each scale, summed over all samples
	cout<<"//--------------------------------------------"<<endl;
	cout<<"[Total integrals for each scale variation]"<<endl<<endl;

	double sum_nominal = 0;
	for(int iscale=0; iscale<v_scaleVariations.size(); iscale++)
	{
		cout<<UNDL(FYEL("-- Scale : "<<v_scaleVariations[iscale]<<""));
		if(v_scaleVariations[iscale] == "") {cout<<UNDL(FYEL("Nominal"));}
		double sum = 0;
		for(int isample=0; isample<sample_list.size(); isample++)
		{
			if(sample_list[isample] == "DATA" || sample_list[isample] == "QFlip" || sample_list[isample].Contains("GammaConv") || sample_list[isample].Contains("Fakes") ) {continue;} //Don't consider these samples for scale variations
			sum+= v_v_integral[isample][iscale];
			if(!iscale) {sum_nominal+= v_v_integral[isample][iscale];}
		}

		cout<<" --> "<<std::setprecision(4)<<sum;
		if(!iscale) {cout<<endl;}
		else
		{
			double variation = ((sum / sum_nominal) - 1) * 100.;
			cout<<" ("; if(variation>0) {cout<<"+";}
			cout<<std::setprecision(2)<<variation<<" %)"<<endl;
		}
	}
	cout<<"//--------------------------------------------"<<endl<<endl<<endl;
//--------------------------------------------

	file_input->Close();

    return;
}
















//--------------------------------------------
// ########  ########  ########
// ##     ## ##     ## ##
// ##     ## ##     ## ##
// ########  ##     ## ######
// ##        ##     ## ##
// ##        ##     ## ##
// ##        ########  ##
//--------------------------------------------

/**
 * For each variation of a given PDF set (hardcode?) : compute e.g. (Yield_PDF_replica - Yield_nominal) and fill histo for all PDF set replicas
 * Should give plot centered around 0 ; can use RMS, extreme values, etc.
 * Ideally, compare distributions for 2 or 3 different PDF sets ; but only 1 or 2 are found in all samples ?
 * CAVEATS : not substrating Fake_MC // Only using PDF variations from some sample // ...
 * cf. PDF4LHC recommandations : should add all variations in quadrature to get uncertainty... ? //NB : different for MC/hessians sets!
 */
void TopEFT_analysis::Make_PDFvariations_Plot()
{
	bool do_plotOnly = false; //true <-> don't reproduce histo, just plot

	//See : https://arxiv.org/pdf/1706.00428.pdf, sec. 6.2
	//Member 0 of PDF set is central value (as = 0.118)
	//Members 1 to 100 correspond to 100 MC replicas / Hessian eigenvectors (as = 0.118)
	//Members 101 & 102 are central value shifted by d_as = +-0.002
	//Different handling of members 101 & 102 for Hessian & MC ?

    TString PDFset_name = "NNPDF31_nnlo_hessian_pdfas"; //HESSIAN //Hessian conversion of NNPDF31_nnlo_as_0118_1000, mem=0 central value => Alphas(MZ)=0.118; mem=1-100 => PDF eig.; mem=101 => central value Alphas(MZ)=0.116; mem=102 => central value Alphas(MZ)=0.120
    // TString PDFset_name = "NNPDF30_nlo_nf_pdfas";  //MC //NNPDF3.0 NLO global fit; mem=0 to mem=100 with alphas(MZ)=0.118, mem=0 => average on replicas 1-100; mem=1-100 => PDF replicas with  alphas(MZ)=0.118; mem=101 => central value for alphas=0.117; mem=102 => central value for alphas=0.119; maximum number of active flavors NF=4
    // TString PDFset_name = "CT14nnlo"; //HESSIAN //CT14nnlo, 3-loop PDF evolution with alphas_s running solution. This set has 57 member PDFs. mem=0 --> central value; mem=1-56 --> eigenvector sets 90% //NB : can exist 2 separate aS variations, not saved
    // TString PDFset_name = "PDF4LHC15_nnlo_100_pdfas";  //HESSIAN //PDF4LHC15_nnlo_100_pdfas. mem=0 => alphas(MZ)=0.118 central value; mem=1-100 => PDF symmetric eigenvectors; mem=101 => alphas(MZ)=0.1165 central value; mem=102 => alphas(MZ)=0.1195

	cout<<endl<<BOLD(FYEL("##################################"))<<endl;
	cout<<FYEL("-- Producing PDF Variations plots --")<<endl;
	cout<<endl<<BOLD(FYEL("##################################"))<<endl;

	TString output_file_name = "outputs/PDFVariations_";
	if(use_3l_events) {output_file_name+= "_3l";}
	else {output_file_name+= "_2l";}
	output_file_name+= "_" + region + filename_suffix + ".root";

	gStyle->SetOptTitle(1);

	if(analysis_type == "FCNC") {sample_list.push_back("TTJets");}

	if(!do_plotOnly)
	{
		TFile* file_output = TFile::Open(output_file_name, "RECREATE");

		TH1F* h_PDF = 0; //Histo representing the distribution of the yields for all PDF replicas

		int nMembers = -1; //Expected nof members in PDF set

        if(PDFset_name == "NNPDF31_nnlo_hessian_pdfas")
		{
			h_PDF = new TH1F("", "", 20, -0.2, +0.2);
			nMembers = 103;
		}
		else if(PDFset_name == "CT14nnlo") //Missing first member of PDF set in some samples
		{
			h_PDF = new TH1F("", "", 10, -0.4, +0.4);
			nMembers = 57;
		}
		else if(PDFset_name == "NNPDF30_nlo_nf_pdfas") //Missing first member of PDF set in some samples
		{
			h_PDF = new TH1F("", "", 10, -2, +2);
			nMembers = 103;
		}
		else if(PDFset_name == "PDF4LHC15_nnlo_100_pdfas") //Missing first member of PDF set in some samples
		{
			h_PDF = new TH1F("", "", 10, -2, +2);
			nMembers = 103;
		}
        else {cout<<"Error : wrong PDF set name !"<<endl; return;}

		vector<double> v_yields_variations;

		for(int isample=0; isample<sample_list.size(); isample++)
		{
	        if(sample_list[isample] == "DATA") {continue;}

			if(analysis_type == "FCNC" && sample_list[isample] != "TTJets") {continue;}

			cout<<"Sample : "<<sample_list[isample]<<endl;

			if(analysis_type == "FCNC") {dir_ntuples = "./input_ntuples/FCNC/"+nLep_cat+"/";} //No ttbar stat in SR
	        TString inputfile_path = dir_ntuples + sample_list[isample] + ".root";
	        if(!Check_File_Existence(inputfile_path) ) {cout<<BOLD(FRED("Ntuple "<<inputfile_path<<" not found ! Skip"))<<endl; continue;}
	        TFile* f_input = TFile::Open(inputfile_path);

	        TTree* t = (TTree*) f_input->Get(t_name);

			/*
	        bool sample_hasPDFvariations = false;

			if(PDFset_name == "NNPDF31_nnlo_hessian_pdfas") //Samples containing this PDF set
			{
				if(sample_list[isample]=="tHq" || sample_list[isample]=="tHW" || sample_list[isample]=="ttH" || sample_list[isample]=="ttZ" || sample_list[isample]=="ttW_PSweights" || sample_list[isample]=="tZq" || sample_list[isample]=="WZ" || sample_list[isample]=="ZZ" || sample_list[isample]=="WpWp" || sample_list[isample]=="ttWW" || sample_list[isample]=="ttWH" || sample_list[isample]== "tttW" || sample_list[isample]== "TTTT" || sample_list[isample]== "GGHZZ4L")
				{
					sample_hasPDFvariations = true;
				}
			}
			else if(PDFset_name == "NNPDF30_nlo_nf_pdfas") //Samples containing this PDF set
			{
				if(sample_list[isample]=="tHq" || sample_list[isample]=="tHW" || sample_list[isample]=="ttH")
				// if(sample_list[isample]=="tHq" || sample_list[isample]=="tHW" || sample_list[isample]=="ttH" || sample_list[isample]=="ttZ" || sample_list[isample]=="ttW_PSweights" || sample_list[isample]=="tZq" || sample_list[isample]=="WZ" || sample_list[isample]=="ZZ" || sample_list[isample]=="ZZZ" || sample_list[isample]=="WZZ" || sample_list[isample]=="WWZ" || sample_list[isample]=="WWW" || sample_list[isample]=="WpWp" || sample_list[isample]=="ttWW" || sample_list[isample]=="ttWH" || sample_list[isample]=="tttW" || sample_list[isample]=="TTTT" || sample_list[isample]=="GGHZZ4L")
				{
					sample_hasPDFvariations = true;
				}
			}
			*/

	        //Basic event weights
	        float weight = 1; float weight_SF = 1;
	        t->SetBranchAddress("weight", &weight);
	        if(sample_list[isample].Contains("Fakes") )
	        {
	            t->SetBranchAddress("weightfake", &weight_SF); //Special SF for DD fakes events
	        }
	        else if(sample_list[isample].Contains("Flip") ) {t->SetBranchAddress("weightflip", &weight_SF);} //Special SF for DD QFlip events

	        //--- Cut on relevant event categories
	        Char_t is_goodCategory = 0; //Categ. of event
	        t->SetBranchAddress(Get_Category_Boolean_Name(nLep_cat, region, analysis_type, sample_list[isample], scheme), &is_goodCategory);

			Float_t mc_weight_originalValue = 1; //Nominal weight
			Float_t sumWeights_nominal = 0; //Sum of nominal weights
			vector<Float_t>* v_PDFset_reweights = new vector<Float_t>; //PDF replicas reweights
			vector<Float_t>* v_PDFset_sumWeights = new vector<Float_t>; //PDF replicas seums of weights
	        // if(sample_hasPDFvariations)
			{
				if(!t->GetListOfBranches()->FindObject("mc_weight_originalValue")) {continue;}
				if(!t->GetListOfBranches()->FindObject("sumWeights_mc_weight_originalValue")) {continue;}
				t->SetBranchAddress("mc_weight_originalValue", &mc_weight_originalValue);
				t->SetBranchAddress("sumWeights_mc_weight_originalValue", &sumWeights_nominal); //Stored per event

                if(PDFset_name == "NNPDF31_nnlo_hessian_pdfas")
                {
					if(!t->GetListOfBranches()->FindObject("v_PDF_weights_1")) {continue;}
					if(!t->GetListOfBranches()->FindObject("v_PDF_SumWeights_1")) {continue;}
                    t->SetBranchAddress("v_PDF_weights_1", &v_PDFset_reweights);
                    t->SetBranchAddress("v_PDF_SumWeights_1", &v_PDFset_sumWeights);
                }
				else if(PDFset_name == "NNPDF30_nlo_nf_pdfas")
                {
					if(!t->GetListOfBranches()->FindObject("v_PDF_weights_2")) {continue;}
					if(!t->GetListOfBranches()->FindObject("v_PDF_SumWeights_2")) {continue;}
                    t->SetBranchAddress("v_PDF_weights_2", &v_PDFset_reweights);
                    t->SetBranchAddress("v_PDF_SumWeights_2", &v_PDFset_sumWeights);
                }
				else if(PDFset_name == "CT14nnlo")
                {
					if(!t->GetListOfBranches()->FindObject("v_PDF_weights_3")) {continue;}
					if(!t->GetListOfBranches()->FindObject("v_PDF_SumWeights_3")) {continue;}
                    t->SetBranchAddress("v_PDF_weights_3", &v_PDFset_reweights);
                    t->SetBranchAddress("v_PDF_SumWeights_3", &v_PDFset_sumWeights);
                }
				else if(PDFset_name == "PDF4LHC15_nnlo_100_pdfas")
                {
					if(!t->GetListOfBranches()->FindObject("v_PDF_weights_4")) {continue;}
					if(!t->GetListOfBranches()->FindObject("v_PDF_SumWeights_4")) {continue;}
                    t->SetBranchAddress("v_PDF_weights_4", &v_PDFset_reweights);
                    t->SetBranchAddress("v_PDF_SumWeights_4", &v_PDFset_sumWeights);
                }
				else {cout<<FRED("Error : PDF set name not recognized !")<<endl; delete v_PDFset_reweights; delete v_PDFset_sumWeights; return;}
	        }
			// else {continue;}



// ###### #    # ###### #    # #####    #       ####   ####  #####
// #      #    # #      ##   #   #      #      #    # #    # #    #
// #####  #    # #####  # #  #   #      #      #    # #    # #    #
// #      #    # #      #  # #   #      #      #    # #    # #####
// #       #  #  #      #   ##   #      #      #    # #    # #
// ######   ##   ###### #    #   #      ######  ####   ####  #

			// int nentries = 1000;
			int nentries = t->GetEntries();

			cout<<endl<< "--- "<<sample_list[isample]<<" : Processing: " << nentries << " events" << std::endl;

			for(int ientry=0; ientry<nentries; ientry++)
			{
				if(ientry%20000==0) cout<<ientry<<" / "<<nentries<<endl;

				weight = 1;
				weight_SF = 1;
				is_goodCategory = 0;
				mc_weight_originalValue = 1;
				sumWeights_nominal = 0;
				v_PDFset_reweights->clear();
				v_PDFset_sumWeights->clear();

				t->GetEntry(ientry);

				if(v_PDFset_reweights->size() == 0) {break;}

				if(sample_list[isample].Contains("Fakes") || sample_list[isample].Contains("Flip") ) {weight*= weight_SF;}

				//--- Cut on category value
				if(!is_goodCategory) {continue;}

//---- APPLY CUTS HERE  ----
				bool pass_all_cuts = true;
				for(int icut=0; icut<v_cut_name.size(); icut++)
				{
					if(v_cut_def[icut] == "") {continue;}

					//Categories are encoded into Char_t. Convert them to float for code automation
					if(v_cut_name[icut].Contains("is_") ) {v_cut_float[icut] = (float) v_cut_char[icut];}

					// cout<<"Cut : name="<<v_cut_name[icut]<<" / def="<<v_cut_def[icut]<<" / value="<<v_cut_float[icut]<<" / pass ? "<<Is_Event_Passing_Cut(v_cut_def[icut], v_cut_float[icut])<<endl;

					if(!Is_Event_Passing_Cut(v_cut_def[icut], v_cut_float[icut]) ) {pass_all_cuts = false; break;}
				}
				if(!pass_all_cuts) {continue;}

				if(v_yields_variations.size() == 0) {v_yields_variations.resize(nMembers+1);} //nominal + all replicas (1 center value, 100/56 variations, possibly 2 aS variations)

				// cout<<"weight "<<weight<<endl;
				v_yields_variations[0]+= weight; //Nominal
				if(v_PDFset_reweights->size() == nMembers-1) {v_yields_variations[1]+= weight;} //If missing 1 member of the PDF set, it is the central value (<-> means that this PDF set was used as the central one for the sample) => fill first member with nominal weight

//--- APPLY THE SCALE VARIATION WEIGHT
				for(int ipdf=0; ipdf<v_PDFset_reweights->size(); ipdf++)
				{
					// cout<<"ipdf = "<<ipdf<<endl;

					float weight_new = 0;

					//-- how to apply PDF weights ?
					weight_new = weight * (v_PDFset_reweights->at(ipdf) / mc_weight_originalValue) * (sumWeights_nominal / v_PDFset_sumWeights->at(ipdf));

					if(v_PDFset_reweights->size() == nMembers-1) {v_yields_variations[ipdf+2]+= weight_new;} //if missing central value, first member of PDF set corresponds to first variation !
					else {v_yields_variations[ipdf+1]+= weight_new;} //first index is nominal

					//Debug printout
					// if(ientry == 0 && ipdf==0)
					// {
					// 	cout<<"//--------------------------------------------"<<endl;
					// 	cout<<"weight = "<<weight<<endl;
					// 	cout<<"weight_new = "<<weight_new<<endl;
					// 	cout<<"//--------------------------------------------"<<endl;
					// }
				} //pdf loop
	        } //nentries loop

			delete v_PDFset_reweights; v_PDFset_reweights = NULL;
			delete v_PDFset_sumWeights; v_PDFset_sumWeights = NULL;
	        f_input->Close();

			// cout<<"sample_hasPDFvariations = "<<sample_hasPDFvariations<<endl;
		}

		double uncert = 0;
		double uncert_plus = 0; //ct14
		double uncert_minus = 0; //ct14

		int ivar_max_up = -1, ivar_max_down = -1; //indices of max variations
		double tmp_up = 0, tmp_down = 0;

		//First element in vector is "nominal", second is "nominal value of considered PDF set"
		//Thus, should compare variations to Member 0 of PDF set, i.e. index 1 in my vector
		bool contains_aS = true; //in some PDF sets, last 2 members are aS variations
		if(nMembers == 57) {contains_aS = false;} //Central PDF set member stored as element 0 of PDF set

		if(v_yields_variations.size() < 2) {cout<<"ERROR : v_yields_variations.size() = "<<v_yields_variations.size()<<endl; return;}

		cout<<"Central integral = "<<v_yields_variations[1]<<endl;

		// cout<<"v_yields_variations.size() = "<<v_yields_variations.size()<<endl;
		for(int ivar=2; ivar<v_yields_variations.size()-2*contains_aS; ivar++) //Last 2 members are aS variations
		{
			double diff = ((v_yields_variations[ivar] - v_yields_variations[1]) / v_yields_variations[1]) * 100.; //variation from nominal, in %

			if(PDFset_name == "NNPDF30_nlo_nf_pdfas") {uncert+= pow(v_yields_variations[ivar] - v_yields_variations[1], 2);} //MC replicas
			else if(PDFset_name == "NNPDF31_nnlo_hessian_pdfas" || PDFset_name == "PDF4LHC15_nnlo_100_pdfas") {uncert+= pow(v_yields_variations[ivar] - v_yields_variations[1], 2);} //Sym. hessian
			else if(PDFset_name == "CT14nnlo" && ivar%2==0)
			{
				double tmp = v_yields_variations[ivar] - v_yields_variations[ivar+1];
				// cout<<"tmp = "<<tmp<<endl;
				uncert+= pow(tmp, 2);

				tmp = std::max(v_yields_variations[ivar]-v_yields_variations[1], v_yields_variations[ivar+1]-v_yields_variations[1]);
				if(tmp<0) {tmp = 0;}
				// cout<<"tmp = "<<tmp<<endl;
				uncert_plus+= pow(tmp, 2);

				tmp = std::max(v_yields_variations[1]-v_yields_variations[ivar], v_yields_variations[1]-v_yields_variations[ivar+1]);
				if(tmp<0) {tmp = 0;}
				// cout<<"tmp = "<<tmp<<endl;
				uncert_minus+= pow(tmp, 2);
			} //Asym hessian
			// cout<<"uncert = "<<uncert<<endl;

			if(diff < 0 && fabs(diff) > fabs(tmp_down) ) {ivar_max_down = ivar; tmp_down = diff;}
			if(diff > 0 && fabs(diff) > fabs(tmp_up) ) {ivar_max_up = ivar; tmp_up = diff;}

			h_PDF->Fill(diff, 1);

			cout<<"Variation "<<ivar<<" = "<<v_yields_variations[ivar]<<" -> Variation = "<<diff<<" %"<<endl;
		}
		// cout<<"h_PDF->GetEntries() = "<<h_PDF->GetEntries()<<endl;

		if(PDFset_name == "NNPDF30_nlo_nf_pdfas") {uncert = sqrt(uncert/(100-1)) / v_yields_variations[1];} //MC replicas
		else if(PDFset_name == "NNPDF31_nnlo_hessian_pdfas" || PDFset_name == "PDF4LHC15_nnlo_100_pdfas") {uncert = sqrt(uncert) / v_yields_variations[1];} //Sym. hessian
		else if(PDFset_name == "CT14nnlo")  //Asym hessian
		{
			uncert = (1./2) * sqrt(uncert) / v_yields_variations[1];
			uncert_plus = sqrt(uncert_plus) / v_yields_variations[1];
			uncert_minus = sqrt(uncert_minus) / v_yields_variations[1];

			cout<<endl<<"* uncert_plus = "<<uncert_plus * 100<<" (%)"<<endl;
			cout<<endl<<"* uncert_minus = "<<uncert_minus * 100<<" (%)"<<endl;
		}

		cout<<endl<<"* Uncert = "<<uncert * 100<<" (%)"<<endl;

		cout<<"* ivar_max_up = "<<ivar_max_up<<endl;
		cout<<"* ivar_max_down = "<<ivar_max_down<<endl;

		file_output->cd();
		h_PDF->Write("h_PDF");

		delete h_PDF; h_PDF = NULL;
		file_output->Close();
	}

//--------------------------------------------
 // #####  #       ####  #####
 // #    # #      #    #   #
 // #    # #      #    #   #
 // #####  #      #    #   #
 // #      #      #    #   #
 // #      ######  ####    #

	TFile* f_input = TFile::Open(output_file_name);
	TH1F* h_PDF = (TH1F*) f_input->Get("h_PDF");

	TCanvas* c = new TCanvas("", "", 1000, 800);
	c->SetBottomMargin(0.15);

	h_PDF->GetXaxis()->SetTitle("Var. from nominal (%)");
	// h_PDF->GetXaxis()->SetTitle("#frac{(X-#mu)}{#mu} [%]");
	h_PDF->GetYaxis()->SetTitle("# of replicas");
	h_PDF->GetXaxis()->SetLabelFont(42);
	h_PDF->GetYaxis()->SetLabelFont(42);
	h_PDF->GetYaxis()->SetTitleFont(42);
	h_PDF->GetYaxis()->SetTitleSize(0.06);
	h_PDF->GetXaxis()->SetTitleSize(0.06);
	h_PDF->GetYaxis()->SetTickLength(0.04);
	h_PDF->GetYaxis()->SetLabelSize(0.048);
	h_PDF->GetXaxis()->SetNdivisions(505);
	h_PDF->GetYaxis()->SetNdivisions(506);
	h_PDF->GetYaxis()->SetTitleOffset(1.2); //down
	h_PDF->GetXaxis()->SetTitleOffset(1.2); //down
    h_PDF->SetTitle(PDFset_name);

	gStyle->SetOptStat("rm");//RMS & mean ; use uppercase to also display errors
	h_PDF->Draw("hist same");

	TString outputname = "plots/PDFVariations/";
	mkdir(outputname.Data(), 0777);
	outputname+= "PDFVar_";
	// if(normalize_all_histograms)
	// {
	// 	mkdir("plots/PDFVariations/norm", 0777);
	// 	outputname = "plots/PDFVariations/norm/PDFVar_";
	// }
	if(use_3l_events) outputname+= "_3l";
	else outputname+= "_2l";
	outputname+= "_" + region + filename_suffix;
	if(PDFset_name.Contains("NNPDF31")) {outputname+= "_NNPDF31";}
	else if(PDFset_name.Contains("NNPDF30")) {outputname+= "_NNPDF30";}
	else if(PDFset_name.Contains("CT14")) {outputname+= "_CT14";}
	else if(PDFset_name.Contains("PDF4LHC15")) {outputname+= "_PDF4LHC15";}
	outputname+= ".png";
	c->SaveAs(outputname);

	delete c; c = NULL;
	f_input->Close();

	if(analysis_type == "FCNC") {sample_list.pop_back();}

	// cout<<endl<<FYEL("==> Created root file: ")<<file_output->GetName()<<endl;

    return;
}








//--------------------------------------------
//  ######   #######  ##     ## ########     ###    ########  ########
// ##    ## ##     ## ###   ### ##     ##   ## ##   ##     ## ##
// ##       ##     ## #### #### ##     ##  ##   ##  ##     ## ##
// ##       ##     ## ## ### ## ########  ##     ## ########  ######
// ##       ##     ## ##     ## ##        ######### ##   ##   ##
// ##    ## ##     ## ##     ## ##        ##     ## ##    ##  ##
//  ######   #######  ##     ## ##        ##     ## ##     ## ########

//  ######  ##     ##    ###    ########  ########  ######
// ##    ## ##     ##   ## ##   ##     ## ##       ##    ##
// ##       ##     ##  ##   ##  ##     ## ##       ##
//  ######  ######### ##     ## ########  ######    ######
//       ## ##     ## ######### ##        ##             ##
// ##    ## ##     ## ##     ## ##        ##       ##    ##
//  ######  ##     ## ##     ## ##        ########  ######
//--------------------------------------------

// Pallabi : x_tHq_hww_CMS_scale_jDown
void TopEFT_analysis::Compare_TemplateShapes_Processes(TString template_name, TString channel)
{
	bool drawInputVars = false;

	bool normalize = true;

	if(drawInputVars)
	{
		// template_name = "dPhiHighestPtSSPair"; //Can hardcode here another variable
		template_name = "BDT2Dlin"; //Can hardcode here another variable
		classifier_name = ""; //For naming convention
	}

//--------------------------------------------
	//group = "tH" <-> tHq+tHW+ttH
	vector<TString> v_samples; vector<TString> v_groups; vector<int> v_colors;
	// v_samples.push_back("tHq"); v_groups.push_back("tHq"); v_colors.push_back(kBlack);
	v_samples.push_back("tH_TT_hut_FCNC"); v_groups.push_back("tH_TT_hut_FCNC"); v_colors.push_back(kBlack);
	// v_samples.push_back("tHW"); v_groups.push_back("tHW"); v_colors.push_back(kYellow);
	// v_samples.push_back("ttH"); v_groups.push_back("ttH"); v_colors.push_back(kOrange);
	v_samples.push_back("ttZ"); v_groups.push_back("ttZ"); v_colors.push_back(kGreen);
	v_samples.push_back("WZ"); v_groups.push_back("WZ"); v_colors.push_back(kGreen+2);
	// v_samples.push_back("ttW_PSweights"); v_groups.push_back("ttW_PSweights"); v_colors.push_back(kGreen+2);
	v_samples.push_back("Fakes"); v_groups.push_back("Fakes"); v_colors.push_back(kBlue);

    vector<TString> v_syst;
    v_syst.push_back("");
    // v_syst.push_back("JESUp");
    // v_syst.push_back("JESDown");
    // v_syst.push_back("FR_be_muUp");
    // v_syst.push_back("FR_be_muDown");
    // v_syst.push_back("FR_be_muUp");
    // v_syst.push_back("FR_be_muDown");
    // v_syst.push_back("FR_be_muUp");
    // v_syst.push_back("FR_be_muDown");


//--------------------------------------------
	if(analysis_type == "ttH" && (region == "CR_WZ" || region == "CR_ZZ" || nLep_cat == "4l") && channel != "") {cout<<"Error ! No subcategories in this region !"<<endl; return;} //no subcat for WZ/ZZ CRs, 4l SR

	cout<<endl<<BOLD(FYEL("##################################"))<<endl;
	if(drawInputVars) {cout<<FYEL("--- Producing Input Vars Plots / channel : "<<channel<<" ---")<<endl;}
	else if(template_name == "ttbar" || template_name == "ttV" || template_name == "2D" || template_name == "2Dlin" || template_name == "categ") {cout<<FYEL("--- Producing "<<template_name<<" Template Plots / channel : "<<channel<<" ---")<<endl;}
	else {cout<<FRED("--- ERROR : invalid template_name value !")<<endl;}
	cout<<BOLD(FYEL("##################################"))<<endl<<endl;


//  ####  ###### ##### #    # #####
// #      #        #   #    # #    #
//  ####  #####    #   #    # #    #
//      # #        #   #    # #####
// #    # #        #   #    # #
//  ####  ######   #    ####  #

	//Get input TFile
	TString input_name;
	if(drawInputVars)
	{
		input_name = "outputs/ControlHistograms_" + nLep_cat + "_" + region + filename_suffix + ".root";
	}
	else
	{
		input_name = "outputs/Templates_" + classifier_name + template_name + "_" + nLep_cat + "_" + region + filename_suffix;
		if(classifier_name == "DNN") {input_name+= "_" + DNN_type;}
		input_name+= ".root";
	}

	if(!Check_File_Existence(input_name))
	{
		cout<<FRED("File "<<input_name<<" (<-> file containing prefit templates) not found ! Did you specify the region/background ? Abort")<<endl;
		return;
	}
	else {cout<<FBLU("--> Using file ")<<input_name<<FBLU(" instead (NB : only stat. error will be included)")<<endl;}

	cout<<endl<<endl<<endl;

	//Input file containing histos
	TFile* file_input = 0;
	file_input = TFile::Open(input_name);

    //Pallabi's file
	TFile* f_pallabi = 0;

	//TH1F* to retrieve distributions
	TH1F* h_tmp = 0; //Tmp storing histo

	vector<vector<TH1F*>> v_histos(v_samples.size()); //store histos, for each sample/syst


// #       ####   ####  #####   ####
// #      #    # #    # #    # #
// #      #    # #    # #    #  ####
// #      #    # #    # #####       #
// #      #    # #    # #      #    #
// ######  ####   ####  #       ####

	//Combine output : all histos are given for subcategories --> Need to sum them all
	for(int ichan=0; ichan<channel_list.size(); ichan++)
	{
		if(channel_list[ichan] != channel) {continue;}

		for(int isample = 0; isample<v_samples.size(); isample++)
		{
			cout<<endl<<UNDL(FBLU("-- Sample : "<<v_samples[isample]<<" : "))<<endl;

			v_histos[isample].resize(v_syst.size());

            TFile* f;
            f = file_input;

			TString samplename = v_samples[isample];

            for(int isyst=0; isyst<v_syst.size(); isyst++)
            {
				if(v_samples[isample].Contains("Fake") && !v_syst[isyst].Contains("Clos") && !v_syst[isyst].Contains("FR") && v_syst[isyst] != "") {continue;}

				// cout<<"syst "<<v_syst[isyst]<<endl;

                h_tmp = 0;

    			TString histo_name = "";
    			histo_name = classifier_name + template_name + "_" + nLep_cat + "_" + region;
    			if(channel != "") {histo_name+= "_" + channel;}
    			histo_name+= + "__" + samplename;
                if(v_syst[isyst] != "") {histo_name+= "__" + v_syst[isyst];}

    			if(!f->GetListOfKeys()->Contains(histo_name) ) {cout<<ITAL("Histogram '"<<histo_name<<"' : not found ! Skip...")<<endl; continue;}

    			h_tmp = (TH1F*) f->Get(histo_name);
				h_tmp->SetDirectory(0); //Dis-associate from TFile
    			cout<<"histo_name "<<histo_name<<endl;
                cout<<"h_tmp->Integral() = "<<h_tmp->Integral()<<endl;

 //  ####   ####  #       ####  #####   ####
 // #    # #    # #      #    # #    # #
 // #      #    # #      #    # #    #  ####
 // #      #    # #      #    # #####       #
 // #    # #    # #      #    # #   #  #    #
 //  ####   ####  ######  ####  #    #  ####

    			//Use color vector filled in main()
    			// h_tmp->SetFillStyle(1001);
				h_tmp->SetFillColor(kWhite);

				h_tmp->SetLineColor(v_colors[isample]);

				//HARDCODED
				if(v_syst[isyst] == "JESUp") {h_tmp->SetLineColor(kRed);}
				else if(v_syst[isyst] == "JESDown") {h_tmp->SetLineColor(kBlue);}

				// h_tmp->SetLineColor(v_colors[isample]+isyst);
				// cout<<"v_colors[isample] "<<v_colors[isample]<<endl;

    			h_tmp->SetLineWidth(3);

				h_tmp->SetMaximum(h_tmp->GetMaximum()*1.5);
				if(normalize) {h_tmp->SetMaximum(0.5);}

                if(v_syst[isyst] != "") {h_tmp->SetLineStyle(2);}

    			if(normalize) {h_tmp->Scale(1./h_tmp->Integral() );}

                v_histos[isample][isyst] = (TH1F*) h_tmp->Clone();

				// cout<<"v_histos["<<isample<<"]["<<isyst<<"]->Integral() "<<v_histos[isample][isyst]->Integral()<<endl;

    			delete h_tmp; h_tmp = 0;
            } //end syst loop

			// f->Close();
		} //end sample loop
	} //subcat loop


// #####  #####    ##   #    #
// #    # #    #  #  #  #    #
// #    # #    # #    # #    #
// #    # #####  ###### # ## #
// #    # #   #  #    # ##  ##
// #####  #    # #    # #    #

	//Canvas definition
	Load_Canvas_Style();
	gStyle->SetOptTitle(1);
	TCanvas* c = new TCanvas("", "", 1000, 800);
	c->SetTopMargin(0.1);

	// c->SetLogy();

	TLegend* qw;
	qw = new TLegend(0.75,.70,1.,1.);

	c->cd();

	for(int isample=0; isample<v_histos.size(); isample++)
	{
		TString systlist = "";

		for(int isyst=0; isyst<v_syst.size(); isyst++)
		{
			if(v_samples[isample].Contains("Fake") && !v_syst[isyst].Contains("Clos") && !v_syst[isyst].Contains("FR") && v_syst[isyst] != "") {continue;}

			if(v_histos[isample][isyst] == 0) {cout<<"Null histo ! Skip"<<endl; continue;}

			if(isample == 0)
			{
				if(v_syst[isyst] != "")
				{
					systlist+= " / " + v_syst[isyst];
				}

				v_histos[isample][isyst]->SetTitle(systlist);
				// if(normalize) {v_histos[isample][isyst]->SetMaximum(0.45);}
				// else {v_histos[isample][isyst]->SetMaximum(6.);}
				// else {v_histos[isample][isyst]->SetMaximum(11.);}

				if(drawInputVars) {v_histos[isample][isyst]->GetXaxis()->SetTitle(template_name);}
				else {v_histos[isample][isyst]->GetXaxis()->SetTitle(classifier_name+" (vs "+template_name + ")");}
			}

			if(isample < v_histos.size() - 1)
			{
				if(v_groups[isample] == v_groups[isample+1])
				{
					v_histos[isample+1][isyst]->Add(v_histos[isample][isyst]); //Merge with next sample
					continue; //Will draw merged histo, not this single one
				}
			}

			if(normalize) {v_histos[isample][isyst]->SetMaximum(0.5);}
			v_histos[isample][isyst]->Draw("hist same");

			if(v_syst[isyst] == "")
			{
				if(v_groups[isample] == "tH") {qw->AddEntry(v_histos[isample][isyst], "tH+ttH", "L");}
				else if(v_groups[isample].Contains("ttH")) {qw->AddEntry(v_histos[isample][isyst], "ttH", "L");}
				else if(v_groups[isample].Contains("tHq") )
				{
					if(use_SM_coupling) {qw->AddEntry(v_histos[isample][isyst], "tHq", "L");}
					else {qw->AddEntry(v_histos[isample][isyst], "tHq(#kappa_{t}=-1)", "L");}
				}
				else if(v_groups[isample].Contains("tHW") )
				{
					if(use_SM_coupling) {qw->AddEntry(v_histos[isample][isyst], "tHW", "L");}
					else {qw->AddEntry(v_histos[isample][isyst], "tHW(#kappa_{t}=-1)", "L");}
				}
				else if(v_groups[isample] == "tZq" && analysis_type == "ttH") {qw->AddEntry(v_histos[isample][isyst], "Rare", "L");}
				else if(v_groups[isample] == "tZq" && analysis_type != "ttH") {qw->AddEntry(v_histos[isample][isyst], "tZq", "L");}
				else if(v_groups[isample] == "tWZ" && analysis_type != "ttH") {qw->AddEntry(v_histos[isample][isyst], "Rares", "L");}
				else if(v_groups[isample].Contains("ttW") ) {qw->AddEntry(v_histos[isample][isyst], "t#bar{t}W", "L");}
				else if(v_groups[isample] == "ttZ") {qw->AddEntry(v_histos[isample][isyst], "t#bar{t}Z", "L");}
				// else if(v_groups[isample] == "WZ") {qw->AddEntry(v_histos[isample][isyst], "EWK", "L");}
				else if(v_groups[isample] == "Fakes") {qw->AddEntry(v_histos[isample][isyst], "Non-prompt", "L");}
				else if(v_groups[isample] == "QFlip") {qw->AddEntry(v_histos[isample][isyst], "Flip", "L");}
				else if(v_groups[isample].Contains("GammaConv") ) {qw->AddEntry(v_histos[isample][isyst], "#gamma-conv.", "L");}
				else {qw->AddEntry(v_histos[isample][isyst], v_samples[isample], "L");}
			}

			//HARDCODED
			if(v_syst[isyst] == "JESUp") {qw->AddEntry(v_histos[isample][isyst], "JES Up", "L");}
			if(v_syst[isyst] == "JESDown") {qw->AddEntry(v_histos[isample][isyst], "JES Down", "L");}
		}
	} //sample loop

	qw->Draw("same");


//  ####   ####   ####  #    # ###### ##### #  ####   ####
// #    # #    # #      ##  ## #        #   # #    # #
// #      #    #  ####  # ## # #####    #   # #       ####
// #      #    #      # #    # #        #   # #           #
// #    # #    # #    # #    # #        #   # #    # #    #
//  ####   ####   ####  #    # ######   #   #  ####   ####

//----------------
// CAPTIONS //
//----------------
// -- using https://twiki.cern.ch/twiki/pub/CMS/Internal/FigGuidelines

	float l = c->GetLeftMargin();
	float t = c->GetTopMargin();

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

	float lumi = ref_luminosity * luminosity_rescale;
	TString lumi_13TeV = Convert_Number_To_TString(lumi);
	lumi_13TeV += " fb^{-1} (13 TeV)";

	latex.SetTextFont(42);
	latex.SetTextAlign(31);
	latex.SetTextSize(0.04);

	// latex.DrawLatex(0.78, 0.92,lumi_13TeV);

	//------------------
	//-- Channel info
	TLatex text2 ;
	text2.SetNDC();
	text2.SetTextAlign(13);
	text2.SetTextSize(0.045);
	text2.SetTextFont(42);

	TString info_data;
	if(nLep_cat == "2l")
	{
		info_data = "2lss l^{#pm}l^{#pm}";

		if(channel == "eu" || channel == "ue" || channel == "em" || channel == "me") {info_data = "2lss e^{#pm}#mu^{#pm}";}
		else if(channel == "uu" || channel == "mm") {info_data = "2lss #mu^{#pm}#mu^{#pm}";}
		if(channel == "ee") {info_data = "2lss e^{#pm}e^{#pm}";}
	}
	else if(nLep_cat == "3l")
	{
		// info_data = "3l";
		info_data = "l^{#pm}l^{#pm}l^{#pm}";
	}

	// if(h_sum_data->GetBinContent(h_sum_data->GetNbinsX() ) > h_sum_data->GetBinContent(1) ) {text2.DrawLatex(0.55,0.87,info_data);}
	// else {text2.DrawLatex(0.20,0.87,info_data);}
	text2.DrawLatex(0.23,0.86,info_data);



// #    # #####  # ##### ######     ####  #    # ##### #####  #    # #####
// #    # #    # #   #   #         #    # #    #   #   #    # #    #   #
// #    # #    # #   #   #####     #    # #    #   #   #    # #    #   #
// # ## # #####  #   #   #         #    # #    #   #   #####  #    #   #
// ##  ## #   #  #   #   #         #    # #    #   #   #      #    #   #
// #    # #    # #   #   ######     ####   ####    #   #       ####    #

	mkdir("plots", 0777);
	mkdir("plots/templates_shapes", 0777);

	//Output
	TString output_plot_name = "plots/templates_shapes/";
	output_plot_name+= classifier_name + template_name + "_" + nLep_cat + "_" + region +"_templatesShapes";
	if(channel != "") {output_plot_name+= "_" + channel;}
	output_plot_name+= this->filename_suffix + this->plot_extension;

	c->SaveAs(output_plot_name);

	for(int isample=0; isample<v_histos.size(); isample++)
	{
		for(int isyst=0; isyst<v_syst.size(); isyst++)
		{
			if(v_samples[isample].Contains("Fake") && !v_syst[isyst].Contains("Clos") && !v_syst[isyst].Contains("FR") && v_syst[isyst] != "") {continue;}

			delete v_histos[isample][isyst];
		}
	}

	delete c; c = NULL;
	delete qw; qw = NULL;

	return;
}



/**
 * Spot differnences in histos between 2 files
 */
void TopEFT_analysis::Compare_Histograms()
{
	TFile* f1 = TFile::Open("/home/ntonon/Bureau/these/tHq/code/Analysis/outputs/pallabi_train/SM/Templates_BDT2Dlin_3l_SR.root");

	TFile* f2 = TFile::Open("/home/ntonon/Bureau/these/tHq/code/Analysis/outputs/Templates_BDT2Dlin_3l_SR.root");

	for(int isample=0; isample<sample_list[isample]; isample++)
	{
		TString histoname = "BDT2Dlin_3l_SR__" + sample_list[isample];

		if(!f1->GetListOfKeys()->Contains(histoname) ) continue;

		TH1F* h1 = (TH1F*) f1->Get(histoname);
		TH1F* h2 = (TH1F*) f2->Get(histoname);

		for(int ibin=0; ibin<h1->GetNbinsX(); ibin++)
		{
			double diff = fabs((h1->GetBinContent(ibin+1) - h2->GetBinContent(ibin+1))) / h1->GetBinContent(ibin+1);

			if(diff > 1./100)
			{
				cout<<"WARNING : sample "<<sample_list[isample]<<" bin "<<ibin+1<<" different ! ("<<h1->GetBinContent(ibin+1)<<" / "<<h2->GetBinContent(ibin+1)<<")"<<endl;
			}
		}

		delete h1;
		delete h2;
	}

	f1->Close();
	f2->Close();

	return;
}


/**
 * MAKE TEMPLATES FOR DIFFERENT CHOICES OF BINNING
 */
void TopEFT_analysis::Produce_Templates_ManyBinnings(TString template_name)
{
	bool debug = true;

	if(debug == false) {cout<<FYEL("--- Producing "<<template_name<<" Templates for several different binning configs ---")<<endl;}

	for(int i=0; i<100; i++)
	{
		cout<<FBLU("== Binning config "<<i<<"")<<endl<<endl;

		if(debug) {Test_Binnings(i, 0, 0, debug);}
		else {Produce_Templates(template_name, false, i);}
	}

	return;
}
