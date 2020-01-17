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

#include <iostream>
#include <fstream>
#include <vector>
#include "TString.h"

using namespace std;



//--------------------------------------------
//  ######   ######  ########  #### ########  ########    ########  #######  ########
// ##    ## ##    ## ##     ##  ##  ##     ##    ##       ##       ##     ## ##     ##
// ##       ##       ##     ##  ##  ##     ##    ##       ##       ##     ## ##     ##
//  ######  ##       ########   ##  ########     ##       ######   ##     ## ########
//       ## ##       ##   ##    ##  ##           ##       ##       ##     ## ##   ##
// ##    ## ##    ## ##    ##   ##  ##           ##       ##       ##     ## ##    ##
//  ######   ######  ##     ## #### ##           ##       ##        #######  ##     ##

// ######## ######## ##     ## ########  ##          ###    ######## ########    ######## #### ########
//    ##    ##       ###   ### ##     ## ##         ## ##      ##    ##          ##        ##     ##
//    ##    ##       #### #### ##     ## ##        ##   ##     ##    ##          ##        ##     ##
//    ##    ######   ## ### ## ########  ##       ##     ##    ##    ######      ######    ##     ##
//    ##    ##       ##     ## ##        ##       #########    ##    ##          ##        ##     ##
//    ##    ##       ##     ## ##        ##       ##     ##    ##    ##          ##        ##     ##
//    ##    ######## ##     ## ##        ######## ##     ##    ##    ########    ##       ####    ##
//--------------------------------------------

/**
 * Produce script containing the commands to produce the datacards (single and combination) automatically
 */
void Script_Datacards_TemplateFit(char include_systematics, char include_statistical, TString template_name, TString region, vector<TString> v_templates, vector<TString> v_channel, vector<TString> v_regions)
{
    //Check if use shape syst or not
	TString systChoice;
	if(include_systematics == 'y') {systChoice = "withShape";}
	else if(include_systematics == 'n') {systChoice = "noShape";}
    else {cout<<"Wrong arguments ! Abort !"<<endl; return;}

    //Check if use stat. uncert. or not
	TString statChoice;
	if(include_statistical == 'y') {statChoice = "withStat";}
	else if(include_statistical == 'n') {statChoice = "noStat";}
    else {cout<<"Wrong arguments ! Abort !"<<endl; return;}

    //If specific arguments were chosen at command line, modify the vectors defined in the main
    // if(template_name != "0") {v_templates.resize(0); v_templates.push_back(template_name);}
	// if(region != "0") //else if region = 0, use vector as it is defined in main
    // {
    //     if(region == "1")
    //     {
    //         v_regions.resize(0);
    //         v_regions.push_back("xxx");
    //     }
    //     else {v_regions.resize(0); v_regions.push_back(region);}
    // }

	ofstream file_out("Script_Make_Datacards_TemplateFit.sh"); //output script

	TString dir = "./datacards_TemplateFit/";
	file_out <<"mkdir "<<dir<<endl<<endl;

//--------------------------------------------
//--- First loop over regions/templates
//===> Commands to produce single datacards
    for(int iregion=0; iregion<v_regions.size(); iregion++) //Loop over regions
    {
        // cout<<"v_regions[iregion] "<<v_regions[iregion]<<endl;

		//Need to add "CR" prefix for CRs
		TString region_tmp = v_regions[iregion];
		if(region_tmp == "ttW" || region_tmp == "ttZ" || region_tmp == "WZ" || region_tmp == "ZZ") {region_tmp = "CR_" + region_tmp;}

        for(int itemplate=0; itemplate<v_templates.size(); itemplate++) //Loop over templates
        {
            TString var = "BDT"+ v_templates[itemplate] + "_" + region_tmp;

			//Make subdir for single datacards
			file_out <<"mkdir "<<dir<<endl<<endl;
			file_out <<"mkdir "<<dir+"/"+region_tmp<<endl<<endl;

		    // TString file_histos = "../templates/Combine_Input.root";
			TString file_histos = "../../../../templates/Templates_BDT"+v_templates[itemplate]+"_"+region_tmp+".root";

			cout<<"---> Will use filepath : "<<file_histos<<endl<<endl;

			for(int ilepchan=0; ilepchan<v_channel.size(); ilepchan++)
			{
				file_out<<"python Parser_Datacard_Template.py "
				+ v_channel[ilepchan] + " "
				+ var + " "
				+ file_histos + " "
				+ systChoice + " "
				+ statChoice + " "
				+ dir + region_tmp + " "
                + v_regions[iregion] + " "
				<<endl;
			}

			file_out<<endl<<endl;
    	} //template loop
    } //region loop

	//Command to execute the script which combines the datacards
	file_out<<"python $CMSSW_BASE/src/HiggsAnalysis/CombinedLimit/scripts/combineCards.py ";

//--------------------------------------------
//--- Second loop over regions/templates
//===> Give all the single datacards as arguments to the script, for combination
    for(int iregion=0; iregion<v_regions.size(); iregion++) //Loop over regions
    {
		//Need to add "CR" prefix for CRs
		TString region_tmp = v_regions[iregion];
		if(region_tmp == "ttW" || region_tmp == "ttZ" || region_tmp == "WZ" || region_tmp == "ZZ") {region_tmp = "CR_" + region_tmp;}

	    for(int itemplate=0; itemplate<v_templates.size(); itemplate++) //Loop over templates
	    {
            TString var = "BDT"+ v_templates[itemplate] + "_" + region_tmp;

			for(int ilepchan=0; ilepchan<v_channel.size(); ilepchan++) //Loop over channels
			{
				file_out<<var;
				if(v_channel[ilepchan] != "all") {file_out<<"_" + v_channel[ilepchan];}
				file_out<<"=" + dir + region_tmp + "/"
				+ "datacard_";
				if(v_channel[ilepchan] != "all") {file_out<<v_channel[ilepchan] + "_";}
				file_out<<var + ".txt ";
			}
		} //template loop
	} //region loop

	TString output_name = "COMBINED_Datacard_TemplateFit";
    if(systChoice == "noShape") output_name+= "_noShape";
    if(statChoice == "noStat") output_name+= "_noStat";
	output_name+= ".txt";

	file_out<<"> "<<output_name<<endl<<endl;

//datacard for single channels
/*
for(int ichan=0; ichan<v_channel.size(); ichan++)
{
	file_out<<"python $CMSSW_BASE/src/HiggsAnalysis/CombinedLimit/scripts/combineCards.py ";

	for(int ivar=0; ivar<var_list.size(); ivar++)
	{
		file_out<<var_list[ivar] + "_" + v_channel[ichan] + "=datacard_"+v_channel[ichan]+"_"+var_list[ivar]+".txt ";
	}

	output_name = "COMBINED_Datacard_TemplateFit_";
	if(systChoice == "noShape") output_name+= "noShape_";
	output_name+= v_channel[ichan]+".txt";

	file_out<<"> "<<output_name<<endl<<endl;

	file_out<<"mv "<<output_name<<" datacards_TemplateFit/"<<endl<<endl;
}
*/

	// file_out<<"mv datacard_*.txt datacards_TemplateFit/"<<endl;

	file_out.close();

	system("chmod 755 Script_Make_Datacards_TemplateFit.sh");

	cout<<FGRN("... Created script ./Script_Make_Datacards_TemplateFit.sh !")<<endl;

	cout<<endl<<endl<<FGRN("-- Executing script ./Script_Make_Datacards_TemplateFit.sh !")<<endl;

	system("./Script_Make_Datacards_TemplateFit.sh");

	return;
}







//--------------------------------------------
//  ######   #######  ##     ## ##     ##    ###    ##    ## ########
// ##    ## ##     ## ###   ### ###   ###   ## ##   ###   ## ##     ##
// ##       ##     ## #### #### #### ####  ##   ##  ####  ## ##     ##
// ##       ##     ## ## ### ## ## ### ## ##     ## ## ## ## ##     ##
// ##       ##     ## ##     ## ##     ## ######### ##  #### ##     ##
// ##    ## ##     ## ##     ## ##     ## ##     ## ##   ### ##     ##
//  ######   #######  ##     ## ##     ## ##     ## ##    ## ########

// ##       #### ##    ## ########       ###    ########   ######    ######
// ##        ##  ###   ## ##            ## ##   ##     ## ##    ##  ##    ##
// ##        ##  ####  ## ##           ##   ##  ##     ## ##        ##
// ##        ##  ## ## ## ######      ##     ## ########  ##   ####  ######
// ##        ##  ##  #### ##          ######### ##   ##   ##    ##        ##
// ##        ##  ##   ### ##          ##     ## ##    ##  ##    ##  ##    ##
// ######## #### ##    ## ########    ##     ## ##     ##  ######    ######
//--------------------------------------------

//Ask user to choose options at command line for script generation
void Choose_Arguments_From_CommandLine(char& include_systematics, char& include_statistical, TString& template_name, TString& region)
{
	//Choose whether to include shape syst or not
	cout<<endl<<FYEL("--- Do you want to include the *shape* systematics as nuisances in the datacards ? (y/n)")<<endl;
	cin>>include_systematics;
	while(include_systematics != 'y' && include_systematics != 'n')
	{
		cin.clear();
		cin.ignore(1000, '\n');

		cout<<" Wrong answer ! Need to type 'y' or 'n' ! Retry :"<<endl;
		cin>>include_systematics;
	}

    //Choose whether to include statistic uncert. or not (e.g. for pulls of NPs, need to remove them, else too long)
    cout<<endl<<FYEL("--- Do you want to include the *statistical* uncert. in the datacards ? (y/n)")<<endl;
    cin>>include_statistical;
    while(include_statistical != 'y' && include_statistical != 'n')
    {
        cin.clear();
        cin.ignore(1000, '\n');

        cout<<" Wrong answer ! Need to type 'y' or 'n' ! Retry :"<<endl;
        cin>>include_statistical;
    }

    // cout<<endl<<FBLU("Type the template name ('0' / 'ttV' / 'ttbar' / '2Dlin') ")<<endl;
    // cout<<"(NB : '0' <-> 2Dlin)"<<endl;
	// cin>>template_name;
	// while(template_name != "0" && template_name != "ttV" && template_name != "ttbar" && template_name != "2Dlin")
	// {
	// 	cin.clear();
	// 	cin.ignore(1000, '\n');
    //
	// 	cout<<" Wrong answer ! Need to type '0' or 'ttV' or 'ttbar' or '2Dlin' ! Retry :"<<endl;
	// 	cin>>template_name;
	// }

    //Choose region (SR / ...)
	// cout<<FBLU("Choose region ('0'/'1'/'2'/'3'/'SR'/'ttW'/'ttZ'/'WZ'/'ZZ') ")<<endl;
    // cout<<"(NB : '0' <-> SR_SThut + SR_TThut)"<<endl;
    // cout<<"(NB : '1' <-> SR_SThut + SR_TThut + CR_ttZ + CR_WZ)"<<endl;
    // cout<<"(NB : '2' <-> SRs + CR_ttW + CR_ttZ + CR_WZ [?overlap?])"<<endl;
    // cout<<"(NB : '3' <-> SRstHq + SRsttH + CRs)"<<endl;
	// cin>>region;
	// while(region != "0" && region != "1" && region != "2" && region != "3" && region != "SR" && region != "ttW" && region != "ttZ" && region != "WZ" && region != "ZZ")
	// {
	// 	cin.clear();
	// 	cin.ignore(1000, '\n');
    //
	// 	cout<<" Wrong answer ! Need to type '0' / '1' / '2' / '3' / 'SR' / 'ttW' / 'ttZ' / 'WZ' / 'ZZ' ! Retry :"<<endl;
	// 	cin>>region;
	// }

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

int main()
{
//Can set options here (can be modified at command line)
//--------------------------------------------
    vector<TString> v_templates;
    v_templates.push_back(""); //Default BDT

    vector<TString> v_channel;
    v_channel.push_back("uuu");
    v_channel.push_back("eeu");
    v_channel.push_back("uue");
    v_channel.push_back("eee");

    vector<TString> v_regions;
	v_regions.push_back("SR");
//--------------------------------------------


//Automated
//--------------------------------------------
    char include_systematics = 'n';
    char include_statistical = 'n';
    TString template_name = "0", region = "0";
    Choose_Arguments_From_CommandLine(include_systematics, include_statistical, template_name, region);

	Script_Datacards_TemplateFit(include_systematics, include_statistical, template_name, region, v_templates, v_channel, v_regions);

	return 0;
}
