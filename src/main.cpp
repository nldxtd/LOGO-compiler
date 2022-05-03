//--------------------------------------------------
//headers

#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <string>
#include <cmath>
#include <cstring>
#include <sstream>
#pragma pack(2) //alligning coordinate
using namespace std;

//--------------------------------------------------
//declarations



//--------------------------------------------------
//definitions

typedef unsigned char  BYTE; //1 byte
typedef unsigned short WORD; //2 bytes
typedef unsigned int  DWORD; //4 bytes
typedef int    LONG; //4 bytes

string prog[1000]; //maximum of lines in .logo: 1000
string varDic[15]; //maximum of vars: 15, distribute an int (0 ~ 14) to each var
double varTbl[15]; //values of each var according to the dictionary
string funcDic[10]; //maximum of funcs: 10, distribute an int (0 ~ 9) to each func
int funcTbl[10][2]; //[start, end)of each func according to the dictionary
string paraDic[10][3]; //maximum of paras for each func: 3
int numofpara[10];
double paravalue[10][3];
int funcnum = 0;
int varnum = 0;
int infunc = -1;
int line = 0;
bool looped = false;
bool funced = false;

int tsize = 0;
int mapwidth = 0;
int mapheight = 0;
int penwidth = 1;
bool cloaked = false;
double x = 0.0;
double y = 0.0;
double a = 0.0; //argument, define y++ as the positive direction, clockwise a++
double pi = 3.14159265358979;
unsigned char r = 0;
unsigned char g = 0;
unsigned char b = 0;

struct  BITMAPFILEHEADER {
	WORD    bfType;
	DWORD   bfSize;
	WORD    bfReserved1;
	WORD    bfReserved2;
	DWORD   bfOffBits;
};

struct BITMAPINFOHEADER {
	DWORD      biSize;
	LONG       biWidth;
	LONG       biHeight;
	WORD       biPlanes;
	WORD       biBitCount;
	DWORD      biCompression;
	DWORD      biSizeImage;
	LONG       biXPelsPerMeter;
	LONG       biYPelsPerMeter;
	DWORD      biClrUsed;
	DWORD      biClrImportant;
};

BITMAPFILEHEADER fileHeader = { 0 };
BITMAPINFOHEADER bitmapHeader = { 0 };

void interprete_prog(BYTE*,int,int);

//--------------------------------------------------
//basic functions

int toint(double z) {
	int zz = z;
	if (z >= 0) {
		if (z - zz < 0.5) return zz;
		else return zz + 1;
	}
	else {
		if (zz - z < 0.5) return zz;
		else return zz - 1;
	}
}

int get_index(double cx, double cy) {
	int intx = toint(cx); int inty = toint(cy);
	while (intx >= mapwidth) intx -= mapwidth;
	while (intx < 0) intx += mapwidth;
	while (inty >= mapheight) inty -= mapheight;
	while (inty < 0) inty += mapheight;
	int i = 0;
	i = (inty * mapwidth + intx) * 3;
	return i;
}

double periodize(double rad) {
	if ((rad >= 0.0) && (rad < 2 * pi))
		return rad;
	else {
		double pa = rad;
		if (pa < 0) {
			pa += 2 * pi;
			return periodize(pa);
		}
		else {
			pa -= 2 * pi;
			return periodize(pa);
		}
	}
}

double get_rad(double angle) {
	double rad = angle * pi / 180.0;
	return rad;
}

double get_value(string& str) {
	double value; 
	if (((int)str[0] >= 97) && ((int)str[0] <= 122)) {
		if (infunc >= 0) {
			int i = 0; //cout << str.c_str() << endl << paraDic[infunc][0] << endl;
			//cout << strcmp(str.c_str(), paraDic[infunc][i].c_str()) << endl;
			//cout << (int)str[4] << " " << (int)paraDic[0][0][4] << endl;
			while (i < numofpara[infunc]) {
				if (strcmp(str.c_str(), paraDic[infunc][i].c_str()) == 0) {
					//cout << paravalue[infunc][i] << endl;
					return paravalue[infunc][i]; 
				}
				i++;
			}
		}
		int i = 0;
		while (i < varnum) {
			if (strcmp(str.c_str(), varDic[i].c_str()) == 0) {
				//cout << varTbl[i];
				return varTbl[i]; 
			}
			i++;
		}
		cout << "[ERROR] Undefined variable(paramount) involved! Please check. :(" << endl << endl;
		exit(103);
	}
	else {
		stringstream ss;
		ss << str;
		ss >> value;
		ss.clear(); //cout << value << endl;
		return value;
	}
}

bool cmpcolor(BYTE* bits, int x, int y, int a, int b, int c) {
	if ((int)bits[get_index((double)x, (double)y)] == a) {
		if ((int)bits[get_index((double)x, (double)y) + 1] == b) {
			if ((int)bits[get_index((double)x, (double)y) + 2] == c) {
				return true;
			}
			else return false;
		}
		else return false;
	}
	else return false;
}

//--------------------------------------------------
//functions

void def(string& name, double value) {
	for (int i = 0; i < varnum; i++) {
		if (name == varDic[i]) {
			varTbl[i] = value; return;
		}
	}
	if (varnum == 15) {
		cout << "[ERROR] Too many variables! :(" << endl << endl;
		exit(104);
	}
	varDic[varnum] = name;
	varTbl[varnum] = value;
	varnum++;
	return;
}

void add(string& var, double value) {
	for (int i = 0; i < varnum; i++) {
		if (var == varDic[i]) {
			varTbl[i] += value;
			return;
		}
	}
	cout << "[ERROR] Undefined variable(paramount) involved! Please check. :(" << endl << endl;
	exit(103);
}

void move(double step, BYTE* bits) {
	if (cloaked == false) {
		double move;
		double nowx, nowy;
		for (move = 0; move <= step; move += 0.5) {
			nowx = x + move * sin(a);
			nowy = y + move * cos(a);
			int wi = penwidth / 2;
			//cout << step << endl;
			//cout << nowx << " " << nowy << endl;
			for (double i = -wi; i <= wi; i += 0.5) {
				double drawx = nowx + i;
				for (double j = -wi; j <= wi; j += 0.5) {
					double drawy = nowy + j;
					bits[get_index(drawx, drawy)] = b;
					bits[get_index(drawx, drawy) + 1] = g;
					bits[get_index(drawx, drawy) + 2] = r;
					//cout << a << " " << drawx << " " << drawy << " " << i << endl;
				}
			}
		}
	}
	x += step * sin(a);
	y += step * cos(a);
	//cout << x << " " << y << endl;
	return;
}

void cmove(double step, BYTE* bits) {
	if (cloaked == false) {
		double move;
		double nowx, nowy;
		for (move = 0; move <= step; move += 0.5) {
			nowx = x + move * sin(a);
			nowy = y + move * cos(a);
			int wi = penwidth / 2;
			//cout << step << endl;
			//cout << nowx << " " << nowy << endl;
			for (double i = -wi; i <= wi; i += 0.5) {
				double drawx = nowx + i * cos(a);
				double drawy = nowy - i * sin(a);
				bits[get_index(drawx, drawy)] = b;
				bits[get_index(drawx, drawy) + 1] = g;
				bits[get_index(drawx, drawy) + 2] = r;
				//cout << a << " " << drawx << " " << drawy << " " << i << endl;
			}
		}
	}
	x += step * sin(a);
	y += step * cos(a);
	//cout << x << " " << y << endl;
	return;
}

void turn(double angle) {
	a = periodize(a + get_rad(angle));
	return;
}

void cloak() {
	if (cloaked)
		cloaked = false;
	else
		cloaked = true;
	return;
}

void color(int sr, int sg, int sb) {
	cloaked = false;
	r = sr;
	g = sg;
	b = sb;
	return;
}

void loop(BYTE* bits, int i, int j, int num) {
	for (int l = 1; l <= num; l++) {
		interprete_prog(bits, i, j);
	}
}

void function(BYTE* bits, int i, int j, int fnum) {	
	int tmp = infunc;
	infunc = fnum; //cout << infunc << endl;
	interprete_prog(bits, i, j);
	infunc = tmp;
	return;
}

void fillone(BYTE* bits, int x, int y, int b, int g, int r) {
	bits[get_index((double)x, (double)y)] = b;
	bits[get_index((double)x, (double)y) + 1] = g;
	bits[get_index((double)x, (double)y) + 2] = r;
}

void fillall(BYTE* bits, int pb, int pg, int pr, int xi, int yi, int nb, int ng, int nr) {
	if (cmpcolor(bits, xi, yi, pb, pg, pr)) {
		fillone(bits, xi, yi, nb, ng, nr);
	}

	int i = yi + 1;
	for (; i < 1080; i++) {
		if (cmpcolor(bits, xi, i, pb, pg, pr)) {
			fillone(bits, xi, i, nb, ng, nr);
		}
		else break;
	}

	int j = yi - 1;
	for (; j >= 0; j--) {
		if (cmpcolor(bits, xi, j, pb, pg, pr)) {
			fillone(bits, xi, j, nb, ng, nr);
		}
		else break;
	}

	for (int k = j + 1; k < i; k++) {
		if (xi + 1 < 1920) {
			if (cmpcolor(bits, xi + 1, k, pb, pg, pr)) {
				fillall(bits, pb, pg, pr, xi + 1, k, nb, ng, nr);
			}
		}
		if (xi - 1 >= 0) {
			if (cmpcolor(bits, xi - 1, k, pb, pg, pr)) {
				fillall(bits, pb, pg, pr, xi - 1, k, nb, ng, nr);
			}
		}
	}
}

void greylize(BYTE* bits, int wch, int zzy) {
	int ndy = wch * zzy;
	for (int i = 0; i < ndy; i++) {
		int average_color = (bits[i] + bits[i + 1] + bits[i + 2]) / 3;
		bits[i] = average_color;
		bits[i + 1] = average_color;
		bits[i + 2] = average_color;
	}
	return;
}

//--------------------------------------------------
//operations

void save_bmp(BYTE* bits) {
	cout << "[Triangle] Output ready. End by entering output file address (with 'FILENAME.bmp' in the end):" << endl << endl;
	string add;
	cout << "[Output File] ";
	cin >> add;
	FILE* output = fopen(&add[0], "wb");
	if (output == NULL)
	{
		cout << endl << "[ERROR] File save failed! :(" << endl << endl;
		exit(101);
	}
	else
	{
		fwrite(&fileHeader, sizeof(BITMAPFILEHEADER), 1, output);
		fwrite(&bitmapHeader, sizeof(BITMAPINFOHEADER), 1, output);
		fwrite(bits, tsize, 1, output);
		fclose(output);
		cout << endl << "[Triangle] Done! You can check the output now. :)" << endl << endl;
	}
	return;
}

void interprete_prog(BYTE* bits, int start, int end) { //interprete codes in between [start, end)
	int i;
	string para0, para1, para2, para3;
	stringstream ss;
	string temp;
	//cout << r << " " << g << " " << b << endl << endl;
	for (i = start; i < end; i++) {
		if ((int)prog[i][0] == 0) continue;
		else {
			ss << prog[i];
			ss >> temp; //cout << "i: " << i << " " << temp << endl;
			if (strcmp(temp.c_str(), "DEF") == 0) {
				ss >> para0 >> para1;
				def(para0, get_value(para1));
			}
			else if (strcmp(temp.c_str(), "ADD") == 0) {
				ss >> para0 >> para1;
				add(para0, get_value(para1));
			}
			else if (strcmp(temp.c_str(), "MOVE") == 0) {
				ss >> para0;
				move(get_value(para0), bits);
			}
			else if (strcmp(temp.c_str(), "CURVEMOVE") == 0) {
				ss >> para0;
				cmove(get_value(para0), bits);
			}
			else if (strcmp(temp.c_str(), "TURN") == 0) {
				ss >> para0;
				turn(get_value(para0));
			}
			else if (strcmp(temp.c_str(), "CLOAK") == 0) {
				cloak();
			}
			else if (strcmp(temp.c_str(), "COLOR") == 0) {
				ss >> para0 >> para1 >> para2;
				color(get_value(para0), get_value(para1), get_value(para2));
			}
			else if (strcmp(temp.c_str(), "LOOP") == 0) {
				string loopnum; int inloop = 1; ss >> loopnum; bool goloop = true;
				int next = 0; looped = true;
				for (int j = i + 1; j < end; j++) {
					stringstream sss; string getend;
					if ((int)prog[j][0] == 0) continue;
					else {
						sss << prog[j]; sss >> getend; //cout << "inloop " << getend << endl;
						if (getend == "END") {
							sss >> getend;// cout << getend << endl;
							if (getend == "LOOP") {
								inloop--;
							}
						}
						else if (getend == "LOOP") {
							inloop++;
						}
						if (inloop == 0 && goloop) {
							loop(bits, i + 1, j, get_value(loopnum)); goloop = false;
							next = j;
						}
					}
				}
				if (inloop != 0) {
					cout << "LOOP!=END LOOP" << endl;
					exit(106);
				}
				i = next;
			}
			else if (strcmp(temp.c_str(), "FUNC") == 0) {
				string funcname; ss >> funcname; int len = strlen(funcname.c_str()); int paranum = 0;//funcname[len-1]==')'
				//cout << funcname << " " << len << endl;
				funced = true;
				for (int k = 0; k < len; k++) {
					if (funcname[k] == '(') {
						if (k >= 15) {
							cout << "function name too long, can't remember :(" << endl;
							exit(112);
						}
						char name[15];
						for (int j = 0; j < k; j++) {
							name[j] = funcname[j];
						}
						name[k] = '\0';
						stringstream str; string dic; str << name; str >> dic; 
						for (int m = 0; m < funcnum; m++) {
							if (dic == funcDic[m]) {
								cout << "multiple function definition :(";
								exit(107);
							}
						}

						if (funcnum >= 10) {
							cout << "too many functions :(" << endl;
							exit(110);
						}

						funcDic[funcnum] = dic;  funcTbl[funcnum][0] = i + 1; 

						int paraleft = k + 1; //cout << paraleft << endl;
						for (int j = k + 1; j < len; j++) {
							if (funcname[j] == ',' || funcname[j] == ')') {
								if (funcname[j] == ')' && paranum == 0 && j == k + 1) break;
								if (j - paraleft >= 15) {
									cout << "paramount name too long :(" << endl;
									exit(113);
								}
								char paraname[15];
								for (int l = paraleft; l <= j - 1; l++) {
									paraname[l - paraleft] = funcname[l];
								}
								paraname[j - paraleft] = '\0'; paraleft = j + 1;
								stringstream s1; string dic1;
								s1 << paraname; s1 >> dic1;
								paraDic[funcnum][paranum] = dic1;//cout << dic1 << endl;
								paranum++; //cout << paranum << endl;
								if (paranum > 3) {
									cout << "too many paramounts :(" << endl;
									exit(109);
								}
							}
						}
						numofpara[funcnum] = paranum; //cout << numofpara[funcnum] << endl;
						break;
					}
				}
				int next = 0, infunction = 1; bool gofunc = true;
				for (int j = i + 1; j < end; j++) {
					//cout << gofunc << endl;
					if ((int)prog[j][0] == 0) continue; 
					else {
						stringstream sss; string getend;
						sss << prog[j]; sss >> getend; //cout << "infunc " << getend << endl;
						if (getend == "END") {
							sss >> getend;// cout << getend << endl;
							if (getend == "FUNC") {
								infunction--; //cout << infunction << "-" << " j: " << j << endl;
							}
						}
						else if (getend == "FUNC") {
							infunction++; //cout << infunction << "+" << " j: " << j << endl;
						}

						if (infunction == 0 && gofunc) {
							funcTbl[funcnum][1] = j; next = j;
							gofunc = false; 
						}
					}
				}
				if (infunction != 0) {
					cout << "FUNC!=END FUNC" << endl; //cout << infunction << endl;
					exit(105);
				}
				funcnum++; i = next;//cout << "next: " << next << endl;
			}
            else if (strcmp(temp.c_str(), "CALL") == 0) {
				string funcname; ss >> funcname; int len = strlen(funcname.c_str()); int paranum = 0;//funcname[len-1]==')'
				int funcn = 0;
				for (int k = 0; k < len; k++) {
					if (funcname[k] == '(') {
						char name[15];
						for (int j = 0; j < k; j++) {
							name[j] = funcname[j];
						}
						name[k] = '\0';
						stringstream str; string dic; str << name; str >> dic; //cout << dic << endl;
						bool getfunc = false;
						while (funcn < funcnum && funcDic[funcn][0] != 0) {
							//cout << funcn << endl;
							if (dic == funcDic[funcn]) {
								getfunc = true; break;
							}
							funcn++;
						}
						if (!getfunc) {
							cout << "undefined function :(" << endl;
							exit(108);
						}

						int paraleft = k + 1;
						for (int j = k + 1; j < len; j++) {
							if (funcname[j] == ',' || funcname[j] == ')') {
								char paraname[15];
								for (int l = paraleft; l <= j - 1; l++) {
									paraname[l - paraleft] = funcname[l];
								}
								paraname[j - paraleft] = '\0'; paraleft = j + 1;
								stringstream s1; string dic1;
								s1 << paraname; s1 >> dic1;
								paravalue[funcn][paranum] = get_value(dic1); //cout << dic1 << endl;
								paranum++;
							}
						}
						function(bits, funcTbl[funcn][0], funcTbl[funcn][1], funcn);
						break;
					}
				}
			}
			else if (strcmp(temp.c_str(), "END") == 0) {
				ss >> para0;
				if (para0 == "LOOP" && !looped) {
					cout << "LOOP!=END LOOP" << endl;
					exit(106);
				}
				if (para0 == "FUNC" && !funced) {
					cout << "FUNC!=END FUNC" << endl;
					exit(105);
				}
			}
			else if (strcmp(temp.c_str(), "PENWIDTH") == 0) {
				ss >> penwidth;
			}
			else if (strcmp(temp.c_str(), "FILL") == 0) {
				int  fb, fg, fr; ss >> fb >> fg >> fr;
				int p1 = bits[get_index(x, y)];
				int p2 = bits[get_index(x, y) + 1];
				int p3 = bits[get_index(x, y) + 2];
				fillall(bits, p1, p2, p3, toint(x), toint(y), fr, fg, fb);
			}
			else if (strcmp(temp.c_str(), "GREY") == 0) {
				greylize(bits, mapwidth, mapheight);
			}
			ss.clear();
		}
	}

	
	/*for (int i = 0; i < tsize; i++) {
		cout << (int)bits[i] << " ";
	}*/

	return;
}

void initialize_bitmap(int w, int h, int ir, int ig, int ib, int ix, int iy, int line, int lineHead) {
	cout << "[Triangle] Initializing..." << endl;
	tsize = w * h * 3;
	BYTE* bits = (BYTE*)malloc(tsize);
	for (int i = 2; i < tsize; i += 3)
		bits[i] = ir;
	for (int i = 1; i < tsize; i += 3)
		bits[i] = ig;
	for (int i = 0; i < tsize; i += 3)
		bits[i] = ib;
	x = ix;
	y = iy;
	fileHeader.bfType = 0x4D42;
	fileHeader.bfReserved1 = 0;
	fileHeader.bfReserved2 = 0;
	fileHeader.bfSize = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + tsize;
	fileHeader.bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);
	bitmapHeader.biSize = sizeof(BITMAPINFOHEADER);
	bitmapHeader.biWidth = w;
	bitmapHeader.biHeight = h;
	bitmapHeader.biPlanes = 1;
	bitmapHeader.biBitCount = 24;
	bitmapHeader.biSizeImage = tsize;
	bitmapHeader.biCompression = 0;
	cout << "[Triangle] Done!" << endl << endl;
	cout << "[Triangle] Interpreting..." << endl;
	interprete_prog(bits, lineHead, line);
	cout << "[Triangle] Done!" << endl << endl;

	save_bmp(bits);
	return;
}

void read_logo_file() {
	for (int i = 0; i < 1000; i++) prog[i] = "\0";
	for (int i = 0; i < 15; i++) varDic[i] = "\0";
	for (int i = 0; i < 10; i++) funcDic[i] = "\0";
	for (int i = 0; i < 10; i++) for (int j = 0; j < 3; j++) paraDic[i][j] = "\0";
	funcnum = 0;
	varnum = 0;
	infunc = -1;
	line = 0;
	looped = false;
	funced = false;
	penwidth = 1;
	cloaked = false;
	a = 0.0;
	r = 0;
	g = 0;
	b = 0;
	cout << "[Triangle] Greetings. Start by entering input .logo file address:" << endl << endl;
	ifstream logo;
	char add[100];
	cout << "[Input File] ";
	cin >> add;//file address
	logo.open(add);
	int lineHead = 0;
	int w = 0, h = 0, ir = 0, ig = 0, ib = 0, ix = 0, iy = 0;
	bool getSize = false, getBackground = false, getPosition = false;
	if (logo) {
		cout << endl << "[Triangle] File open successful. Scanning program..." << endl;
		while (!logo.eof()) {
			if (line >= 1000) {
				cout << "program too long, can't handle :(" << endl;
				exit(111);
			}
			getline(logo, prog[line]);			
			line++;			
		}
		cout << "[Triangle] Done!" << endl << endl << "[Triangle] Reading header..." << endl;
		for (lineHead = 0; prog[lineHead][0] == '@'; lineHead++) {
			stringstream ss;
			string temp;
			ss << prog[lineHead];
			ss >> temp;
			if (strcmp(temp.c_str(), "@SIZE") == 0) {
				ss >> w >> h; mapwidth = w; mapheight = h;
				getSize = true;
			}
			else if (strcmp(temp.c_str(), "@BACKGROUND") == 0) {
				ss >> ir >> ig >> ib;
				getBackground = true;
			}
			else if (strcmp(temp.c_str(), "@POSITION") == 0) {
				ss >> ix >> iy;
				getPosition = true;
			}
			ss.clear();
		}
		if ((getSize + getBackground + getPosition) == 3)
			cout << "[Triangle] Done!" << endl << endl;
		else {
			cout << "[ERROR] Header missing! Please check. :(" << endl << endl;
			exit(102);
		}
	}
	else {
		cout << endl << "[ERROR] File open failed! :(" << endl << endl;
		exit(100);
	}
	logo.close();
	initialize_bitmap(w, h, ir, ig, ib, ix, iy, line, lineHead);
	return;
}

//--------------------------------------------------
//main

int main() {
	int choice;
	cout << "[Triangle] Greetings, repeatedly press 1 to translate 0 to exit:)" << endl;
	do {
		cin >> choice;
		switch (choice)
		{
		case 1:read_logo_file();
		case 0:break;
		}
	} while (choice != 0);
	cout << "[Triangle] Thanks for using, see you";
	return 0;
}

//--------------------------------------------------
//errorcodes

// 100 - input file not found
// 101 - output file write failed
// 102 - missing one or more header lines
// 103 - using or adding undefined variable(paramount)
// 104 - variables limit exceeded
// 105 - more or fewer END FUNC than FUNC
// 106 - more or fewer END LOOP than LOOP
// 107 - mutiple function definition
// 108 - undefined function called
// 109 - paramount limit exceeded
// 110 - functions limit exceeded
// 111 - program too long
// 112 - function name too long
// 113 - paramount name too long




//--------------------------------------------------

/*

/Users/niedingyi/Desktop/test.logo
/Users/niedingyi/Desktop/output.bmp

*/
