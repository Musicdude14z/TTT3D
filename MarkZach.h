#include <iostream>
#include <vector>
#include <limits>
#include <cstdlib>

#include "ttt3d.h"

namespace MZ {

using namespace std;

typedef unsigned long long ull;
	enum player_type
	{
		MAX, MIN //Maximizer or minimizer node
	};

	struct A_B_Node
	{
	vector<A_B_Node*> children;
	ull x; //Data, i.e. value
	ull P; //State associated with each node
	ull E;
	ull P_next; //Best next possible move
	player_type type;
	A_B_Node(player_type type_, ull P_, ull E_) : type(type_), P(P_), E(E_)
	{

	}
};

class MZ : public TTT3D
{
	public:
		MZ(const duration<double> total_time_allowed) : TTT3D(total_time_allowed)
		{
			MAX_DEPTH = 4;
			initialize_wins();
			P = 0;
			E = 0;
			history.reserve(64);
		}
		void next_move(int mv[3]) //Takes an int array of size 3, and then changes the array to indicate next move
		{ //If we go first, then we will be passed an illegal move
			if(mv[0] >= 0 && mv[0] <= 3 && mv[1] >= 0 && mv[1] <= 3 && mv[2] >= 0 && mv[2] <= 3) 
			{
				history.push_back(foreign_convert(mv)); //I.e. we didn't go first
				E += foreign_convert(mv); //Update our record of the enemy state
			}
			else history.push_back(0);
			A_B_Node *root = new A_B_Node(MAX, P, E);
			int depth = MAX_DEPTH;
			gen_tree(root, depth, MAX); //Generate the search tree rooted at root
			ull move = A_B(root, depth);
			P += move; //Update our record of our state
			foreign_convert(move, mv); //Store the new move.
			history.push_back(move); //reserve (max 64 moves)
		}
	private:
		ull P;
		ull E;
		int MAX_DEPTH; //Maximum depth for search
		vector<ull> history; //History of all moves made by players. The first value is 0 if we went first. Else it just starts.
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
		vector<ull> wins; //Vector holding each possible line
		ull convert(int c_1, int c_2, int c_3, int c_4)
		{
			ull u = 0;
			u += (1 << c_1);
			u += (1 << c_2);
			u += (1 << c_3);
			u += (1 << c_4);
			return u;
		}
		void initialize_wins() //Push the 76 winning lines the the vector called "wins"
		{
			//Put the stuff inside, do it for size 4.
			for(int i = 0; i <= 60; i += 4) //16 horizontal rows
			{
				wins.push_back(convert(i, i + 1, i + 2, i + 3));
			}
			for(int i = 0; i <= 48; i += 16)
			{
				for(int j = i; j <= i + 3; ++j) //16 vertical rows
				{
					wins.push_back(convert(j, j + 4, j + 8, j + 12));
				}
			}
			for(int i = 0; i <= 15; ++i) //16 out-facing rows
			{
				wins.push_back(convert(i, i + 16, i + 32, i + 48));
			}
			for(int i = 0; i <= 48; i += 16) //8 outer-face diagonals
			{
				wins.push_back(convert(i, i + 5, i + 10, i + 15));
				wins.push_back(convert(i + 3, i + 6, i + 9, i + 12));
			}
			for(int i = 0; i <= 3; ++i) //8 horizontal-face diagonals
			{
				wins.push_back(convert(i, i + 20, i + 40, i + 60));
				wins.push_back(convert(i + 12, i + 24, i + 36, i + 48));
			}
			for(int i = 0; i <= 12; i += 4) //8 vertical-face diagonals
			{
				wins.push_back(convert(i, i + 17, i + 34, i + 51));
				wins.push_back(convert(i + 3, i + 18, i + 33, i + 48));
			}
			wins.push_back(convert(0, 21, 42, 63)); //The 4 space diagonals
			wins.push_back(convert(3, 22, 41, 60));
			wins.push_back(convert(48, 37, 26, 15));
			wins.push_back(convert(12, 25, 38, 51));
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

