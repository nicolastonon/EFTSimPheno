#include "TopEFT_analysis.h"

using namespace std;

int main(int argc, char **argv)
{
//---------------------------------------------------------------------------
//  #######  ########  ######## ####  #######  ##    ##  ######
// ##     ## ##     ##    ##     ##  ##     ## ###   ## ##    ##
// ##     ## ##     ##    ##     ##  ##     ## ####  ## ##
// ##     ## ########     ##     ##  ##     ## ## ## ##  ######
// ##     ## ##           ##     ##  ##     ## ##  ####       ##
// ##     ## ##           ##     ##  ##     ## ##   ### ##    ##
//  #######  ##           ##    ####  #######  ##    ##  ######
//---------------------------------------------------------------------------

    //-- Analysis type
    TString signal_process = "tZq";

    //-- Analysis main settings
    double set_luminosity = 41.5; //2017 lumi
    bool split_analysis_by_channel = false;
    bool use_systematics = false;

    //-- MVA
    TString classifier_name = "BDT";
    TString DNN_type = "";

    //-- Other options
    TString plot_extension = ".png"; //extension of plots

    //--- Templates options
    TString template_name = "";
    bool show_pulls_ratio = false;
    TString region_choice = "SR";

//------------------------------------
//Apply choices given via command line, if any
	Apply_CommandArgs_Choices(argc, argv, template_name, region_choice);

//-----------------------------------------------------------------------------------------
//   ######  ##     ## ########  ######
//  ##    ## ##     ##    ##    ##    ##
//  ##       ##     ##    ##    ##
//  ##       ##     ##    ##     ######
//  ##       ##     ##    ##          ##
//  ##    ## ##     ##    ##    ##    ##
//   ######   #######     ##     ######
//-----------------------------------------------------------------------------------------
//ex: set_v_cut_name.push_back("NBJets"); set_v_cut_def.push_back(">0 && <4"); set_v_cut_IsUsedForBDT.push_back(false);
//NB : if variable is to be used in BDT, can't ask it to take unique value (==) !

	vector<TString> set_v_cut_name;
	vector<TString> set_v_cut_def;
	vector<bool> set_v_cut_IsUsedForBDT;

    // set_v_cut_name.push_back("nJets");  set_v_cut_def.push_back("==3 || ==4"); set_v_cut_IsUsedForBDT.push_back(false);


//---------------------------------------------------------------------------
//  ######  ##     ##    ###    ##    ## ##    ## ######## ##        ######
// ##    ## ##     ##   ## ##   ###   ## ###   ## ##       ##       ##    ##
// ##       ##     ##  ##   ##  ####  ## ####  ## ##       ##       ##
// ##       ######### ##     ## ## ## ## ## ## ## ######   ##        ######
// ##       ##     ## ######### ##  #### ##  #### ##       ##             ##
// ##    ## ##     ## ##     ## ##   ### ##   ### ##       ##       ##    ##
//  ######  ##     ## ##     ## ##    ## ##    ## ######## ########  ######
//---------------------------------------------------------------------------
    std::vector<TString > thechannellist;

    thechannellist.push_back(""); //KEEP ! (<-> no subcategorization, used for plots, etc.)

    if(split_analysis_by_channel)
    {
        thechannellist.push_back("uuu");
        thechannellist.push_back("uue");
        thechannellist.push_back("eeu");
        thechannellist.push_back("eee");
    }


//---------------------------------------------------------------------------
//  ######     ###    ##     ## ########  ##       ########  ######
// ##    ##   ## ##   ###   ### ##     ## ##       ##       ##    ##
// ##        ##   ##  #### #### ##     ## ##       ##       ##
//  ######  ##     ## ## ### ## ########  ##       ######    ######
//       ## ######### ##     ## ##        ##       ##             ##
// ##    ## ##     ## ##     ## ##        ##       ##       ##    ##
//  ######  ##     ## ##     ## ##        ######## ########  ######
//---------------------------------------------------------------------------

//NB : if use_2016_ntuples==True, will automatically select the ntuples
    vector<TString> thesamplelist, thesamplegroups;
//-------------------
    //DATA --- Single sample, in first position
    // thesamplelist.push_back("DATA"); thesamplegroups.push_back("DATA");

    thesamplelist.push_back("tZq"); thesamplegroups.push_back("tZq");

    thesamplelist.push_back("ttZ"); thesamplegroups.push_back("ttZ");


//---------------------------------------------------------------------------
// ########  ########  ########       ##     ##    ###    ########   ######
// ##     ## ##     ##    ##          ##     ##   ## ##   ##     ## ##    ##
// ##     ## ##     ##    ##          ##     ##  ##   ##  ##     ## ##
// ########  ##     ##    ##          ##     ## ##     ## ########   ######
// ##     ## ##     ##    ##           ##   ##  ######### ##   ##         ##
// ##     ## ##     ##    ##            ## ##   ##     ## ##    ##  ##    ##
// ########  ########     ##             ###    ##     ## ##     ##  ######
//---------------------------------------------------------------------------
//Variables used in BDT //NB : always pass floats to the TMVA Reader !

    std::vector<TString > thevarlist;
    thevarlist.push_back("metEt");


//---------------------------------------------------------------------------
//  #######  ######## ##     ## ######## ########       ##     ##    ###    ########   ######
// ##     ##    ##    ##     ## ##       ##     ##      ##     ##   ## ##   ##     ## ##    ##
// ##     ##    ##    ##     ## ##       ##     ##      ##     ##  ##   ##  ##     ## ##
// ##     ##    ##    ######### ######   ########       ##     ## ##     ## ########   ######
// ##     ##    ##    ##     ## ##       ##   ##         ##   ##  ######### ##   ##         ##
// ##     ##    ##    ##     ## ##       ##    ##         ## ##   ##     ## ##    ##  ##    ##
//  #######     ##    ##     ## ######## ##     ##         ###    ##     ## ##     ##  ######
//---------------------------------------------------------------------------
//Can add additionnal vars which are NOT used in TMVA NOR for cuts, only for CR plots !
//NOTE : Branch can be linked to only *one* variable via SetBranchAddress ; if additional variable is already present in other variable vector, it is removed from this vector !

    vector<TString> set_v_add_var_names;
    // set_v_add_var_names.push_back("nMediumBJets");


//---------------------------------------------------------------------------
//  ######  ##    ##  ######  ######## ######## ##     ##    ###    ######## ####  ######   ######
// ##    ##  ##  ##  ##    ##    ##    ##       ###   ###   ## ##      ##     ##  ##    ## ##    ##
// ##         ####   ##          ##    ##       #### ####  ##   ##     ##     ##  ##       ##
//  ######     ##     ######     ##    ######   ## ### ## ##     ##    ##     ##  ##        ######
//       ##    ##          ##    ##    ##       ##     ## #########    ##     ##  ##             ##
// ##    ##    ##    ##    ##    ##    ##       ##     ## ##     ##    ##     ##  ##    ## ##    ##
//  ######     ##     ######     ##    ######## ##     ## ##     ##    ##    ####  ######   ######
//---------------------------------------------------------------------------

    vector<TString> theSystWeights; //List of systematics implemented as event weights
    theSystWeights.push_back(""); //KEEP ! (<-> nominal event weight)

    vector<TString> theSystTree; //List of systematics implemented as separate TTrees
    theSystTree.push_back(""); //KEEP ! (<-> nominal TTree)

    if(use_systematics)
    {
        //-- Implemented as separate TTrees
        // theSystTree.push_back("JESDown"); theSystTree.push_back("JESUp");

        //-- Implementend as event weights
        // theSystWeights.push_back("PUDown"); theSystWeights.push_back("PUUp");
    }


//---------------------------------------------------------------------------
// ######## ##     ## ##    ##  ######  ######## ####  #######  ##    ##        ######     ###    ##       ##        ######
// ##       ##     ## ###   ## ##    ##    ##     ##  ##     ## ###   ##       ##    ##   ## ##   ##       ##       ##    ##
// ##       ##     ## ####  ## ##          ##     ##  ##     ## ####  ##       ##        ##   ##  ##       ##       ##
// ######   ##     ## ## ## ## ##          ##     ##  ##     ## ## ## ##       ##       ##     ## ##       ##        ######
// ##       ##     ## ##  #### ##          ##     ##  ##     ## ##  ####       ##       ######### ##       ##             ##
// ##       ##     ## ##   ### ##    ##    ##     ##  ##     ## ##   ###       ##    ## ##     ## ##       ##       ##    ##
// ##        #######  ##    ##  ######     ##    ####  #######  ##    ##        ######  ##     ## ######## ########  ######
//---------------------------------------------------------------------------

//*** CHOOSE HERE FROM BOOLEANS WHAT YOU WANT TO DO !

//-----------------    TRAINING
    bool train_BDT = false; //Train selected BDT in selected region (with events in training category)

//-----------------    TEMPLATES CREATION
    bool create_templates = false; //Create templates of selected BDT, in selected region

//-----------------    CONTROL HISTOGRAMS
    bool create_inputVar_histograms = false; //Create histograms of input variables, for plotting

//-----------------    PLOTS
    TString plotChannel = ""; //Can choose to plot particular subchannel //uu, ue, ee, ...

    bool draw_templates = false; //Plot templates of selected BDT, in selected region
        bool prefit = false; //true <-> plot prefit templates ; else postfit (requires combine output file)
        bool use_combine_file = false; //true <-> use MLF output file from Combine (can get postfit plots, total error, etc.)

    bool draw_input_vars = false; //Plot input variables
        bool draw_input_allChannels = false; //true <-> also draw for eachs split channel

    bool compare_template_shapes = false;

//-----------------    OTHER

//-----------------










//--------------------------------------------
//--------------------------------------------
//--- Automated from here -- no need to modify
//--------------------------------------------
//--------------------------------------------

//--------------------------------------------
//    ###    ##     ## ########  #######  ##     ##    ###    ######## ####  ######
//   ## ##   ##     ##    ##    ##     ## ###   ###   ## ##      ##     ##  ##    ##
//  ##   ##  ##     ##    ##    ##     ## #### ####  ##   ##     ##     ##  ##
// ##     ## ##     ##    ##    ##     ## ## ### ## ##     ##    ##     ##  ##
// ######### ##     ##    ##    ##     ## ##     ## #########    ##     ##  ##
// ##     ## ##     ##    ##    ##     ## ##     ## ##     ##    ##     ##  ##    ##
// ##     ##  #######     ##     #######  ##     ## ##     ##    ##    ####  ######
//--------------------------------------------

//--------------------------------------------
//---> AUTOMATIZED FUNCTION CALLS FROM BOOLEANS

    //#############################################
    //  CREATE INSTANCE OF CLASS & INITIALIZE
    //#############################################

    TopEFT_analysis* theAnalysis = new TopEFT_analysis(thesamplelist, thesamplegroups, theSystWeights, theSystTree, thechannellist, thevarlist, set_v_cut_name, set_v_cut_def, set_v_cut_IsUsedForBDT, set_v_add_var_names, plot_extension, set_luminosity, show_pulls_ratio, region_choice, signal_process, classifier_name, DNN_type);
    if(theAnalysis->stop_program) {return 1;}

    //#############################################
    // TRAINING
    //#############################################

    if(train_BDT)
    {
        theAnalysis->Train_BDT("", true);
    }

    //#############################################
    //  TEMPLATES CREATION
    //#############################################

    if(create_templates) {theAnalysis->Produce_Templates(template_name, false);}

    //#############################################
    //  CONTROL HISTOGRAMS
    //#############################################

    if(create_inputVar_histograms) {theAnalysis->Produce_Templates(template_name, true);}

    //#############################################
    //  DRAW PLOTS
    //#############################################

    //All channels
    if(draw_templates)
    {
        theAnalysis->Draw_Templates(false, plotChannel, template_name, prefit, use_combine_file); //chosen channel

        if(plotChannel == "") //By default, also want to plot templates in subchannels
        {
            for(int ichan=1; ichan<thechannellist.size(); ichan++)
            {
                theAnalysis->Draw_Templates(false, thechannellist[ichan], template_name, prefit, use_combine_file);
            }
        }
    }

    if(draw_input_vars)
    {
        theAnalysis->Draw_Templates(true, plotChannel);
        if(draw_input_allChannels)
        {
            for(int ichan=1; ichan<thechannellist.size(); ichan++)
            {
                theAnalysis->Draw_Templates(true, thechannellist[ichan]);
            }
        }
    }

    //#############################################
    //  OTHER FUNCTIONS
    //#############################################

    if(compare_template_shapes) {theAnalysis->Compare_TemplateShapes_Processes(template_name, plotChannel);}

//--------------------------------------------
    delete theAnalysis;
}
