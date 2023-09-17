#include <iostream>
#include <fstream>
#include <string>
#include <stdlib.h>
#include <vector>

using namespace std;

ifstream Inp_File("Huseyni.txt");

int PC;
int ACC = 0;
int IS = 0;
int FLG = 0;

int n = 0;
int HLT_Line = 0;
string Line;
vector<string> V_Words;
unsigned int V_Words_Idx;

bool Is_Vars = false;
bool Over_flow = false;

int Ram[100][2];

struct Command
{
	string Cmd;
	int Code = 0;
};
Command Commands[16] = {{"ADD", 1}, {"SUB", 2}, {"MUL", 3}, {"DIV", 4}, {"STA", 5}, {"LDA", 6}, {"BRA", 7}, {"BRZ", 8}, {"BRP", 9}, {"BRN", 10}, {"BRG", 11}, {"INP", 1201}, {"OUT", 1202}, {"HLT", 0}, {"DAT", -1}};

string User_Codes[100][3] = {};
unsigned int UC_Line_Idx = 0;

struct Var
{
	string Name;
	int Address = 0;
	int Value = 0;
};
Var Vars[20];
unsigned int Idx_Vars = 0;

struct Label
{
	int Address = 0;
	string Name;
};
Label Labels[20];
int Idx_Labels = 0;

string Addresses[10][10];

int To_HLT_Without_free_Lines = 0;

//========================================================================================================================================

string Str_Upper(string Str);
int Search_In_Commands(string Word);
int Search_In_Vars(string Word);
int Search_In_Labels(string Word);
void Line_To_Words(string Line);
void Detect_Vars();
void Find_Labels();
void Print();
bool TryParse(string str, int &x);
void loadProgram();
void printErrorMessage(int, int errorCode);
int lineErrorCode(string line[3]);
void Translator();
int Check_Lab_Var_Repetitious();
void Set_Lab_Var_Addr();
void printRam();
void readFile();
void execute();

//========================================================================================================================================

int main()
{

	loadProgram();

	printRam();

	execute();

	char Temp;
	cout << "\n\n Do you like to run \"(print) function\" ? (Y/N) : ";
	cin >> Temp;
	if (Temp == 'N')
		exit(-1);
	else if (Temp == 'Y')
	{
		// Translator();

		Print();
		Inp_File.close();
		return 0;
	}
}

void Print()
{
	/*cout << endl << endl << "Your Words = [";
	for (unsigned int i = 0; i < V_Words.size(); i++)
	{
		cout << "\"" << V_Words[i] << "\"";
		if (i != n-1)
			cout << ", ";
	}
	cout << "]" << endl;
	cout << "Num of Your Words : " << V_Words.size() << endl << endl << endl;*/

	// ---> for Print Varies
	cout << "\n\nYour Varies : " << endl
		 << endl;
	for (size_t i = 0; i < Idx_Vars; i++)
	{
		cout << "Name : " << Vars[i].Name << "  ,  ";
		cout << "Address : " << Vars[i].Address << "  ,  ";
		cout << "Value: " << Vars[i].Value << endl;
	}
	cout << "============================" << endl
		 << endl;

	cout << "Your Laabels : " << endl
		 << endl;
	for (size_t i = 0; i < Idx_Labels; i++)
	{
		cout << "Name   : " << Labels[i].Name << "  ,  ";
		cout << "Address: " << Labels[i].Address << endl;
	}
	cout << "============================" << endl
		 << endl;

	// ---> Print Array
	for (size_t x = 0; x < UC_Line_Idx; x++)
	{
		cout << "Words in Line (" << x + 1 << ") : [";

		for (size_t y = 0; y < 3; y++)
		{
			cout << "\"" << User_Codes[x][y] << "\"";
			if (y != 2)
				cout << ",";
		}
		cout << "]" << endl;
	}
}

int Search_In_Commands(string Word)
{
	Commands[15].Cmd = Word;

	int i = 0;
	while (Commands[i++].Cmd != Word)
		;
	i--;

	if (i == 15)
		return -1;

	return i;
}

int Search_In_Vars(string Word)
{
	Vars[Idx_Vars].Name = Word;

	int i = 0;
	while (Vars[i++].Name != Word)
		;
	i--;

	if (i == Idx_Vars)
		return -1;

	return i;
}

int Search_In_Labels(string Word)
{
	Labels[Idx_Labels].Name = Word;

	int i = 0;
	while (Labels[i++].Name != Word)
		;
	i--;

	if (i == Idx_Labels)
		return -1;

	return i;
}

void Line_To_Words(string Line)
{
	char Word[50];
	const char Tab = 9;
	int i = 0;
	int j;
	unsigned int UC_Word_Idx = 0; // User Code

	while (Line[i])
	{
		while ((Line[i] == ' ') || (Line[i] == Tab))
			i++;

		if ((Line[i] != ' ') && (Line[i] != Tab) && (Line[i] != 0))
		{
			j = 0;
			while ((Line[i] != ' ') && (Line[i] != Tab) && (Line[i] != 0))
				Word[j++] = Line[i++];

			Word[j] = 0;

			string Upper;
			Upper = Str_Upper(Word);
			V_Words.push_back(Upper);

			if (Upper == "HLT")
				HLT_Line = UC_Line_Idx;

			V_Words_Idx++;

			if (UC_Word_Idx < 3)
				User_Codes[UC_Line_Idx][UC_Word_Idx] = Upper;
			else if (!Over_flow)
			{
				cerr << " ! Error-0 : Warning you can write 3 part in 1 line \n\n";
				Over_flow = true;
			}
			UC_Word_Idx++;
			n++;
		}
	}
}

string Str_Upper(string Str)
{
	string Finaly;
	for (unsigned int i = 0; i < Str.length(); i++)
		Finaly += toupper(Str[i]);

	return Finaly;
}

void Detect_Vars()
{
	for (size_t i = (HLT_Line + 1); i < UC_Line_Idx; i++)
	{
		if (Search_In_Commands(User_Codes[i][0]) == -1)
		{
			if (User_Codes[i][1] == "DAT")
			{
				if (Search_In_Vars(User_Codes[i][0]) == -1)
				{
					Vars[Idx_Vars].Name = User_Codes[i][0];

					int val;
					if (TryParse(User_Codes[i][2], val) == false)
						cerr << "\n ! Error-3 : in --->> Line [" << i + 1 << "] Illegal Value.!!!";
					else
						Vars[Idx_Vars].Value = val;
					Idx_Vars++;
				}
				else
					cerr << "\n ! Error-4 : The name " << User_Codes[i][0] << " has been used twice as a variable \n";
			}
			else if ((User_Codes[i][1] != "") && (User_Codes[i][0] != "")) // #Error
				cerr << "\n ! Error-1 : "
					 << "Line [" << i + 1 << "] has typographical errors --- (DAT)";
		}
		else // #Error
		{
			cerr << "\n ! Error-2 : You cannot select \"" << User_Codes[i][0] << "\" in line [" << i + 1
				 << "] as your variable name because it is one of the commands";
		}
	}
}

void Find_Labels()
{
	for (size_t i = 0; i <= HLT_Line; i++)
		if (User_Codes[i][0] != "")
			if (Search_In_Commands(User_Codes[i][0]) == -1)
			{
				if (Search_In_Labels(User_Codes[i][0]) == -1)
				{
					Labels[Idx_Labels].Name = User_Codes[i][0];
					Idx_Labels++;
				}
				else
					cerr << " ! Error-4 : The name " << User_Codes[i][0] << " has been used twice as a lable";
			}
}

bool TryParse(string str, int &x)
{
	bool Flag = true;
	x = 0;
	int i = 0;
	while ((i < str.length()) && ((str[i] >= 48) && (str[i] <= 57)))
		x = x * 10 + str[i++] - 48;

	if (i == str.length())
		return true;

	return false;
}

void readFile()
{
	// خواندن فابل و از بین بردن فاصله ها و تبدیل به حروف
	do
	{
		getline(Inp_File, Line);
		Line = Str_Upper(Line);

		Line_To_Words(Line);

		UC_Line_Idx++;

	} while (!Inp_File.eof());

	if (Inp_File.eof())
		cout << " Load your file successfuly :)" << endl
			 << " We were able to scan  [" << UC_Line_Idx << "] lines in your file\n";
}

void loadProgram()
{
	readFile();

	Detect_Vars();
	Find_Labels();
	Set_Lab_Var_Addr();

	int row = 0;
	int errorCode = 0;
	while ((errorCode == 0) && (row <= HLT_Line))
		errorCode = lineErrorCode(User_Codes[row++]);

	if (errorCode)
	{
		printErrorMessage(row, errorCode);
		exit(-1);
	}

	errorCode = Check_Lab_Var_Repetitious();
	if (errorCode != -1)
	{
		cout << "\n This Label --->> [" << Labels[errorCode].Name << "] is available in variables \n";
		exit(-1);
	}
}

int lineErrorCode(string line[3])
{
	if (line[0] == "")
		return 0;

	int col = 0;
	if (Search_In_Labels(line[col]) != -1)
		col++;

	int commandIdx = Search_In_Commands(line[col]);
	if (commandIdx != -1) // Command
	{
		int commandCode = Commands[commandIdx].Code;
		col++;
		if ((commandCode >= 1) && (commandCode <= 6)) //    یررسی وچود متغیر
		{
			if (Search_In_Vars(line[col]) == -1)
				return 2;
		}
		else if ((commandCode <= 11) && (commandCode >= 6)) //    یررسی وچود برچسب
		{
			if (Search_In_Labels(line[col]) == -1)
				return 3;
		}
		else
		{
			if (line[col] != "") //    وجود کلمه اضافی
				return 4;
		}
	}
	else
		return 1; // دستورغلط

	return 0;
}

void printErrorMessage(int row, int errorCode)
{
	cout << "\n\n ! Error-1 : Your --->>	 \"";
	switch (errorCode)
	{
	case 1:
		cout << "Command";
		break;
	case 2:
		cout << "Var";
		break;
	case 3:
		cout << "Label";
		break;
	case 4:
		cout << "Extera Word";
		break;
	}
	cout << "\"	<<---	in line [" << row << "] is invalid This error may be typed";
}

void Translator()
{
	int i = 0;
	for (size_t Row = 0; Row <= HLT_Line; Row++)
	{
		int col = 0, j = 0;
		if (User_Codes[Row][col] != "")
		{
			if (Search_In_Labels(User_Codes[Row][col]) != -1)
				col++;
			int returned_Index = Search_In_Commands(User_Codes[Row][col]);
			if (returned_Index != -1)
			{

				if (returned_Index < 11)
				{
					Ram[i][j++] = Commands[returned_Index].Code;
					col++;

					returned_Index = Search_In_Vars(User_Codes[Row][col]);
					if (returned_Index != -1)
					{
						Ram[i][j] = Vars[returned_Index].Address;
					}
					else
					{
						returned_Index = Search_In_Labels(User_Codes[Row][col]);

						Ram[i][j] = Labels[returned_Index].Address;
					}
				}
				else
				{
					Ram[i][j] = Commands[returned_Index].Code;
					//	Ram[i][j] = Commands[returned_Index].Code;
				}
			}

			i++;
		}
	}

	for (size_t Line = HLT_Line + 1; Line < UC_Line_Idx; Line++)
	{
		if (User_Codes[Line][0] != "")
			Ram[i][1] = Vars[Search_In_Vars(User_Codes[Line][0])].Value;
		i++;
	}
}

void Set_Lab_Var_Addr()
{
	int Line_Index = 0;
	for (size_t i = 0; i < UC_Line_Idx; i++)
	{
		int Lab_Var_Index;

		if (User_Codes[i][0] != "")
		{
			Lab_Var_Index = Search_In_Labels(User_Codes[i][0]);
			if (Lab_Var_Index != -1)
				Labels[Lab_Var_Index].Address = Line_Index;
			else
			{
				Lab_Var_Index = Search_In_Vars(User_Codes[i][0]);
				if (Lab_Var_Index != -1)
					Vars[Lab_Var_Index].Address = Line_Index;
			}

			Line_Index++;
		}

		// Line_Index dar in halghe baraye jodasazie khotoote khali az khotoote mofid mibashd;
	}
}

int Check_Lab_Var_Repetitious()
{
	bool Error = false;
	int i;
	for (i = 0; (i < Idx_Labels) && (!Error); i++)
		for (size_t j = 0; j < Idx_Vars; j++)
			if (Labels[i].Name == Vars[j].Name)
				Error = true;

	if (!Error)
		return -1;
	else
		return --i;
}

void printRam()
{
	Translator();
	for (size_t i = 0; i < 100; i++)
	{
		for (size_t j = 0; j < 2; j++)
		{
			if (Ram[i][0] == 0)
			{
				// ^^^^^^^^^^^^^^^^ in shart baraye chap nakardane 0 e ezafi ghabl az value e notaghayer hayee ast ke meghdar dehi shode and
				if ((Ram[i + 1][0] == 0) && (Ram[i - 1][0] != 0))
					// Tanha HLT in khasiat ra darad ke khane 1e ghablish to ram pore va khone 1 badish to ram khalie chon meghdare motaghayerharo
					// toye tabe translator dar sotone 2e ram ba andise 1 anjam mishe pas dar har sorat khone satre badie HLT sotone 1esh khalie
					cout << "0000";
				else
					cout << Ram[i][1];
				j++;
			}
			else if (Ram[i][0] > 100)
			{
				cout << Ram[i][0];
				j++;
			}
			else
			{
				if (Ram[i][j] < 10)
					cout << "0";

				cout << Ram[i][j];
			}
		}
		cout << " ";

		if ((i + 1) % 10 == 0)
			cout << "\n";
	}
}

void execute()
{
	PC = 0;
	while (Ram[PC][0])
	{
		switch (Ram[PC][0])
		{
		case 1:
			ACC += Ram[Ram[PC][1]][1];
			PC++;
			break;
		case 2:
			ACC -= Ram[Ram[PC][1]][1];
			PC++;
			break;
		case 3:
			ACC *= Ram[Ram[PC][1]][1];
			PC++;
			break;
		case 4:
			if (ACC)
				ACC = Ram[Ram[PC][1]][1] / ACC;
			else
				FLG = 1;
			PC++;
			break;
		case 5:
			Ram[Ram[PC][1]][1] = ACC;
			PC++;
			break;
		case 6:
			ACC = Ram[Ram[PC][1]][1];
			PC++;
			break;
		case 7:
			PC = Ram[PC][1];
			break;
		case 8:
			if (ACC == 0)
				PC = Ram[PC][1];
			else
				PC++;
			break;
		case 9:
			if (ACC >= 0)
				PC = Ram[PC][1];
			else
				PC++;
			break;
		case 10:
			if (ACC < 0)
				PC = Ram[PC][1];
			else
				PC++;
			break;
		case 11:
			if (FLG == 1)
				PC = Ram[PC][1];
			else
				PC++;
			break;
		case 1201:
			cout << " INPUT : ";
			cin >> ACC;
			PC++;
			break;
		case 1202:
			cout << ACC;
			PC++;
			break;
		}
	}
}