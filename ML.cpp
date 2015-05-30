/*
Eventually if we have time machine learning will be used to get the coefficients for the static evaluation.
The Christmas_Tree class helps give us the "answers" for supervised learning for our
algorithm to try to fit the function to.
*/

#include <cstdlib>
#include <vector>
#include <iostream>
#include <ctime>
#include <fstream>
#define WINS 76

typedef unsigned long long ull;

const int ALLOWANCE = 2000; //Factor determining approximately how many nodes can be selected to proceed to next stage.

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

/*
Christmas_Tree is a class that is similar to a regular search tree, but allows to generate nodes all the way to the leaves of the graph by
randomly selected a maximum number of nodes for which to create children (the number of allowed nodes is determined by allowance). This means that
a search tree can be generated even from a game with only 1-move (in reasonable time). For a search tree starting at 12 moves for both players, and allowance
equal to 2000, the total size of the search tree is still very manageable.

The evaluation function built off the Christmas tree takes into account not only the number of wins, but also the average distance of each win down the tree,
all the while weighing the calculated average win distance for each player with multiplier factors.

So, for example, setting a high multiplier factor for mult[0 to 2] says that states that can win in 1 move a VERY IMPORTANT.

The resulting score is the average enemy distance - the average player distance.

Positive scores favor the player, while negative scores favor the enemy.
*/

class Christmas_Tree
{
public:
	Christmas_Tree(ABNode *root, int allowance = ALLOWANCE) : root(root), allowance(allowance), n_nodes(1)
	{
		for(int i = 0; i < 64; ++i)
		{
			P_wins_count.push_back(0);
			E_wins_count.push_back(0);
			mult.push_back(1);
		}
		mult[2] = 2000;
		mult[1] = 2000;
		mult[0] = 2000;
		vector<ABNode *> first_row;
		first_row.push_back(root);
		rows.push_back(first_row);
		int i = 0;
		while(!rows.at(i).empty())
		{
			gen_tree_row(rows.at(i));
			++i;
		}
		int P_sum = 0; //In the end, we want wins that are in fewer moves to be more urgent (for both us and the enemy)
		/*
			Note that since depth is all measured from the same node, the algortihm is inherently biased towards the player (enemy is 1 move extra)
			So if we have a state where the player has atari, and another one where the enemy has atari (remember it's our turn)
			the player atari will be favored.
		*/
		for(int i = 0; i < P_wins.size(); ++i) //These two for loops calculate the weighted average distance to a win for us.
		{
			P_sum += mult.at(P_wins.at(i)->depth)*(P_wins.at(i)->depth);
		}
		double P_denom = 0;
		for(int i = 0; i < P_wins_count.size(); ++i)
		{
			P_denom += mult.at(i) * P_wins_count.at(i);
		}
		P_wins_depth = (double)P_sum / P_denom;
		int E_sum = 0;
		for(int i = 0; i < E_wins.size(); ++i)  //These two for loops calculate the weighted average distance to a win for them.
		{
			E_sum += mult.at(E_wins.at(i)->depth)*(E_wins.at(i)->depth);
		}
		double E_denom = 0;
		for(int i = 0; i < E_wins_count.size(); ++i)
		{
			E_denom += mult.at(i) * E_wins_count.at(i);
		}
		E_wins_depth = (double)E_sum / E_denom;
		score_ = E_wins_depth - P_wins_depth;
	}
	ABNode *root;
	vector< vector<ABNode *> > rows;
	vector<ABNode *> P_wins;
	vector<ABNode *> E_wins;
	vector<int> P_wins_count;
	vector<int> E_wins_count;
	vector<double> mult; //Multiplier factors for weighting the individual components of the average distance
	double P_wins_depth;
	double E_wins_depth;
	int size()
	{
		return n_nodes;
	}
	double score()
	{
		return score_;
	}
private:
	const int allowance;
	int n_nodes; //Number of nodes in the tree
	void gen_tree_row(const vector<ABNode *> &prev)
	{
		//Select the ones that will go on to the next row and store them in vector "seed"
		vector<ABNode *> temp(prev);
		vector<ABNode *> seed; //Contains the ones that are selected to go on to the next generation
		int size = temp.size();
		vector<int> indices;
		for(int i = 0; i < size; ++i) indices.push_back(i);
		int added_so_far = 0;
		while(added_so_far < allowance && !temp.empty())
		{
			int choice = rand() % temp.size();
			seed.push_back(temp.at(choice));
			temp.erase(temp.begin() + choice);
			++added_so_far;
		}
		//cout << size << "\n";
		//cout << seed.size() << "\n\n";
		//Create the children and push them also to a new row vector
		vector<ABNode *> next_row;
		for(int i = 0; i < seed.size(); ++i)
		{
			ABNode *ptr = seed.at(i);
			if(!won(ptr->P) && !won(ptr->E)) //If ok, then make the children
			{
				ull state = ptr->P | ptr->E; //Combined state
				ull indicator = 1;
				while(state > 0) //I.e. the spot is empty
				{
					if(!(state & 1))
					{
						ABNode *new_node = new ABNode(ptr->type == MAX ? MIN : MAX, ptr->E, (ptr->P) | indicator, (ptr->depth) + 1);
						ptr->children.push_back(new_node);
						++n_nodes;
						next_row.push_back(new_node);
					}
					indicator <<= 1;
					state >>= 1;
				}
			}
			if(won(ptr->E))
			{
				if(ptr->type == MIN) 
				{
					P_wins.push_back(ptr);
					++(P_wins_count.at(ptr->depth));
				}
				else
				{
					E_wins.push_back(ptr);
					++(E_wins_count.at(ptr->depth));
				}
			}
		}
		rows.push_back(next_row);
	}
	double score_;
};

double eval(ull P, ull E)
{
	ABNode *root = new ABNode(MAX, P, E);
	Christmas_Tree ct(root);
	return ct.score();
}
		
int main()
{
	srand(time(NULL));
	//ull P = ull_rand_v(12);
	ull P = 7;
	ull E = ull_comp_v(P);
	cout << P << " " << E << "\n";
	cout << eval(P, E) << "\n";
	return 0;
}
