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
	A_B_Node(ABType type, ull P, ull E) : type(type), P(P), E(E)
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
			initialize_wins(); //i will change this to be prerendered and loaded at compile time to save us runtime!!
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
			ull move = A_B(root, depth);
			P += move; //Update our record of our state
			foreign_convert(move, mv); //Store the new move.
			history.push_back(move); //reserve (max 64 moves)
		}
	private:
		ull P = 0, E = 0; //storage for board, set to 0 initially
		const int MAX_DEPTH = 4; //Maximum depth for search (moved from constructor so it is set compile time
		vector<ull> history; 	//History of all moves made by players. The first value is 0 if we went first. Else it just starts.
		ull wins[] = { //pre generated and written in literal format, so it is done during compile time instead of wasting runtime
		        15ULL, 240ULL, 3840ULL, 61440ULL, 983040ULL, 15728640ULL, 251658240ULL, 18446744073441116160ULL, 15ULL, 240ULL, 3840ULL, 61440ULL, 983040ULL, 15728640ULL, 
			251658240ULL, 18446744073441116160ULL, 4369ULL, 8738ULL, 17476ULL, 34952ULL, 286326784ULL, 572653568ULL, 1145307136ULL, 18446744071705198592ULL, 4369ULL, 
			8738ULL, 17476ULL, 34952ULL, 286326784ULL, 572653568ULL, 1145307136ULL, 18446744071705198592ULL, 131074ULL, 262148ULL, 524296ULL, 1048592ULL, 2097184ULL, 
			4194368ULL, 8388736ULL, 16777472ULL, 33554944ULL, 67109888ULL, 134219776ULL, 268439552ULL, 536879104ULL, 1073758208ULL, 2147516416ULL, 18446744069414649856ULL, 
			33825ULL, 4680ULL, 18446744071631339520ULL, 306708480ULL, 33825ULL, 4680ULL, 18446744071631339520ULL, 306708480ULL, 269484289ULL, 16846864ULL, 538968578ULL, 
			33693728ULL, 1077937156ULL, 67387456ULL, 18446744071570458632ULL, 134774912ULL, 655365ULL, 327690ULL, 10485840ULL, 5243040ULL, 167773440ULL, 83888640ULL, 
			18446744072098959360ULL, 1342218240ULL, 18446744071564166145ULL, 272630280ULL, 67207200ULL, 34082880ULL
		};
		void foreign_convert(ull u, int mv[])
		{
			int pwr = 0;
			while(u > 1)
			{
				u >>= 1;
				++pwr;
			}
			mv[2] = pwr/16;
			mv[1] = (pwr - 16 * mv[2])/4;
			mv[0] = pwr % 4;
		}
		ull foreign_convert(int mv[])
		{
			int x = 16*mv[2] + 4*mv[1] + mv[0]; //The power exponent of the ull.
			return ((ull)1) << x;
		}
		bool is_valid(ull A, ull B) //Returns true if there are no intersections
		{
			return !(A & B);
		}
		bool won(ull P)
		{
			for(int i = 0; i < wins.size(); ++i)
			{
				if(won(P, wins[i])) return true;
			}
			return false;
		}
		bool won(ull P, ull w)
		{
			return (P & w) == w;
		}
		ull atari(ull P, ull E) //Essentially, the method returns the spot where player P should play to avoid losing to E.
		{ //Returns 0 if safe, i.e. no atari.
			for(int i = 0; i < wins.size(); ++i)
			{
				ull u = E & wins[i];
				ull u_ = u;
				while(u_ > 0)
				{
					if(u_ % 2)
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
		ull convert(int c_1, int c_2, int c_3, int c_4)
		{
			ull u = 0;
			u += (1 << c_1);
			u += (1 << c_2);
			u += (1 << c_3);
			u += (1 << c_4);
			return u;
		}
		ull eval(ull P, ull E) //Static evaluation function
		{
			return rand();
		}
		void prune(A_B_Node *root, int i) //Cuts off all other trees after i
		{
			for(int j = i + 1; j < root->children.size(); ++j)
			{
				delete(root->children[j]);
			}
			cout << "prune\n";
		}
		ull A_B(A_B_Node *root, int depth, ull alpha, ull beta) //To make this easier, also pass up the game states (so the AI knows which move to take next)
		{
			if(depth == MAX_DEPTH || root->children.empty()) return eval(root->P, root->E);
			if(root->type == MAX)
			{
				for(int i = 0; i < root->children.size(); ++i)
				{
					ull alpha_ = A_B(root->children[i], depth + 1, alpha, beta);
					if(alpha_ > alpha)
					{
						alpha = alpha_;
						(root->P_next) = root->children[i]->E;
					}
					if(beta <= alpha) prune(root, i);
				}
				return alpha;
			}
			if(root->type == MIN)
			{
				for(int i = 0; i < root->children.size(); ++i)
				{
					ull beta_ = A_B(root->children[i], depth + 1, alpha, beta);
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
		void gen_tree(A_B_Node *root, int depth, player_type p = MAX) //Optimize this to get rid of symmetric cases
		{
			if(depth > 0)
			{
				ull state = root->P | root->E;
				ull indicator = 1;
				for(int i = 0; i < 64; ++i)
				{
					if(!(state % 2)) //i.e. it's empty
					{
						A_B_Node *n; 
						if(p == MAX) n = new A_B_Node(MIN, root->E, (root->P | indicator));
						else n = new A_B_Node(MAX, root->E, (root->P | indicator));
						root->children.push_back(n);
						if(p == MAX) gen_tree(n, depth - 1, MIN);
						else gen_tree(n, depth - 1, MAX);
					}
					indicator <<= 1;
					state >>= 1;
				}
			}
		}
		ull A_B(A_B_Node *root, int depth) //NOTE: This does not return the value, rather the next move (encoded in ull) for the AI to make.
		{
			cout << "\n" << A_B(root, depth, 0, numeric_limits<ull>::max()) << "\n";
			return (root->P_next) - (root->P);
		}
};

}

//Another thing. Before you generate the game tree, you may be able to separate all states into different symmetry classes
//so that you don't have too many branches in the search tree

