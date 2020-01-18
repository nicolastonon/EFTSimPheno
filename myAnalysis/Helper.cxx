#include "Helper.h"

using namespace std;

//--------------------------------------------
//  ######   ######## ##    ## ######## ########  ####  ######
// ##    ##  ##       ###   ## ##       ##     ##  ##  ##    ##
// ##        ##       ####  ## ##       ##     ##  ##  ##
// ##   #### ######   ## ## ## ######   ########   ##  ##
// ##    ##  ##       ##  #### ##       ##   ##    ##  ##
// ##    ##  ##       ##   ### ##       ##    ##   ##  ##    ##
//  ######   ######## ##    ## ######## ##     ## ####  ######

// ##     ## ######## ##       ########  ######## ########
// ##     ## ##       ##       ##     ## ##       ##     ##
// ##     ## ##       ##       ##     ## ##       ##     ##
// ######### ######   ##       ########  ######   ########
// ##     ## ##       ##       ##        ##       ##   ##
// ##     ## ##       ##       ##        ##       ##    ##
// ##     ## ######## ######## ##        ######## ##     ##

// ######## ##     ## ##    ##  ######   ######
// ##       ##     ## ###   ## ##    ## ##    ##
// ##       ##     ## ####  ## ##       ##
// ######   ##     ## ## ## ## ##        ######
// ##       ##     ## ##  #### ##             ##
// ##       ##     ## ##   ### ##    ## ##    ##
// ##        #######  ##    ##  ######   ######
//--------------------------------------------

//Use stat function (from library sys/stat) to check if a file exists
bool Check_File_Existence(const TString& name)
{
  struct stat buffer;
  return (stat (name.Data(), &buffer) == 0); //true if file exists
}

//Move file with bash command 'mv'
void MoveFile(TString origin_path, TString dest_path)
{
	TString command = "mv "+origin_path + " " + dest_path;
	system(command.Data() );

	return;
}

//Copy file with bash command 'cp'
void CopyFile(TString origin_path, TString dest_path)
{
	TString command = "cp "+origin_path + " " + dest_path;
	system(command.Data() );

	return;
}

//Convert a double into a TString
// precision --> can choose if TString how many digits the TString should display
TString Convert_Number_To_TString(double number, int precision/*=3*/)
{
	stringstream ss;
	ss << std::setprecision(precision) << number;
	TString ts = ss.str();
	return ts;
}

//Convert a TString into a float
float Convert_TString_To_Number(TString ts)
{
	float number = 0;
	string s = ts.Data();
	stringstream ss(s);

    //Bad characters may produce errors. Read 'out' these chars
    while(ss >> number || !ss.eof())
    {
        if(ss.fail()) //Found bad char. Remove it, read the remaining string
        {
            ss.clear();
            char dummy;
            ss >> dummy;
            continue;
        }
    }

	return number;
}

//Find a number into a TString, and returns it as a float
float Find_Number_In_TString(TString ts)
{
	TString tmp = ""; int number = 0;
	string s = ts.Data(); int ts_size = s.size(); //Only to get TString size

	for (int i=0; i < ts_size; i++)
	{
		if( isdigit(ts[i]) )
		{
			do
			{
				tmp += ts[i];
				i++;
			} while(isdigit(ts[i]) || ts[i] == '.'); //NB : Pay attention to quotes : "a" creates a 2-char array (letter+terminator) -> string. 'a' identifies a single character !

		  break;
		}
	}

	return Convert_TString_To_Number(tmp);
}

//Translates signs into words
TString Convert_Sign_To_Word(TString ts_in)
{
	TString sign = "";

	if(ts_in.Contains("<=")) {sign = "MaxEq";}
	else if(ts_in.Contains(">=")) {sign = "MinEq";}
	else if(ts_in.Contains(">")) {sign = "Min";}
	else if(ts_in.Contains("<")) {sign = "Max";}
	else if(ts_in.Contains("==")) {sign = "Eq";}
	else if(ts_in.Contains("!=")) {sign = "Diff";}

	return sign;
}

//Used when a "cut" is composed of 2 conditions -> breaks it into a pair of TStrings <cut1,cut2>
pair<TString,TString> Break_Cuts_In_Two(TString multiple_cut)
{
	TString cut1 = "", cut2 = "";

	string s = multiple_cut.Data(); int size = s.size(); //To get TString size

	for (int i=0; i < size; i++) //Extract condition 1
	{
		if(multiple_cut[i] == '&' || multiple_cut[i] == '|') {break;} //stop when we find '&&' or '||'
		if(multiple_cut[i] != ' ') {cut1 += multiple_cut[i];} //else, store characters in TString
	}

	TString tmp = "";
	for (int i=0; i < size; i++) //Extract condition 2
	{
		tmp+= multiple_cut[i];
		if( (!tmp.Contains("&&") && !tmp.Contains("||")) || multiple_cut[i] == '&' || multiple_cut[i] == '|') {continue;}
		if(multiple_cut[i] != ' ') {cut2 += multiple_cut[i];}
	}

	pair<TString,TString> the_cuts; the_cuts.first = cut1; the_cuts.second = cut2;

	return the_cuts;
}

//Check if a given value passes a given cut definition
bool Is_Event_Passing_Cut(TString cut_def, double value)
{
	double cut_tmp;

	if(!cut_def.Contains("&&") && !cut_def.Contains("||")) //If cut contains only 1 condition
	{
		cut_tmp = Find_Number_In_TString(cut_def);
		if(cut_def.Contains(">=") && value < cut_tmp)		 {return false;}
		else if(cut_def.Contains("<=") && value > cut_tmp)	 {return false;}
		else if(cut_def.Contains(">") && !cut_def.Contains(">=") && value <= cut_tmp)	 {return false;}
		else if(cut_def.Contains("<") && !cut_def.Contains("<=") && value >= cut_tmp) 	 {return false;}
		else if(cut_def.Contains("==") && value != cut_tmp)  {return false;}
		else if(cut_def.Contains("!=") && value == cut_tmp)  {return false;}
	}
	else if(cut_def.Contains("&&") && cut_def.Contains("||"))
	{
		cout<<BOLD(FRED("ERROR ! Cut definition too complex ; break it !"))<<endl;
	}
	else if(cut_def.Contains("&&") )//If '&&' in the cut def, break it in 2
	{
		TString cut1 = Break_Cuts_In_Two(cut_def).first;
		TString cut2 = Break_Cuts_In_Two(cut_def).second;
		//CUT 1
		cut_tmp = Find_Number_In_TString(cut1);
		if(cut1.Contains(">=") && value < cut_tmp)			 {return false;}
		else if(cut1.Contains("<=") && value > cut_tmp)		 {return false;}
		else if(cut1.Contains(">") && value <= cut_tmp) 	{return false;}
		else if(cut1.Contains("<") && value >= cut_tmp) 	{return false;}
		else if(cut1.Contains("==") && value != cut_tmp) 	 {return false;}
		else if(cut1.Contains("!=") && value == cut_tmp) 	 {return false;}
		//CUT 2
		cut_tmp = Find_Number_In_TString(cut2);
		if(cut2.Contains(">=") && value < cut_tmp)			 {return false;}
		else if(cut2.Contains("<=") && value > cut_tmp)		 {return false;}
		else if(cut2.Contains(">") && value <= cut_tmp)	{return false;}
		else if(cut2.Contains("<") && value >= cut_tmp)  {return false;}
		else if(cut2.Contains("==") && value != cut_tmp) 	 {return false;}
		else if(cut2.Contains("!=") && value == cut_tmp) 	 {return false;}
	}
	else if(cut_def.Contains("||") )//If '||' in the cut def, break it in 2
	{
		TString cut1 = Break_Cuts_In_Two(cut_def).first;
		TString cut2 = Break_Cuts_In_Two(cut_def).second;

		bool pass_cut1 = true; bool pass_cut2 = true; //Need to pass at least 1 cut

		//CUT 1
		cut_tmp = Find_Number_In_TString(cut1);
		if(cut1.Contains(">=") && value < cut_tmp)			 {pass_cut1 = false;}
		else if(cut1.Contains("<=") && value > cut_tmp)		 {pass_cut1 = false;}
		else if(cut1.Contains(">") && value <= cut_tmp)	{pass_cut1 = false;}
		else if(cut1.Contains("<") && value >= cut_tmp) 	{pass_cut1 = false;}
		else if(cut1.Contains("==") && value != cut_tmp) 	 {pass_cut1 = false;}
		else if(cut1.Contains("!=") && value == cut_tmp) 	 {pass_cut1 = false;}
		//CUT 2
		cut_tmp = Find_Number_In_TString(cut2);
		if(cut2.Contains(">=") && value < cut_tmp)			 {pass_cut2 = false;}
		else if(cut2.Contains("<=") && value > cut_tmp)		 {pass_cut2 = false;}
		else if(cut2.Contains(">") && value <= cut_tmp)		 	{pass_cut2 = false;}
		else if(cut2.Contains("<") && value >= cut_tmp) 	{pass_cut2 = false;}
		else if(cut2.Contains("==") && value != cut_tmp) 	 {pass_cut2 = false;}
		else if(cut2.Contains("!=") && value == cut_tmp) 	 {pass_cut2 = false;}

		if(!pass_cut1 && !pass_cut2) {return false;}
	}

	//If no condition was matched, it means the values passes the cut
	return true;
}

//Rescales the input value so that it retains its relative position in the new scale
double Compute_RangeScaled_Value(double value, double min_old, double max_old, double min_new, double max_new)
{
	return ((value - min_old) / (max_old - min_old)) * (max_new - min_new) + min_new;
}

//Returns the nof std deviations w.r.t. central value of the input value
double Compute_StdDevScaled_Value(double value, double mean, double dev)
{
	return (value - mean) / dev;
}

//Pass the names of the dir contents as references
bool Get_Dir_Content(string dir, vector<TString> &files)
{
    DIR *dp;
    struct dirent *dirp;
    if((dp  = opendir(dir.c_str())) == NULL)
    {
        cout << "Error(" << errno << ") opening " << dir << endl;
        return false;
    }

    while ((dirp = readdir(dp)) != NULL)
    {
        files.push_back(string(dirp->d_name));
    }

    closedir(dp);

    return true;
}









//--------------------------------------------
// ########   #######   #######  ########
// ##     ## ##     ## ##     ##    ##
// ##     ## ##     ## ##     ##    ##
// ########  ##     ## ##     ##    ##
// ##   ##   ##     ## ##     ##    ##
// ##    ##  ##     ## ##     ##    ##
// ##     ##  #######   #######     ##

//  ######  ########  ########  ######  #### ######## ####  ######
// ##    ## ##     ## ##       ##    ##  ##  ##        ##  ##    ##
// ##       ##     ## ##       ##        ##  ##        ##  ##
//  ######  ########  ######   ##        ##  ######    ##  ##
//       ## ##        ##       ##        ##  ##        ##  ##
// ##    ## ##        ##       ##    ##  ##  ##        ##  ##    ##
//  ######  ##        ########  ######  #### ##       ####  ######

// ######## ##     ## ##    ##  ######
// ##       ##     ## ###   ## ##    ##
// ##       ##     ## ####  ## ##
// ######   ##     ## ## ## ## ##
// ##       ##     ## ##  #### ##
// ##       ##     ## ##   ### ##    ##
// ##        #######  ##    ##  ######
//--------------------------------------------

//Increment weight of last bin by 'weight'
void Fill_Last_Bin_TH1F(TH1F* h, double weight)
{
	h->SetBinContent(h->GetNbinsX(), h->GetBinContent(h->GetNbinsX()) + weight);

	return;
}

//Increment weight of first bin by 'weight'
void Fill_First_Bin_TH1F(TH1F* h, double weight)
{
	h->SetBinContent(1, h->GetBinContent(1) + weight);

	return;
}

//Enforce desired canvas style
void Load_Canvas_Style()
{
	// For the canvas:
	gStyle->SetCanvasBorderMode(0);
	gStyle->SetCanvasColor(0); // must be kWhite but I dunno how to do that in PyROOT
	gStyle->SetCanvasDefH(600); //Height of canvas
	gStyle->SetCanvasDefW(600); //Width of canvas
	gStyle->SetCanvasDefX(0);   //POsition on screen
	gStyle->SetCanvasDefY(0);
	gStyle->SetPadBorderMode(0);
	gStyle->SetPadColor(0); // kWhite
	gStyle->SetPadGridX(0); //false
	gStyle->SetPadGridY(0); //false
	gStyle->SetGridColor(0);
	gStyle->SetGridStyle(3);
	gStyle->SetGridWidth(1);
	gStyle->SetFrameBorderMode(0);
	gStyle->SetFrameBorderSize(1);
	gStyle->SetFrameFillColor(0);
	gStyle->SetFrameFillStyle(0);
	gStyle->SetFrameLineColor(1);
	gStyle->SetFrameLineStyle(1);
	gStyle->SetFrameLineWidth(1);
	gStyle->SetHistLineColor(1);
	gStyle->SetHistLineStyle(0);
	gStyle->SetHistLineWidth(1);
	gStyle->SetEndErrorSize(2);
	gStyle->SetOptFit(1011);
	gStyle->SetFitFormat("5.4g");
	gStyle->SetFuncColor(2);
	gStyle->SetFuncStyle(1);
	gStyle->SetFuncWidth(1);
	gStyle->SetOptDate(0);
	gStyle->SetOptFile(0);
	gStyle->SetOptStat(0); // To display the mean and RMS:   SetOptStat("mr");
	gStyle->SetStatColor(0); // kWhite
	gStyle->SetStatFont(42);
	gStyle->SetStatFontSize(0.04);
	gStyle->SetStatTextColor(1);
	gStyle->SetStatFormat("6.4g");
	gStyle->SetStatBorderSize(1);
	gStyle->SetStatH(0.1);
	gStyle->SetStatW(0.15);
	gStyle->SetPadTopMargin(0.07);
	gStyle->SetPadBottomMargin(0.13);
	gStyle->SetPadLeftMargin(0.16);
	gStyle->SetPadRightMargin(0.03);
	gStyle->SetOptTitle(0);
	gStyle->SetTitleFont(42);
	gStyle->SetTitleColor(1);
	gStyle->SetTitleTextColor(1);
	gStyle->SetTitleFillColor(10);
	gStyle->SetTitleFontSize(0.05);
	gStyle->SetTitleColor(1, "XYZ");
	gStyle->SetTitleFont(42, "XYZ");
	gStyle->SetTitleSize(0.06, "XYZ");
	gStyle->SetTitleXOffset(0.9);
	gStyle->SetTitleYOffset(1.25);
	gStyle->SetLabelColor(1, "XYZ");
	gStyle->SetLabelFont(42, "XYZ");
	gStyle->SetLabelOffset(0.007, "XYZ");
	gStyle->SetLabelSize(0.05, "XYZ");
	gStyle->SetAxisColor(1, "XYZ");
	gStyle->SetStripDecimals(1); // kTRUE
	gStyle->SetTickLength(0.03, "XYZ");
	gStyle->SetNdivisions(510, "XYZ");
	gStyle->SetPadTickX(1);  // To get tick marks on the opposite side of the frame
	gStyle->SetPadTickY(1);
	gStyle->SetOptLogx(0);
	gStyle->SetOptLogy(0);
	gStyle->SetOptLogz(0);
	gStyle->SetPaperSize(20.,20.);
}

//Enforce the Combine name conventions
TString Combine_Naming_Convention(TString name)
{
	if(name.Contains("Up") || name.Contains("Down") ) {return name;} //If systematics is already following Combine convention

	else if(name.Contains("__plus"))
	{
		int i = name.Index("__plus"); //Find index of substring
		name.Remove(i); //Remove substring
		name+= "Up"; //Add Combine syst. suffix
	}

	else if(name.Contains("__minus"))
	{
		int i = name.Index("__minus"); //Find index of substring
		name.Remove(i); //Remove substring
		name+= "Down"; //Add Combine syst. suffix
	}

	return name;
}

//Modifies the TMVA standard output to extract only the useful ranking information (hard-coded way)
void Extract_Ranking_Info(TString TMVA_output_file, TString channel)
{
	ifstream file_in(TMVA_output_file.Data() );

	ofstream file_out("ranking_tmp.txt");

	file_out<<"--- "<<channel<<" : Variable Ranking ---"<<endl<<endl;

	string string_start = "Ranking input variables (method specific)...";
	string string_stop = "----------------------------------------------";

	string line;

	while(!file_in.eof( ) )
	{
		getline(file_in, line);

		if(line.find(string_start) != std::string::npos) {break;} //When 'string_start' is found, start copying lines into output file
	}

	int k=0;
	while(!file_in.eof( ) )
	{
		getline(file_in, line);

		if(k>3) //We know that the first 3 lines are useless
		{
			if(line.find(string_stop) != std::string::npos) {break;} //Don't stop until all variables are read

			TString string_to_write = line;
			int index = string_to_write.First(":"); //Find first occurence of ':' in the lines, and remove the text before it (useless)
		    string_to_write.Remove(0,index);

		    string s = string_to_write.Data(); int size = s.size(); //Compute size of the TString this way (ugly)
		    for(int i=0; i<size; i++)
		    {
		        if(isdigit(string_to_write[i]) ) //Only copy what is after the ranking index, remove the rest
		        {
		            string_to_write.Remove(0,i);
		            break;
		        }
		    }

			file_out<<string_to_write.Data()<<endl;
		}

		k++;
	}

	TString mv_command = "mv ranking_tmp.txt " + TMVA_output_file;
	system(mv_command.Data() ); //Replace TMVA output textfile by modified textfile !

	return;
}

//Read modified TMVA ranking file in order to fill 2 vectors (passed by reference) ordered by decreasing rank (var names & importances)
//NB : Hard-coded !
void Get_Ranking_Vectors(TString channel, vector<TString> &v_BDTvar_name, vector<double> &v_BDTvar_importance)
{
	TString file_rank_path = "./outputs/Rankings/RANKING_"+channel+".txt";
	if(!Check_File_Existence(file_rank_path.Data()) ) {std::cout<<file_rank_path<<" not found -- Abort"<<endl; return;}
	ifstream file_in(file_rank_path);

	string line;
	getline(file_in, line); getline(file_in, line);

	while(!file_in.eof())
	{
		getline(file_in, line);

		TString ts = line; //Convert to TString for easy modification

		int index = ts.First(":"); //Find first occurence of ':' in the lines, and remove the text before it (useless)
		ts.Remove(0,index+1); //Remove first useless characters

		TString var_name = "";
		string ts_tmp = ts.Data(); int size = ts_tmp.size();
		for(int ichar=0; ichar<size; ichar++)
		{
			if(var_name != "" && ts[ichar] == ' ') {break;} //end of var name
			else if(ts[ichar] == ' ') {continue;} //Begin at var name

			var_name+=ts[ichar];
		}

		v_BDTvar_name.push_back(var_name);


		index = ts.First(":"); //Find other occurence of ':' (placed before variable importance number)
		ts.Remove(0,index+1); //After that, only the importance number remains in the TString

 		TString var_importance_string = "";
 		ts_tmp = ts.Data(); size = ts_tmp.size();
		for(int ichar=0; ichar<size; ichar++)
		{
			if(ts[ichar] == ' ') {continue;}
			var_importance_string+=ts[ichar];
		}

		//Convert tstring to number (importance of the var)
		double var_importance = 0;
		if(var_importance_string.Contains("e-01") )
		{
			index = var_importance_string.Index('e');
			var_importance_string.Remove(index);
			var_importance = Convert_TString_To_Number(var_importance_string);
			var_importance*= 0.1;
		}
		else if(var_importance_string.Contains("e-02") )
		{
			index = var_importance_string.Index('e');
			var_importance_string.Remove(index);
			var_importance = Convert_TString_To_Number(var_importance_string);
			var_importance*= 0.01;
		}

		v_BDTvar_importance.push_back(var_importance);
	}


	//Because while loop on 'eof' --> one line too many at the end --> erase last entry
	v_BDTvar_name.erase(v_BDTvar_name.begin() + v_BDTvar_name.size()-1);
	v_BDTvar_importance.erase(v_BDTvar_importance.begin() + v_BDTvar_importance.size()-1);

	return;
}

//Loof for differences between 2 different histos stored in 2 different TFiles
void Compare_Histograms(TString filepath1, TString filepath2, TString histname1, TString histname2)
{
    TFile* f1 = TFile::Open(filepath1, "READ");
    TFile* f2 = TFile::Open(filepath2, "READ");

    if(!f1->GetListOfKeys()->Contains(histname1) ) {cout<<FRED("Error : "<<histname1<<" not found in file "<<filepath1<<"")<<endl; return;}
    if(!f2->GetListOfKeys()->Contains(histname2) ) {cout<<FRED("Error : "<<histname2<<" not found in file "<<filepath2<<"")<<endl; return;}

	TH1F* h1 = (TH1F*) f1->Get(histname1);
	TH1F* h2 = (TH1F*) f2->Get(histname2);

	for(int ibin=0; ibin<h1->GetNbinsX(); ibin++)
	{
		double diff = fabs((h1->GetBinContent(ibin+1) - h2->GetBinContent(ibin+1))) / h1->GetBinContent(ibin+1);

		if(diff > 1./100)
		{
			cout<<"WARNING : bin "<<ibin+1<<" different ! ("<<h1->GetBinContent(ibin+1)<<" / "<<h2->GetBinContent(ibin+1)<<")"<<endl;
		}
	}

	delete h1;
	delete h2;

	f1->Close();
	f2->Close();

	return;
}










//--------------------------------------------
//    ###    ##    ##    ###    ##       ##    ##  ######  ####  ######
//   ## ##   ###   ##   ## ##   ##        ##  ##  ##    ##  ##  ##    ##
//  ##   ##  ####  ##  ##   ##  ##         ####   ##        ##  ##
// ##     ## ## ## ## ##     ## ##          ##     ######   ##   ######
// ######### ##  #### ######### ##          ##          ##  ##        ##
// ##     ## ##   ### ##     ## ##          ##    ##    ##  ##  ##    ##
// ##     ## ##    ## ##     ## ########    ##     ######  ####  ######

//  ######  ########  ########  ######  #### ######## ####  ######
// ##    ## ##     ## ##       ##    ##  ##  ##        ##  ##    ##
// ##       ##     ## ##       ##        ##  ##        ##  ##
//  ######  ########  ######   ##        ##  ######    ##  ##
//       ## ##        ##       ##        ##  ##        ##  ##
// ##    ## ##        ##       ##    ##  ##  ##        ##  ##    ##
//  ######  ##        ########  ######  #### ##       ####  ######

// ######## ##     ## ##    ##  ######   ######
// ##       ##     ## ###   ## ##    ## ##    ##
// ##       ##     ## ####  ## ##       ##
// ######   ##     ## ## ## ## ##        ######
// ##       ##     ## ##  #### ##             ##
// ##       ##     ## ##   ### ##    ## ##    ##
// ##        #######  ##    ##  ######   ######
//--------------------------------------------

//Modifies arguments passed by reference according to command args
bool Apply_CommandArgs_Choices(int argc, char **argv, TString& template_name, TString& region_choice)
{
	if(argc >= 2)
	{
		if(argc > 2)
		{
			if(!strcmp(argv[2],"ttbar") || !strcmp(argv[2],"tt")) {template_name = "ttbar";}
			else if(!strcmp(argv[2],"ttv") || !strcmp(argv[2],"ttV") ) {template_name = "ttV";}
			else if(!strcmp(argv[2],"2Dlin")) {template_name = "2Dlin";}
			else if(!strcmp(argv[2],"2D")) {template_name = "2D";}
			else if(!strcmp(argv[2],"categ")) {template_name = "categ";}

			else if(!strcmp(argv[2],"SR")) {region_choice = "SR";}
			else if(!strcmp(argv[2],"SRtHq")) {region_choice = "SRtHq";}
			else if(!strcmp(argv[2],"SRttH")) {region_choice = "SRttH";}
            else if(!strcmp(argv[2],"FCNC")) {region_choice = "FCNC";}
            else if(!strcmp(argv[2],"Fake")) {region_choice = "CR_Fake";}
			else if(!strcmp(argv[2],"CR_ttW") || !strcmp(argv[2],"ttW")) {region_choice = "CR_ttW";}
			else if(!strcmp(argv[2],"CR_ttWttH") || !strcmp(argv[2],"ttWttH")) {region_choice = "CR_ttWttH";}
			else if(!strcmp(argv[2],"CR_ttZ") || !strcmp(argv[2],"ttZ")) {region_choice = "CR_ttZ";}
			else if(!strcmp(argv[2],"CR_WZ") || !strcmp(argv[2],"WZ")) {region_choice = "CR_WZ";}
			else if(!strcmp(argv[2],"CR_Z") || !strcmp(argv[2],"Z")) {region_choice = "CR_Z";}
			// else if(!strcmp(argv[2],"Training") || !strcmp(argv[2],"Train")) {region_choice = "Training";}

			else
			{
				cout<<BOLD(FRED("Error : wrong arguments at execution !"))<<endl;
				cout<<"argc = "<<argc<<endl;
				cout<<"argv[1] = '"<<argv[1]<<"'"<<endl;
				cout<<"argv[2] = '"<<argv[2]<<"'"<<endl;
				if(argc > 3) {cout<<"argv[3] = '"<<argv[3]<<"'"<<endl;}
				cout<<UNDL("--> Syntax : ./analysis_main.exe [3l or 2l] [Region or template name] [Region or template name]")<<endl;

				return 0;
			}
		}

		if(argc > 3)
		{
			if(!strcmp(argv[3],"ttbar") || !strcmp(argv[3],"tt")) {template_name = "ttbar";}
			else if(!strcmp(argv[3],"ttv") || !strcmp(argv[3],"ttV") ) {template_name = "ttV";}
			else if(!strcmp(argv[3],"2Dlin")) {template_name = "2Dlin";}
			else if(!strcmp(argv[3],"2D")) {template_name = "2D";}
			else if(!strcmp(argv[3],"categ")) {template_name = "categ";}

			else if(!strcmp(argv[3],"SR")) {region_choice = "SR";}
			else if(!strcmp(argv[3],"SRtHq")) {region_choice = "SRtHq";}
			else if(!strcmp(argv[3],"SRttH")) {region_choice = "SRttH";}
			else if(!strcmp(argv[3],"FCNC")) {region_choice = "FCNC";}
			else if(!strcmp(argv[3],"Fake")) {region_choice = "CR_Fake";}
			else if(!strcmp(argv[3],"CR_ttW") || !strcmp(argv[3],"ttW")) {region_choice = "CR_ttW";}
			else if(!strcmp(argv[3],"CR_ttWttH") || !strcmp(argv[3],"ttWttH")) {region_choice = "CR_ttWttH";}
			else if(!strcmp(argv[3],"CR_ttZ") || !strcmp(argv[3],"ttZ")) {region_choice = "CR_ttZ";}
			else if(!strcmp(argv[3],"CR_WZ") || !strcmp(argv[3],"WZ")) {region_choice = "CR_WZ";}
			else if(!strcmp(argv[3],"CR_Z") || !strcmp(argv[3],"Z")) {region_choice = "CR_Z";}
			// else if(!strcmp(argv[3],"Training") || !strcmp(argv[2],"Train")) {region_choice = "Training";}

			else
			{
				cout<<BOLD(FRED("Error : wrong arguments at execution !"))<<endl;
				cout<<"argc = "<<argc<<endl;
				cout<<"argv[1] = '"<<argv[1]<<"'"<<endl;
				cout<<"argv[2] = '"<<argv[2]<<"'"<<endl;
				cout<<"argv[3] = '"<<argv[3]<<"'"<<endl;
				cout<<UNDL("--> Syntax : ./analysis_main.exe [3l or 2l] [Region or template name] [Region or template name]")<<endl;

				return 0;
			}
		}
	}


	return 1;
}

//Select a color for each activated sample
void Get_Samples_Colors(vector<int>& v_colors, vector<TString> v_samples, int color_scheme)
{
	int i_skipData = 0; //index //include data in color vector for now (empty element)

//--------------------------------------------
	if(color_scheme == 0)
	{
		for(int isample=0; isample<v_samples.size(); isample++)
		{
            //Signals
            if(v_samples[isample] == "tZq") {v_colors[isample-i_skipData] = kOrange+10;}
            else if(v_samples[isample] == "ttZ") {v_colors[isample-i_skipData] = kViolet-6;}

            //ttX
            else if(v_samples[isample] == "ttH") {v_colors[isample-i_skipData] = kOrange+6;}
            else if(v_samples[isample] == "ttW") {v_colors[isample-i_skipData] = kOrange+6;}
            else if(v_samples[isample] == "tttt") {v_colors[isample-i_skipData] = kOrange+6;}
            else if(v_samples[isample] == "ttZZ") {v_colors[isample-i_skipData] = kOrange+6;}
            else if(v_samples[isample] == "ttWW") {v_colors[isample-i_skipData] = kOrange+6;}
            else if(v_samples[isample] == "ttWZ") {v_colors[isample-i_skipData] = kOrange+6;}
            else if(v_samples[isample] == "ttZH") {v_colors[isample-i_skipData] = kOrange+6;}
            else if(v_samples[isample] == "ttWH") {v_colors[isample-i_skipData] = kOrange+6;}

            //tX
            else if(v_samples[isample] == "tHq") {v_colors[isample-i_skipData] = kViolet;}
            else if(v_samples[isample] == "tHW") {v_colors[isample-i_skipData] = kViolet;}
            else if(v_samples[isample] == "tGJets") {v_colors[isample-i_skipData] = kViolet;}
            else if(v_samples[isample] == "ST") {v_colors[isample-i_skipData] = kViolet;}

            //VV(V)
            else if(v_samples[isample] == "WZ") {v_colors[isample-i_skipData] = kGreen-2;}
            else if(v_samples[isample] == "ZZ4l") {v_colors[isample-i_skipData] = kGreen-2;}
            else if(v_samples[isample] == "ZZZ") {v_colors[isample-i_skipData] = kGreen-2;}
            else if(v_samples[isample] == "WZZ") {v_colors[isample-i_skipData] = kGreen-2;}
            else if(v_samples[isample] == "WWW") {v_colors[isample-i_skipData] = kGreen-2;}
            else if(v_samples[isample] == "WWZ") {v_colors[isample-i_skipData] = kGreen-2;}
            else if(v_samples[isample] == "WZ2l2q") {v_colors[isample-i_skipData] = kGreen-2;}
            else if(v_samples[isample] == "ZZ2l2q") {v_colors[isample-i_skipData] = kGreen-2;}
            else if(v_samples[isample] == "ZG2l2g") {v_colors[isample-i_skipData] = kGreen-2;}

            //Fakes
            else if(v_samples[isample] == "DY") {v_colors[isample-i_skipData] = kCyan;}
            else if(v_samples[isample] == "TTbar_DiLep") {v_colors[isample-i_skipData] = kPink-4;}
            else if(v_samples[isample] == "TTbar_SemiLep") {v_colors[isample-i_skipData] = kPink-4;}
		}
	}

	return;
}

//Use custom color palette
//-- Idea : take good-looking/efficient color palettes from web and apply it manually
void Set_Custom_ColorPalette(vector<TColor*> &v_custom_colors, vector<int> &v)
{
    // TColor* col = new TColor(1700, 141./255., 211./255., 199./255.);
    // col.SetRGB(141./255., 211./255., 199./255.);
    // cout<<col->GetNumber()<<endl;

	v.resize(9);
	v_custom_colors.resize(9);

	//-- Colors definition : Number + RGB value, from 0 to 1
	v_custom_colors[0] = new TColor(9001, 166./255., 206./255., 227./255.);
    v_custom_colors[1] = new TColor(9002, 31./255., 120./255., 180./255.);
    v_custom_colors[2] = new TColor(9003, 178./255., 223./255., 138./255.);
    v_custom_colors[3] = new TColor(9004, 51./255., 160./255., 44./255.);
    v_custom_colors[4] = new TColor(9005, 251./255., 251./255., 153./255.);
	v_custom_colors[5] = new TColor(9006, 227./255., 26./255., 28./255.);
	v_custom_colors[6] = new TColor(9007, 1, 0, 0);
	v_custom_colors[7] = new TColor(9008, 0, 1, 0);
	v_custom_colors[8] = new TColor(9009, 0, 0, 1);

    v[0] = v_custom_colors[0]->GetNumber();
    v[1] = v_custom_colors[1]->GetNumber();
    v[2] = v_custom_colors[2]->GetNumber();
    v[3] = v_custom_colors[3]->GetNumber();
    v[4] = v_custom_colors[3]->GetNumber();
	v[5] = v_custom_colors[4]->GetNumber();
	v[6] = v_custom_colors[5]->GetNumber();
	v[7] = v_custom_colors[6]->GetNumber();
	v[8] = v_custom_colors[7]->GetNumber();

    return;
}

//Store here the binnings and ranges of all the variables to be plotted via ControlHistograms files
bool Get_Variable_Range(TString var, int& nbins, double& xmin, double& xmax)
{
    //Categories are either 0 or 1 (NB : encoded in Char_t!)
    if(var.BeginsWith("is_") || var.BeginsWith("passed") ) {nbins = 2; xmin = 0; xmax = 2;}

    else if(var == "metEt") {nbins = 20; xmin = 0; xmax = 300;}
	else if(var == "dEtaFwdJetBJet") {nbins = 15; xmin = 0; xmax = 5;}
	else if(var == "dEtaFwdJetClosestLep") {nbins = 15; xmin = 0; xmax = 5;}
	else if(var == "minDRll") {nbins = 15; xmin = 0; xmax = 3.5;}
    else if(var == "maxEtaJet") {nbins = 10; xmin = 0; xmax = 5;}
	else if(var == "dPhiHighestPtSSPair") {nbins = 20; xmin = 0; xmax = 3.2;}
	else if(var == "nJetEta1") {nbins = 7; xmin = 0.5; xmax = 7.5;}
	else if(var == "lepCharge") {nbins = 3; xmin = -1.5; xmax = 1.5;}
	else if(var == "hardestBjetPt") {nbins = 20; xmin = 20; xmax = 300;}
	else if(var == "hardestBjetEta") {nbins = 40; xmin = 0; xmax = 0.5;}
	else if(var == "FwdJetPt") {nbins = 20; xmin = 20; xmax = 200;}
	else if(var == "inv_mll") {nbins = 20; xmin = 50; xmax = 130;}
    else if(var == "mHT") {nbins = 10; xmin = 200; xmax = 1200;}
    else if(var == "top_mass") {nbins = 15; xmin = 100; xmax = 300;}
    else if(var == "dPhijj_max") {nbins = 10; xmin = 0; xmax = 3;}
    else if(var == "m3l" || var == "Mass_3l") {nbins = 20; xmin = 100; xmax = 500;}
    else if(var == "leptonCharge") {nbins = 3; xmin = -1.5; xmax = 1.5;}
    else if(var.Contains("CSV")) {nbins = 20; xmin = 0.; xmax = 1.1;}

    else if(var.Contains("dR") || var.Contains("DelR") ) {nbins = 20; xmin = 0; xmax = 7.;}

    // else if(var == "Lep3Pt") {nbins = 20; xmin = 0; xmax = 80;}
	else {return false;}

	return true;
}

//Get name of category, depending on desired region, sample, etc.
TString Get_Category_Boolean_Name()
{
    TString categ_bool_name = "";

    return categ_bool_name;
}

//Computes total nof entries which will be processed by the Produce_Templates() function, so that the Timebar is correct
//NB1 : don't account for nof syst weights, since they are computed all at once when reading an event
//NB2 : this returns the nof entries which will get read, not processed (else, should take cuts into account, etc.)
float Count_Total_Nof_Entries(TString dir_ntuples, TString t_name, vector<TString> v_samples, vector<TString> v_systTrees, vector<TString> v_vars)
{
    float total_nentries = 0;

    for(int isample=0; isample<v_samples.size(); isample++)
    {
        // cout<<"v_samples[isample] "<<v_samples[isample]<<endl;

        //Open input TFile
        TString inputfile = dir_ntuples + v_samples[isample] + ".root";
        // cout<<"inputfile "<<inputfile<<endl;
        if(!Check_File_Existence(inputfile))
        {
            // cout<<endl<<"File "<<inputfile<<FRED(" not found!")<<endl;
            continue;
        }
        TFile* file_input = TFile::Open(inputfile, "READ");

        for(int itree=0; itree<v_systTrees.size(); itree++)
		{
			TTree* tree = 0;
            TString tmp = v_systTrees[itree];
			if(tmp == "") {tmp = t_name;}
            tree = (TTree*) file_input->Get(tmp);
			if(!tree)
			{
				// cout<<BOLD(FRED("ERROR : tree '"<<tmp<<"' not found in file : "<<inputfile<<" ! Skip !"))<<endl;
				continue; //Skip sample
			}

            float nentries_tmp = tree->GetEntries();
            // float nentries_tmp = tree->GetEntries("passedBJets");

            for(int ivar=0; ivar<v_vars.size(); ivar++)
            {
                total_nentries+= nentries_tmp; //Multiply nof entries by nof loops

                // cout<<"total_nentries = "<<total_nentries<<endl;
            } //vars
        } //trees
    } //samples

    // cout<<"total_nentries "<<total_nentries<<endl;

    return total_nentries;
}

//In Potato code, systematics variations weights are encoded into vectors
//NB : inconsistent indices : down variation may be element 0 or 1...
//Use this function to hard-code which vector element corresponds to which systematics, and set the address
/*
void SetBranchAddress_SystVariationArray(TTree* t, TString systname, double* var)
{
    TString original_variable_name = ""; //Name of the systematic as stored in the ntuple

    if(!t) {return;}

    if(systname == "") {return;}
    else if(systname == "PUDown") {original_variable_name = "varWeightPU[0]";}
    else if(systname == "PUUp") {original_variable_name = "varWeightPU[1]";}

    else if(systname == "prefiringWeightDown") {original_variable_name = "varWeightPrefire[0]";}
    else if(systname == "prefiringWeightUp") {original_variable_name = "varWeightPrefire[1]";}

    else if(systname == "BtagHDown") {original_variable_name = "btagEventWeight[1]";}
    else if(systname == "BtagHUp") {original_variable_name = "btagEventWeight[0]";}

    else if(systname == "BtagLDown") {original_variable_name = "btagEventWeight[3]";}
    else if(systname == "BtagLUp") {original_variable_name = "btagEventWeight[2]";}

    else if(systname == "LepEff_muLooseDown") {original_variable_name = "var_weight_Mu[0]";}
    else if(systname == "LepEff_muLooseUp") {original_variable_name = "var_weight_Mu[1]";}
    else if(systname == "LepEff_muTightDown") {original_variable_name = "var_weight_Mu[2]";}
    else if(systname == "LepEff_muTightUp") {original_variable_name = "var_weight_Mu[3]";}

    else if(systname == "LepEff_elLooseDown") {original_variable_name = "var_weight_El[0]";}
    else if(systname == "LepEff_elLooseUp") {original_variable_name = "var_weight_El[1]";}
    else if(systname == "LepEff_elTightDown") {original_variable_name = "var_weight_El[2]";}
    else if(systname == "LepEff_elTightUp") {original_variable_name = "var_weight_El[3]";}

    else{cout<<FRED("ERROR ! Systematic '"<<systname<<"' not included in function SetBranchAddress_SystVariation() from Helper.cxx ! Can *not* compute it !")<<endl; return;}

    t->SetBranchStatus(original_variable_name, 1);
    t->SetBranchAddress(original_variable_name, var);

    return;
}
*/
