#include "Func_other.h"

using namespace std;

//--------------------------------------------
// ##     ## ######## ##       ########  ######## ########
// ##     ## ##       ##       ##     ## ##       ##     ##
// ##     ## ##       ##       ##     ## ##       ##     ##
// ######### ######   ##       ########  ######   ########
// ##     ## ##       ##       ##        ##       ##   ##
// ##     ## ##       ##       ##        ##       ##    ##
// ##     ## ######## ######## ##        ######## ##     ##
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
double Convert_TString_To_Number(TString ts)
{
	double number = 0;
	string s = ts.Data();
	stringstream ss(s);
	ss >> number;
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

//Make sure that systematic name follows Combine conventions
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









//--------------------------------------------
// ######## ##     ## ##     ##    ###       ########     ###    ##    ## ##    ##
//    ##    ###   ### ##     ##   ## ##      ##     ##   ## ##   ###   ## ##   ##
//    ##    #### #### ##     ##  ##   ##     ##     ##  ##   ##  ####  ## ##  ##
//    ##    ## ### ## ##     ## ##     ##    ########  ##     ## ## ## ## #####
//    ##    ##     ##  ##   ##  #########    ##   ##   ######### ##  #### ##  ##
//    ##    ##     ##   ## ##   ##     ##    ##    ##  ##     ## ##   ### ##   ##
//    ##    ##     ##    ###    ##     ##    ##     ## ##     ## ##    ## ##    ##
//--------------------------------------------



//Modifies the TMVA standard output to extract only the useful ranking information (the hard-coded way)
void Extract_Ranking_Info(TString TMVA_output_file, TString bdt_type, TString channel)
{
	ifstream file_in(TMVA_output_file.Data() );

	ofstream file_out("ranking_tmp.txt");

	file_out<<"--- "<<bdt_type<<" "<<channel<<" : Variable Ranking ---"<<endl<<endl;

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
void Get_Ranking_Vectors(TString bdt_type, TString channel, vector<TString> &v_BDTvar_name, vector<double> &v_BDTvar_importance)
{
	TString file_rank_path = "./outputs/Rankings/"+bdt_type+"/RANKING_"+bdt_type+"_"+channel+".txt";
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

//Order the variables from file created via BDT-optimization sectrion of the main(), by decreasing significance LOSS caused by the removal of corresponding Variable
//--> the variable which affects the most the final significance will be ranked first
void Order_BDTvars_By_Decreasing_Signif_Loss(TString file_path)
{
	ifstream file_in(file_path.Data() );

	ofstream file_out( (file_path+"_ordered") );

	vector<TString> v_variables; vector<float> v_signif;

	string line; TString ts;
	int count_line=0;
	while(!file_in.eof() )
	{
		getline(file_in, line); count_line++; ts = line;
		if(count_line<7) {file_out<<ts<<endl; continue;}

		TString var_name_tmp;
		for(int ichar=0; ichar<line.size(); ichar++)
		{
			if(ts[ichar] != ' ') {var_name_tmp+= ts[ichar];}
			else {break;}
		}

		v_variables.push_back(var_name_tmp);

		int index = ts.First(">");
		ts.Remove(0, index+2);

		v_signif.push_back(Convert_TString_To_Number(ts) );
	}
	file_in.close();

	//Because while loop on 'eof' --> one line too many at the end --> erase last entry
	v_variables.erase(v_variables.begin() + v_variables.size()-1);
	v_signif.erase(v_signif.begin() + v_signif.size()-1);


	//--- Ordering
	vector<TString> v_variables_ordered; vector<float> v_signif_ordered;

	float signif_min = 999; int index_signif_min = 999;

	while(v_variables.size() != 0)
	{
		signif_min = 999; index_signif_min = 999;

		for(int i=0; i<v_signif.size(); i++)
		{
			if(v_signif[i] < signif_min)
			{
				signif_min = v_signif[i];
				index_signif_min = i;
			}
		}

		v_variables_ordered.push_back(v_variables[index_signif_min]);
		v_signif_ordered.push_back(v_signif[index_signif_min]);

		v_variables.erase(v_variables.begin() + index_signif_min);
		v_signif.erase(v_signif.begin() + index_signif_min);
	}

	//--- Write ordered vars into output file
	for(int i=0; i<v_variables_ordered.size(); i++)
	{
		file_out<<v_variables_ordered[i]<<" "<<v_signif_ordered[i]<<endl;
	}

	//--- Replace old file by new one
	// MoveFile( (file_path+"_ordered"), file_path );


	return;
}


//Order the sets of cuts from file created via cut-optimization section of the main(), by decreasing significance
void Order_Cuts_By_Decreasing_Signif_Loss(TString file_path)
{
	ifstream file_in(file_path.Data() );

	ofstream file_out( (file_path+"_ordered") );

	vector<TString> v_cuts; vector<float> v_signif;

	string line; TString ts;
	int count_line=0;
	while(!file_in.eof() )
	{
		getline(file_in, line); count_line++; ts = line;
		if(count_line<10) {file_out<<ts<<endl; continue;} //first lines useless

		TString cut_name_tmp;
		for(int ichar=0; ichar<line.size(); ichar++)
		{
			if(ts[ichar] != ' ') {cut_name_tmp+= ts[ichar];}
			else {break;}
		}

		v_cuts.push_back(cut_name_tmp);

		int index = ts.Index("->");
		ts.Remove(0, index+3);

		v_signif.push_back(Convert_TString_To_Number(ts) );
	}
	file_in.close();

	//Because while loop on 'eof' --> one line too many at the end --> erase last entry
	v_cuts.erase(v_cuts.begin() + v_cuts.size()-1);
	v_signif.erase(v_signif.begin() + v_signif.size()-1);


	//--- Ordering
	vector<TString> v_cuts_ordered; vector<float> v_signif_ordered;

	float signif_max = -999; int index_signif_max = -999;

	while(v_cuts.size() != 0)
	{
		signif_max = -999; index_signif_max = -999;

		for(int i=0; i<v_signif.size(); i++)
		{
			if(v_signif[i] > signif_max)
			{
				signif_max = v_signif[i];
				index_signif_max = i;
			}
		}

		v_cuts_ordered.push_back(v_cuts[index_signif_max]);
		v_signif_ordered.push_back(v_signif[index_signif_max]);

		v_cuts.erase(v_cuts.begin() + index_signif_max);
		v_signif.erase(v_signif.begin() + index_signif_max);
	}

	//--- Write ordered vars into output file
	for(int i=0; i<v_cuts_ordered.size(); i++)
	{
		file_out<<v_cuts_ordered[i]<<" "<<v_signif_ordered[i]<<endl;
	}

	//--- Replace old file by new one
	// MoveFile( (file_path+"_ordered"), file_path );


	return;
}


//--------------------------------------------
// ######## #### ##       ##          ##     ## ####  ######  ########  #######
// ##        ##  ##       ##          ##     ##  ##  ##    ##    ##    ##     ##
// ##        ##  ##       ##          ##     ##  ##  ##          ##    ##     ##
// ######    ##  ##       ##          #########  ##   ######     ##    ##     ##
// ##        ##  ##       ##          ##     ##  ##        ##    ##    ##     ##
// ##        ##  ##       ##          ##     ##  ##  ##    ##    ##    ##     ##
// ##       #### ######## ########    ##     ## ####  ######     ##     #######
//--------------------------------------------

//Increment weight of first bin by 'weight'
// void Fill_TH1F_UnderOverflow(TH1F* h, double value, double weight)
// {
// 	if(value < h->GetXaxis()->GetXmax() && value > h->GetXaxis()->GetXmin() ) {h->Fill(value, weight);}
// 	else if(value > h->GetXaxis()->GetXmax() ) {h->AddBinContent(h->GetNbinsX(), weight);} //overflow
// 	else if(value < h->GetXaxis()->GetXmin() ) {h->AddBinContent(1, weight);} //underflow
//
// 	return;
// }

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



//--------------------------------------------
//  ######  ##     ## ########  ######
// ##    ## ##     ##    ##    ##    ##
// ##       ##     ##    ##    ##
// ##       ##     ##    ##     ######
// ##       ##     ##    ##          ##
// ##    ## ##     ##    ##    ##    ##
//  ######   #######     ##     ######
//--------------------------------------------

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

//--------------------------------------------
// ##     ##    ###    ########         ########     ###    ##    ##  ######   ########
// ##     ##   ## ##   ##     ##        ##     ##   ## ##   ###   ## ##    ##  ##
// ##     ##  ##   ##  ##     ##        ##     ##  ##   ##  ####  ## ##        ##
// ##     ## ##     ## ########         ########  ##     ## ## ## ## ##   #### ######
//  ##   ##  ######### ##   ##          ##   ##   ######### ##  #### ##    ##  ##
//   ## ##   ##     ## ##    ##  ###    ##    ##  ##     ## ##   ### ##    ##  ##
//    ###    ##     ## ##     ## ###    ##     ## ##     ## ##    ##  ######   ########
//--------------------------------------------

//Store here the binnings and ranges of all the variables to be plotted via ControlHistograms files
bool Get_Variable_Range(TString var, int& nbins, double& xmin, double& xmax)
{
    //Categories are either 0 or 1 (NB : encoded in Char_t!)
    if(var.Contains("is_") ) {nbins = 2; xmin = 0; xmax = 2;}

	else if(var == "Lep3Pt") {nbins = 20; xmin = 0; xmax = 80;}
	else if(var == "dEtaFwdJetBJet") {nbins = 15; xmin = 0; xmax = 5;}
	else if(var == "dEtaFwdJet2BJet") {nbins = 6; xmin = 0; xmax = 4;}
	else if(var == "dEtaFwdJetClosestLep") {nbins = 15; xmin = 0; xmax = 5;}
	else if(var == "nJet25") {nbins = 8; xmin = -0.5; xmax = 7.5;}
	else if(var == "minDRll") {nbins = 15; xmin = 0; xmax = 3.5;}
    // else if(var == "maxEtaJet25") {nbins = 40; xmin = 0; xmax = 5;}
    else if(var == "maxEtaJet25") {nbins = 10; xmin = 0; xmax = 5;}
	else if(var == "dPhiHighestPtSSPair") {nbins = 20; xmin = 0; xmax = 3.2;}
	else if(var == "nJetEta1") {nbins = 7; xmin = 0.5; xmax = 7.5;}
	else if(var == "lepCharge") {nbins = 3; xmin = -1.5; xmax = 1.5;}
	else if(var == "hardestBjetPt") {nbins = 20; xmin = 20; xmax = 300;}
	else if(var == "hardestBjetEta") {nbins = 40; xmin = 0; xmax = 0.5;}
	else if(var == "FwdJetPt") {nbins = 20; xmin = 20; xmax = 200;}
	else if(var == "inv_mll") {nbins = 20; xmin = 50; xmax = 130;}

	//Additional vars
	else if(var == "lep1Pt") {nbins = 10; xmin = 0; xmax = 200;}
	else if(var == "lep2Pt") {nbins = 10; xmin = 0; xmax = 200;}
	else if(var == "lep3Pt") {nbins = 10; xmin = 0; xmax = 200;}
    else if(var == "nLightJets_Fwd40") {nbins = 3; xmin = 0; xmax = 3;}
    else if(var == "MET" || var == "metpt") {nbins = 10; xmin = 0; xmax = 400;}
    else if(var == "nMediumBJets") {nbins = 5; xmin = 0; xmax = 5;}
    else if(var == "nLooseBJets") {nbins = 5; xmin = 0; xmax = 5;}
    else if(var.Contains("signal_TT") ) {nbins = 10; xmin = -1; xmax = 1;}
    else if(var.Contains("signal_") ) {nbins = 20; xmin = -1; xmax = 1;}
	else if(var == "lep1_conePt") {nbins = 10; xmin = 0; xmax = 200;}
	else if(var == "lep2_conePt") {nbins = 10; xmin = 0; xmax = 200;}
	else if(var == "lep3_conePt") {nbins = 10; xmin = 0; xmax = 150;}
	else if(var == "mindr_lep1_jet") {nbins = 20; xmin = 0; xmax = 4;}
	else if(var == "mindr_lep2_jet") {nbins = 20; xmin = 0; xmax = 4;}
	else if(var == "mT_lep1") {nbins = 20; xmin = 50; xmax = 250;}
	else if(var == "mT_lep2") {nbins = 20; xmin = 50; xmax = 250;}
    else if(var == "max_lep_eta") {nbins = 10; xmin = 0; xmax = 2.5;}
    else if(var.Contains("dR") ) {nbins = 20; xmin = 0; xmax = 5;}
    else if(var == "LeadJetEta") {nbins = 20; xmin = -5; xmax = 5;}
    else if(var == "FwdJetEta") {nbins = 20; xmin = -5; xmax = 5;}
    else if(var == "lW_asym_mtop") {nbins = 20; xmin = -2.5; xmax = 2.5;}
    else if(var == "mTW") {nbins = 20; xmin = 0; xmax = 150;}
    else if(var.Contains("Lep", TString::kIgnoreCase) && var.Contains("Phi", TString::kIgnoreCase)) {nbins = 20; xmin = -3.2; xmax = 3.2;}
    else if(var.Contains("Lep", TString::kIgnoreCase) && var.Contains("Eta", TString::kIgnoreCase)) {nbins = 20; xmin = -2.5; xmax = 2.5;}
    else if(var == "metphi") {nbins = 20; xmin = -3.2; xmax = 3.2;}
    else if(var == "metLD") {nbins = 20; xmin = 0; xmax = 150;}
	else if(var == "mHT") {nbins = 20; xmin = 0; xmax = 300;}
	else if(var == "sum_jetPt") {nbins = 20; xmin = 0; xmax = 800;}
	else if(var == "resHTT") {nbins = 20; xmin = -2; xmax = 2;}
	else if(var == "HjTagger") {nbins = 20; xmin = -1; xmax = 0.6;}
	else if(var == "jet1_pt" || var == "jet2_pt" || var == "jet3_pt" || var == "jet4_pt") {nbins = 15; xmin = 0; xmax = 300;}
    else if(var == "Mjj_max") {nbins = 15; xmin = 0; xmax = 600;}
    else if(var == "top_mass") {nbins = 15; xmin = 100; xmax = 300;}
	else if(var == "dPhijj_max") {nbins = 10; xmin = 0; xmax = 3;}
	else if(var == "m3l") {nbins = 20; xmin = 0; xmax = 150;}
	else if(var == "mc_mem_thj_weight_log") {nbins = 20; xmin = -45; xmax = -20;}
	else if(var == "mc_mem_ttz_weight_log") {nbins = 20; xmin = -65; xmax = -40;}
	else if(var == "mc_mem_ttw_weight_log") {nbins = 20; xmin = -45; xmax = -25;}
	else if(var == "ratio_lep3pt_closestJetPt") {nbins = 20; xmin = -45; xmax = -25;}

    //BDTs
    else if(var.Contains("BDT") ) {nbins = 10; xmin = -1; xmax = 1;}

    //Testing, xchecks, ...
    else if(var == "JetNoisyHCALPt" || var == "JetNoisyHCAL2Pt") {nbins = 40; xmin = 20; xmax = 40;}
    else if(var == "JetNoisyHCALEta" || var == "JetNoisyHCAL2Eta") {nbins = 30; xmin = 2.7; xmax = 3.;}

	else {return false;}

	return true;
}



//--------------------------------------------
// ########  ######## ########  #### ##    ##
// ##     ## ##       ##     ##  ##  ###   ##
// ##     ## ##       ##     ##  ##  ####  ##
// ########  ######   ########   ##  ## ## ##
// ##   ##   ##       ##     ##  ##  ##  ####
// ##    ##  ##       ##     ##  ##  ##   ###
// ##     ## ######## ########  #### ##    ##
//--------------------------------------------

/**
 * NEW : according to code, swapped 7<->9 & 2<->5
 * https://github.com/stiegerb/cmgtools-lite/blob/80X_M17_tHqJan30/TTHAnalysis/python/plotter/tHq-multilepton/functionsTHQ.cc
 */
int Get_tHq2017_Binning(double mva_ttbar, double mva_ttV, TString nLep_cat)
{
    const float x_1 = -0.5;
    const float x_2 = 0.4;
    const float x_3 = 0.7;
    const float y_1 = -0.5;
    const float y_2 = 0.4;
    const float y_3 = 0.7;

    const float a_1 = -0.3;
    const float a_2 = 0.3;
    const float a_3 = 0.7;
    const float b_1 = -0.3;
    const float b_2 = 0.2;
    const float b_3 = 0.6;

	const float w_1 = -0.4;
	const float w_2 = 0.4;
	const float w_3 = 0.7;
	const float z_1 = -0.5;
	const float z_2 = 0.2;

 //  #####
 // #     # #
 //       # #
 //  #####  #
 //       # #
 // #     # #
 //  #####  ######

    if(nLep_cat == "3l")
    {
//-------------------------------------------- // TEST
		// if( mva_ttbar  > 0.6  && mva_ttV  >  0.8 ) return 9+1;
		// if( mva_ttbar  > 0.2  && mva_ttV  >  0.8 ) return 8+1;
		// if( mva_ttbar  > -1  && mva_ttV  >  0.8 ) return 4+1;
		// if( mva_ttbar  > 0.6  && mva_ttV  >  0.4 ) return 7+1;
		// if( mva_ttbar  > 0.2  && mva_ttV  >  0.4 ) return 6+1;
		// if( mva_ttbar  > -0.2  && mva_ttV  >  0.4 ) return 5+1;
		// if( mva_ttbar  > -1  && mva_ttV  >  0.4 ) return 4+1;
		// if( mva_ttbar  > 0.2  && mva_ttV  >  0 ) return 4;
		// if( mva_ttbar  > 0.2  && mva_ttV  >  -1 ) return 3;
		// if( mva_ttbar > -0.4 && mva_ttV  >  -1) return 2;
		// return 1;

//-------------------------------------------- // Best results yet (with MEM)
		if( mva_ttbar  > 0.6  && mva_ttV  >  0.8 ) return 9;
		if( mva_ttbar  > 0  && mva_ttV  >  0.8 ) return 8;
		if( mva_ttbar  > -1  && mva_ttV  >  0.8 ) return 4;
		if( mva_ttbar  > 0.6  && mva_ttV  >  0.4 ) return 7;
		if( mva_ttbar  > 0.2  && mva_ttV  >  0.4 ) return 6;
		if( mva_ttbar  > -0.2  && mva_ttV  >  0.4 ) return 5;
		if( mva_ttbar  > -1  && mva_ttV  >  0.4 ) return 4;
		if( mva_ttbar  > 0.2  && mva_ttV  >  -1 ) return 3;
		if( mva_ttbar > -0.4 && mva_ttV  >  -1) return 2;
		return 1;

// //-------------------------------------------- //0.8242
		// if( mva_ttbar  > 0.7  && mva_ttV  >  0.7 ) return 10;
		// if( mva_ttbar  > 0.7  && mva_ttV  >  0.4 ) return 9;
		// if( mva_ttbar  > 0.4  && mva_ttV  >  0.7 ) return 8;
		// if( mva_ttbar  > 0.4  && mva_ttV  >  0.4 ) return 7;
		// if( mva_ttbar  > -0.5  && mva_ttV  >  0.4 ) return 6;
		// if( mva_ttbar  > 0.4  && mva_ttV  >  -0.5 ) return 5;
		// if( mva_ttbar  > -0.5  && mva_ttV  >  -0.5 ) return 4;
		// if( mva_ttbar  > -0.5  && mva_ttV  >  -1 ) return 3;
		// if( mva_ttbar > -1 && mva_ttV  >  -0.5) return 2;
		// return 1;
//-------------------------------------------- // //0.847
		// if( mva_ttbar  > 0.7  && mva_ttV  >  0.7 ) return 10;
		// if( mva_ttbar  > 0.4  && mva_ttV  >  0.7 ) return 9;
		// if( mva_ttbar  > 0.2  && mva_ttV  >  0.7 ) return 8;
		// if( mva_ttbar  > 0.7  && mva_ttV  >  0.2 ) return 7;
		// if( mva_ttbar  > 0.2  && mva_ttV  >  0.2 ) return 6;
		// if( mva_ttbar  > 0.2  && mva_ttV  >  -0.5 ) return 5;
		// if( mva_ttbar  > -0.5  && mva_ttV  >  0.2 ) return 4;
		// if( mva_ttbar > -0.5 && mva_ttV  >  -0.5 ) return 2;
		// if( mva_ttbar > -0.5 && mva_ttV  >  -1) return 3;
		// // if( mva_ttbar > -1 && mva_ttV  >  -0.4 ) return 2;
		// return 1;

//-------------------------------------------- //16 bins ==> for visualization ! cf 2016 tHq AN
		// if( mva_ttbar  > 0.5  && mva_ttV  >  0.5 ) return 16;
		// if( mva_ttbar  > 0.5  && mva_ttV  >  0 ) return 14;
		// if( mva_ttbar  > 0.  && mva_ttV  >  0.5 ) return 13;
		// if( mva_ttbar  > 0  && mva_ttV  >  0 ) return 15;
	    // if( mva_ttbar  > 0.5  && mva_ttV  >  -0.5 ) return 9;
	    // if( mva_ttbar  > 0  && mva_ttV  >  -0.5 ) return 12;
		// if( mva_ttbar  > -0.5 && mva_ttV  > 0.5 ) return 8;
		// if( mva_ttbar  > 0.5  && mva_ttV  >  -1 ) return 7;
		// if( mva_ttbar  > -0.5 && mva_ttV  > 0 ) return 11;
		// if( mva_ttbar  > -0.5  && mva_ttV  > -0.5 ) return 10;
		// if( mva_ttbar  > 0  && mva_ttV  > -1) return 5;
		// if( mva_ttbar  > -1  && mva_ttV  > 0.5 ) return 6;
		// if( mva_ttbar  > -1 && mva_ttV  >  0) return 4;
		// if( mva_ttbar  > -0.5 && mva_ttV  >  -1 ) return 3;
		// if( mva_ttbar  > -1 && mva_ttV  >  -0.5 ) return 2;
		// return 1;
//--------------------------------------------
    }

 //  #####
 // #     # #
 //       # #
 //  #####  #
 // #       #
 // #       #
 // ####### ######

    else if(nLep_cat == "2l")
    {
//-------------------------------------------- //limit 1.89 in eu... (instead of 1.73 !)
		// if( mva_ttbar  > 0.5  && mva_ttV  >  -0.5 ) return 10;
		// if( mva_ttbar  > 0.  && mva_ttV  >  0.75 ) return 7;
		// if( mva_ttbar  > -0.5  && mva_ttV  >  0.5 ) return 6;
		// if( mva_ttbar  > 0  && mva_ttV  >  0.25 ) return 9;
		// if( mva_ttbar  > -0.5 && mva_ttV  > 0.25 ) return 8;
		// if( mva_ttbar  > -0.5 && mva_ttV  > 0 ) return 5;
		// if( mva_ttbar  > 0  && mva_ttV  >  -1 ) return 3;
		// if( mva_ttbar  > -0.5  && mva_ttV  > -0.5 ) return 4;
		// if( mva_ttbar  > -0.5  && mva_ttV  >  -1 ) return 3;
		// if( mva_ttbar  > -1 && mva_ttV  >  -0.5 ) return 2;
		// return 1;

//-------------------------------------------- //16 bins ==> for visualization ! cf 2016 tHq AN
		// if( mva_ttbar  > 0.5  && mva_ttV  >  0.5 ) return 16;
		// if( mva_ttbar  > 0.5  && mva_ttV  >  0 ) return 14;
		// if( mva_ttbar  > 0.  && mva_ttV  >  0.5 ) return 13;
		// if( mva_ttbar  > 0  && mva_ttV  >  0 ) return 15;
	    // if( mva_ttbar  > 0.5  && mva_ttV  >  -0.5 ) return 9;
	    // if( mva_ttbar  > 0  && mva_ttV  >  -0.5 ) return 12;
		// if( mva_ttbar  > -0.5 && mva_ttV  > 0.5 ) return 8;
		// if( mva_ttbar  > 0.5  && mva_ttV  >  -1 ) return 7;
		// if( mva_ttbar  > -0.5 && mva_ttV  > 0 ) return 11;
		// if( mva_ttbar  > -0.5  && mva_ttV  > -0.5 ) return 10;
		// if( mva_ttbar  > 0  && mva_ttV  > -1) return 5;
		// if( mva_ttbar  > -1  && mva_ttV  > 0.5 ) return 6;
		// if( mva_ttbar  > -1 && mva_ttV  >  0) return 4;
		// if( mva_ttbar  > -0.5 && mva_ttV  >  -1 ) return 3;
		// if( mva_ttbar  > -1 && mva_ttV  >  -0.5 ) return 2;
		// return 1;
//-------------------------------------------- //First try, for default BDT -- limit 1.91
		// if( mva_ttbar  > 0.4  && mva_ttV  >  0.8 ) return 10;
		// if( mva_ttbar  > 0.4  && mva_ttV  >  0.4 ) return 9;
		// if( mva_ttbar  > 0 && mva_ttV  > 0.4 ) return 8;
		// if( mva_ttbar  > 0  && mva_ttV  >  0 ) return 7;
		// if( mva_ttbar  > -0.4  && mva_ttV  >  0 ) return 6;
		// if( mva_ttbar  > -0.2 && mva_ttV  >  -0.4 ) return 5;
		// if( mva_ttbar  > -1  && mva_ttV  >  0 ) return 4; //switched with 5
		// if( mva_ttbar  > -0.6 && mva_ttbar  < 0 && mva_ttV  >  -0.6 ) return 3;
		// if( mva_ttbar  > -0.6 && mva_ttV  >  -1 ) return 2;
		// return 1;
//-------------------------------------------- //NEW BINNING FROM PALLABI (8 bins)

    	//I switched bins 3 and 4 for shape
        // if( mva_ttbar  > w_3  && mva_ttV  >  z_2 ) return 8;
        // if( mva_ttbar  > w_2  && mva_ttV  >  z_2 ) return 6;
        // if( mva_ttbar  > w_1  && mva_ttV  >  z_2 ) return 2;
        // if( mva_ttbar >= -1.0 && mva_ttV  >  z_2 ) return 5;
        // if( mva_ttbar  > w_3  && mva_ttV  >  z_1 ) return 7;
        // if( mva_ttbar  > w_2  && mva_ttV  >  z_1 ) return 7;
        // if( mva_ttbar  > w_1  && mva_ttV  >  z_1 ) return 1;
        // if( mva_ttbar >= -1.0 && mva_ttV  >  z_1 ) return 5;
        // if( mva_ttbar  > w_3  && mva_ttV >= -1.0 ) return 4; //switched with bin 3
        // if( mva_ttbar  > w_2  && mva_ttV >= -1.0 ) return 4;
        // if( mva_ttbar  > w_1  && mva_ttV >= -1.0 ) return 4;
        // if( mva_ttbar >= -1.0 && mva_ttV >= -1.0 ) return 3; //switched with bin 4

	    if( mva_ttbar  > w_3  && mva_ttV  >  z_2 ) return 8;
	    if( mva_ttbar  > w_2  && mva_ttV  >  z_2 ) return 6;
	    if( mva_ttbar  > w_1  && mva_ttV  >  z_2 ) return 2;
	    if( mva_ttbar >= -1.0 && mva_ttV  >  z_2 ) return 5;
	    if( mva_ttbar  > w_3  && mva_ttV  >  z_1 ) return 7;
	    if( mva_ttbar  > w_2  && mva_ttV  >  z_1 ) return 7;
	    if( mva_ttbar  > w_1  && mva_ttV  >  z_1 ) return 1;
	    if( mva_ttbar >= -1.0 && mva_ttV  >  z_1 ) return 5;
	    if( mva_ttbar  > w_3  && mva_ttV >= -1.0 ) return 3;
	    if( mva_ttbar  > w_2  && mva_ttV >= -1.0 ) return 3;
	    if( mva_ttbar  > w_1  && mva_ttV >= -1.0 ) return 3;
	    if( mva_ttbar >= -1.0 && mva_ttV >= -1.0 ) return 4;

	    //-- old
	    if( mva_ttbar  > w_3  && mva_ttV  >  z_2 ) return 8;
	    if( mva_ttbar  > w_2  && mva_ttV  >  z_2 ) return 7;
	    if( mva_ttbar  > w_1  && mva_ttV  >  z_2 ) return 6;
	    if( mva_ttbar >= -1.0 && mva_ttV  >  z_2 ) return 4;
	    if( mva_ttbar  > w_3  && mva_ttV  >  z_1 ) return 5;
	    if( mva_ttbar  > w_2  && mva_ttV  >  z_1 ) return 5;
	    if( mva_ttbar  > w_1  && mva_ttV  >  z_1 ) return 2;
	    if( mva_ttbar >= -1.0 && mva_ttV  >  z_1 ) return 4;
	    if( mva_ttbar  > w_3  && mva_ttV >= -1.0 ) return 3;
	    if( mva_ttbar  > w_2  && mva_ttV >= -1.0 ) return 3;
	    if( mva_ttbar  > w_1  && mva_ttV >= -1.0 ) return 3;
	    if( mva_ttbar >= -1.0 && mva_ttV >= -1.0 ) return 1;

//--------------------------------------------
	}
	else {return -1;}

	return -1;
}


//--------------------------------------------
// ######## ##     ##  #######
//    ##    ##     ## ##     ##
//    ##    ##     ## ##     ##
//    ##    ######### ##     ##
//    ##    ##     ## ##  ## ##
//    ##    ##     ## ##    ##
//    ##    ##     ##  ##### ##
//--------------------------------------------

int Get_tHq2017_Binning_compPallabi(double mva_ttbar, double mva_ttV, TString nLep_cat)
{
	const float x_1 = -0.5;
	const float x_2 = 0.4;
	const float x_3 = 0.7;
	const float y_1 = -0.5;
	const float y_2 = 0.4;
	const float y_3 = 0.7;

	const float a_1 = -0.3;
	const float a_2 = 0.3;
	const float a_3 = 0.7;
	const float b_1 = -0.3;
	const float b_2 = 0.2;
	const float b_3 = 0.6;

	const float w_1 = -0.4;
	const float w_2 = 0.4;
	const float w_3 = 0.7;
	const float z_1 = -0.5;
	const float z_2 = 0.2;

	//-------------------------------------------- //tHq2016 -- 1.44 limit (w/ my default templates)
	if(nLep_cat == "3l")
	{
		if( mva_ttbar  > x_3  && mva_ttV  >  y_3 ) return 10;
		if( mva_ttbar  > x_2  && mva_ttV  >  y_3 ) return 9;
		if( mva_ttbar  > x_1  && mva_ttV  >  y_3 ) return 6;
		if( mva_ttbar >= -1.0 && mva_ttV  >  y_3 ) return 5;

		if( mva_ttbar  > x_3  && mva_ttV  >  y_2 ) return 8;
		if( mva_ttbar  > x_2  && mva_ttV  >  y_2 ) return 7;
		if( mva_ttbar  > x_1  && mva_ttV  >  y_2 ) return 6;
		if( mva_ttbar >= -1.0 && mva_ttV  >  y_2 ) return 5;

		if( mva_ttbar  > x_3  && mva_ttV  >  y_1 ) return 4;
		if( mva_ttbar  > x_2  && mva_ttV  >  y_1 ) return 4;
		if( mva_ttbar  > x_1  && mva_ttV  >  y_1 ) return 2;
		if( mva_ttbar >= -1.0 && mva_ttV  >  y_1 ) return 5;

		if( mva_ttbar  > x_3  && mva_ttV >= -1.0 ) return 3;
		if( mva_ttbar  > x_2  && mva_ttV >= -1.0 ) return 3;
		if( mva_ttbar  > x_1  && mva_ttV >= -1.0 ) return 3;
		if( mva_ttbar >= -1.0 && mva_ttV >= -1.0 ) return 1;
	}
	//-------------------------------------------- //tHq2016 //limit 1.84
	else //2l
	{
        if( mva_ttbar  > a_3  && mva_ttV  >  b_3 ) return 9;
        if( mva_ttbar  > a_2  && mva_ttV  >  b_3 ) return 7;
        if( mva_ttbar  > a_1  && mva_ttV  >  b_3 ) return 5;
        if( mva_ttbar >= -1.0 && mva_ttV  >  b_3 ) return 4;

        if( mva_ttbar  > a_3  && mva_ttV  >  b_2 ) return 10;
        if( mva_ttbar  > a_2  && mva_ttV  >  b_2 ) return 6;
        if( mva_ttbar  > a_1  && mva_ttV  >  b_2 ) return 5;
        if( mva_ttbar >= -1.0 && mva_ttV  >  b_2 ) return 4;

        if( mva_ttbar  > a_3  && mva_ttV  >  b_1 ) return 8;
        if( mva_ttbar  > a_2  && mva_ttV  >  b_1 ) return 8;
        if( mva_ttbar  > a_1  && mva_ttV  >  b_1 ) return 3;
        if( mva_ttbar >= -1.0 && mva_ttV  >  b_1 ) return 4;

        if( mva_ttbar  > a_3  && mva_ttV >= -1.0 ) return 2;
        if( mva_ttbar  > a_2  && mva_ttV >= -1.0 ) return 2;
        if( mva_ttbar  > a_1  && mva_ttV >= -1.0 ) return 2;
        if( mva_ttbar >= -1.0 && mva_ttV >= -1.0 ) return 1;
	}

	return -1;
}


int Get_ttH2017_Binning(TH2F* h, double mva_ttbar, double mva_ttV)
{
    return h->GetBinContent(h->FindBin(mva_ttbar, mva_ttV)) + 1;
}




//--------------------------------------------
// ########  ######  ##    ##  ######
// ##       ##    ## ###   ## ##    ##
// ##       ##       ####  ## ##
// ######   ##       ## ## ## ##
// ##       ##       ##  #### ##
// ##       ##    ## ##   ### ##    ##
// ##        ######  ##    ##  ######
//--------------------------------------------


int Get_FCNC_Binning(double mva_ttbar, double mva_ttV, TString nLep_cat, TString signal_process)
{
	//TESTING
	if(nLep_cat == "2l") //too much signal in bin 3 ?
    {
		if( mva_ttbar  > 0.7  && mva_ttV  >  -0.4 ) return 10;
		if( mva_ttbar  > 0.4  && mva_ttV  >  0.7 ) return 10;
		if( mva_ttbar  > 0  && mva_ttV  >  0.7 ) return 8;
		if( mva_ttbar  > 0.4  && mva_ttV  >  0 ) return 9;
		if( mva_ttbar > 0 && mva_ttV  >  0.4 ) return 7;
		if( mva_ttbar > 0 && mva_ttV  >  0 ) return 6;
        if( mva_ttbar  > -0.4  && mva_ttV  >  0 ) return 2;
		if( mva_ttbar  > 0  && mva_ttV  >  -0.4 ) return 5;
		if( mva_ttbar  > 0  && mva_ttV  >  -1 ) return 4;
		if( mva_ttbar  > -0.4  && mva_ttV  >  -0.4 ) return 1;
		if( mva_ttbar >= -1.0 && mva_ttV  >  -0.4 ) return 3;
		return 1;
    }
    else //3l
    {
		if( mva_ttbar  > 0.5  && mva_ttV  >  0.5 ) return 10;
		if( mva_ttbar  > 0.5  && mva_ttV  >  0 ) return 9;
        if( mva_ttbar  > 0.2  && mva_ttV  >  0.5 ) return 8;
		if( mva_ttbar  > 0.2  && mva_ttV  >  0. ) return 7;
		if( mva_ttbar  > 0.  && mva_ttV  >  0. ) return 6;
		if( mva_ttbar > -0.4 && mva_ttV  > 0 ) return 4;
		if( mva_ttbar  > -0.2  && mva_ttV  >  -0.2 ) return 5;
        if( mva_ttbar  > -0.2  && mva_ttV  >  -1 ) return 3;
		if( mva_ttbar >= -1.0 && mva_ttV  >  -0.2 ) return 2;
		return 1;
    }

//--------------------------------------------
	//Previous, AN
    // if(nLep_cat == "2l") //too much signal in bin 3 ?
    // {
	// 	if( mva_ttbar  > 0.7  && mva_ttV  >  -0.4 ) return 10;
    //     if( mva_ttbar  > 0  && mva_ttV  >  0.7 ) return 8;
	// 	if( mva_ttbar  > 0.4  && mva_ttV  >  0 ) return 9;
	// 	if( mva_ttbar > 0 && mva_ttV  >  0.4 ) return 7;
	// 	if( mva_ttbar > 0 && mva_ttV  >  0 ) return 6;
    //     if( mva_ttbar  > -0.4  && mva_ttV  >  0 ) return 3;
	// 	if( mva_ttbar  > 0  && mva_ttV  >  -0.4 ) return 5;
	// 	// if( mva_ttbar  > -0.2  && mva_ttV  >  -0.4 ) return 5; //reduced, benefit to 1
	// 	if( mva_ttbar  > 0  && mva_ttV  >  -1 ) return 4;
	// 	if( mva_ttbar  > -0.4  && mva_ttV  >  -0.4 ) return 1; //added
	// 	if( mva_ttbar >= -1.0 && mva_ttV  >  -0.4 ) return 2;
	// 	return 1;
    // }
    // else //3l
    // {
	// 	if( mva_ttbar  > 0.5  && mva_ttV  >  0.5 ) return 10;
	// 	if( mva_ttbar  > 0.5  && mva_ttV  >  0 ) return 9;
    //     if( mva_ttbar  > 0.2  && mva_ttV  >  0.5 ) return 8;
	// 	if( mva_ttbar  > 0.2  && mva_ttV  >  0. ) return 7;
	// 	if( mva_ttbar  > 0.  && mva_ttV  >  0. ) return 6;
	// 	if( mva_ttbar > -0.4 && mva_ttV  > 0 ) return 4;
	// 	if( mva_ttbar  > -0.2  && mva_ttV  >  -0.2 ) return 5;
    //     if( mva_ttbar  > -0.2  && mva_ttV  >  -1 ) return 3;
	// 	if( mva_ttbar >= -1.0 && mva_ttV  >  -0.2 ) return 2;
	// 	return 1;
    // }

    return 0;
}


//--------------------------------------------
//  ######   #######  ##     ## ##     ##    ###    ##    ## ########     ##       #### ##    ## ########
// ##    ## ##     ## ###   ### ###   ###   ## ##   ###   ## ##     ##    ##        ##  ###   ## ##
// ##       ##     ## #### #### #### ####  ##   ##  ####  ## ##     ##    ##        ##  ####  ## ##
// ##       ##     ## ## ### ## ## ### ## ##     ## ## ## ## ##     ##    ##        ##  ## ## ## ######
// ##       ##     ## ##     ## ##     ## ######### ##  #### ##     ##    ##        ##  ##  #### ##
// ##    ## ##     ## ##     ## ##     ## ##     ## ##   ### ##     ##    ##        ##  ##   ### ##
//  ######   #######  ##     ## ##     ## ##     ## ##    ## ########     ######## #### ##    ## ########
//--------------------------------------------

/**
Modifies the region choice booleans, according to command args values

 */
bool Apply_CommandArgs_Choices(int argc, char **argv, TString& template_name, bool& use_3l_events, TString& region_choice)
{
	if(argc >= 2)
	{
		if(!strcmp(argv[1],"3l") ) {use_3l_events = true;}
		else {use_3l_events = false;}

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






//--------------------------------------------
//  ######   ######## ########     ######     ###    ##     ## ########  ##       ########     ######   #######  ##        #######  ########
// ##    ##  ##          ##       ##    ##   ## ##   ###   ### ##     ## ##       ##          ##    ## ##     ## ##       ##     ## ##     ##
// ##        ##          ##       ##        ##   ##  #### #### ##     ## ##       ##          ##       ##     ## ##       ##     ## ##     ##
// ##   #### ######      ##        ######  ##     ## ## ### ## ########  ##       ######      ##       ##     ## ##       ##     ## ########
// ##    ##  ##          ##             ## ######### ##     ## ##        ##       ##          ##       ##     ## ##       ##     ## ##   ##
// ##    ##  ##          ##       ##    ## ##     ## ##     ## ##        ##       ##          ##    ## ##     ## ##       ##     ## ##    ##
//  ######   ########    ##        ######  ##     ## ##     ## ##        ######## ########     ######   #######  ########  #######  ##     ##
//--------------------------------------------
/**
Select a color for each activated sample
 */
void Get_Samples_Colors(vector<int>& v_colors, vector<TString> v_samples, int color_scheme)
{
	int i_skipData = 0; //index //include data in color vector for now (empty element)

//--------------------------------------------
	if(color_scheme == 0) //From ttH/tHq 2016
	{
		for(int isample=0; isample<v_samples.size(); isample++)
		{
            //Signals
			if(v_samples[isample].Contains("tHq")) {v_colors[isample-i_skipData] = kOrange+10;}
			else if(v_samples[isample].Contains("tHW")) {v_colors[isample-i_skipData] = kOrange+6;} //changed from orange+6
			else if(v_samples[isample].Contains("ttH")) {v_colors[isample-i_skipData] = kOrange+2;}
            else if(v_samples[isample].Contains("tH_ST")) {v_colors[isample-i_skipData] = kRed;}
            else if(v_samples[isample].Contains("tH_TT")) {v_colors[isample-i_skipData] = kRed;}
            else if(v_samples[isample].Contains("FCNC")) {v_colors[isample-i_skipData] = kRed;}

            //ttZ, ttbar
			else if(v_samples[isample].Contains("ttW") ) {v_colors[isample-i_skipData] = kGreen-5;}
			else if(v_samples[isample].Contains("ttZ")) {v_colors[isample-i_skipData] = kSpring+2;}
            else if(v_samples[isample].Contains("TTbar") || v_samples[isample].Contains("TTJet")) {v_colors[isample-i_skipData] = kSpring+2;}

            //EWK
			else if(v_samples[isample] == "WZ" || v_samples[isample].Contains("WZ_") ) {v_colors[isample-i_skipData] = kViolet-4;}
			else if(v_samples[isample] == "ZZ") {v_colors[isample-i_skipData] = kViolet-4;}
			else if(v_samples[isample].Contains("DY")) {v_colors[isample-i_skipData] = kViolet-4;}
			else if(v_samples[isample] == "WJets") {v_colors[isample-i_skipData] = kViolet-4;}

            //Rares
            else if(v_samples[isample] == "tZq") {v_colors[isample-i_skipData] = kAzure-7;}
            // else if(v_samples[isample] == "tZq") {v_colors[isample-i_skipData] = kAzure-9;}
			else if(v_samples[isample] == "tWZ") {v_colors[isample-i_skipData] = kAzure-9;}
			// else if(v_samples[isample] == "tWZ") {v_colors[isample-i_skipData] = kAzure;} //Different coloring for tWZ
			else if(v_samples[isample] == "WZZ") {v_colors[isample-i_skipData] = kAzure-9;}
            else if(v_samples[isample] == "WWZ") {v_colors[isample-i_skipData] = kAzure-9;}
            else if(v_samples[isample] == "WZZ") {v_colors[isample-i_skipData] = kAzure-9;}
            else if(v_samples[isample] == "ZZZ") {v_colors[isample-i_skipData] = kAzure-9;}
            else if(v_samples[isample] == "WWW") {v_colors[isample-i_skipData] = kAzure-9;}
			else if(v_samples[isample] == "TTTT") {v_colors[isample-i_skipData] = kAzure-9;}
			else if(v_samples[isample] == "tGJets") {v_colors[isample-i_skipData] = kAzure-9;}
			else if(v_samples[isample] == "WpWp") {v_colors[isample-i_skipData] = kAzure-9;}
            else if(v_samples[isample] == "WW_DPS") {v_colors[isample-i_skipData] = kAzure-9;}
            else if(v_samples[isample] == "ttG") {v_colors[isample-i_skipData] = kAzure-9;}
            else if(v_samples[isample] == "tttW") {v_colors[isample-i_skipData] = kAzure-9;}
            else if(v_samples[isample] == "ttWW") {v_colors[isample-i_skipData] = kAzure-9;}
            else if(v_samples[isample] == "ttWH") {v_colors[isample-i_skipData] = kAzure-9;}
            else if(v_samples[isample] == "WZG") {v_colors[isample-i_skipData] = kAzure-9;}
			else if(v_samples[isample] == "GGHZZ4L") {v_colors[isample-i_skipData] = kAzure-9;}
			else if(v_samples[isample] == "WG") {v_colors[isample-i_skipData] = kAzure-9;}
			else if(v_samples[isample] == "VHToNonbb") {v_colors[isample-i_skipData] = kAzure-9;}


			else if(v_samples[isample].Contains("QFlip")) {v_colors[isample-i_skipData] = kBlack;}
			else if(v_samples[isample].Contains("Fakes")) {v_colors[isample-i_skipData] = kBlack;}
			else if(v_samples[isample].Contains("Conv") ) {v_colors[isample-i_skipData] = kOrange;}
		}
	}

	return;
}


//Use custom color palette
//-- //-- Idea : take good-looking/efficient color palettes from web and apply it manually
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






//--------------------------------------------
//--------------------------------------------
//  #######    #####      ##    #######      ######     ###    ##     ## ########  ##       ########  ######
// ##     ##  ##   ##   ####   ##     ##    ##    ##   ## ##   ###   ### ##     ## ##       ##       ##    ##
//        ## ##     ##    ##   ##           ##        ##   ##  #### #### ##     ## ##       ##       ##
//  #######  ##     ##    ##   ########      ######  ##     ## ## ### ## ########  ##       ######    ######
// ##        ##     ##    ##   ##     ##          ## ######### ##     ## ##        ##       ##             ##
// ##         ##   ##     ##   ##     ##    ##    ## ##     ## ##     ## ##        ##       ##       ##    ##
// #########   #####    ######  #######      ######  ##     ## ##     ## ##        ######## ########  ######
//--------------------------------------------
//--------------------------------------------

void Get_SampleList_tHq2016(vector<TString>& v_samples, bool use_3l_events)
{
	//Erase the 2017 samples from the vector
	v_samples.clear();


	//Fill with 2016 samples

	//DATA --- 1 sample, in first position
	v_samples.push_back("DATA");

	//Signals
	v_samples.push_back("tHq");
	v_samples.push_back("tHW");

	//Major BKG
	v_samples.push_back("ttH");
	v_samples.push_back("ttW");
	v_samples.push_back("ttZ");
	v_samples.push_back("WZ");

	//Minor BKG
	v_samples.push_back("ZZ");
	v_samples.push_back("tZq");
	v_samples.push_back("tWZ");
	v_samples.push_back("WZZ");
	v_samples.push_back("WWZ");
	v_samples.push_back("ZZZ");
	v_samples.push_back("TTTT");
	if(!use_3l_events) {v_samples.push_back("WW_EWK");}
	if(!use_3l_events) {v_samples.push_back("WW_DPS");}

	//Gamma-conv
	v_samples.push_back("GammaConv"); //From ttG, tG, ZG, WG samples

	//Fakes -- MC or data-driven
	v_samples.push_back("Fakes"); //Data-driven

	v_samples.push_back("ST");
	v_samples.push_back("TTbar_DiLep");
	v_samples.push_back("TTbar_SingleLep");
	// v_samples.push_back("WWTo2L2Nu");
	// v_samples.push_back("DY"); //Wrong weights ?

	//Charge flip
	if(!use_3l_events) {v_samples.push_back("QFlip");} //Data-driven

	return;
}



/**
 * Obsolete func to get name of boolean corresponding to 2016 categories
 */
TString Find_Corresponding_Category_2016(TString nlep, TString region, TString sample/*="tHq"*/)
{
	if(nlep == "3l")
	{
		if(region=="SR")
		{
			if(sample == "Fakes") {return "is_3l_AppFakes_SR";}
			else if(sample == "GammaConv") {return "is_3l_GammaConv_SR";}
			else {return "is_3l_THQ_SR";}

		}
		else if(region=="Training") {return "is_3l_THQ_Training";}
		else if(region=="CR") {return "is_3l_Z_CR";}
	}
	else
	{
		if(region=="SR")
		{
			if(sample == "Fakes") {return "is_2l_AppFakes_SR";}
			else if(sample == "GammaConv") {return "is_2l_GammaConv_SR";}
			else {return "is_2l_THQ_SR";}

		}
		else if(region=="Training") {return "is_2l_THQ_Training";}
		// else if(region=="CR") {return "is_2l_EMU_CR";}
		else if(region=="CR") {return "is_2l_TTW_CR";}
	}

	return "";
}














//--------------------------------------------
//  ######   ######## ########     ######     ###    ######## ########  ######
// ##    ##  ##          ##       ##    ##   ## ##      ##    ##       ##    ##
// ##        ##          ##       ##        ##   ##     ##    ##       ##
// ##   #### ######      ##       ##       ##     ##    ##    ######   ##   ####
// ##    ##  ##          ##       ##       #########    ##    ##       ##    ##
// ##    ##  ##          ##       ##    ## ##     ##    ##    ##       ##    ##  ###
//  ######   ########    ##        ######  ##     ##    ##    ########  ######   ###
//--------------------------------------------

/**
 * Get name of category, depending on desired region, sample, etc.
 * Hard-coded
 */
TString Get_Category_Boolean_Name(TString nLep_cat, TString region, TString analysis_type, TString sample, TString scheme)
{
    TString categ_bool_name = "";

    if(analysis_type == "ttH") //ttH2017 categories, predefined at NTProd level
    {
        categ_bool_name = "is_ttH_";
	}
	else if(analysis_type == "tHq") //custom tHq categories
    {
        categ_bool_name = "is_tHq_";

        if(region.Contains("ttH")) {categ_bool_name = "is_ttH_";} //SRttH, ...
        if((scheme.Contains("njet") || scheme.Contains("fwd")) && region.Contains("ttW")) {categ_bool_name = "is_ttH_";} //For categ. studies, want to use the "is_ttH_ttWctrl" booleans !
	}
	else if(analysis_type == "FCNC") //custom tHq categories
    {
        categ_bool_name = "is_tHqFCNC_";

        // categ_bool_name = "is_tHqFCNC_ATLAS_";

        if(region.Contains("CR") && !region.Contains("Fake") && !region.Contains("_Z")) {categ_bool_name = "is_ttH_";} //For ttH CRs, use same booleans as for ttH analysis //Fakectrl and Zctrl are related to FCNC analysis only
	}

    if(region.Contains("SR") || region == "FCNC" || region == "Training")
	{
		if(nLep_cat == "3l") {categ_bool_name+= "3l";}
	    else if(nLep_cat == "2l") {categ_bool_name+= "2lSS";}
		else {cout<<FRED("nLep_cat choice problem !")<<endl; return "";}

        if(region == "Training") {categ_bool_name+= "_Training"; return categ_bool_name;}
	}
    else if(region == "CR_ttZ") {categ_bool_name+= "ttZctrl";}
    else if(region == "CR_ttW") {categ_bool_name+= "ttWctrl";}
    else if(region == "CR_ttWttH") {categ_bool_name+= "ttWctrl";}
	else if(region == "CR_WZ") {categ_bool_name+= "WZctrl";}
    else if(region == "CR_Fake") {categ_bool_name+= "Fakectrl";}
    else if(region == "CR_Z") {categ_bool_name+= "Zctrl";}
	else {cout<<FRED("Region choice problem !")<<endl; return "";}

    if(sample.Contains("Fake") ) {categ_bool_name+= "_Fake";}
    else if(sample == "QFlip") {categ_bool_name+= "_Flip";}
    else if(sample.Contains("Conv") ) {categ_bool_name+= "_GammaConv";}
    else {categ_bool_name+= "_SR";}

    return categ_bool_name;
}


/**
 * Get the path of the directory containing Ntuples for given region (NB : events from different categories are separated in different dirs/ntuples !)
 * Return the path without the suffix corresponding to the region, so that it can be modified easily in the main code
 */
TString Get_RootDirNtuple_Path(bool use_2016_ntuples, TString analysis_type, TString nLep_cat)
{
    TString dir_ntuples = "input_ntuples/";
	if(use_2016_ntuples) {dir_ntuples+= "2016/";}
	if(analysis_type == "ttH") {dir_ntuples+= "ttH2017/";}
	else if(analysis_type == "tHq") {dir_ntuples+= "tHq2017/";}
	else if(analysis_type == "FCNC") {dir_ntuples+= "FCNC/";}
	// dir_ntuples+= nLep_cat + "/";

    return dir_ntuples;
}

// bool Get_Dir_Content(string dir, vector<string> &files)
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
//  ######  ##     ##  #######   #######   ######  ########
// ##    ## ##     ## ##     ## ##     ## ##    ## ##
// ##       ##     ## ##     ## ##     ## ##       ##
// ##       ######### ##     ## ##     ##  ######  ######
// ##       ##     ## ##     ## ##     ##       ## ##
// ##    ## ##     ## ##     ## ##     ## ##    ## ##
//  ######  ##     ##  #######   #######   ######  ########


// #### ##    ## ########  ##     ## ########    ##     ##    ###    ########
//  ##  ###   ## ##     ## ##     ##    ##       ##     ##   ## ##   ##     ##
//  ##  ####  ## ##     ## ##     ##    ##       ##     ##  ##   ##  ##     ##
//  ##  ## ## ## ########  ##     ##    ##       ##     ## ##     ## ########
//  ##  ##  #### ##        ##     ##    ##        ##   ##  ######### ##   ##
//  ##  ##   ### ##        ##     ##    ##         ## ##   ##     ## ##    ##
// #### ##    ## ##         #######     ##          ###    ##     ## ##     ##
//--------------------------------------------

/**
 * Fill vector of variables, passed as reference
 * Fill it here rather than in main(), for easier testing
 * LIST OF ALL AVAILABLE VARS :
nJet25
maxEtaJet25
lepCharge
nJetEta1
dEtaFwdJetBJet
dEtaFwdJet2BJet
dEtaFwdJetClosestLep
dPhiHighestPtSSPair
minDRll
Lep3Pt
FwdJetEta
max_lep_eta
mT_lep1
lep1_conePt
lep2_conePt
minv_FwdJetBJet
dRjj
LeadJetEta
LeadJetPt
Mjj_max
m3l
dPhiLepLep_max
dPhijj_max
deepCSV_2nd
mindr_lep1_jet
FwdJetPt
hardestBjetPt
dPhiLepBJet_max
top_mass
mTW
lW_asym_mtop
dRBjetRecoilJet
dRLepWRecoilJet
RecoilJetPt
RecoilJetEta
LepWPt
LepWEta
top_Pt
deepCSV_max
hardestBjetEta
metpt
mindr_lep2_jet
Hj_tag_max
 */
void Get_Input_Variables(vector<TString> & thevarlist, TString template_name, bool use_3l_events)
{
	if(template_name == "ttbar")
	{
		if(!use_3l_events) //ttbar 2l
		{
			thevarlist.push_back("nJet25");
			thevarlist.push_back("maxEtaJet25");
			thevarlist.push_back("lepCharge");
			thevarlist.push_back("nJetEta1");
			thevarlist.push_back("dEtaFwdJetBJet");
			thevarlist.push_back("dEtaFwdJetClosestLep");
			thevarlist.push_back("dPhiHighestPtSSPair");
			thevarlist.push_back("minDRll");
			thevarlist.push_back("FwdJetEta");
			thevarlist.push_back("max_lep_eta");
			thevarlist.push_back("minv_FwdJetBJet");
			thevarlist.push_back("dRjj_max");
			thevarlist.push_back("LeadJetEta");
			thevarlist.push_back("Mjj_max");
			thevarlist.push_back("dPhiLepLep_max");
			thevarlist.push_back("dRBjetRecoilJet");
			thevarlist.push_back("FwdJetPt");
			thevarlist.push_back("lW_asym_mtop");
			thevarlist.push_back("mindr_lep1_jet");
			// thevarlist.push_back("dEtaFwdJet2BJet");
			// thevarlist.push_back("Lep3Pt");
			// thevarlist.push_back("hardestBjetPt");
		}
		else
		{
			thevarlist.push_back("nJet25");
			thevarlist.push_back("maxEtaJet25");
			thevarlist.push_back("lepCharge");
			thevarlist.push_back("nJetEta1");
			thevarlist.push_back("dEtaFwdJetBJet");
			thevarlist.push_back("dEtaFwdJet2BJet");
			thevarlist.push_back("dEtaFwdJetClosestLep");
			thevarlist.push_back("dPhiHighestPtSSPair");
			thevarlist.push_back("minDRll");
			thevarlist.push_back("FwdJetEta");
			thevarlist.push_back("max_lep_eta");
			thevarlist.push_back("minv_FwdJetBJet");
			thevarlist.push_back("dRjj_max");
			thevarlist.push_back("LeadJetEta");
			thevarlist.push_back("Mjj_max");
			thevarlist.push_back("dPhiLepLep_max");
			thevarlist.push_back("dRBjetRecoilJet");
			thevarlist.push_back("FwdJetPt");
			thevarlist.push_back("hardestBjetPt");
			thevarlist.push_back("lW_asym_mtop");
			thevarlist.push_back("mindr_lep1_jet");
			// thevarlist.push_back("Lep3Pt");
		}
	}
	else if(template_name == "ttV")
	{
		if(!use_3l_events) //ttV 2l
		{
			thevarlist.push_back("nJet25");
			thevarlist.push_back("maxEtaJet25");
			thevarlist.push_back("nJetEta1");
			thevarlist.push_back("dEtaFwdJetBJet");
			thevarlist.push_back("dEtaFwdJet2BJet");
			thevarlist.push_back("dEtaFwdJetClosestLep");
			thevarlist.push_back("dPhiHighestPtSSPair");
			thevarlist.push_back("minDRll");
			thevarlist.push_back("Lep3Pt");
			thevarlist.push_back("FwdJetEta");
	        thevarlist.push_back("max_lep_eta");
	        thevarlist.push_back("lep1_conePt");
	        thevarlist.push_back("lep2_conePt");
	        thevarlist.push_back("minv_FwdJetBJet");
	        thevarlist.push_back("dRjj_max");
	        thevarlist.push_back("LeadJetEta");
	        thevarlist.push_back("Mjj_max");
	        thevarlist.push_back("dPhiLepLep_max");
	        thevarlist.push_back("dRBjetRecoilJet");
            thevarlist.push_back("mindr_lep1_jet");
			// thevarlist.push_back("lepCharge");
		}
		else //ttV 3l //+5...
		{
			thevarlist.push_back("nJet25");
			thevarlist.push_back("maxEtaJet25");
			thevarlist.push_back("lepCharge");
			thevarlist.push_back("nJetEta1");
			thevarlist.push_back("dEtaFwdJetBJet");
			thevarlist.push_back("dEtaFwdJet2BJet");
			thevarlist.push_back("dEtaFwdJetClosestLep");
			thevarlist.push_back("dPhiHighestPtSSPair");
			thevarlist.push_back("minDRll");
			thevarlist.push_back("Lep3Pt");
			thevarlist.push_back("FwdJetEta");
			thevarlist.push_back("max_lep_eta");
			thevarlist.push_back("lep1_conePt");
			thevarlist.push_back("lep2_conePt");
			thevarlist.push_back("minv_FwdJetBJet");
			thevarlist.push_back("dRjj_max");
			thevarlist.push_back("LeadJetEta");
			thevarlist.push_back("Mjj_max");
			thevarlist.push_back("dPhiLepLep_max");
			thevarlist.push_back("dRBjetRecoilJet");
			thevarlist.push_back("m3l");
			thevarlist.push_back("FwdJetPt");
			thevarlist.push_back("dPhijj_max");
			thevarlist.push_back("mindr_lep1_jet");
		}
	}


}









//--------------------------------------------
 // #####  ######  ####   ####    ##   #      # #    #  ####
 // #    # #      #      #    #  #  #  #      # ##   # #    #
 // #    # #####   ####  #      #    # #      # # #  # #
 // #####  #           # #      ###### #      # #  # # #  ###
 // #   #  #      #    # #    # #    # #      # #   ## #    #
 // #    # ######  ####   ####  #    # ###### # #    #  ####
 //--------------------------------------------

/**
 * Returns min/max values for variable in arg
 * Will check all samples given in arg, and compute global min/max
 * Used to rescale input variables, necessary when using DNN
 * WARNING : some hardcoding here (LO samples, ...)
 */
pair<double,double> Get_MinMax_Variable(TString var, TString signal_process, TString bkg_type, TString nLep_cat, vector<TString> v_samples)
{
	pair<double, double> result;
	double min=pow(10, 10), max=-pow(10, 10);

	for(int isample=0; isample<v_samples.size(); isample++)
	{
        if(bkg_type == "ttV" && !v_samples[isample].Contains("ttZ") && !v_samples[isample].Contains("ttW_") && !v_samples[isample].Contains(signal_process)) {continue;}
        else if(bkg_type == "ttbar" && !v_samples[isample].Contains("TTJet") && !v_samples[isample].Contains("TTbar") && !v_samples[isample].Contains(signal_process) ) {continue;}
        else if(bkg_type .Contains("2D") && !v_samples[isample].Contains("TTJet") && !v_samples[isample].Contains("TTbar") && !v_samples[isample].Contains("ttZ") && !v_samples[isample].Contains("ttW_") && !v_samples[isample].Contains(signal_process) ) {continue;}

		TString filepath = "./input_ntuples/tHq2017/" + nLep_cat + "/" + v_samples[isample] + ".root";
		if(!Check_File_Existence(filepath)) {cout<<BOLD(FRED("Error : ntuple "<<filepath<<".root not found ! Abort"))<<endl; return result;}

		TFile* f = TFile::Open(filepath);
		TTree* t = (TTree*) f->Get("Tree");

		if(t->GetMinimum(var) < min) {min = t->GetMinimum(var);}
		if(t->GetMaximum(var) > max) {max = t->GetMaximum(var);}

		f->Close();
	}

	result.first = min; result.second = max;

	return result;
}









//--------------------------------------------
// ########  ########  ######   ######     ###    ##       ########    ##     ##    ###    ########
// ##     ## ##       ##    ## ##    ##   ## ##   ##       ##          ##     ##   ## ##   ##     ##
// ##     ## ##       ##       ##        ##   ##  ##       ##          ##     ##  ##   ##  ##     ##
// ########  ######    ######  ##       ##     ## ##       ######      ##     ## ##     ## ########
// ##   ##   ##             ## ##       ######### ##       ##           ##   ##  ######### ##   ##
// ##    ##  ##       ##    ## ##    ## ##     ## ##       ##            ## ##   ##     ## ##    ##
// ##     ## ########  ######   ######  ##     ## ######## ########       ###    ##     ## ##     ##
//--------------------------------------------

/**
 * Get mean and RMS of var given in arg
 * Similar to Get_MinMax
 * NB : want to access mean and RMS averaged over all training samples => Need to merge histograms before accessing values
 * /!\ /!\ NOT TESTED YET ! BUGGY !!!
 */
/*
pair<double,double> Get_Mean_RMS_Variable(TString var, TString signal_process, TString bkg_type, TString nLep_cat)
{
	pair<double, double> result;
	double mean = 0, RMS = 0;

	vector<TString> v_samples;
	v_samples.push_back(signal_process);
	if(bkg_type == "ttV")
	{
		v_samples.push_back("ttW_LO");
		v_samples.push_back("ttZ_LO");
	}
	else if(bkg_type == "ttbar")
	{
		v_samples.push_back("TTbar_DiLep_PSweights");
		v_samples.push_back("TTbar_SemiLep_PSweights");
	}
	else {cout<<"Error : wrong argument ! Abort"<<endl; return result;}

	TH1F* h_tot = 0;

	for(int isample=0; isample<v_samples.size(); isample++)
	{
		TString filepath = "./input_ntuples/tHq2017/" + nLep_cat + "/" + v_samples[isample] + ".root";
		if(!Check_File_Existence(filepath)) {cout<<BOLD(FRED("Error : ntuple "<<filepath<<".root not found ! Abort"))<<endl; return result;}

		TFile* f = TFile::Open(filepath);
		TTree* t = (TTree*) f->Get("Tree");

		t->Draw(var + " >> hnew"); //Pipeline into histogram

		TH1F* htemp = (TH1F*) gPad->GetPrimitive("hnew");

		if(!h_tot) {h_tot = (TH1F*) htemp->Clone();}
		else {h_tot->Add(htemp);}

		// cout<<"htemp->GetEntries() = "<<htemp->GetEntries()<<endl;
		// cout<<"h_tot = "<<h_tot<<endl;
		// cout<<"h_tot->GetEntries() = "<<h_tot->GetEntries()<<endl;

		// cout<<"Mean = "<<htemp->GetMean()<<" / RMS = "<<htemp->GetRMS()<<endl;

		delete htemp; htemp = NULL; //can delete it here ?

		f->Close();
	}

	mean = h_tot->GetMean();
	RMS = h_tot->GetRMS();

	result.first = mean; result.second = RMS;

	return result;
}
*/


double Compute_RangeScaled_Value(double value, double min_old, double max_old, double min_new, double max_new)
{
	return ((value - min_old) / (max_old - min_old)) * (max_new - min_new) + min_new;
}

double Compute_StdDevScaled_Value(double value, double mean, double dev)
{
	return (value - mean) / dev;
}


















//--------------------------------------------
//  #######  ##     ## ######## ########  ##          ###    ########
// ##     ## ##     ## ##       ##     ## ##         ## ##   ##     ##
// ##     ## ##     ## ##       ##     ## ##        ##   ##  ##     ##
// ##     ## ##     ## ######   ########  ##       ##     ## ########
// ##     ##  ##   ##  ##       ##   ##   ##       ######### ##
// ##     ##   ## ##   ##       ##    ##  ##       ##     ## ##
//  #######     ###    ######## ##     ## ######## ##     ## ##
//--------------------------------------------


/**
 * Read overlap histogram stored in ntuples => Get overlap for all tHq/ttH categories
 * From "absolute" overlaps, also make "relative overlap" histograms
 * Warning : must have categ. vectors perfectly in sync with NTA code !

NB : could also access total yields this way... :
//Multiply by weight => Integral is yield
variable = "weight >> hnew"; cut = "weight*(is_" + v_tHq_cat[i] + "==1)";
t->Draw(variable, cut); //Pipeline into histogram
htemp = (TH1F*) gPad->GetPrimitive("hnew");
double total_yield_tHq_cat  = htemp->Integral();
delete htemp; htemp = NULL;

 */
void Plot_Overlap_Categories(TFile* f, std::ofstream& outfile, TString sample)
{
	gStyle->SetPaintTextFormat("5.2f"); //Set the number of decimals to be draw as bin contents

    TTree* t = (TTree*) f->Get("Tree");
	float nentries_total = t->GetEntries();

	//Get overlaps, and total yields for each thq and tth category individually
	TH2F* h_overlap_ttH_tHq_cat = (TH2F*) f->Get("h_overlap_ttH_tHq_cat");
    if(!h_overlap_ttH_tHq_cat) {cout<<"Error : null histogram h_overlap_ttH_tHq_cat !"<<endl; return;}
	TH1F* h_totalYield_tHq_cat = (TH1F*) f->Get("h_totalYield_tHq_cat");
    if(!h_totalYield_tHq_cat) {cout<<"Error : null histogram h_totalYield_tHq_cat !"<<endl; return;}
	TH1F* h_totalYield_ttH_cat = (TH1F*) f->Get("h_totalYield_ttH_cat");
    if(!h_totalYield_ttH_cat) {cout<<"Error : null histogram h_totalYield_ttH_cat !"<<endl; return;}

	//Will compute several relative overlaps -- start by cloning absolute overlap histo
	TH2F* h_relOverlap_wrtBOTH = (TH2F*) h_overlap_ttH_tHq_cat->Clone();
	TH2F* h_relOverlap_wrtTHQ = (TH2F*) h_overlap_ttH_tHq_cat->Clone();
	TH2F* h_relOverlap_wrtTTH = (TH2F*) h_overlap_ttH_tHq_cat->Clone();

	//-- Set tHq/ttH categories *exactly* as in NTA code
	vector<TString> v_tHq_cat(0);
    // v_tHq_cat.push_back("tHq_2lSS");
    v_tHq_cat.push_back("tHq_2lSS_SR");
    // v_tHq_cat.push_back("tHq_2lSS_Training");
    // v_tHq_cat.push_back("tHq_2lSS_Fake");
    // v_tHq_cat.push_back("tHq_2lSS_Flip");
    // v_tHq_cat.push_back("tHq_2lSS_GammaConv");
    // v_tHq_cat.push_back("tHq_ttWctrl");
    v_tHq_cat.push_back("tHq_ttWctrl_SR");
    // v_tHq_cat.push_back("tHq_ttWctrl_Fake");
    // v_tHq_cat.push_back("tHq_ttWctrl_Flip");
    // v_tHq_cat.push_back("tHq_ttWctrl_GammaConv");
    // v_tHq_cat.push_back("ttH_3l");
    v_tHq_cat.push_back("tHq_3l_SR");
    // v_tHq_cat.push_back("tHq_3l_Training");
    // v_tHq_cat.push_back("tHq_3l_Fake");
    // v_tHq_cat.push_back("tHq_ttZctrl");
    v_tHq_cat.push_back("tHq_ttZctrl_SR");
    // v_tHq_cat.push_back("tHq_ttZctrl_Fake");
    // v_tHq_cat.push_back("tHq_WZctrl");
    v_tHq_cat.push_back("tHq_WZctrl_SR");
    // v_tHq_cat.push_back("tHq_WZctrl_Fake");

    vector<TString> v_ttH_cat(0);
    // v_ttH_cat.push_back("ttH_2lSS");
    v_ttH_cat.push_back("ttH_2lSS_SR");
    // v_ttH_cat.push_back("ttH_2lSS_Training");
    // v_ttH_cat.push_back("ttH_2lSS_Fake");
    // v_ttH_cat.push_back("ttH_2lSS_Flip");
    // v_ttH_cat.push_back("ttH_2lSS_GammaConv");
    // v_ttH_cat.push_back("ttH_ttWctrl");
    v_ttH_cat.push_back("ttH_ttWctrl_SR");
    // v_ttH_cat.push_back("ttH_ttWctrl_Fake");
    // v_ttH_cat.push_back("ttH_ttWctrl_Flip");
    // v_ttH_cat.push_back("ttH_ttWctrl_GammaConv");
    // v_ttH_cat.push_back("ttH_3l");
    v_ttH_cat.push_back("ttH_3l_SR");
    // v_ttH_cat.push_back("ttH_3l_Training");
    // v_ttH_cat.push_back("ttH_3l_Fake");
    // v_ttH_cat.push_back("ttH_ttZctrl");
    v_ttH_cat.push_back("ttH_ttZctrl_SR");
    // v_ttH_cat.push_back("ttH_ttZctrl_Fake");
    // v_ttH_cat.push_back("ttH_WZctrl");
    v_ttH_cat.push_back("ttH_WZctrl_SR");
    // v_ttH_cat.push_back("ttH_WZctrl_Fake");

	TString variable = "", cut = "";
	TH1F* htemp = 0;
	for(int i=0; i<v_tHq_cat.size(); i++)
    {
		//Get total yield of events (from both analyses) entering current thq cat
		float total_yield_tHq_cat = h_totalYield_tHq_cat->GetBinContent(i+1);

		for(int j=0; j<v_ttH_cat.size(); j++)
        {
			//Get total yield of events (from both analyses) entering current ttH cat
			float total_yield_ttH_cat = h_totalYield_ttH_cat->GetBinContent(j+1);

			//Get total yield of events overlapping between both categs
			double content = h_overlap_ttH_tHq_cat->GetBinContent(i+1, j+1);

			//Compute several types of relative overlap proportions
			//-> w.r.t. total yield in tHq cat, ttH cat, and their sum
			double proportion_wrtBOTH = 0;
			if(total_yield_tHq_cat > 0 || total_yield_ttH_cat > 0) {proportion_wrtBOTH = content / (total_yield_tHq_cat+total_yield_ttH_cat-content);} //union of 2 categs
			double proportion_wrtTHQ  = 0;
			if(total_yield_tHq_cat > 0) {proportion_wrtTHQ = content / total_yield_tHq_cat;}
			double proportion_wrtTTH  = 0;
			if(total_yield_ttH_cat > 0) {proportion_wrtTTH = content / total_yield_ttH_cat;}

			if(i+1==-1 && j+1==-1) //debug
			{
				cout<<"-- Overlap between "<<v_tHq_cat[i]<<" & "<<v_ttH_cat[j]<<endl;
				cout<<"Total yield in tHq cat. => "<<total_yield_tHq_cat<<endl;
				cout<<"Total yield in ttH cat. => "<<total_yield_ttH_cat<<endl;
				cout<<"Absolute : "<<content<<endl;
				cout<<"Relative w.r.t. BOTH: "<<proportion_wrtBOTH<<endl;
				cout<<"Relative w.r.t. tHq category: "<<proportion_wrtTHQ<<endl;
				cout<<"Relative w.r.t. ttH category: "<<proportion_wrtTTH<<endl;
			}

            if(content != 0) //Save absolute overlap in txt file
            {
                outfile<<v_tHq_cat[i]<<" / "<<v_ttH_cat[j]<<" ==> "<<content<<" events"<<endl;
            }

			//Draw relative proportions of overlap instead of absolute yields
			h_relOverlap_wrtBOTH->SetBinContent(i+1, j+1, proportion_wrtBOTH);
			h_relOverlap_wrtTHQ->SetBinContent(i+1, j+1, proportion_wrtTHQ);
			h_relOverlap_wrtTTH->SetBinContent(i+1, j+1, proportion_wrtTTH);
			// h_relOverlap_wrtBOTH->Fill(i, j, proportion_wrtBOTH);
			// h_relOverlap_wrtTHQ->Fill(i, j, proportion_wrtTHQ);
			// h_relOverlap_wrtTTH->Fill(i, j, proportion_wrtTTH);

			//Remove prefixes
			TString newstring_tHq = v_tHq_cat[i]; newstring_tHq.ReplaceAll("tHq_", "");
			TString newstring_ttH = v_ttH_cat[j]; newstring_ttH.ReplaceAll("ttH_", "");
			h_overlap_ttH_tHq_cat->GetXaxis()->SetLabelSize(0.045);
			h_overlap_ttH_tHq_cat->GetYaxis()->SetLabelSize(0.045);
			h_overlap_ttH_tHq_cat->GetXaxis()->SetBinLabel(i+1, newstring_tHq);
			h_overlap_ttH_tHq_cat->GetYaxis()->SetBinLabel(j+1, newstring_ttH);
			h_overlap_ttH_tHq_cat->GetYaxis()->SetTitle("TTH CATEGORIES");
			h_overlap_ttH_tHq_cat->GetXaxis()->SetTitle("THQ CATEGORIES");
			h_overlap_ttH_tHq_cat->GetXaxis()->SetTitleOffset(3.25);
			h_overlap_ttH_tHq_cat->GetYaxis()->SetTitleOffset(2.5);

			h_relOverlap_wrtBOTH->GetXaxis()->SetLabelSize(0.045);
			h_relOverlap_wrtBOTH->GetYaxis()->SetLabelSize(0.045);
			h_relOverlap_wrtBOTH->GetXaxis()->SetBinLabel(i+1, newstring_tHq);
			h_relOverlap_wrtBOTH->GetYaxis()->SetBinLabel(j+1, newstring_ttH);
			h_relOverlap_wrtBOTH->GetYaxis()->SetTitle("TTH CATEGORIES");
			h_relOverlap_wrtBOTH->GetXaxis()->SetTitle("THQ CATEGORIES");
			h_relOverlap_wrtBOTH->GetXaxis()->SetTitleOffset(3.25);
			h_relOverlap_wrtBOTH->GetYaxis()->SetTitleOffset(2.5);

			h_relOverlap_wrtTHQ->GetXaxis()->SetLabelSize(0.045);
			h_relOverlap_wrtTHQ->GetYaxis()->SetLabelSize(0.045);
			h_relOverlap_wrtTHQ->GetXaxis()->SetBinLabel(i+1, newstring_tHq);
			h_relOverlap_wrtTHQ->GetYaxis()->SetBinLabel(j+1, newstring_ttH);
			h_relOverlap_wrtTHQ->GetYaxis()->SetTitle("TTH CATEGORIES");
			h_relOverlap_wrtTHQ->GetXaxis()->SetTitle("THQ CATEGORIES");
			h_relOverlap_wrtTHQ->GetXaxis()->SetTitleOffset(3.25);
			h_relOverlap_wrtTHQ->GetYaxis()->SetTitleOffset(2.5);

			h_relOverlap_wrtTTH->GetXaxis()->SetLabelSize(0.045);
			h_relOverlap_wrtTTH->GetYaxis()->SetLabelSize(0.045);
			h_relOverlap_wrtTTH->GetXaxis()->SetBinLabel(i+1, newstring_tHq);
			h_relOverlap_wrtTTH->GetYaxis()->SetBinLabel(j+1, newstring_ttH);
			h_relOverlap_wrtTTH->GetYaxis()->SetTitle("TTH CATEGORIES");
			h_relOverlap_wrtTTH->GetXaxis()->SetTitle("THQ CATEGORIES");
			h_relOverlap_wrtTTH->GetXaxis()->SetTitleOffset(3.25);
			h_relOverlap_wrtTTH->GetYaxis()->SetTitleOffset(2.5);
        }
    }

    TString plotdir = "./plots/overlap/";
    mkdir(plotdir.Data(), 0777);

    TCanvas* c = new TCanvas("", "", 1000, 800);
	c->SetRightMargin(0.10);
	// c->SetBottomMargin(0.15);
	c->SetBottomMargin(0.20);

	gStyle->SetOptTitle(1);
	// gStyle->SetPalette(53); //dark body radiator
	h_overlap_ttH_tHq_cat->SetTitle("ttH cat #cap tHq cat (absolute yields)");
	h_relOverlap_wrtBOTH->SetTitle("ttH cat #cap tHq cat");
	h_relOverlap_wrtTHQ->SetTitle("(ttH cat #cap tHq cat) | tHq cat");
	h_relOverlap_wrtTTH->SetTitle("(ttH cat #cap tHq cat) | ttH cat");

    h_overlap_ttH_tHq_cat->Draw("colz text");
	h_overlap_ttH_tHq_cat->SetMarkerSize(1.8);
	h_overlap_ttH_tHq_cat->GetXaxis()->SetRange(1, v_tHq_cat.size());
	h_overlap_ttH_tHq_cat->GetYaxis()->SetRange(1, v_ttH_cat.size());
	h_overlap_ttH_tHq_cat->GetXaxis()->LabelsOption("v");
	TString outname = plotdir + "AbsOverlaps_" + sample + ".png";
	c->SaveAs(outname);

	h_relOverlap_wrtBOTH->Draw("colz text");
    // h_relOverlap_wrtBOTH->SetMaximum(1.);
	h_relOverlap_wrtBOTH->SetMarkerSize(1.8);
	h_relOverlap_wrtBOTH->GetXaxis()->SetRange(1, v_tHq_cat.size());
	h_relOverlap_wrtBOTH->GetYaxis()->SetRange(1, v_ttH_cat.size());
	h_relOverlap_wrtBOTH->GetXaxis()->LabelsOption("v");
	outname = plotdir + "RelOverlapsBOTH_" + sample + ".png";
	c->SaveAs(outname);

	h_relOverlap_wrtTHQ->Draw("colz text");
    // h_relOverlap_wrtTHQ->SetMaximum(1.);
	h_relOverlap_wrtTHQ->SetMarkerSize(1.8);
	h_relOverlap_wrtTHQ->GetXaxis()->SetRange(1, v_tHq_cat.size());
	h_relOverlap_wrtTHQ->GetYaxis()->SetRange(1, v_ttH_cat.size());
	h_relOverlap_wrtTHQ->GetXaxis()->LabelsOption("v");
	outname = plotdir + "RelOverlapsTHQ_" + sample + ".png";
	c->SaveAs(outname);

	h_relOverlap_wrtTTH->Draw("colz text");
    // h_relOverlap_wrtTTH->SetMaximum(1.);
	h_relOverlap_wrtTTH->SetMarkerSize(1.8);
	h_relOverlap_wrtTTH->GetXaxis()->SetRange(1, v_tHq_cat.size());
	h_relOverlap_wrtTTH->GetYaxis()->SetRange(1, v_ttH_cat.size());
	h_relOverlap_wrtTTH->GetXaxis()->LabelsOption("v");
	outname = plotdir + "RelOverlapsTTH_" + sample + ".png";
	c->SaveAs(outname);

	delete h_overlap_ttH_tHq_cat; h_overlap_ttH_tHq_cat = NULL;
	delete h_relOverlap_wrtBOTH; h_relOverlap_wrtBOTH = NULL;
	delete h_relOverlap_wrtTHQ; h_relOverlap_wrtTHQ = NULL;
	delete h_relOverlap_wrtTTH; h_relOverlap_wrtTTH = NULL;
    delete c; c = NULL;

    return;
}


void Represent_Overlapping_Categories()
{
	// int nBLoose = 0;
	// int nBMedium = 0;
	// int nLight = 0;
	int total_jets = 0;

	int min_loose = 1, max_loose = 5;
	int min_medium = 0, max_medium = 5;
	int min_light = 0, max_light = 5;

    TH3F* h_2lSS_SR = new TH3F("", "", (max_loose-min_loose), min_loose, max_loose, (max_medium-min_medium), min_medium, max_medium, (max_light-min_light), min_light, max_light);
	TH3F* h_2lSS_SR_NotPhysical = new TH3F("", "", (max_loose-min_loose), min_loose, max_loose, (max_medium-min_medium), min_medium, max_medium, (max_light-min_light), min_light, max_light);
	TH3F* h_2lSS_SR_NoOverlap = new TH3F("", "", (max_loose-min_loose), min_loose, max_loose, (max_medium-min_medium), min_medium, max_medium, (max_light-min_light), min_light, max_light);

	TH3F* h_3l_SR = new TH3F("", "", (max_loose-min_loose), min_loose, max_loose, (max_medium-min_medium), min_medium, max_medium, (max_light-min_light), min_light, max_light);
	TH3F* h_3l_SR_NotPhysical = new TH3F("", "", (max_loose-min_loose), min_loose, max_loose, (max_medium-min_medium), min_medium, max_medium, (max_light-min_light), min_light, max_light);
	TH3F* h_3l_SR_NoOverlap = new TH3F("", "", (max_loose-min_loose), min_loose, max_loose, (max_medium-min_medium), min_medium, max_medium, (max_light-min_light), min_light, max_light);

	for(int iloose=min_loose; iloose<max_light; iloose++)
	{
		for(int imedium=min_medium; imedium<max_medium; imedium++)
		{
			// if(imedium > iloose) {continue;}//there can not be less BM than BL tags

			for(int ilight=min_light; ilight<max_light; ilight++)
			{
				total_jets = iloose	 + ilight;

				//Overlap in 2l cat
				bool belongs_to_TTH_2lSS_SR = (total_jets >= 4 && (iloose >= 2 || imedium >= 1) );
				bool belongs_to_THQ_2lSS_SR = (total_jets >= 2 && (imedium >= 1 && ilight >= 1) );
				if(belongs_to_TTH_2lSS_SR && belongs_to_THQ_2lSS_SR && imedium <= iloose)
				{
					h_2lSS_SR->SetBinContent(iloose-min_loose+1, imedium-min_medium+1, ilight-min_light+1, 1);
				}

				//Overlap in 3l cat.
				bool belongs_to_TTH_3l_SR = (total_jets >= 2 && (iloose >= 2 || imedium >= 1) );
				bool belongs_to_THQ_3l_SR = (total_jets >= 2 && (imedium >= 1 && ilight >= 1) );
				if(belongs_to_TTH_3l_SR && belongs_to_THQ_3l_SR)
				{
					h_3l_SR->SetBinContent(iloose-min_loose+1, imedium-min_medium+1, ilight-min_light+1, 1);
				}

				//Unphysical cases (more medium than loose btags)
                if(imedium > iloose)
				{
					h_2lSS_SR_NotPhysical->SetBinContent(iloose-min_loose+1, imedium-min_medium+1, ilight-min_light+1, 1);
					h_3l_SR_NotPhysical->SetBinContent(iloose-min_loose+1, imedium-min_medium+1, ilight-min_light+1, 1);
				}

				//Cat without overlap
				if(imedium <= iloose)
				{
					if(!h_2lSS_SR_NotPhysical->GetBinContent(iloose-min_loose+1, imedium-min_medium+1, ilight-min_light+1) && !h_2lSS_SR->GetBinContent(iloose-min_loose+1, imedium-min_medium+1, ilight-min_light+1) )
					{
						h_2lSS_SR_NoOverlap->SetBinContent(iloose-min_loose+1, imedium-min_medium+1, ilight-min_light+1, 1);
					}
					if(!h_3l_SR_NotPhysical->GetBinContent(iloose-min_loose+1, imedium-min_medium+1, ilight-min_light+1) && !h_3l_SR->GetBinContent(iloose-min_loose+1, imedium-min_medium+1, ilight-min_light+1) )
					{
						h_3l_SR_NoOverlap->SetBinContent(iloose-min_loose+1, imedium-min_medium+1, ilight-min_light+1, 1);
					}
				}

				// cout<<iloose<<" Loose b-tags / "<<imedium<<" Medium b-tags / "<<ilight<<" Light jets"<<endl;
				// cout<<"belongs_to_TTH_2lSS_SR "<<belongs_to_TTH_2lSS_SR<<endl;
				// cout<<"belongs_to_THQ_2lSS_SR "<<belongs_to_THQ_2lSS_SR<<endl;
				// cout<<"belongs_to_TTH_3l_SR "<<belongs_to_TTH_3l_SR<<endl;
				// cout<<"belongs_to_THQ_3l_SR "<<belongs_to_THQ_3l_SR<<endl;

			} //loop on light jets
		} //loop on medium btags
	} //loop on loose btags

	TString outputname = "";
	TCanvas* c = new TCanvas("", "", 1000, 800);
	// c->SetRightMargin(0.10);
	// c->SetBottomMargin(0.15);

	gStyle->SetOptTitle(1);

	//Rotate view
	// gPad->SetTheta(90); // default is 30 //up-down
	gPad->SetPhi(220); // default is 30 //rotate counter-clockwise
	gPad->Update();

	//Represent non-overlapping cats instead of overlapping ones
	{
		delete h_2lSS_SR;
		h_2lSS_SR = h_2lSS_SR_NoOverlap;
	}

    h_2lSS_SR->Draw("BOX1");
	h_2lSS_SR->SetTitle("Jet cat. without overlap (2lSS SR)");
	// h_2lSS_SR->SetFillColorAlpha(kRed, 0.5);
	h_2lSS_SR->SetFillColorAlpha(kGreen, 0.9);
	h_2lSS_SR->GetXaxis()->SetTitle("Loose b-jets");
	h_2lSS_SR->GetXaxis()->CenterLabels();
	h_2lSS_SR->GetXaxis()->SetNdivisions(105);
	h_2lSS_SR->GetYaxis()->SetTitle("Medium b-jets");
	h_2lSS_SR->GetYaxis()->	CenterLabels();
	h_2lSS_SR->GetYaxis()->SetNdivisions(105);
	h_2lSS_SR->GetZaxis()->SetTitle("Light jets");
	h_2lSS_SR->GetZaxis()->CenterLabels();
	h_2lSS_SR->GetZaxis()->SetNdivisions(105);

	// h_2lSS_SR_NoOverlap->Draw("BOX1 SAME");
	// h_2lSS_SR_NoOverlap->SetFillColorAlpha(kGreen+2, 0.9);
	h_2lSS_SR_NoOverlap->SetFillStyle(3012);

	// h_2lSS_SR_NotPhysical->Draw("BOX1 same");
	// h_2lSS_SR_NotPhysical->SetFillColor(kGray);
	// h_2lSS_SR_NotPhysical->SetFillColorAlpha(kYellow-10, 0.10);

	outputname = "./plots/overlap/Repres_2lSS_SR_Overlaps.png";
	c->SaveAs(outputname);


	//Represent non-overlapping cats instead of overlapping ones
	{
		delete h_3l_SR;
		h_3l_SR = h_3l_SR_NoOverlap;
	}

	h_3l_SR->Draw("BOX1");
	h_3l_SR->SetTitle("Jet cat. without overlap (3l SR)");
	h_3l_SR->SetFillColorAlpha(kGreen, 0.9);
	h_3l_SR->GetXaxis()->SetTitle("Loose b-jets");
	h_3l_SR->GetXaxis()->CenterLabels();
	h_3l_SR->GetXaxis()->SetNdivisions(105);
	h_3l_SR->GetYaxis()->SetTitle("Medium b-jets");
	h_3l_SR->GetYaxis()->CenterLabels();
	h_3l_SR->GetYaxis()->SetNdivisions(105);
	h_3l_SR->GetZaxis()->SetTitle("Light jets");
	h_3l_SR->GetZaxis()->CenterLabels();
	h_3l_SR->GetZaxis()->SetNdivisions(105);

	// h_3l_SR_NoOverlap->Draw("BOX1 same");
	// h_3l_SR_NoOverlap->SetFillColor(kGreen+2);

	outputname = "./plots/overlap/Repres_3l_SR_Overlaps.png";
	c->SaveAs(outputname);

	delete c; c = NULL;
	delete h_2lSS_SR; h_2lSS_SR = NULL;
	delete h_3l_SR; h_3l_SR = NULL;

	return;
}


void Compare_Before_After_Orthogonalization()
{
	TString dir_ntuples_before = "./input_ntuples/ttH2017/allRegionsEvents/";
	TString dir_ntuples_after = "./input_ntuples/overlap/ttH2017_fwd/allRegionsEvents/";

	TString hname_tHqAnalysis = "h_totalYield_tHq_cat";
	TString hname_ttHAnalysis = "h_totalYield_ttH_cat";

	vector<TString> v_process;
	v_process.push_back("tHq");
	v_process.push_back("ttH");
	v_process.push_back("tHW");

	vector<TString> v_cat;
    v_cat.push_back("2lSS_SR");
    v_cat.push_back("ttWctrl_SR");
    v_cat.push_back("3l_SR");
    v_cat.push_back("ttZctrl_SR");
    v_cat.push_back("WZctrl_SR");

	cout<<"Comparison of yield after/before orthogonalization of analyses"<<endl<<endl<<endl;

	for(int iproc=0; iproc<v_process.size(); iproc++)
	{
		cout<<endl<<"* Process : "<<v_process[iproc]<<endl;

		TString filename = dir_ntuples_before + v_process[iproc] + ".root";
		if(!Check_File_Existence(filename)) {cout<<"File not found !"<<endl; return;}
		TFile* f_before = TFile::Open(filename);

		filename = dir_ntuples_after + v_process[iproc] + ".root";
		if(!Check_File_Existence(filename)) {cout<<"File not found !"<<endl; return;}
		TFile* f_after = TFile::Open(filename);

		float yield_before = 0, yield_after = 0;
		float ratio_tHq_analysis = 0, ratio_ttH_analysis = 0;
		TH1F *h_before = 0, *h_after = 0;
		for(int icat=0; icat<v_cat.size(); icat++)
		{
			h_before = (TH1F*) f_before->Get(hname_tHqAnalysis);
			h_after = (TH1F*) f_after->Get(hname_tHqAnalysis);
			yield_before = h_before->GetBinContent(icat+1);
			yield_after = h_after->GetBinContent(icat+1);
			// cout<<"before = "<<yield_before<<endl;
			// cout<<"after = "<<yield_after<<endl;
			ratio_tHq_analysis = yield_after / yield_before;

			h_before = (TH1F*) f_before->Get(hname_ttHAnalysis);
			h_after = (TH1F*) f_after->Get(hname_ttHAnalysis);
			yield_before = h_before->GetBinContent(icat+1);
			yield_after = h_after->GetBinContent(icat+1);
			ratio_ttH_analysis = yield_after / yield_before;

			cout<<"# Category "<<v_cat[icat]<<" --> "<<ratio_tHq_analysis<<"(tHq analysis)";
			cout<<" / "<<ratio_ttH_analysis<<"(ttH analysis)"<<endl;
		}

		f_before->Close();
		f_after->Close();
	}

	return;
}


void Get_Yield_Per_Jet_Categ(TString analysis_type)
{
	//Choose variable to plot : 'nLooseBJets' / 'nMediumBJets' / 'nLightJets' (light+fwd) / 'nLightJets_Fwd' (only fwd) / 'nJets' (all jets)
	//=============================================
	TString nameVar_Xaxis = "nJets";
	TString nameVar_Yaxis = "nLooseBJets";
	//=============================================

	gStyle->SetPaintTextFormat("5.2f"); //Set the number of decimals to be draw as bin contents

	if(analysis_type == "ttH") //Want to force the use of the "tHq jet collections"
	{
		if(nameVar_Xaxis=="nLightJets") {nameVar_Xaxis = "nLightJets_tHq";} //special name for complete jet collec
		if(nameVar_Yaxis=="nLightJets") {nameVar_Yaxis = "nLightJets_tHq";} //special name for complete jet collec

		if(nameVar_Xaxis=="nJets") {nameVar_Xaxis = "nJets_tHq";} //special name for complete jet collec
		if(nameVar_Yaxis=="nJets") {nameVar_Yaxis = "nJets_tHq";} //special name for complete jet collec
	}

	vector<TString> v_process;
	v_process.push_back("tHq");
	v_process.push_back("ttH");
	v_process.push_back("ttW");
	// v_process.push_back("tHW");

	vector<TString> v_cat;
    v_cat.push_back("2lSS_SR");
    // v_cat.push_back("ttWctrl_SR");
    v_cat.push_back("3l_SR");
    // v_cat.push_back("ttZctrl_SR");
    // v_cat.push_back("WZctrl_SR");

	TString dir_ntuples = "./input_ntuples/overlap/";
	TString variable = "", cut = "";
	TString plotdir = "./plots/overlap/";
    mkdir(plotdir.Data(), 0777);

	for(int iproc=0; iproc<v_process.size(); iproc++)
	{
		cout<<endl<<"* Process : "<<v_process[iproc]<<endl;

		TString filename = dir_ntuples + v_process[iproc] + ".root";
		if(!Check_File_Existence(filename)) {cout<<"File not found !"<<endl; return;}
		TFile* f = TFile::Open(filename);
		TTree* t = (TTree*) f->Get("Tree");

		int min_x = 0, max_x = 0;
		int min_y = 0, max_y = 0;

		if(nameVar_Xaxis=="nLooseBJets") {min_x = 1; max_x = 5;}
		else if(nameVar_Xaxis=="nMediumBJets") {min_x = 0; max_x = 4;}
		else if(nameVar_Xaxis=="nLightJets" || nameVar_Xaxis=="nLightJets_tHq") {min_x = 0; max_x = 5;}
		else if(nameVar_Xaxis=="nLightJets_Fwd") {min_x = 0; max_x = 4;}
		else if(nameVar_Xaxis=="nJets" || nameVar_Xaxis=="nJets_tHq") {min_x = 2; max_x = 9;}
		else {cout<<"WRONG NAME !"<<endl; return;}

		if(nameVar_Yaxis=="nLooseBJets") {min_y = 1; max_y = 5;}
		else if(nameVar_Yaxis=="nMediumBJets") {min_y = 0; max_y = 4;}
		else if(nameVar_Yaxis=="nLightJets" || nameVar_Yaxis=="nLightJets_tHq") {min_y = 0; max_y = 5;}
		else if(nameVar_Yaxis=="nLightJets_Fwd") {min_y = 0; max_y = 4;}
		else if(nameVar_Yaxis=="nJets" || nameVar_Yaxis=="nJets_tHq") {min_y = 2; max_y = 9;}
		else {cout<<"WRONG NAME !"<<endl; return;}

		// cout<<"min_x = "<<min_x<<" / max_x = "<<max_x<<endl;
		// cout<<"min_y = "<<min_y<<" / max_y = "<<max_y<<endl;

		TCanvas* c = new TCanvas("", "", 1000, 800);
		c->SetRightMargin(0.15);
		gStyle->SetOptTitle(1);

		for(int icat=0; icat<v_cat.size(); icat++)
		{
			TH2F* h = new TH2F("", "", (max_x-min_x), min_x, max_x, (max_y-min_y), min_y, max_y);

			double sum_yields = 0; //make sure sum of numbers is 1

			variable = "weight >> htmp"; cut = "weight*(is_tHq_" + v_cat[icat] + "==1 || is_ttH_"+v_cat[icat]+" ==1)"; //relative yield, from any of 2 cats
			t->Draw(variable, cut); //Pipeline into histogram
			TH1F* h_total_cat = (TH1F*) gPad->GetPrimitive("htmp");
			double total_yield_cat  = h_total_cat->Integral();
			delete h_total_cat; h_total_cat = NULL;

			for(int ix=min_x; ix<max_x; ix++)
			{
				for(int iy=min_y; iy<max_y; iy++)
				{
					// cout<<endl<<"* ix "<<ix<<" / iy "<<iy<<endl;

					//Some jets configs are not physical (e.g. more btags than total jets)
					if(nameVar_Xaxis == "nLooseBJets" && nameVar_Yaxis=="nMediumBJets" && iy > ix) {continue;}
					else if(nameVar_Xaxis == "nMediumBJets" && nameVar_Yaxis=="nLooseBJets" && ix > iy) {continue;}

					else if((nameVar_Xaxis == "nLightJets" || nameVar_Xaxis == "nLightJets_tHq") && nameVar_Yaxis=="nLightJets_Fwd" && iy > ix) {continue;}
					else if(nameVar_Xaxis=="nLightJets_Fwd" && (nameVar_Yaxis == "nLightJets" || nameVar_Yaxis == "nLightJets_tHq") && ix > iy) {continue;}

					else if((nameVar_Xaxis=="nJets" || nameVar_Xaxis=="nJets_tHq") && ix < iy) {continue;}
					else if((nameVar_Yaxis=="nJets" || nameVar_Yaxis=="nJets_tHq") && ix > iy) {continue;}

					TString additional_cut = ""; //can specify here add. cut, to study only subset of event //else leave blank
					variable = "weight >> htmp"; cut = "weight*((is_tHq_" + v_cat[icat] + "==1 || is_ttH_"+v_cat[icat]+" ==1) && "+nameVar_Xaxis+"=="+Convert_Number_To_TString(ix)+" && "+nameVar_Yaxis+"=="+Convert_Number_To_TString(iy)+additional_cut+")"; //relative yield, from any of 2 cats

					t->Draw(variable, cut); //Pipeline into histogram
					TH1F* h_tmp = (TH1F*) gPad->GetPrimitive("htmp");
					double tmp_yield  = h_tmp->Integral();
					delete h_tmp; h_tmp = NULL;

					sum_yields+= tmp_yield;
					double proportion = tmp_yield / total_yield_cat;

					// cout<<"variable = "<<variable<<endl;
					// cout<<"cut = "<<cut<<endl;
					// cout<<"-> tmp_yield = "<<tmp_yield<<endl;
					// cout<<"-> total_yield_cat = "<<total_yield_cat<<endl;
					// cout<<"-> prop = "<<proportion<<endl;

					h->Fill(ix, iy, proportion);

					// -- show overlap categs in grey
					if(false)
					{
						gStyle->SetPalette(kGreyScale);
						if(proportion==0) {h->Fill(ix, iy, 1.);} //white
						else {h->Fill(ix, iy, 0.5);} //grey
					}

					// -- show overlap categs in grey
					if(false)
					{
						gStyle->SetPalette(kRainBow);
						h->SetMaximum(1.);
						if(ix==1 && iy==1) {h->Fill(ix, iy, 0.2);} //blue
						else if(ix==1 && iy==0) {h->Fill(ix, iy, 0.);} //white, hardcoded
						else {h->Fill(ix, iy, 0.8);} //red
					}
				}
			}


			h->SetTitle("Event fractions per jet cat. in " + v_cat[icat] + " region");
            h->Draw("colz text");
			// h->Draw("col");
			h->SetMarkerSize(1.8);
			h->GetXaxis()->SetTitle("Loose btags");
			h->GetXaxis()->CenterLabels();
			h->GetXaxis()->SetNdivisions(105);
			h->GetYaxis()->SetTitle("Medium btags");
			h->GetYaxis()->CenterLabels();
			h->GetYaxis()->SetNdivisions(105);

			if(nameVar_Xaxis == "nLooseBJets") {h->GetXaxis()->SetTitle("Loose btags");}
			else if(nameVar_Xaxis == "nMediumBJets") {h->GetXaxis()->SetTitle("Medium btags");}
			else if(nameVar_Xaxis == "nLightJets" || nameVar_Xaxis == "nLightJets_tHq") {h->GetXaxis()->SetTitle("Light jets");}
            else if(nameVar_Xaxis == "nLightJets_Fwd") {h->GetXaxis()->SetTitle("Forward jets");}
            else if(nameVar_Xaxis == "nJets" || nameVar_Xaxis == "nJets_tHq") {h->GetXaxis()->SetTitle("Jets (+fwd)"); h->GetXaxis()->SetNdivisions(107);}
			// h->GetXaxis()->SetNdivisions(107);

			if(nameVar_Yaxis == "nLooseBJets") {h->GetYaxis()->SetTitle("Loose btags");}
			else if(nameVar_Yaxis == "nMediumBJets") {h->GetYaxis()->SetTitle("Medium btags");}
			else if(nameVar_Yaxis == "nLightJets" || nameVar_Yaxis == "nLightJets_tHq") {h->GetYaxis()->SetTitle("Light jets");}
			else if(nameVar_Yaxis == "nLightJets_Fwd") {h->GetYaxis()->SetTitle("Forward jets");}
            else if(nameVar_Yaxis == "nJets" || nameVar_Yaxis == "nJets_tHq") {h->GetYaxis()->SetTitle("Jets (+fwd)"); h->GetYaxis()->SetNdivisions(107);}
			// h->GetYaxis()->SetNdivisions(107);

			TString outname = plotdir + "Yield_Btag_"+v_cat[icat]+"_" + v_process[iproc] + ".png";
			c->SaveAs(outname);

			cout<<"==> Sum of yields for cat. "<<v_cat[icat]<<" is "<<sum_yields/total_yield_cat<<endl<<endl;

			delete h; h = NULL;
		}

		delete c; c = NULL;
	}

	return;
}

void Check_ChangeYields_Orthogonalization_FwdJet()
{
	//--------------------------------------------
	// TString suffix_newCat = "_btag";
	// TString suffix_newCat = "_fwd";
	// TString suffix_newCat = "_fwd2";
	TString suffix_newCat = "_njet";
	// TString suffix_newCat = "_njet2";
	//--------------------------------------------

	vector<TString> v_process;
	v_process.push_back("tHq");
    v_process.push_back("ttH");
    v_process.push_back("ttW");
	// v_process.push_back("tHW");

	vector<TString> v_cat;
    v_cat.push_back("2lSS_SR");
    v_cat.push_back("3l_SR");

	TString dir_ntuples = "./input_ntuples/overlap/";

	for(int iproc=0; iproc<v_process.size(); iproc++)
	{
		cout<<endl<<endl<<"* Process : "<<v_process[iproc]<<" (difference after orthogonalization)"<<endl<<endl;

		TString filename = dir_ntuples + v_process[iproc] + ".root";
		if(!Check_File_Existence(filename)) {cout<<"File not found !"<<endl; return;}
		TFile* f = TFile::Open(filename);
		TTree* t = (TTree*) f->Get("Tree");

		Float_t weight = 0;
		Char_t is_tHqCat_before = 0;
		Char_t is_ttHCat_before = 0;
		Char_t is_tHqCat_after = 0;
		Char_t is_ttHCat_after = 0;

		t->SetBranchAddress("weight", &weight);
		int nentries = t->GetEntries();

		for(int icat=0; icat<v_cat.size(); icat++)
		{
			double yield_tHqCat_before = 0, yield_tHqCat_after = 0;
			double yield_ttHCat_before = 0, yield_ttHCat_after = 0;

			TString suffix_tmp = suffix_newCat;
			if(suffix_newCat == "_njet2" && !v_cat[icat].Contains("2l") ) {suffix_tmp = "_njet";}

			t->SetBranchAddress("is_tHq_" + v_cat[icat], &is_tHqCat_before);
			t->SetBranchAddress("is_tHq_" + v_cat[icat] + suffix_tmp, &is_tHqCat_after);
			t->SetBranchAddress("is_ttH_" + v_cat[icat], &is_ttHCat_before);
			t->SetBranchAddress("is_ttH_" + v_cat[icat] + suffix_tmp, &is_ttHCat_after);

			// t->SetBranchAddress("is_tHq_" + v_cat[icat], &is_tHqCat_before);
			// t->SetBranchAddress("is_tHq_" + v_cat[icat] + suffix_tmp, &is_tHqCat_after);
			// t->SetBranchAddress("is_ttH_ttWctrl_SR", &is_ttHCat_before);
			// t->SetBranchAddress("is_ttH_ttWctrl_SR" + suffix_tmp, &is_ttHCat_after);


			double proportion_remaining_tHq = 0, proportion_remaining_ttH = 0;
			double proportion_migration_to_tHq_wasOverlap = 0, proportion_migration_to_ttH_wasOverlap = 0;
			double proportion_migration_to_tHq_wasNotOverlap = 0, proportion_migration_to_ttH_wasNotOverlap = 0;
			double proportion_loss_tHq = 0, proportion_loss_ttH = 0;
			double overlap_total = 0;
			for(int ientry=0; ientry<nentries; ientry++)
			{
				t->GetEntry(ientry);

				if(is_tHqCat_before==1)
				{
					yield_tHqCat_before+= weight;
					if(!is_tHqCat_after)
					{
						if(is_ttHCat_after==1)
						{
							if(is_ttHCat_before) {proportion_migration_to_ttH_wasOverlap+= weight;}
							else {proportion_migration_to_ttH_wasNotOverlap+= weight;}
						}
						else {proportion_loss_tHq+= weight;}
					}
				}

				if(is_ttHCat_before==1)
				{
					yield_ttHCat_before+= weight;
					if(!is_ttHCat_after)
					{
						if(is_tHqCat_after==1)
						{
							if(is_tHqCat_before) {proportion_migration_to_tHq_wasOverlap+= weight;}
							else {proportion_migration_to_tHq_wasNotOverlap+= weight;}
						}
						else {proportion_loss_ttH+= weight;}
					}
				}

				if(is_tHqCat_after==1) {yield_tHqCat_after+= weight;}
				if(is_ttHCat_after==1) {yield_ttHCat_after+= weight;}

				if(is_ttHCat_before && is_tHqCat_before) {overlap_total+= weight;}
			}

            //-- If want absolute yields
			proportion_remaining_tHq = yield_tHqCat_after - yield_tHqCat_before;
			proportion_remaining_ttH = yield_ttHCat_after - yield_ttHCat_before;

			cout<<endl<<setprecision(3)<<FYEL("-- Cat. "<<v_cat[icat]<<"_tHq :")<<endl;
			cout<<"-> Before : "<<yield_tHqCat_before<<" / After : "<<yield_tHqCat_after<<endl;
			cout<<"-> Total overlap was : "<<overlap_total<<endl;
			cout<<"of which : "<<proportion_migration_to_tHq_wasOverlap<<" events went to new tHq cat. / "<<proportion_migration_to_ttH_wasOverlap<<" events went to new ttH cat."<<endl;
			cout<<"-> "<<proportion_migration_to_tHq_wasNotOverlap<<" events *not* in overlap migrated from ttH to tHq cat."<<endl;
			cout<<"-> "<<proportion_loss_tHq<<" events migrated to ttW CR (from tHq 2lSS SR) or were lost..."<<endl;

			cout<<endl<<setprecision(3)<<FYEL("-- Cat. "<<v_cat[icat]<<"_ttH : ");
			cout<<"-> Before : "<<yield_ttHCat_before<<" / After : "<<yield_ttHCat_after<<endl;
			cout<<"-> Total overlap was : "<<overlap_total<<endl;
			cout<<"of which : "<<proportion_migration_to_ttH_wasOverlap<<" events went to new ttH cat. / "<<proportion_migration_to_tHq_wasOverlap<<" events went to new ttH cat."<<endl;
			cout<<"-> "<<proportion_migration_to_ttH_wasNotOverlap<<" events *not* in overlap migrated from tHq to ttH cat."<<endl;
			cout<<"-> "<<proportion_loss_ttH<<" events were lost..."<<endl;

            //-- If want relative proportions
			// proportion_remaining_tHq/= yield_tHqCat_before / 100.;
			// proportion_remaining_ttH/= yield_ttHCat_before / 100.;
			// proportion_migration_tHq/= yield_tHqCat_before / 100.;
			// proportion_migration_ttH/= yield_ttHCat_before / 100.;
			// proportion_loss_tHq/= yield_tHqCat_before / 100.;
			// proportion_loss_ttH/= yield_ttHCat_before / 100.;
			// cout<<setprecision(3)<<FYEL("-- Cat. "<<v_cat[icat]<<"_tHq ==> "<<proportion_remaining_tHq<<"% ("<<proportion_migration_tHq<<"% migrated & "<<proportion_loss_tHq<<"% was lost)")<<endl;
			// cout<<setprecision(3)<<FYEL("-- Cat. "<<v_cat[icat]<<"_ttH ==> "<<proportion_remaining_ttH<<"% ("<<proportion_migration_ttH<<"% migrated & "<<proportion_loss_ttH<<"% was lost)")<<endl;
		}

		f->Close();
	}

	return;
}


void Make_1DPlot_Yield_EachCat_Overlap()
{
	bool superimpose_yields_after_ortho = true; //true <-> sumperimpose yields before/after ortho

	// TString suffix_newCat = "_btag";
	// TString suffix_newCat = "_fwd"; //doesn't ortho thq 2l SR and ttW CR
	// TString suffix_newCat = "_fwd2";
	// TString suffix_newCat = "_njet"; //doesn't ortho thq 2l SR and ttW CR
	// TString suffix_newCat = "_njet2";
	// TString suffix_newCat = "_njet3";
	TString suffix_newCat = "_njet4";

	vector<TString> v_process; //NB -- if want to add another another process, must create dedicated histo
	v_process.push_back("tHq");
	v_process.push_back("ttH");
	// v_process.push_back("tHW");

	vector<TString> v_cat; //Keep all cats and skip in loop
	v_cat.push_back("2lSS_SR");
    v_cat.push_back("ttWctrl_SR");
    v_cat.push_back("3l_SR");
    v_cat.push_back("ttZctrl_SR");
    v_cat.push_back("WZctrl_SR");

	// TString dir_ntuples = "./input_ntuples/overlap/";
	TString dir_ntuples = "./input_ntuples/tHq2017/allRegionsEvents/";

	TString hname_tHqAnalysis = "h_totalYield_tHq_cat";
	TString hname_ttHAnalysis = "h_totalYield_ttH_cat";
	TString hname_overlap = "h_overlap_ttH_tHq_cat";

	TH1F* h = 0;
	TH2F* h_overlap = 0;

	for(int icat=0; icat<v_cat.size(); icat++)
	{
		if(!v_cat[icat].Contains("2l") && !v_cat[icat].Contains("3l")) {continue;} //only SRs

		TCanvas* c = new TCanvas("", "", 1000, 800);
		// c->SetBottomMargin(0.15);
		TH1F* h_output_thq = new TH1F("", "", 3, 0, 3);
		TH1F* h_output_tth = new TH1F("", "", 3, 0, 3);
		TH1F* h_output_thq_after = new TH1F("", "", 3, 0, 3);
		TH1F* h_output_tth_after = new TH1F("", "", 3, 0, 3);
		TLegend* qw = 0;
		if(superimpose_yields_after_ortho) {qw = new TLegend(0.45,.70,0.68,0.90);}
		else {qw = new TLegend(0.49,.80,0.64,0.90);}

		double ymax = 0;

		for(int iproc=0; iproc<v_process.size(); iproc++)
		{
			cout<<endl<<endl<<"* Process : "<<v_process[iproc]<<endl<<endl;

			TString suffix_tmp = suffix_newCat;
			if((suffix_newCat == "_njet2" || suffix_newCat == "_njet3") && v_cat[icat].Contains("2l") ) {suffix_tmp = "_njet";}
			if(suffix_newCat == "_fwd2") {suffix_tmp = "_fwd";} //correct manually below

			double yield_after_thq = 0, yield_after_tth = 0;

			TString filename = dir_ntuples + v_process[iproc] + ".root";
			if(!Check_File_Existence(filename)) {cout<<"File "<<filename<<" not found !"<<endl; return;}
			TFile* f = TFile::Open(filename);

			if(superimpose_yields_after_ortho)
			{
				TTree* t = (TTree*) f->Get("Tree");
				Float_t weight = 0;
				Char_t is_tHqCat_after = 0;
				Char_t is_ttHCat_after = 0;

				TString catname = "is_tHq_"+v_cat[icat]+suffix_tmp;
				if(suffix_newCat == "_fwd2" && catname=="is_tHq_2lSS_SR_fwd") {catname="is_tHq_2lSS_SR_fwd2";}
				TString variable = "weight >> htmp"; TString cut = "weight*("+catname+"==1)"; //relative yield, from any of 2 cats
				t->Draw(variable, cut); //Pipeline into histogram
				TH1F* h_after = (TH1F*) gPad->GetPrimitive("htmp");
				yield_after_thq  = h_after->Integral();
				delete h_after; h_after = NULL;

				catname = "is_ttH_"+v_cat[icat]+suffix_tmp;
				if(suffix_newCat == "_fwd2" && catname=="is_ttH_ttWctrl_SR_fwd") {catname="is_ttH_ttWctrl_SR_fwd2";}
				variable = "weight >> htmp"; cut = "weight*("+catname+"==1)"; //relative yield, from any of 2 cats
				t->Draw(variable, cut); //Pipeline into histogram
				h_after = (TH1F*) gPad->GetPrimitive("htmp");
				yield_after_tth  = h_after->Integral();
				delete h_after; h_after = NULL;
			}

			h = (TH1F*) f->Get(hname_tHqAnalysis);
			float total_yield_tHq_cat = h->GetBinContent(icat+1);
			delete h; h = NULL;

			h = (TH1F*) f->Get(hname_ttHAnalysis);
			float total_yield_ttH_cat = h->GetBinContent(icat+1);
			delete h; h = NULL;

			h_overlap = (TH2F*) f->Get(hname_overlap);
			float total_yield_overlap = h_overlap->GetBinContent(icat+1, icat+1);
			delete h; h = NULL;

			if(total_yield_tHq_cat > ymax) {ymax = total_yield_tHq_cat;}
			if(total_yield_ttH_cat > ymax) {ymax = total_yield_ttH_cat;}

			// cout<<"total_yield_ttH_cat = "<<total_yield_ttH_cat<<endl;
			// cout<<"total_yield_overlap = "<<total_yield_overlap<<endl;
			// cout<<"total_yield_tHq_cat = "<<total_yield_tHq_cat<<endl;
			// cout<<"yield_after_thq "<<yield_after_thq<<endl;
			// cout<<"yield_after_tth "<<yield_after_tth<<endl;

			//Use "h_output" for everything not specific to thq or tth
			TH1F *h_output = 0, *h_output_after = 0;
			if(v_process[iproc] == "tHq") {h_output = h_output_thq; h_output_after = h_output_thq_after;}
			else if(v_process[iproc] == "ttH") {h_output = h_output_tth; h_output_after = h_output_tth_after;}
			else {cout<<"error"<<endl; return;}

			h_output->SetBinContent(1, total_yield_ttH_cat);
			h_output->SetBinContent(2, total_yield_overlap);
			h_output->SetBinContent(3, total_yield_tHq_cat);
			h_output->SetMinimum(0.);

			if(superimpose_yields_after_ortho)
			{
				h_output_after->SetBinContent(1, yield_after_tth);
				h_output_after->SetBinContent(3, yield_after_thq);
				h_output_after->SetLineWidth(3.);
				h_output_after->SetLineStyle(1);
				h_output->SetLineStyle(2);
			}

			h_output->SetLineWidth(3.);
			gStyle->SetOptTitle(1);
			TString hist_title = "Overlap in the tH & ttH cat. (";
			if(v_cat[icat] == "2lSS_SR") {hist_title+= "2lss SR)";}
			else {hist_title+= "3l SR)";}
			h_output->SetTitle(hist_title);
			// h_output->GetXaxis()->SetTitle("Category");
			h_output->GetYaxis()->SetTitle("Yield");
			h_output->GetXaxis()->SetLabelSize(0.06);
			h_output->GetXaxis()->SetBinLabel(1, "ttH cat. (1)");
			h_output->GetXaxis()->SetBinLabel(2, "Overlap (1 & 2)");
			h_output->GetXaxis()->SetBinLabel(3, "tHq cat. (2)");
			// h_output->GetXaxis()->SetTitleOffset(3.25);


			if(iproc==0) //THQ
			{
				h_output->SetLineColor(kBlue);
				// h_output->Draw("");

				if(superimpose_yields_after_ortho)
				{
					qw->AddEntry(h_output, "tHq (before)", "L");
					qw->AddEntry(h_output_after, "tHq (after)", "L");
					h_output_after->SetLineColor(kBlue);
				}
				else {qw->AddEntry(h_output, "tHq(#kappa_{t}=-1)", "L");}

			}
			else //TTH
			{
				h_output->SetLineColor(kRed);
				// h_output->Draw("same");

				if(superimpose_yields_after_ortho)
				{
					qw->AddEntry(h_output, "ttH (before)", "L");
					qw->AddEntry(h_output_after, "ttH (after)", "L");
					h_output_after->SetLineColor(kRed);
				}
				else {qw->AddEntry(h_output, "ttH", "L");}

			}

			f->Close();
		}

		h_output_thq->SetMaximum(ymax*1.3);
		h_output_tth->SetMaximum(ymax*1.3);

		h_output_thq->Draw("");
		h_output_tth->Draw("same");

		if(superimpose_yields_after_ortho)
		{
			h_output_thq_after->Draw("same");
			h_output_tth_after->Draw("same");
		}

		qw->Draw("same");

		TLine l;
		// l.SetLineStyle(2);
		l.DrawLine(1,0,1,ymax*1.3);
		l.DrawLine(2,0,2,ymax*1.3);
		// l.Draw("same");

		mkdir("./plots/overlap/", 0777);
		TString outputname = "./plots/overlap/1Dplot_yields_overlap_"+v_cat[icat]+".png";
		c->SaveAs(outputname);

		delete c; c = NULL;
		delete h_output_thq; h_output_thq = NULL;
		delete h_output_tth; h_output_tth = NULL;
		delete h_output_thq_after; h_output_thq_after = NULL;
		delete h_output_tth_after; h_output_tth_after = NULL;
	}

	return;
}




//--------------------------------------------
// ########  #### ##       ######## ##     ## ########
// ##     ##  ##  ##       ##       ##     ## ##     ##
// ##     ##  ##  ##       ##       ##     ## ##     ##
// ########   ##  ##       ######   ##     ## ########
// ##         ##  ##       ##       ##     ## ##
// ##         ##  ##       ##       ##     ## ##
// ##        #### ######## ########  #######  ##
//--------------------------------------------


void Make_Pileup_Plots(vector<TString> v_listSamples, TString analysis_type, TString PU_or_PV)
{
    TString var = "";
    if(PU_or_PV == "PU") {var = "PU";}
    else if(PU_or_PV == "PV") {var = "nPV";}
    else {cout<<FRED("Error ! Must choose 'PU' or 'PV' as arg !")<<endl; return;}

	mkdir("./plots/tmp/", 0777);
	mkdir("./plots/tmp/nPV", 0777);
	mkdir("./plots/tmp/PU", 0777);

    TString filepath = "./input_ntuples/";
    if(analysis_type == "ttH") {filepath+= "ttH2017/";}
    else if(analysis_type == "tHq") {filepath+= "tHq2017/";}
    else {return;}
    filepath+= "allRegionsEvents/";

	vector<double> v_eyl, v_eyh, v_exl, v_exh, v_x, v_y; //Contain the systematic errors (used to create the TGraphError)

	//DATA histogram
    TFile* f_data = 0;
    TH1F* h_data = 0, *h_data_up = 0, *h_data_down = 0;
	TString filename_data = "";
    if(var == "PV") {filename_data = filepath + "DATA.root";}
    else {filename_data = "./data/PU/PileupData_ReRecoJSON_Full2017.root";}
	if(!Check_File_Existence(filename_data)) {cout<<BOLD(FRED("File "<<filename_data<<" not found !"))<<endl; return;}
	f_data = TFile::Open(filename_data);
	if(var == "PV") {h_data = (TH1F*) f_data->Get("h_"+var+"_noCorr");}
    else
	{
		h_data = (TH1F*) f_data->Get("pileup");
		h_data_up = (TH1F*) f_data->Get("pileup_plus");
		h_data_down = (TH1F*) f_data->Get("pileup_minus");
	}
	h_data->SetLineColor(kBlack);
	h_data->SetLineWidth(2);
	h_data->SetLineStyle(2);
	h_data->Scale(1./h_data->Integral() );

	if(var=="PU")
	{
        h_data_up->SetLineColor(kBlack);
        h_data_down->SetLineColor(kBlack);
        h_data_up->SetLineStyle(2);
		h_data_down->SetLineStyle(2);
		h_data_up->Scale(1./h_data_up->Integral() );
		h_data_down->Scale(1./h_data_down->Integral() );
	}

//--------------------------------------------
	// int nofbins = h_data->GetNbinsX();
	int nofbins = 100;
	for(int ibin=0; ibin<nofbins; ibin++) //init vectors
	{
		v_eyl.push_back(0); v_eyh.push_back(0);
		v_exl.push_back(0.5); v_exh.push_back(0.5);
		v_x.push_back(ibin+0.5);
		v_y.push_back(0);
	}

	for(int ibin=0; ibin<nofbins; ibin++)
	{
		double error_up = h_data_up->GetBinContent(ibin+1) - h_data->GetBinContent(ibin+1);
		double error_down = h_data_down->GetBinContent(ibin+1) - h_data->GetBinContent(ibin+1);
		if(error_up < 0 && error_down < 0)
		{
			if(fabs(error_up) > fabs(error_down)) {v_eyl[ibin] = fabs(error_up);}
			else {v_eyl[ibin] = fabs(error_down);}
		}
		else if(error_up > 0 && error_down > 0)
		{
			if(fabs(error_up) > fabs(error_down)) {v_eyh[ibin] = fabs(error_up);}
			else {v_eyh[ibin] = fabs(error_down);}
		}
		else if(error_up >= 0 && error_down < 0)
		{
			v_eyh[ibin] = fabs(error_up);
			v_eyl[ibin] = fabs(error_down);
		}
		else if(error_up <= 0 && error_down >= 0)
		{
			v_eyh[ibin] = fabs(error_down);
			v_eyl[ibin] = fabs(error_up);
		}
		else {cout<<"ERROR !"<<endl; cout<<"error_up "<<error_up<<endl; cout<<"error_down "<<error_down<<endl;}

		v_y[ibin] = h_data->GetBinContent(ibin+1);

		// cout<<endl<<"ibin "<<ibin<<endl;
		// cout<<"v_eyl[ibin] "<<v_eyl[ibin]<<endl;
		// cout<<"v_eyh[ibin] "<<v_eyh[ibin]<<endl;
		// cout<<"v_x[ibin] "<<v_x[ibin]<<endl;
		// cout<<"v_y[ibin] "<<v_y[ibin]<<endl;
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
	// gr_error->Draw("e2 same"); //Superimposes the uncertainties on stack
//--------------------------------------------

    //-- Hardcode input files
    v_listSamples.clear();
    v_listSamples.push_back("tHq_PUpresel");

	for(int isample=0; isample<v_listSamples.size(); isample++)
	{
		if(v_listSamples[isample] == "DATA") {continue;}

		// cout<<"Sample "<<v_listSamples[isample]<<endl;

        TString filename = filepath + v_listSamples[isample] + ".root";
        if(!Check_File_Existence(filename)) {cout<<BOLD(FRED("File "<<filename<<" not found !"))<<endl; return;}
	    TFile* f = TFile::Open(filename);
		TString hname = "h_"+var+"_noCorr";
		if(!f->GetListOfKeys()->Contains(hname) ) {cout<<"Histo "<<hname<<" not found in file : "<<filename<<" ! Skip"<<endl; continue;}
	    TH1F* h_noCorr = (TH1F*) f->Get(hname);
		hname = "h_"+var+"_withCorr";
		if(!f->GetListOfKeys()->Contains(hname) ) {cout<<"Histo "<<hname<<" not found in file : "<<filename<<" ! Skip"<<endl; continue;}
	    TH1F* h_withCorr = (TH1F*) f->Get(hname);

		h_noCorr->SetLineColor(kRed);
		h_noCorr->SetLineStyle(3);
		h_withCorr->SetLineColor(kRed);
		h_withCorr->SetLineWidth(3);

		h_noCorr->Scale(1./h_noCorr->Integral() );
		h_withCorr->Scale(1./h_withCorr->Integral() );

		h_noCorr->SetMaximum(h_noCorr->GetMaximum() * 1.4);
		h_noCorr->GetXaxis()->SetTitle(var);

	    TCanvas* c = new TCanvas("", "", 1000, 800);
	    h_noCorr->Draw("hist same");
		h_withCorr->Draw("hist same");
		h_data->Draw("hist same");

		TLegend* qw = new TLegend(0.70,.75,0.90,0.90);
		qw->AddEntry(h_noCorr, "No corr.", "L");
		qw->AddEntry(h_withCorr, "With corr.", "L");
		qw->AddEntry(h_data, "Data", "L");
		qw->AddEntry(gr_error, "Variation", "F");
		qw->Draw("same");

		if(var == "PU")
		{
			// h_data_up->Draw("hist same");
			// h_data_down->Draw("hist same");

			// qw->AddEntry(h_data_up, "Data (up/down)", "L");
			// qw->AddEntry(h_data_down, "Data (down)", "L");
		}

		gr_error->Draw("e2 same"); //Superimposes the uncertainties on stack

	    c->SaveAs("./plots/tmp/" + var + "/" + v_listSamples[isample] + "_"+var+".png");

		delete qw;
	    delete h_noCorr;
	    delete h_withCorr;
	    delete c;
		f->Close();
	}

	delete h_data;
	if(var=="PU") {delete h_data_up; delete h_data_down;}
	f_data->Close();

	return;
}





//--------------------------------------------
 //  ####  #    # #####   ####    ##   ##### ######  ####   ####  #####  # ######  ####
 // #      #    # #    # #    #  #  #    #   #      #    # #    # #    # # #      #
 //  ####  #    # #####  #      #    #   #   #####  #      #    # #    # # #####   ####
 //      # #    # #    # #      ######   #   #      #  ### #    # #####  # #           #
 // #    # #    # #    # #    # #    #   #   #      #    # #    # #   #  # #      #    #
 //  ####   ####  #####   ####  #    #   #   ######  ####   ####  #    # # ######  ####
//--------------------------------------------

//Hard-coded function : check if event satisfies criteria to enter given subcategory
bool Check_isEvent_passSubCategory(TString subcategory, float channel, float nMediumBJets, float lepCharge)
{
    // cout<<"subcategory = "<<subcategory<<endl;
    // cout<<"channel = "<<channel<<endl;
    // cout<<"nMediumBJets = "<<nMediumBJets<<endl;
    // cout<<"lepCharge = "<<lepCharge<<endl;

	//3l lepton flavours
	if(subcategory == "uuu")
	{
		if(channel == 0) {return true;}
		else {return false;}
	}
	else if(subcategory == "uue")
	{
		if(channel == 1) {return true;}
		else {return false;}
	}
	else if(subcategory == "eeu")
	{
		if(channel == 2) {return true;}
		else {return false;}
	}
	else if(subcategory == "eee")
	{
		if(channel == 3) {return true;}
		else {return false;}
	}

	//2lSS lepton flavours
	if(subcategory == "uu" || subcategory == "mm")
	{
		if(channel == 0) {return true;}
		else {return false;}
	}
	else if(subcategory == "ue" || subcategory == "eu" || subcategory == "em" || subcategory == "me")
	{
		if(channel == 1) {return true;}
		else {return false;}
	}
	else if(subcategory == "ee")
	{
		if(channel == 2) {return true;}
		else {return false;}
	}


	//3l ttH2017 subcategories
	if(subcategory == "bl_neg")
	{
		if(nMediumBJets < 2 && lepCharge < 0) {return true;}
		else {return false;}
	}
	else if(subcategory == "bl_pos")
	{
		if(nMediumBJets < 2 && lepCharge > 0) {return true;}
		else {return false;}
	}
	else if(subcategory == "bt_neg")
	{
		if(nMediumBJets >= 2 && lepCharge < 0) {return true;}
		else {return false;}
	}
	else if(subcategory == "bt_pos")
	{
		if(nMediumBJets >= 2 && lepCharge > 0) {return true;}
		else {return false;}
	}

	//2lss ttH2017 subcategories
	if(subcategory == "ee_neg")
	{
		if(channel == 2 && lepCharge < 0) {return true;}
		else {return false;}
	}
	else if(subcategory == "ee_pos")
	{
		if(channel == 2 && lepCharge > 0) {return true;}
		else {return false;}
	}
	else if(subcategory == "em_bl_neg")
	{
		if(channel == 1 && nMediumBJets < 2 && lepCharge < 0) {return true;}
		else {return false;}
	}
	else if(subcategory == "em_bl_pos")
	{
		if(channel == 1 && nMediumBJets < 2 && lepCharge > 0) {return true;}
		else {return false;}
	}
	else if(subcategory == "em_bt_neg")
	{
		if(channel == 1 && nMediumBJets >= 2 && lepCharge < 0) {return true;}
		else {return false;}
	}
	else if(subcategory == "em_bt_pos")
	{
		if(channel == 1 && nMediumBJets >= 2 && lepCharge > 0) {return true;}
		else {return false;}
	}
	else if(subcategory == "mm_bl_neg")
	{
		if(channel == 0 && nMediumBJets < 2 && lepCharge < 0) {return true;}
		else {return false;}
	}
	else if(subcategory == "mm_bl_pos")
	{
		if(channel == 0 && nMediumBJets < 2 && lepCharge > 0) {return true;}
		else {return false;}
	}
	else if(subcategory == "mm_bt_neg")
	{
		if(channel == 0 && nMediumBJets >= 2 && lepCharge < 0) {return true;}
		else {return false;}
	}
	else if(subcategory == "mm_bt_pos")
	{
		if(channel == 0 && nMediumBJets >= 2 && lepCharge > 0) {return true;}
		else {return false;}
	}

	cout<<FRED("ERROR : Wrong subcategory name : "<<subcategory<<"")<<endl;

	return false;
}

//Used by ttH2017 to define some shape systematyics
float lnN1D_p1(float kappa, float x, float xmin, float xmax)
{
    return std::pow(kappa,(x-xmin)/(xmax-xmin));
}

//Hard-code here the parameters needed by lnN1D_p1(), depending on the name of the systematics
//Taken from : https://github.com/peruzzim/cmgtools-lite/blob/94X_dev_ttH/TTHAnalysis/python/plotter/ttH-multilepton/systsUnc.txt#L74
//NB : when not applying subcat, don't compute Fakes closure syst (how to ?)
float Get_lnN1D_Syst_Weight(TString systname, TString samplename, TString catname, TString nLep_cat, float mva_ttbar, float mva_ttV, TString analysis_type)
{
	// cout<<endl<<"systname "<<systname<<endl;
	// cout<<"samplename "<<samplename<<endl;
	// cout<<"catname "<<catname<<endl;
	// cout<<"nLep_cat "<<nLep_cat<<endl;
	// cout<<"mva_ttbar "<<mva_ttbar<<endl;
	// cout<<"mva_ttV "<<mva_ttV<<endl;

	//If 1 of the MVA is not filled correctly, set it to 0 to remove it from computation
	if(mva_ttbar == -9) {mva_ttbar = 0;}
	if(mva_ttV == -9) {mva_ttV = 0;}

	if(nLep_cat != "2l" && nLep_cat != "3l") {cout<<FRED("Error : wrong nLep_cat !")<<endl; return 0;}

	if(systname == "thu_shape_ttH")
	{
		if(!samplename.Contains("ttH") ) {return 0;}

		if(nLep_cat == "2l") {return lnN1D_p1(1.05, mva_ttbar, -1, 1) * lnN1D_p1(1.02, mva_ttV, -1, 1) / 1.04;}
		else {return lnN1D_p1(1.05, mva_ttbar, -1, 1) * lnN1D_p1(1.10, mva_ttV, -1, 1) / 1.08;}
	}
	else if(systname == "thu_shape_ttW")
	{
		if(!samplename.Contains("ttW") ) {return 0;}

		if(nLep_cat == "2l") {return lnN1D_p1(1.02, mva_ttbar, -1, 1) * lnN1D_p1(1.03, mva_ttV, -1, 1) / 1.04;}
		else {return lnN1D_p1(1.04, mva_ttbar, -1, 1) * lnN1D_p1(1.10, mva_ttV, -1, 1) / 1.08;}
	}
	else if(systname == "thu_shape_ttZ")
	{
		if(!samplename.Contains("ttZ") && !samplename.Contains("TTJets") ) {return 0;} //Must be careful to also run it for TTJets, since we merge it with ttZ? Must cross-check !

		if(nLep_cat == "2l") {return lnN1D_p1(1.06, mva_ttbar, -1, 1) * lnN1D_p1(1.06, mva_ttV, -1, 1) / 1.04;}
		else {return lnN1D_p1(1.08, mva_ttbar, -1, 1) * lnN1D_p1(1.13, mva_ttV, -1, 1) / 1.08;}
	}
	else if(systname == "Clos_e_shape")
	{
		if(!samplename.Contains("Fakes") ) {return 0;}

		if(nLep_cat == "2l")
		{
			if(catname.Contains("ee")) {return lnN1D_p1(1.5, mva_ttbar, -1, 1) / 1.2;}
			else if(catname.Contains("em") || catname.Contains("eu") || catname.Contains("me") || catname.Contains("ue")) {return lnN1D_p1(1.25, mva_ttbar, -1, 1) / 1.1;}
		}
	}
	else if(systname == "Clos_m_shape")
	{
		if(!samplename.Contains("Fakes") ) {return 0;}

		if(nLep_cat == "2l")
		{
			if(catname.Contains("em") || catname.Contains("eu") || catname.Contains("me") || catname.Contains("ue")) {return lnN1D_p1(1.5, mva_ttV, -1, 1) / 1.24;}
			else if(catname.Contains("mm") || catname.Contains("uu")) {return lnN1D_p1(1.8, mva_ttV, -1, 1) / 1.35;}
		}
	}
	else {cout<<FRED("Error : wrong systname : "<<systname<<" !")<<endl; return 0;}

	return 0;
}


/**
 * Set histo to a flat distribution close to zero
 * This is used e.g. to avoid having any histo with norm <= 0, to avoid combine crashes
 */
void Set_Histogram_FlatZero(TH1F*& h, bool printout, TString name)
{
	if(printout)
	{
		cout<<endl<<FRED("Histo "<<name<<" has integral = "<<h->Integral()<<" <= 0 ! Distribution set to ~>0 (flat), to avoid crashes in COMBINE !")<<endl;
	}

	for(int ibin=1; ibin<h->GetNbinsX()+1; ibin++)
	{
		h->SetBinContent(ibin, pow(10, -9));

		if(h->GetBinError(ibin) == 0) {h->SetBinError(ibin, 0.1);}
	}

	return;
}


//Get mirror histo (rescaled to nominal if necessary)
//As done by Benjamin/ttH : https://github.com/stiegerb/cmgtools-lite/blob/80X_M17_tHqJan30_bbcombination/TTHAnalysis/python/plotter/makeShapeCardsTHQ.py#L609-L633
//Directly modify the histos passed in args
void Get_Mirror_Histogram(TH1F*& hcentral, TH1F*& hvariation, TH1F*& hmirror, bool is_ShapeOnly)
{
	hmirror = 0; //empty at beginning
	hmirror = (TH1F*) hcentral->Clone(); //Get binning, etc.

	//Rescale variation to nominal (shape-only)
    if(is_ShapeOnly) {hvariation->Scale(hcentral->Integral() / hvariation->Integral());}

	for(int ibin=1; ibin<=hcentral->GetNbinsX(); ibin++)
	{
		double yA = hvariation->GetBinContent(ibin); //'up = alternate'
		double y0 = hcentral->GetBinContent(ibin); //'central'
		double yM = y0; //'down = mirror'

		if(y0 > 0 && yA > 0) {yM = y0 * y0/yA;}
		else if(yA == 0) {yM = 2*y0;}

		hmirror->SetBinContent(ibin, yM);
	}

    if(is_ShapeOnly) {hmirror->Scale(hcentral->Integral() / hmirror->Integral());} //Rescale mirror to nominal (shape-only)
	else //Mirror normalization, cf github
	{
    	double mirror_norm = pow(hcentral->Integral(), 2) / hvariation->Integral();
		hmirror->Scale(mirror_norm / hvariation->Integral());
	}

	return;
}

//Get mirror histo, as done by ttH2017 :  https://github.com/CERN-PH-CMG/cmgtools-lite/blob/759bdc4213c50db48cb695ae498f7a97794a1410/TTHAnalysis/python/plotter/uncertaintyFile.py#L106
//Directly modify the hdown histo
void Get_TemplateSymm_Histogram(TH1F*& hcentral, TH1F*& hvariation, TH1F*& hmirror, bool is_ShapeOnly)
{
	hmirror = 0; //empty at beginning
	hmirror = (TH1F*) hcentral->Clone();

    if(hcentral->Integral() <= 0 || hvariation->Integral() <= 0) {return;}

	//Protection -- check if I artificially set the bin to a float ~null distribution
	//If that is the case, just do mirror = central
	if(hcentral->Integral() == hcentral->GetNbinsX() * hcentral->GetBinContent(1) || hvariation->Integral() == hvariation->GetNbinsX() * hvariation->GetBinContent(1))
	{
		return;
	}

	hmirror->Multiply(hmirror);
	hmirror->Divide(hvariation);

	// cout<<"hmirror->Integral() "<<hmirror->Integral()<<endl;

	return;
}


/**
 * In FCNC analysis, we want to reuse the closure shape systematics from ttH, even though they were derived in different regions
 * Thus we want to inflate these systematics by 20%. Can be done in datacard parser for norm syst. For shape syst, check the different (syst - nominal) in each bin, and inflate by 20%
 */
void Inflate_Syst_inShapeTemplate(TH1F*& h_toInflate, TH1F* h_nominal, float inflation_factor)
{
	if(!h_toInflate || !h_nominal || !h_toInflate->GetEntries() || !h_nominal->GetEntries()) {cout<<"Error in Inflate_Syst_inShapeTemplate() !"<<endl; return;}

	for(int ibin=1; ibin<h_toInflate->GetNbinsX()+1; ibin++)
	{
		//syst - nominal
		float diff_bin = h_toInflate->GetBinContent(ibin) - h_nominal->GetBinContent(ibin);

		h_toInflate->SetBinContent(ibin, h_toInflate->GetBinContent(ibin) + inflation_factor * diff_bin); //+- X%, depending on direction of variation
	}

	return;
}


float Get_Binning_SubCategory(TString nLep_cat, TString channel)
{
    if(nLep_cat == "2l")
    {
        if(channel.Contains("ee") ) { return 1;}
        else if(channel.Contains("em_bl") ) { return 2;}
        else if(channel.Contains("em_bt") ) { return 3;}
        else if(channel.Contains("mm_bl") ) { return 4;}
        else if(channel.Contains("mm_bt") ) { return 5;}
        else {cout<<"Wrong channel !"<<endl; return -1;}
    }
    else if(nLep_cat == "3l")
    {
        if(channel.Contains("bl") ) { return 1;}
        else if(channel.Contains("bt") ) { return 2;}
        else {cout<<"Wrong channel !"<<endl; return -1;}
    }
    else {cout<<"Wrong nLep_cat !"<<endl; return -1;}

    return -1;
}


TString Get_CERN_CombineNamingConvention(TString nLep, TString sample, TString syst)
{
	//Modified sample names
	TString sample_new = "";

    if(sample == "ttWW") {sample_new = "TTWW";}
    else if(sample == "ttW_PSweights") {sample_new = "TTW";}
	else if(sample == "DATA") {sample_new = "data_obs";}
	else if(sample == "tZq") {sample_new = "TZQ";}
	else if(sample.Contains("GammaConv") ) {sample_new = "Convs";}
	else if(sample.Contains("Fakes") ) {sample_new = "data_fakes";}
	else if(sample.Contains("Flip") ) {sample_new = "data_flips";}
	else  {sample_new = sample;}

	//Modified systematics name
	TString syst_new = "";

	if(syst.Contains("JES") ) {syst_new = "CMS_scale_j";}
	else if(syst.Contains("TrigEff") ) {syst_new = "CMS_ttHl17_trigger";}
	else if(syst.Contains("LepEff_elLoose") ) {syst_new = "CMS_ttHl16_lepEff_elloose";}
	else if(syst.Contains("LepEff_muLoose") ) {syst_new = "CMS_ttHl16_lepEff_muloose";}
	else if(syst.Contains("LepEff_elTight") ) {syst_new = "CMS_ttHl16_lepEff_eltight";}
	else if(syst.Contains("LFcont") ) {syst_new = "CMS_ttHl16_btag_LF";}
	else if(syst.Contains("HFstats1") ) {syst_new = "CMS_ttHl17_btag_HFStats1";}
	else if(syst.Contains("HFstats2") ) {syst_new = "CMS_ttHl17_btag_HFStats2";}
	else if(syst.Contains("CFerr1") ) {syst_new = "CMS_ttHl16_btag_cErr1";}
	else if(syst.Contains("CFerr2") ) {syst_new = "CMS_ttHl16_btag_cErr2";}
	else if(syst.Contains("HFcont") ) {syst_new = "CMS_ttHl16_btag_HF";}
	else if(syst.Contains("LFstats1") ) {syst_new = "CMS_ttHl17_btag_LFStats1";}
	else if(syst.Contains("LFstats2") ) {syst_new = "CMS_ttHl17_btag_LFStats2";}
	else if(syst.Contains("FR_be_el") ) {syst_new = "CMS_ttHl16_FRe_be";}
	else if(syst.Contains("FR_be_mu") ) {syst_new = "CMS_ttHl16_FRm_be";}
	else if(syst.Contains("FR_norm_el") ) {syst_new = "CMS_ttHl16_FRe_norm";}
	else if(syst.Contains("FR_norm_mu") ) {syst_new = "CMS_ttHl16_FRm_norm";}
	else if(syst.Contains("FR_pt_el") ) {syst_new = "CMS_ttHl16_FRe_pt";}
	else if(syst.Contains("FR_pt_mu") ) {syst_new = "CMS_ttHl16_FRm_pt";}
	else if(syst.Contains("Clos_e_") ) {syst_new = "CMS_ttHl17_Clos_e_shape";}
	else if(syst.Contains("Clos_m_") ) {syst_new = "CMS_ttHl17_Clos_m_shape";}
	else if(syst.Contains("thu_shape_ttH") ) {syst_new = "CMS_ttHl_thu_shape_ttH";}
	else if(syst.Contains("thu_shape_ttW") ) {syst_new = "CMS_ttHl_thu_shape_ttW";}
	else if(syst.Contains("thu_shape_ttZ") ) {syst_new = "CMS_ttHl_thu_shape_ttZ";}
	else if(syst.Contains("prefiringWeight") ) {syst_new = "CMS_tHql_prefiring";}
	else if(syst.Contains("fwdJet") ) {syst_new = "CMS_tHql_fwdJet";}
	else if(syst != "") {return "";}

	if(syst.Contains("Down")) {syst_new+= "Down";}
	else {syst_new+= "Up";}

	//Complete name
	TString CERN_histoname = "x_" + sample_new;
	if(syst != "") {CERN_histoname+= "_" + syst_new;}

	return CERN_histoname;
}

void Copy_TemplateFile_allCouplings(vector<TString> v_couplingPoints, TString nLep_cat, TString channel)
{
	mkdir("./outputs/templates_allCouplingPoints", 0777);

	TString origin_path, command, dest_path;

	origin_path = "./outputs/tHq_";
	if(nLep_cat == "2l")
	{
		if(channel == "uu" || channel == "mm") {origin_path+= "2lss_mm";}
		else if(channel == "eu" || channel == "ue" || channel == "em" || channel == "me") {origin_path+= "2lss_em";}
	}
	else {origin_path+= "3l";}

	origin_path+= ".input.root";

	for(int i=0; i<v_couplingPoints.size(); i++)
	{
		dest_path = "./outputs/templates_allCouplingPoints/tHq_";
		if(nLep_cat == "2l")
		{
			if(channel == "uu" || channel == "mm") {dest_path+= "2lss_mm";}
			else if(channel == "eu" || channel == "ue" || channel == "em" || channel == "me") {dest_path+= "2lss_em";}
		}
		else {dest_path+= "3l";}
		dest_path+= "_"+v_couplingPoints[i]+".input.root";

		command = "cp "+origin_path + " " + dest_path;
		system(command.Data() );
	}

	//Hardcode copy of ITC points
	command = "cp "+origin_path + " ./outputs/templates_allCouplingPoints/tHq_1_m1.input.root";
	system(command.Data() );

	command = "cp "+origin_path + " ./outputs/templates_allCouplingPoints/tHq_m1p0.input.root";
	system(command.Data() );

	return;
}

/**
 * Read hSumWeights and hLHE files to reads sums of weights (and pass by reference)
 * NB : 'ttH.root' is actually the SWE file for ttH_ctcvcp !
 */
void Get_SumWeights_allCouplings(TString sample, vector<float>& v_SWE_LHE, float& SWE_nominal)
{
	TString samplename = "";
	if(sample.Contains("tHq")) {samplename = "tHq";}
	if(sample.Contains("tHW")) {samplename = "tHW";}
	if(sample.Contains("ttH")) {samplename = "ttH";}

	TString input_name = "./data/sumWeightHistograms/" + samplename + ".root";
	if(!Check_File_Existence(input_name) ) {cout<<endl<<FRED("File "<<input_name<<" not found! Return !")<<endl; return;}
	TFile* f = TFile::Open(input_name, "READ");

	//Nominal
	if(!f->GetListOfKeys()->Contains("hSumWeights") ) {cout<<FRED("Histo 'hSumWeights' not found in file "<<input_name<<" ! Return !")<<endl; return;}
	TH1F* h = (TH1F*) f->Get("hSumWeights");
	SWE_nominal = h->GetBinContent(2);
	delete h; h = 0;

	//LHE
	v_SWE_LHE.resize(70); //at most 70 (with CP) or 51 (kt/kv)

	int firstWeightIndex = 883; //THQ_ctcvcp and TTH_ctcvcp
	if(sample.Contains("tHW")) {firstWeightIndex = 1081;}

	if(!f->GetListOfKeys()->Contains("hLHE") ) {cout<<FRED("Histo 'LHE' not found in file "<<input_name<<" ! Return !")<<endl; return;}
	h = (TH1F*) f->Get("hLHE");
	for(int i=0; i<v_SWE_LHE.size(); i++)
	{
		v_SWE_LHE[i] = h->GetBinContent(firstWeightIndex + i);
	}
	delete h; h = 0;

	return;
}


/**
 * CERN wants all their templates rescaled to SM xsec
 * --> Return SF corresponding to (xsec_SM / xsec_coupling)
 * Values of all xsecs are hardcoded here, for all signal samples
 */
float Get_scalingFactor_toSMxsec(TString couplingPoint, TString sample)
{
	float xsec_SM = 0, xsec_ITC = 0;
	float xsec_coupling = 0;

	if(sample.Contains("tHq"))
	{
		xsec_SM = 0.07096;
		// xsec_ITC = 0.7927; //CERN templates scaled to SM by default

		if(couplingPoint == "1_3") {xsec_coupling = 2.991;}
		else if(couplingPoint == "1_2") {xsec_coupling = 1.706;}
		else if(couplingPoint == "1_1p5") {xsec_coupling = 1.205;}
		else if(couplingPoint == "1_1p25") {xsec_coupling = 0.9869;}
		else if(couplingPoint == "1_0p75") {xsec_coupling = 0.6212;}
		else if(couplingPoint == "1_0p5") {xsec_coupling = 0.4723;}
		else if(couplingPoint == "1_0p25") {xsec_coupling = 0.3505;}
		else if(couplingPoint == "1_0p0001") {xsec_coupling = 0.2482;}
		else if(couplingPoint == "1_m0p25") {xsec_coupling = 0.1694;}
		else if(couplingPoint == "1_m0p75") {xsec_coupling = 0.1133;}
		else if(couplingPoint == "1_m0p5") {xsec_coupling = 0.08059;}
		else if(couplingPoint == "1_m1") {xsec_coupling = 0.07096;} //SM
		else if(couplingPoint == "1_m1p25") {xsec_coupling = 0.0839;}
		else if(couplingPoint == "1_m1p5") {xsec_coupling = 0.1199;}
		else if(couplingPoint == "1_m2") {xsec_coupling = 0.2602;}
		else if(couplingPoint == "1_m3") {xsec_coupling = 0.8210;}
		else if(couplingPoint == "1p5_3") {xsec_coupling = 3.845;}
		else if(couplingPoint == "1p5_2") {xsec_coupling = 2.371;}
		else if(couplingPoint == "1p5_1p5") {xsec_coupling = 1.784;}
		else if(couplingPoint == "1p5_1p25") {xsec_coupling = 1.518;}
		else if(couplingPoint == "1p5_1") {xsec_coupling = 1.287;}
		else if(couplingPoint == "1p5_0p75") {xsec_coupling = 1.067;}
		else if(couplingPoint == "1p5_0p5") {xsec_coupling = 0.8744;}
		else if(couplingPoint == "1p5_0p25") {xsec_coupling = 0.7029;}
		else if(couplingPoint == "1p5_0p0001") {xsec_coupling = 0.5577;}
		else if(couplingPoint == "1p5_m0p25") {xsec_coupling = 0.4365;}
		else if(couplingPoint == "1p5_m0p75") {xsec_coupling = 0.3343;}
		else if(couplingPoint == "1p5_m0p5") {xsec_coupling = 0.2558;}
		else if(couplingPoint == "1p5_m1") {xsec_coupling = 0.2003;}
		else if(couplingPoint == "1p5_m1p25") {xsec_coupling = 0.1689;}
		else if(couplingPoint == "1p5_m1p5") {xsec_coupling = 0.1594;}
		else if(couplingPoint == "1p5_m2") {xsec_coupling = 0.2105;}
		else if(couplingPoint == "1p5_m3") {xsec_coupling = 0.5889;}
		else if(couplingPoint == "0p5_3") {xsec_coupling = 2.260;}
		else if(couplingPoint == "0p5_2") {xsec_coupling = 1.160;}
		else if(couplingPoint == "0p5_1p5") {xsec_coupling = 0.7478;}
		else if(couplingPoint == "0p5_1p25") {xsec_coupling = 0.5726;}
		else if(couplingPoint == "0p5_1") {xsec_coupling = 0.4273;}
		else if(couplingPoint == "0p5_0p75") {xsec_coupling = 0.2999;}
		else if(couplingPoint == "0p5_0p5") {xsec_coupling = 0.1982;}
		else if(couplingPoint == "0p5_0p25") {xsec_coupling = 0.1189;}
		else if(couplingPoint == "0p5_0p0001") {xsec_coupling = 0.06223;}
		else if(couplingPoint == "0p5_m0p25") {xsec_coupling = 0.02830;}
		else if(couplingPoint == "0p5_m0p75") {xsec_coupling = 0.01778;}
		else if(couplingPoint == "0p5_m0p5") {xsec_coupling = 0.03008;}
		else if(couplingPoint == "0p5_m1") {xsec_coupling = 0.06550;}
		else if(couplingPoint == "0p5_m1p25") {xsec_coupling = 0.1241;}
		else if(couplingPoint == "0p5_m1p5") {xsec_coupling = 0.2047;}
		else if(couplingPoint == "0p5_m2") {xsec_coupling = 0.4358;}
		else if(couplingPoint == "0p5_m3") {xsec_coupling = 1.177;}
	}
	else if(sample.Contains("tHW"))
	{
		xsec_SM = 0.01561;
		// xsec_ITC = 0.1472; //CERN templates scaled to SM by default

		if(couplingPoint == "1_3") {xsec_coupling = 0.6409;}
		else if(couplingPoint == "1_2") {xsec_coupling = 0.3458;}
		else if(couplingPoint == "1_1p5") {xsec_coupling = 0.2353;}
		else if(couplingPoint == "1_1p25") {xsec_coupling = 0.1876;}
		else if(couplingPoint == "1_0p75") {xsec_coupling = 0.1102;}
		else if(couplingPoint == "1_0p5") {xsec_coupling = 0.07979;}
		else if(couplingPoint == "1_0p25") {xsec_coupling = 0.05518;}
		else if(couplingPoint == "1_0p0001") {xsec_coupling = 0.03581;}
		else if(couplingPoint == "1_m0p25") {xsec_coupling = 0.02226;}
		else if(couplingPoint == "1_m0p75") {xsec_coupling = 0.01444;}
		else if(couplingPoint == "1_m0p5") {xsec_coupling = 0.01222;}
		else if(couplingPoint == "1_m1") {xsec_coupling = 0.01561;}
		else if(couplingPoint == "1_m1p25") {xsec_coupling = 0.02481;}
		else if(couplingPoint == "1_m1p5") {xsec_coupling = 0.03935;}
		else if(couplingPoint == "1_m2") {xsec_coupling = 0.08605;}
		else if(couplingPoint == "1_m3") {xsec_coupling = 0.2465;}
		else if(couplingPoint == "1p5_3") {xsec_coupling = 0.7825;}
		else if(couplingPoint == "1p5_2") {xsec_coupling = 0.4574;}
		else if(couplingPoint == "1p5_1p5") {xsec_coupling = 0.3290;}
		else if(couplingPoint == "1p5_1p25") {xsec_coupling = 0.2749;}
		else if(couplingPoint == "1p5_1") {xsec_coupling = 0.2244;}
		else if(couplingPoint == "1p5_0p75") {xsec_coupling = 0.1799;}
		else if(couplingPoint == "1p5_0p5") {xsec_coupling = 0.1410;}
		else if(couplingPoint == "1p5_0p25") {xsec_coupling = 0.1081;}
		else if(couplingPoint == "1p5_0p0001") {xsec_coupling = 0.08056;}
		else if(couplingPoint == "1p5_m0p25") {xsec_coupling = 0.05893;}
		else if(couplingPoint == "1p5_m0p75") {xsec_coupling = 0.04277;}
		else if(couplingPoint == "1p5_m0p5") {xsec_coupling = 0.03263;}
		else if(couplingPoint == "1p5_m1") {xsec_coupling = 0.02768;}
		else if(couplingPoint == "1p5_m1p25") {xsec_coupling = 0.02864;}
		else if(couplingPoint == "1p5_m1p5") {xsec_coupling = 0.03509;}
		else if(couplingPoint == "1p5_m2") {xsec_coupling = 0.06515;}
		else if(couplingPoint == "1p5_m3") {xsec_coupling = 0.1930;}
		else if(couplingPoint == "0p5_3") {xsec_coupling = 0.5136;}
		else if(couplingPoint == "0p5_2") {xsec_coupling = 0.2547;}
		else if(couplingPoint == "0p5_1p5") {xsec_coupling = 0.1591;}
		else if(couplingPoint == "0p5_1p25") {xsec_coupling = 0.1204;}
		else if(couplingPoint == "0p5_1") {xsec_coupling = 0.08696;}
		else if(couplingPoint == "0p5_0p75") {xsec_coupling = 0.05885;}
		else if(couplingPoint == "0p5_0p5") {xsec_coupling = 0.03658;}
		else if(couplingPoint == "0p5_0p25") {xsec_coupling = 0.01996;}
		else if(couplingPoint == "0p5_0p0001") {xsec_coupling = 0.008986;}
		else if(couplingPoint == "0p5_m0p25") {xsec_coupling = 0.003608;}
		else if(couplingPoint == "0p5_m0p75") {xsec_coupling = 0.003902;}
		else if(couplingPoint == "0p5_m0p5") {xsec_coupling = 0.009854;}
		else if(couplingPoint == "0p5_m1") {xsec_coupling = 0.02145;}
		else if(couplingPoint == "0p5_m1p25") {xsec_coupling = 0.03848;}
		else if(couplingPoint == "0p5_m1p5") {xsec_coupling = 0.06136;}
		else if(couplingPoint == "0p5_m2") {xsec_coupling = 0.1246;}
		else if(couplingPoint == "0p5_m3") {xsec_coupling = 0.3172;}
	}

	return xsec_SM / xsec_coupling;
}



/**
 * Compute SF to rescale nominal weight to different coupling
 */
float Get_SF_DifferentCoupling(TString sample, TString couplingName, vector<float>* v_LHEweight, float mc_weight_originalValue, float SWE_nominal, float SWE_LHE)
{
	float SF;

	//Get correct LHE index, depends on sample
	int firstWeightIndex = 882; //THQ_ctcvcp and TTH_ctcvcp
	if(sample.Contains("tHW")) {firstWeightIndex = 1080;}

	int ilhe = 0; //final index to look for

	//kV = 1
	if(couplingName == "1_3") {ilhe = 0;}
	else if(couplingName == "1_2") {ilhe = 1;}
	else if(couplingName == "1_1p5") {ilhe = 2;}
	else if(couplingName == "1_1p25") {ilhe = 3;}
	else if(couplingName == "1_0p75") {ilhe = 4;}
	else if(couplingName == "1_0p5") {ilhe = 5;}
	else if(couplingName == "1_0p25") {ilhe = 6;}
	else if(couplingName == "1_0p0001") {ilhe = 7;}
	else if(couplingName == "1_m0p25") {ilhe = 8;}
	else if(couplingName == "1_m0p75") {ilhe = 9;}
	else if(couplingName == "1_m0p5") {ilhe = 10;}
	else if(couplingName == "1_m1") {ilhe = 11;} //ITC
	else if(couplingName == "1_m1p25") {ilhe = 12;}
	else if(couplingName == "1_m1p5") {ilhe = 13;}
	else if(couplingName == "1_m2") {ilhe = 14;}
	else if(couplingName == "1_m3") {ilhe = 15;}

	//kV = 1.5
	else if(couplingName == "1p5_3") {ilhe = 16 + 0;}
	else if(couplingName == "1p5_2") {ilhe = 16 + 1;}
	else if(couplingName == "1p5_1p5") {ilhe = 16 + 2;}
	else if(couplingName == "1p5_1p25") {ilhe = 16 + 3;}
	else if(couplingName == "1p5_1") {ilhe = 16 + 4;} //Needed to interspece +1 value
	else if(couplingName == "1p5_0p75") {ilhe = 16 + 4 + 1;}
	else if(couplingName == "1p5_0p5") {ilhe = 16 + 5 + 1;}
	else if(couplingName == "1p5_0p25") {ilhe = 16 + 6 + 1;}
	else if(couplingName == "1p5_0p0001") {ilhe = 16 + 7 + 1;}
	else if(couplingName == "1p5_m0p25") {ilhe = 16 + 8 + 1;}
	else if(couplingName == "1p5_m0p75") {ilhe = 16 + 9 + 1;}
	else if(couplingName == "1p5_m0p5") {ilhe = 16 + 10 + 1;}
	else if(couplingName == "1p5_m1") {ilhe = 16 + 11 + 1;}
	else if(couplingName == "1p5_m1p25") {ilhe = 16 + 12 + 1;}
	else if(couplingName == "1p5_m1p5") {ilhe = 16 + 13 + 1;}
	else if(couplingName == "1p5_m2") {ilhe = 16 + 14 + 1;}
	else if(couplingName == "1p5_m3") {ilhe = 16 + 15 + 1;}

	//kV = 0.5
	else if(couplingName == "0p5_3") {ilhe = 33 + 0;}
	else if(couplingName == "0p5_2") {ilhe = 33 + 1;}
	else if(couplingName == "0p5_1p5") {ilhe = 33 + 2;}
	else if(couplingName == "0p5_1p25") {ilhe = 33 + 3;}
	else if(couplingName == "0p5_1") {ilhe = 33 + 4;} //Needed to interspece +1 value
	else if(couplingName == "0p5_0p75") {ilhe = 33 + 4 + 1;}
	else if(couplingName == "0p5_0p5") {ilhe = 33 + 5 + 1;}
	else if(couplingName == "0p5_0p25") {ilhe = 33 + 6 + 1;}
	else if(couplingName == "0p5_0p0001") {ilhe = 33 + 7 + 1;}
	else if(couplingName == "0p5_m0p25") {ilhe = 33 + 8 + 1;}
	else if(couplingName == "0p5_m0p75") {ilhe = 33 + 9 + 1;}
	else if(couplingName == "0p5_m0p5") {ilhe = 33 + 10 + 1;}
	else if(couplingName == "0p5_m1") {ilhe = 33 + 11 + 1;}
	else if(couplingName == "0p5_m1p25") {ilhe = 33 + 12 + 1;}
	else if(couplingName == "0p5_m1p5") {ilhe = 33 + 13 + 1;}
	else if(couplingName == "0p5_m2") {ilhe = 33 + 14 + 1;}
	else if(couplingName == "0p5_m3") {ilhe = 33 + 15 + 1;}

    //CP PHASE
    // else if(couplingName == "m1p0") {ilhe = 50 + 0;} //ITC?
    else if(couplingName == "m0p9") {ilhe = 49 + 1;}
    else if(couplingName == "m0p8") {ilhe = 49 + 2;}
    else if(couplingName == "m0p7") {ilhe = 49 + 3;}
    else if(couplingName == "m0p6") {ilhe = 49 + 4;}
    else if(couplingName == "m0p5") {ilhe = 49 + 5;}
    else if(couplingName == "m0p4") {ilhe = 49 + 6;}
    else if(couplingName == "m0p3") {ilhe = 49 + 7;}
    else if(couplingName == "m0p2") {ilhe = 49 + 8;}
    else if(couplingName == "m0p1") {ilhe = 49 + 9;}
    else if(couplingName == "0p0") {ilhe = 49 + 10;}
    else if(couplingName == "0p1") {ilhe = 49 + 11;}
    else if(couplingName == "0p2") {ilhe = 49 + 12;}
    else if(couplingName == "0p3") {ilhe = 49 + 13;}
    else if(couplingName == "0p4") {ilhe = 49 + 14;}
    else if(couplingName == "0p5") {ilhe = 49 + 15;}
    else if(couplingName == "0p6") {ilhe = 49 + 16;}
    else if(couplingName == "0p7") {ilhe = 49 + 17;}
    else if(couplingName == "0p8") {ilhe = 49 + 18;}
    else if(couplingName == "0p9") {ilhe = 49 + 19;}
    // else if(couplingName == "1p0") {ilhe = 50 + 20;} //SM ?

	ilhe = firstWeightIndex + ilhe;

	// cout<<"v_LHEweight->at(ilhe) "<<v_LHEweight->at(ilhe)<<endl;
	// cout<<"mc_weight_originalValue "<<mc_weight_originalValue<<endl;
	// cout<<"SWE_nominal "<<SWE_nominal<<endl;
	// cout<<"SWE_LHE "<<SWE_LHE<<endl;

	SF = (v_LHEweight->at(ilhe) / mc_weight_originalValue) * (SWE_nominal / SWE_LHE);

	return SF;
}

//--------------------------------------------
// ######## ########  ######  ########
//    ##    ##       ##    ##    ##
//    ##    ##       ##          ##
//    ##    ######    ######     ##
//    ##    ##             ##    ##
//    ##    ##       ##    ##    ##
//    ##    ########  ######     ##
//--------------------------------------------

int Test_Binnings(int bin_choice, float mva_ttbar, float mva_ttV, bool debug)
{
	float x1=-1, x2=-1;
	float y1=-1, y2=-1;

	//Borders to test
	vector<float> v_x1, v_x2, v_y1, v_y2;
    v_x1.push_back(-0.2);
    v_x1.push_back(0.);
    v_x1.push_back(0.2);
	v_x1.push_back(0.4);

	v_x2.push_back(0.6);
	v_x2.push_back(0.7);

    v_y1.push_back(-0.2);
    v_y1.push_back(0.);
    v_y1.push_back(0.2);
	v_y1.push_back(0.4);

	v_y2.push_back(0.6);
	v_y2.push_back(0.7);
	v_y2.push_back(0.8);

	int index = 0;
	for(int ix1=0; ix1<v_x1.size(); ix1++)
	{
		for(int ix2=0; ix2<v_x2.size(); ix2++)
		{
			for(int iy1=0; iy1<v_y1.size(); iy1++)
			{
				for(int iy2=0; iy2<v_y2.size(); iy2++)
				{
					if(index == bin_choice)
					{
						x1 = v_x1[ix1];
						x2 = v_x2[ix2];
						y1 = v_y1[iy1];
						y2 = v_y2[iy2];
						// cout<<"x1 --> "<<x1<<endl;
						break;
					}
					index++;
				}
				if(x1 != -1) {break;} //found
			}
			if(x1 != -1) {break;} //found
		}
		if(x1 != -1) {break;} //found
	}
	if(x1 == -1) {cout<<FRED("Error : wrong value of 'bin_choice' = "<<bin_choice<<" !")<<endl; return -1;}

	if(debug)
	{
		cout<<endl<<"-- BINNING CHOICE "<<bin_choice<<" : "<<endl;
		cout<<"[x1 = "<<x1<<"]"<<endl;
		cout<<"[x2 = "<<x2<<"]"<<endl;
		cout<<"[y1 = "<<y1<<"]"<<endl;
		cout<<"[y2 = "<<y2<<"]"<<endl;
		cout<<"mva_ttbar "<<mva_ttbar<<endl;
		cout<<"mva_ttV "<<mva_ttV<<endl;
	}

	if( mva_ttbar  > x2  && mva_ttV  >  y2 ) return 9;
	if( mva_ttbar  > x1  && mva_ttV  >  y2 ) return 8;
	if( mva_ttbar  > -1  && mva_ttV  >  y2 ) return 5;
	if( mva_ttbar  > x2  && mva_ttV  >  y1 ) return 7;
	if( mva_ttbar  > x1  && mva_ttV  > y1 ) return 6;
	if( mva_ttbar  > -1  && mva_ttV  >  y1 ) return 4;
	if( mva_ttbar  > x2  && mva_ttV  >  -1 ) return 3;
	if( mva_ttbar > x1 && mva_ttV  >  -1) return 2;
	return 1;
}
