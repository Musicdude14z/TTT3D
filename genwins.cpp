#include <iostream>
#include <vector>

using namespace std;

typedef unsigned long long ull;

ostream &operator<<(ostream &o, vector<ull> v) {
	cout << "{\n\t";
	for(ull t : v) {
		cout << t << "ULL, ";
	}
	return cout << "\b\b\n}"; //backspaces aren't working..
}

ull get_const(int x, int y, int z) {
	return (1ULL) << (x + (y << 2) + (z << 4));
}

int main() {
	vector<ull> wins;
	wins.reserve(76);
	
	ull winxyz = 0, winxyZ = 0, winxYz = 0, winxYZ = 0;
	for(int z = 0; z < 4; ++z) {
		ull winxy = 0, winxY = 0, winyz = 0, winyZ = 0, winzx = 0, winzX = 0;

		for(int y = 0; y < 4; ++y) {
			//Row Stuff
			ull winx = 0, winy = 0, winz = 0;

			for(int x = 0; x < 4; ++x) {
				winx |= get_const(x, y, z); //yz const while x varies
				winy |= get_const(y, x, z); //xz const while y varies
				winz |= get_const(y, z, x); //xy const while z varies
			}
			wins.push_back(winx);
			wins.push_back(winy);
			wins.push_back(winz);
			
			//Face Diagonal Stuff
			//z is slicing plane, y cross +/- y is the diagonal in plane
			winxy |= get_const(y, y, z);
			winxY |= get_const(y, 4-y, z);
			
			winyz |= get_const(z, y, y);
			winyZ |= get_const(z, y, 4-y);
			
			winzx |= get_const(y, z, y);
			winzX |= get_const(y, z, 4-y);
		}
		wins.push_back(winxy);
		wins.push_back(winxY);
		wins.push_back(winyz);
		wins.push_back(winyZ);
		wins.push_back(winzx);
		wins.push_back(winzX);

		//Spatial Daigonal Stuff
		winxyz |= get_const(z, z, z);
		winxyZ |= get_const(z, z, 4-z);
		winxYz |= get_const(z, 4-z, z);
		winxYZ |= get_const(z, 4-z, 4-z);
	}
	wins.push_back(winxyz);
	wins.push_back(winxyZ);
	wins.push_back(winxYz);
	wins.push_back(winxYZ);

	cout << wins << endl;

	return 0;
}
