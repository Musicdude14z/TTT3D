#include "ttt3d.h"
#include "AI_Test.cpp"
#include <iostream>
#include <cstring>
#include <cstdlib>
#include <cstdio>

using namespace std;

typedef unsigned long long ull;

const char WINS = 76;
constexpr ull wins[WINS] = {
	15ULL, 4369ULL, 281479271743489ULL, 240ULL, 8738ULL, 562958543486978ULL, 3840ULL, 17476ULL, 1125917086973956ULL, 61440ULL, 34952ULL, 2251834173947912ULL, 33825ULL, 4680ULL,
	1152922604119523329ULL, 281543712968704ULL, 2251816993685505ULL, 281483566907400ULL, 983040ULL, 286326784ULL, 4503668347895824ULL, 15728640ULL, 572653568ULL, 9007336695791648ULL, 
	251658240ULL, 1145307136ULL, 18014673391583296ULL, 4026531840ULL, 2290614272ULL, 36029346783166592ULL, 2216755200ULL, 306708480ULL, 2305845208239046658ULL, 563087425937408ULL,
	36029071898968080ULL, 4503737070518400ULL, 64424509440ULL, 18764712116224ULL, 72058693566333184ULL, 1030792151040ULL, 37529424232448ULL, 144117387132666368ULL, 16492674416640ULL,
	75058848464896ULL, 288234774265332736ULL, 263882790666240ULL, 150117696929792ULL, 576469548530665472ULL, 145277268787200ULL, 20100446945280ULL, 4611690416478093316ULL,
	1126174851874816ULL, 576465150383489280ULL, 72059793128294400ULL, 4222124650659840ULL, 1229764173248856064ULL, 1152939097061330944ULL, 67553994410557440ULL, 2459528346497712128ULL,
	2305878194122661888ULL, 1080863910568919040ULL, 4919056692995424256ULL, 4611756388245323776ULL, 17293822569102704640ULL, 9838113385990848512ULL, 9223512776490647552ULL, 
	9520891087237939200ULL, 1317302891005870080ULL, 9223380832956186632ULL, 2252349703749632ULL, 9223442406135828480ULL, 1152956690052710400ULL, 9223376434903384065ULL, 
	281612482805760ULL, 2252074725150720ULL, 1152923703634296840ULL
}, *winEnd = const_cast<ull *>(wins) + WINS; //not sure why it's forcing me to use a const_cast..

void print_edge(bool top = true) {
	for(int i = 0; i < 4; ++i) {
		cout << "   " << (top ? '.' : '\'');
		for(int j = 0; j < 15; ++j) {
			cout << '-';
		}
		cout << (top ? '.' : '\'');
	}              
	cout << '\n';
}

void print_hsep() {
	for(int i = 0; i < 4; ++i) {
		cout << "   " << '|';
		for(int j = 0; j < 3; ++j) {
			cout << "---" << '+';
		}
		cout << "---" << '|';
	}
	cout << '\n';
}

void print_board(ull P, ull E) {
	int pwr = 0;
	char s[4];
	print_edge();
	for(int z = 0; z < 4; ++z) {
		for(int y = 0; y < 4; ++y) {
			sprintf(s, "%3d", pwr);
			cout << s << '|';
			for(int x = 0; x < 4; ++x) {
				cout << ' ';
				if(P & 1)
					cout << 'X' << ' ';
				else if(E & 1)
					cout << 'O' << ' ';
				else {
					cout << ' ' << ' ';
				}
				cout << '|';
				P >>= 1;
				E >>= 1;
				++pwr;
			}
		}
		cout << '\n';
		if(z < 3) print_hsep();
	}
	print_edge(false);
}

void print_clock(TTT3D *X, TTT3D *O) {
	cout << "TIME USED FOR X: " << X->time_used() << "s\n" << "TIME USED FOR O: " << O->time_used() << "s\n";
}

bool check_win(bool X, ull s) {
	for(ull win : wins) {
		if(win & s == win) {
			cout << (X ? 'X' : 'O') << " WON!" << endl;
			return true;
		}
	}
	return false;
}

ull convert(int mv[3]) {
	return (1ULL) << ((mv[2]<<4) + (mv[1]<<2) + mv[0]);
}

//USE ONLY ON A CHILD OF TTT3D
template <class T>
void reset_AI(T *a, duration<double> length) {
	delete a;
	a = new T(length);
	a->init_clock();
}

//simulates game between X and O (Where X moves first)
//resets players so their clocks are new
//1 if X won, -1 if O won, and 0 if draw
//SHOULD ONLY BE USED WITH CHILDREN OF TTT3D
template <class T, class K>
int sim(T *X, K *O, duration<double> length) {
	int mv[3] = {-1,-1,-1};
	ull P = 0, E = 0;
	
	//reset players
	reset_AI(X, length);
	reset_AI(O, length);
	cout << "Initializaton Complete" << endl;
	
	print_board(P, E);
	print_clock(X, O);
	for(int i = 0; i < 32; ++i) {
		X->sqzzl(mv);
		P |= convert(mv);
		print_board(P, E);
		print_clock(X, O);
		if(check_win(true, P))
			return 1;

		O->sqzzl(mv);
		E |= convert(mv);
		print_board(P, E);
		print_clock(X, O);
		if(check_win(false, E))
			return -1;
	}
	cout << "DRAW!!" << endl;
	return 0;
}

int main() {
    auto length = minutes(3);

    //example call MAKE SURE YOU STORE THE VARIABLES IN THEIR OWN TYPES, NOT TTT3D. IT WILL NOT WORK 
    //
    //AI_TYPE1 *ai1;
    //AI_TYPE2 *ai2;
    //sim(ai1, ai2, length);
    
    MZ::MZ *test1 = new MZ::MZ(length);
    MZ::MZ *test2 = new MZ::MZ(length);
    sim(test1, test2, length);

    return 0;
}
