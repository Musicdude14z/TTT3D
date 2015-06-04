#include <iostream>
#include <vector>
#include <fstream>

using namespace std;

typedef unsigned long long ull;

ofstream out("wins.txt");

ostream &operator<<(ostream &o, vector<ull> v) {
	out << "{\n\t";
	for(ull t : v) {
		out << t << "ULL, ";
	}
	return out << "\n}"; //backspaces aren't working..
}

ull convert(int c_1, int c_2, int c_3, int c_4)
{
	ull u = 0;
	u += ((ull)1 << c_1);
	u += ((ull)1 << c_2);
	u += ((ull)1 << c_3);
	u += ((ull)1 << c_4);
	return u;
}

int main() {
	vector<ull> wins;
	wins.reserve(76);
	
	for(int i = 0; i <= 60; i += 4) //16 horizontal rows - OK
	{
		wins.push_back(convert(i, i + 1, i + 2, i + 3));
	}
	for(int i = 0; i <= 48; i += 16)
	{
		for(int j = i; j <= i + 3; ++j) //16 vertical rows - OK
		{
			wins.push_back(convert(j, j + 4, j + 8, j + 12));
		}
	}
	for(int i = 0; i <= 15; ++i) //16 out-facing rows - OK
	{
		wins.push_back(convert(i, i + 16, i + 32, i + 48));
	}
	for(int i = 0; i <= 48; i += 16) //8 outer-face diagonals - OK
	{
		wins.push_back(convert(i, i + 5, i + 10, i + 15));
		wins.push_back(convert(i + 3, i + 6, i + 9, i + 12));
	}
	for(int i = 0; i <= 3; ++i) //8 horizontal-face diagonals - OK
	{
		wins.push_back(convert(i, i + 20, i + 40, i + 60));
		wins.push_back(convert(i + 12, i + 24, i + 36, i + 48));
	}
	for(int i = 0; i <= 12; i += 4) //8 vertical-face diagonals - OK
	{
		wins.push_back(convert(i, i + 17, i + 34, i + 51));
		wins.push_back(convert(i + 3, i + 18, i + 33, i + 48));
	}
	wins.push_back(convert(0, 21, 42, 63)); //The 4 space diagonals
	wins.push_back(convert(3, 22, 41, 60));
	wins.push_back(convert(48, 37, 26, 15));
	wins.push_back(convert(12, 25, 38, 51));

	cout << wins << endl;

	return 0;
}
