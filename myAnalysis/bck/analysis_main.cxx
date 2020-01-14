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
    TString signal_process = "tHq"; //Choose signal for train, etc.. //"tHq"/"ttH"/"hut_FCNC"/"hct_FCNC"/"VLQ" . If multiple samples, string should be a label common to all the signal samples only (e.g. 'FCNC')

    //-- Analysis main settings
    double set_luminosity = 41.5; //2017 lumi
    bool split_analysis_by_channel = false;
    bool use_systematics = true;

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
	Apply_CommandArgs_Choices(argc, argv, template_name, use_3l_events, region_choice);

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
    thevarlist_ttV.push_back("xxx");


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
    set_v_add_var_names.push_back("nMediumBJets");


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
    bool create_inputVar_histograms = true; //Create histograms of input variables, for plotting

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
        if(classifier_name == "DNN" && DNN_type != "TMVA")  //Must call dedicated python script to perform training
        {
            TString python_call = "python Train_Neural_Network.py";
            // python_call+= " " + DNN_type;
            // python_call+= " " + Convert_Number_To_TString(use_ttH_ntuples); //OBSOLETE, replaced by 'analysis_type'
            python_call+= " " + Convert_Number_To_TString(use_3l_events);

            cout<<endl<<FYEL("=== Will call the python script 'Train_Neural_Network.py' to perform Neural Network training, with following options : ===")<<endl<<endl;
            // cout<<FYEL("* DNN_type : ")<<DNN_type<<endl;
            // cout<<FYEL("* use_ttH_ntuples : ")<<use_ttH_ntuples<<endl; //OBSOLETE, replaced by 'analysis_type'
            cout<<FYEL("* use_3l_events : ")<<use_3l_events<<endl<<endl<<endl;

            system(python_call.Data());
        }
        else {theAnalysis->Train_BDT("", template_name, true);}
    }

    //#############################################
    //  TEMPLATES CREATION
    //#############################################
    if(create_templates) {theAnalysis->Produce_Templates(template_name, false);}

    //#############################################
    //  CONTROL HISTOGRAMS
    //#############################################
    // if(create_inputVar_histograms) {theAnalysis->Produce_Control_Histograms(split_analysis_by_channel);}
    if(create_inputVar_histograms) {theAnalysis->Produce_Templates(template_name, true);}

    //#############################################
    //  DRAW PLOTS
    //#############################################

    //All channels
    if(draw_templates)
    {
        if(template_name == "2D") {theAnalysis->Draw_ColZ_All();}
        else
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
    }

    // if(draw_input_vars) {theAnalysis->Draw_Control_Plots("all");}
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

    if(compare_template_shapes) {theAnalysis->Compare_TemplateShapes_Processes(template_name, plotChannel);}

    if(prodTemplates_compareBinning) {theAnalysis->Produce_Templates_ManyBinnings(template_name);}

    //#############################################
    //  OTHER FUNCTIONS
    //#############################################

    if(produce_ROCHistos_DNN_Keras) {theAnalysis->Produce_KerasDNN_Histos_For_ROC(template_name, 100);}

    if(plot_npu) {Make_Pileup_Plots(thesamplelist, analysis_type, "PU");}
    if(plot_npv) {Make_Pileup_Plots(thesamplelist, analysis_type, "PV");}


    //#############################################
    //  OVERLAP FUNCTIONS
    //#############################################

    if(write_overlap) {theAnalysis->Make_Overlap_Plots();}

    if(represent_overlap_cases) {Represent_Overlapping_Categories();}

    if(compare_after_orthogonalization) {Compare_Before_After_Orthogonalization();}

    if(get_yield_per_Jet_categ) {Get_Yield_Per_Jet_Categ(analysis_type);}

    if(check_effect_yield_orthogonalization) {Check_ChangeYields_Orthogonalization_FwdJet();}

    if(make_1dplot_yields_overlap) {Make_1DPlot_Yield_EachCat_Overlap();}

    //#############################################
    //  SCALE/PDF VARIATIONS FUNCTIONS
    //#############################################

    if(make_scaleVariation) {theAnalysis->Make_ScaleVariations_Histograms(scale_plotOnly, false, normalize_all_histograms);}
    if(make_PDF_plot) {theAnalysis->Make_PDFvariations_Plot();}

//--------------------------------------------
    delete theAnalysis;
}
