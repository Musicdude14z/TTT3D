#include <iostream>
#include <cstdlib>
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
	box &operator=(const box &b) {	//this is not a true copy, just what's needed (should only be used in bucket sort)
		loss = b.loss; //probably not needed
		pos = b.pos;
		score = b.score;
	}
	
	void reset(unsigned char pos) {
		p = e = numP1 = numP2 = numE1 = numE2 = 0;
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
	

	for(t = temp; t != te; ++t)
		++buckets[1 + (p->score>>16 & 0xFFFF)];

	for(b = buckets + 1; b != be; ++b)
		*b += *(b-1);

	for(t = temp; t != te; ++t)
		s[buckets[p->score>>16 & 0xFFFF]++] = *t; //this line requires random access iterator	
}

const char WINS = 76;
constexpr ull wins[WINS] = {
 	15ULL, 4369ULL, 281479271743489ULL, 240ULL, 8738ULL, 562958543486978ULL, 3840ULL, 17476ULL, 1125917086973956ULL, 61440ULL, 34952ULL, 2251834173947912ULL, 33825ULL, 
	74880ULL, 1152922604119523329ULL, 4504699407433729ULL, 2251816993685505ULL, 562967133814785ULL, 983040ULL, 286326784ULL, 4503668347895824ULL, 15728640ULL, 
	572653568ULL, 9007336695791648ULL, 251658240ULL, 1145307136ULL, 18014673391583296ULL, 4026531840ULL, 2290614272ULL, 36029346783166592ULL, 2216755200ULL, 
	4907335680ULL, 2305845208239046658ULL, 9009398814867458ULL, 36029071898968080ULL, 9007474141036560ULL, 64424509440ULL, 18764712116224ULL, 72058693566333184ULL, 
	1030792151040ULL, 37529424232448ULL, 144117387132666368ULL, 16492674416640ULL, 75058848464896ULL, 288234774265332736ULL, 263882790666240ULL, 150117696929792ULL,
	576469548530665472ULL, 145277268787200ULL, 321607151124480ULL, 4611690416478093316ULL, 18018797629734916ULL, 576465150383489280ULL, 144119586256584960ULL, 
	4222124650659840ULL, 1229764173248856064ULL, 1152939097061330944ULL, 67553994410557440ULL, 2459528346497712128ULL, 2305878194122661888ULL, 1080863910568919040ULL,
	4919056692995424256ULL, 4611756388245323776ULL, 17293822569102704640ULL, 9838113385990848512ULL, 9223512776490647552ULL, 9520891087237939200ULL, 
	2630102182384369665ULL, 9223380832956186632ULL, 36037595259469832ULL, 9223442406135828480ULL, 2305913380105359360ULL, 9223376434903384065ULL, 9011599448735745ULL, 
	36033195602411520ULL, 2305847407268659200ULL
}, *winEnd = const_cast<ull *>(wins) + WINS; //not sure why it's forcing me to use a const_cast..


//Modifies scores
//Scores will be in order of importance, search as deep as you want
//IF RETURN IS TRUE:
//	Head of scores is a move that will win the game or attempt to save the game (avert loss), just move there
//ELSE:
//	Scores is sorted in descreasing score order, essentially best moves come first (pos in each box is the bit shift required to get move)
bool eval(ull P, ull E) {
	box *pts[4], **p; 	// pointers to the four boxes in the line, **p is an iterator
	ull pw, ew;		// holds the combinations of each line and current board state for player and enemy
	char pwr, np, ne;	// holds the power of the current shift, the number of players in the line, and the number of enemies in the line respectively
	reset_all(scores, scoresEnd);

	for(ull win : wins) {
		pw = P & win;
		ew = E & win;
		p = pts;
		pwr = np = ne = 0;
		
		if(!pw && !ew)	//if no pieces in line at all, nothing to do continue
			continue;
		//normally I would say to not do anything if there exists pieces in the line for both enemy and player as well, but I need to minimally set those boxes' scores to MIN

		while(win > 0) {
			if(win & 1) {
				*p = scores + pwr;
				if(pw & 1) {
					(*p)->p = 1;
					(*p)->score = box::MIN;
					++np;
				} else if(ew & 1) {
					(*p)->e = 1;
					(*p)->score = box::MIN;
					++ne;
				}
				++p;
			}
			++pwr;
			win >>= 1;
			pw >>= 1;
			ew >>= 1;
		}

		for(box *pt : pts) {
			if(pt->p || pt->e || pt->loss) //if a piece is here or if already loss aversion state, no need to adjust score or any other values
				continue;
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
						scores[0] = *pt;
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

		bucket_sort(scores, scoresEnd);
		return scores[0].loss;
	}

}

int main() {
	cout << sizeof(box) << endl;
	return 0;
}
