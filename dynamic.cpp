#include <iostream>
#include <cstdio>
#include <cstring>
#include <limits>

using namespace std;

typedef unsigned long long ull;

struct box {
	//const static values for determing score (as of now P1 is 1 more than E2, this is to, for example, prioritize creating a fork over blocking the creation of one)
	static constexpr uint32_t
		MIN = 0,
		MAX = numeric_limits<uint32_t>::max(),
		MID = MAX / 2,
		P1 = MID / 7,		//notice P1 > P2 but 2(P2) > P1
		P2 = P1 * 3 / 4,
		E2 = MID / 7 - 1,	//notice E1 << E2 (specifically 7(E1) < E2)
		E1 = E2 / 7 - 1;

	box() : p(0), e(0), numP1(0), numP2(0), numE1(0), numE2(0), loss(0), pos(0), score(0) {}
	box(const box &b) : p(b.p), e(b.e), numP1(b.numP1), numP2(b.numP2), numE1(b.numE1), numE2(b.numE2), loss(b.loss), pos(b.pos), score(b.score) {}
	box &operator=(const box &b) {	//this is not a true copy, just what's needed (should only be used in bucket sort)
		loss = b.loss; //probably not needed
		pos = b.pos;
		score = b.score;
	}
	
	void reset(unsigned char pos) {
		p = e = numP1 = numP2 = numE1 = numE2 = loss = 0;
		score = MID;
		this->pos = pos;
	}

	unsigned char
		p     : 1,	//1 if a player is in the spot, else 0
		e     : 1,	//-------enemy -----------------------
		numP1 : 3,	//the number of lines going through this point that contain exactly one player piece
		numP2 : 3,	//------------------------------------------------------------------two-------------s
		numE1 : 3,	//------------------------------------------------------------------one enemy ------
		numE2 : 3,	//------------------------------------------------------------------two-------------s
		loss  : 1,	//0 if normal box, but 1 if moving here will avert a loss
		      : 1,	//padding bit to avoid issues with byte borders
		pos;		//holds position in array, important after sorting (0-63, but letting be full size to avoid masking commands)
	uint32_t score : 32;	//score, higher is better (using uint32_t because I can sort by this key quickly using bucket sort)
};

//index in the array represents power of 2 in ULL format we're using for the board
box scores[64] = {}, temp[64] = {}, *scoresEnd= scores + 64, *te = temp + 64;

void reset_all(box *s, box *e) {
	for(unsigned char i = 0; s < e; ++s, ++i)
		s->reset(i);
}

//specialized and messy
void bucket_sort(box *s, box *e) {
	uint32_t buckets[0x10001] = {}, *be = buckets + 0x10001, *b;
	box *p, *t;

	for(p = s; p != e; ++p) //count frequency
		++buckets[1 + (p->score & 0xFFFF)];

	for(b = buckets + 1; b != be; ++b) //make list of indexes
		*b += *(b-1);

	for(p = s; p != e; ++p) //sort once
		temp[buckets[p->score & 0xFFFF]++] = *p;
	
	memset(buckets, 0, 0x10001 * 4); //reset bucket
	
	for(t = temp; t != te; ++t)
		++buckets[1 + (t->score >> 16)];

	for(b = buckets + 1; b != be; ++b)
		*b += *(b-1);

	for(t = temp; t != te; ++t)
		s[buckets[t->score >> 16]++] = *t; //this line requires random access iterator
}

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


//Modifies scores
//Scores will be in reverse order of importance, search as deep as you want, start from the back
//IF RETURN IS TRUE:
//	Tail of scores is a move that will win the game or attempt to save the game (avert loss), just move there
//ELSE:
//	Scores is sorted in increasing score order, essentially best moves come last (pos in each box is the bit shift required to get move)
bool eval(ull P, ull E) {
	box *pts[4], **p; 	// pointers to the four boxes in the line, **p is an iterator
	ull cp_p, cp_e;		// holds the combinations of each line and current board state for player and enemy
	char pwr, np, ne;	// holds the power of the current shift, the number of players in the line, and the number of enemies in the line respectively
	reset_all(scores, scoresEnd);

	for(ull win : wins) {
		cp_p = P;
		cp_e = E;
		p = pts;
		pwr = np = ne = 0;
		
		//to account for some spaces being geometrically better, we will add one to each space each time we go over it

		while(win > 0) {
			if(win & 1) {
				*p = scores + pwr;
				if((*p)->loss) //shouldn't tamper with these
					goto skip;

				if(cp_p & 1) {
					(*p)->p = 1;
					(*p)->score = box::MIN;
					++np;
				} else if(cp_e & 1) {
					(*p)->e = 1;
					(*p)->score = box::MIN;
					++ne;
				} else {
					++((*p)->score);
				}
skip:
				++p;
			}
			++pwr;
			win >>= 1;
			cp_p >>= 1;
			cp_e >>= 1;
		}

		for(box *pt : pts) {
			if(pt->p || pt->e || pt->loss) //if a piece is here or if already loss aversion state, no need to adjust score or any other values
				continue;
			//if(pt->pos == 4)
			//	cout << (int)np << ", " << (int)ne << endl;
			if(np) {
				if(!ne) {	//just players in line
					switch(np) {
					case 1:
						if(!pt->numP2 && !pt->numP1)	//first helpful line through this box
							pt->score += box::P1;
						else if(pt->numP2 < 2) 		//either 1 or 0 (if 0 then numP1 > 0), so it's an intersection of 1 and 1 or 1 and 2 but not 2 and 2
							pt->score -= box::P2;
						else				//numP2 >= 2 so this is a good move (creates a fork and a 2 in this line)
							pt->score += box::P2;

						++(pt->numP1);
						break;
					case 2:
						if(!pt->numP2 && !pt->numP1)	//first helpul line through this box
							pt->score += box::P2;
						else if(pt->numP2)		//FORKKK!!!!
							pt->score += box::P1 + box::P1;
						else				//Intersection of 2 and 1
							pt->score -= box::P2;

						++(pt->numP2);
						break;
					case 3:	//WE WON!!!!!!!
						scores[63] = *pt;
						return true;
					}
				}		//else is both, but nothing to do
			} else if (ne) {	//just enemies in line
				switch(ne) {
				case 1:
					pt->score += box::E1;
					++(pt->numE1);
					break;
				case 2:
					if(pt->numE2 < 2) //cap out enemy score contribution after fork of two (fork of three etc. is just as dangerous, nothing special)
						pt->score += box::E2;
					++(pt->numE2);
					break;
				case 3:	//This could avert a loss, keep looking though in case there is a win state 
					pt->score = box::MAX;
					pt->loss = 1;
				}
			}			//else is neither, but will never happen (taken care of above)
		}
	}

	bucket_sort(scores, scoresEnd);
	return scores[0].loss;
}

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

void print_board(ull P, ull E, int num = 0) {
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
					bool print = false;
					for(int i = 1; i <= num; ++i)
						if(scores[64 - i].pos == pwr) {
							sprintf(s, "%2d", i);
							cout << s;
							print = true;
							break;
						}
					if(!print) 
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

//good for checking that wins generated correctly
void print_wins() {
	for(ull win : wins) {
		print_board(win, 0);
	}
}

void print_scores() {
	for(box b : scores) {
		cout << b.score << " #" << (int)b.pos << endl;
	}
}

int main() {
	ull P = (1ULL) << 1, E = 0;
	print_board(P, E);

	eval(E, P);
	E |= (1ULL) << scores[63].pos;
	print_board(P, E);

	for(int i = 0; i < 31; ++i) {
		eval(P, E);
		P |= (1ULL) << scores[63].pos;
		
		//print_scores();

		print_board(P, E, 3);
		for(ull win : wins) {
			if((P & win) == win) {
				cout << "Player 'X' won!" << endl;
				print_board(win, 0);
				return 0;
			}
		}

		eval(E, P);
		E |= (1ULL) << scores[63].pos;
		
		//print_scores();
		
		print_board(P, E, 3);
		for(ull win : wins) {
			if((E & win) == win) {
				cout << "Player 'O' won!" << endl;
				print_board(0, win);
				return 0;
			}
		}
	}
	cout << "Draw!!" << endl;
	return 0;
}
