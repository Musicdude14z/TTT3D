/*
Eventually this will be the machine learning framework
*/

#include <cstdlib>
#include <vector>
#include <iostream>
#include <ctime>
#include <fstream>
#define WINS 76

typedef unsigned long long ull;

using namespace std;

ull total_push;

ofstream out("search_tree_bounds.txt"); //Prob don't need this right now

enum ABType{ //set underlying integer type to char because we only have two values (char is smallest size 1 byte{
	MAX, MIN //Maximizer or minimizer node
};

struct ABNode {
	ABNode(ABType type, ull P, ull E, int depth = 0) : type(type), P(P), E(E), depth(depth)
	{

	}

	vector<ABNode*> children;
	ull	x, //Data, i.e. value
	 	P, //State associated with each node
		E,
		P_next; //Best next possible move
	int depth; //Distance from root
	ABType type;
};

ull wins[76] = {
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
				},  *winEnd = const_cast<ull *>(wins) + WINS;
		
ull atari(ull P, ull E) //Essentially, the method returns the spot where player P should play to avoid losing to E.
{ //Returns 0 if safe, i.e. no atari.
	for(const ull *i = wins; i < winEnd; ++i)
	{
		ull 	u = E & *i,
			u_ = u;
		while(u_ > 0)
		{
			if(u_ & 1)
			{
				if(u_ == 1)
				{
					if(P & u) break; //True if the spot is blocked
					else return u; //I.e. the spot is open, so you have to play there.
				}
				else break; //For example, something like u = 10001000.
			}
			else(u_ >>= 1);
		}
	}
	return 0; //There is no atari.
}

bool won(ull P, ull w)
{
	return (P & w) == w;
}

bool won(ull P)
{
	for(const ull *i = wins; i < winEnd; ++i)
	{
		if(won(P, *i)) return true;
	}
	return false;
}
	
bool is_valid(ull A, ull B) //Returns true if there are no intersections
{
	return !(A & B);
}

void gen_tree(ABNode *root, int max_depth, ABType p = MAX, int depth = 0) //Generates the tree to the very bottom (or to max_depth, whichever is first)
{
	if(max_depth > 0)
	{
		ull state = root->P | root->E;
		ull indicator = 1;
		for(int i = 0; i < 64; ++i)
		{
			if(!(state & 1) && !won(root->P) && !won(root->E)) //i.e. it's empty and has not won
			{	
				ABType newstate = p == MAX ? MIN : MAX;
				ABNode *n = new ABNode(newstate, root->E, (root->P | indicator), depth + 1);
				++total_push;
				root->children.push_back(n);
				gen_tree(n, max_depth - 1, newstate, depth + 1);
			}
			indicator <<= 1;
			state >>= 1;
		}
	}
}

ull ull_rand(int n, ull mask = 0) //Returns random ull u with n of its bits equal to 1 and such that u & mask == 0
{
	ull u = 0;
	vector<int> indices;
	indices.reserve(64);
	int pwr_counter = 0;
	while(pwr_counter < 64)
	{
		if(!(mask & 1)) indices.push_back(pwr_counter);
		++pwr_counter;
		mask >>= 1;
	}
	int counter = 0;
	while(counter < n && !indices.empty())
	{
		int chosen = rand() % indices.size();
		u += (1ULL << indices[chosen]);
		indices.erase(indices.begin() + chosen);
		++counter;
	}
	return u;
}

ull ull_rand_v(int n, ull mask = 0) //Runs the above function until it returns a valid position (i.e. not won)
{
	ull P = ull_rand(n, mask);
	while(won(P)) P = ull_rand(n, mask);
	return P;	
}

int ull_sum(ull u) //Returns digital sum of ull in binary
{
	int counter = 0;
	while(u > 0)
	{
		if(u & 1) ++counter;
		u >>= 1;
	}
	return counter;
}

ull ull_comp(ull P) //returns a complement ull E for a given P such that (P, E) is a valid state
{
	int P_sum = ull_sum(P);
	ull E = ull_rand(P_sum, P);
	int diff = ull_sum(P) - ull_sum(E);
	if(diff != 0 && diff != -1 && diff != 1) return 0;
	return E;
}

ull ull_comp_v(ull P) //does the above function's job but checks for E's validity
{
	int P_sum = ull_sum(P);
	ull E = ull_rand_v(P_sum, P);
	int diff = ull_sum(P) - ull_sum(E);
	if(diff != 0 && diff != -1 && diff != 1) return 0;
	return E;
}
		
int main()
{
	srand(time(NULL));
	total_push = 0;
	ull P = ull_rand_v(16);
	ull E = ull_comp_v(P);
	ABNode *root = new ABNode(MAX, P, E);
	gen_tree(root, 4);
	cout << total_push;
	return 0;
}
