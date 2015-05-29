#include <iostream>
#include <vector>
#include <limits>
#include <cstdlib>

#include "ttt3d.h"

//our namespace
namespace MZ {

using namespace std;

typedef unsigned long long ull;

enum class ABType : char { //set underlying integer type to char because we only have two values (char is smallest size 1 byte{
	MAX, MIN //Maximizer or minimizer node
};

struct ABNode {
	ABNode(ABType type, ull P, ull E) : type(type), P(P), E(E)
	{

	}

	vector<ABNode*> children;
	ull	x, //Data, i.e. value
	 	P, //State associated with each node
		E,
		P_next; //Best next possible move
	ABType type;
};

class MZ : public TTT3D {
	public:
		MZ(const duration<double> total_time_allowed) : TTT3D(total_time_allowed) {
			history.reserve(64);
		}
		void next_move(int mv[3]) { //Takes an int array of size 3, and then changes the array to indicate next move
		 //If we go first, then we will be passed an illegal move
			if(mv[0] >= 0 && mv[0] <= 3 && mv[1] >= 0 && mv[1] <= 3 && mv[2] >= 0 && mv[2] <= 3) 
			{
				history.push_back(foreign_convert(mv)); //I.e. we didn't go first
				E += foreign_convert(mv); //Update our record of the enemy state
			}
			else history.push_back(0);
			ABNode *root = new ABNode(ABType::MAX, P, E);
			int depth = MAX_DEPTH;
			gen_tree(root, depth, ABType::MAX); //Generate the search tree rooted at root
			ull move = AB(root, depth);
			P += move; //Update our record of our state
			foreign_convert(move, mv); //Store the new move.
			history.push_back(move); //reserve (max 64 moves)
		}
	private:
		ull P = 0, E = 0; //storage for board, set to 0 initially
		vector<ull> history; 	//History of all moves made by players. The first value is 0 if we went first. Else it just starts.

		static const char MAX_DEPTH = 4, //Maximum depth for search (moved from constructor so it is set compile time
		     		  WINS = 76;
		static constexpr ull wins[WINS] = {
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

		static void foreign_convert(ull u, int mv[3])
		{
			int pwr = 0;
			while(u > 1)
			{
				u >>= 1;
				++pwr;
			}
			mv[2] = pwr >> 4; 			// pwr/16
			mv[1] = (pwr - (mv[2] << 4)) >> 2;  	// (pwr - 16*mv[2]) / 4
			mv[0] = pwr & 3;			// pwr % 4
		}
		static ull foreign_convert(int mv[3])
		{
			return (1ULL) << ( (mv[2] << 4) + (mv[1] << 2) + mv[0] ); //The power exponent of the ull. (Originally 16*mv[2] + 4*mv[1] + mv[0])
		}
		static bool is_valid(ull A, ull B) //Returns true if there are no intersections
		{
			return !(A & B);
		}
		static bool won(ull P)
		{
			for(const ull *i = wins; i < winEnd; ++i)
			{
				if(won(P, *i)) return true;
			}
			return false;
		}
		static bool won(ull P, ull w)
		{
			return (P & w) == w;
		}
		static ull atari(ull P, ull E) //Essentially, the method returns the spot where player P should play to avoid losing to E.
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
		static ull eval(ull P, ull E) //Static evaluation function
		{
			return rand();
		}
		static void prune(ABNode *root, int i) //Cuts off all other trees after i
		{
			for(int j = i + 1; j < root->children.size(); ++j)
			{
				delete(root->children[j]);
			}
			cout << "prune\n";
		}
		static ull AB(ABNode *root, int depth, ull alpha, ull beta) //To make this easier, also pass up the game states (so the AI knows which move to take next)
		{
			if(depth == MAX_DEPTH || root->children.empty()) return eval(root->P, root->E);
			switch(root->type) { //added a switch statement instead of two if's (better practice for enums)

			case ABType::MAX:
				for(int i = 0; i < root->children.size(); ++i)
				{
					ull alpha_ = AB(root->children[i], depth + 1, alpha, beta);
					if(alpha_ > alpha)
					{
						alpha = alpha_;
						(root->P_next) = root->children[i]->E;
					}
					if(beta <= alpha) prune(root, i);
				}
				return alpha;

			case ABType::MIN:
				for(int i = 0; i < root->children.size(); ++i)
				{
					ull beta_ = AB(root->children[i], depth + 1, alpha, beta);
					if(beta_ < beta)
					{
						beta = beta_;
						(root->P_next) = root->children[i]->E;
					}
					if(beta <= alpha) prune(root, i);
				}
				return beta;
			}
		}
		static void gen_tree(ABNode *root, int depth, ABType p = ABType::MAX) //Optimize this to get rid of symmetric cases
		{
			if(depth > 0)
			{
				ull state = root->P | root->E;
				ull indicator = 1;
				for(int i = 0; i < 64; ++i)
				{
					if(!(state & 1)) //i.e. it's empty
					{	
						ABType newstate = p == ABType::MAX ? ABType::MIN : ABType::MAX;
						ABNode *n = new ABNode(newstate, root->E, (root->P | indicator));

						root->children.push_back(n);
						gen_tree(n, depth - 1, newstate);
					}
					indicator <<= 1;
					state >>= 1;
				}
			}
		}
		static ull AB(ABNode *root, int depth) //NOTE: This does not return the value, rather the next move (encoded in ull) for the AI to make.
		{
			cout << "\n" << AB(root, depth, 0, numeric_limits<ull>::max()) << "\n";
			return (root->P_next) - (root->P);
		}
};

}

//Another thing. Before you generate the game tree, you may be able to separate all states into different symmetry classes
//so that you don't have too many branches in the search tree

