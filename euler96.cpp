#include <iostream>
#include <vector>
#include <bitset>
#include <algorithm>
#include <fstream>
#include <sstream>
#define SUBGRID 3
#define DIMENSION SUBGRID * SUBGRID
#define SIZE DIMENSION * DIMENSION
#define tr(i, lim) \
	for(int i=0; i<lim; ++i)
#define trit(container, it) \
	for(typeof((container).begin()) it = (container).begin(); it != (container).end(); ++it)

using namespace std;

typedef int boardcol;
typedef vector<boardcol> boardrow;
typedef vector<boardrow> boardgrid;

template<class RandomAccessIterator, class Compare>
void insertion_sort(RandomAccessIterator first, RandomAccessIterator last, Compare comp) {
	RandomAccessIterator it1 = first, it2;

	while(it1 != last) {
		it2 = it1;
		while(it2 != first) {
			--it2;
			if(comp(*(it2 + 1), *it2))
				swap(*(it2+1), *it2);
			else
				break;
		}
		++it1;
	}
}

class Board {
	public:

	Board(const vector<vector<int> >& initial) :
		solved(false)
	{
		vector<move> fixed;

		for(int i=0; i<DIMENSION; ++i)
			for(int j=0; j<DIMENSION; ++j) {
				grid[i][j].candidates.set();
				grid[i][j].row = i;
				grid[i][j].col = j;

				if(initial[i][j] == 0)
					freeCells.push_back(&grid[i][j]);
				else {
					fixed.push_back(move(i,j,initial[i][j]));
				}
			}
		
		for(vector<move>::iterator it = fixed.begin(); it != fixed.end(); ++it) {
			make_move(*it);
		}

		sort(freeCells.begin(), freeCells.end(), compare);

		solve();
	}

	friend ostream& operator<<(ostream& os, const Board& board);

	int upperleft() {
		stringstream number;
		number << grid[0][0].state << grid[0][1].state << grid[0][2].state;
		return atoi(number.str().c_str());
	}

	private:

		void solve() {
			bool print = false;
			if(freeCells.begin() == freeCells.end())
				solved = true;

			if(!solved) {
				cell best = **(freeCells.end()-1);

				if(best.candidates.count() == 0)
					return;

				if(best.row == 0 && best.col == 3) {
					print = true;
				}


				for(int i=0; i<DIMENSION; ++i) {
					if(!best.candidates[i])
						continue;

					move next(best.row, best.col, i+1);
					freeCells.erase(freeCells.end()-1);
					make_move(next);

					sort(freeCells.begin(), freeCells.end(), compare);
					solve();

					if(solved)
						return;
					else {
						freeCells.push_back(&grid[next.row][next.col]);
						unmake_move(next);
					}
				}
			}
		}

		struct move {
			move(int r, int c, int v) : row(r), col(c), value(v) {}
			int row,col,value;
		};

		struct cell {
			cell() : state(0), row(0), col(0), candidates(), numrestrictions(DIMENSION,0) {}
			bitset<DIMENSION> candidates;
			vector<int> numrestrictions;
			int state;
			int row, col;

		};

		struct CellCompare {
			bool operator() (const cell* lhs, const cell* rhs) {
				return lhs->candidates.count() > rhs->candidates.count();
			}
		};

		void process_move(move m, bool forward) {
			if(forward)
				grid[m.row][m.col].state = m.value;
			else
				grid[m.row][m.col].state = 0;

			for(int j=0; j<DIMENSION; ++j) {
				cell& c = grid[m.row][j];
				if(forward) {
					++c.numrestrictions[m.value-1];
					c.candidates.set(m.value-1, false);
				} else if(--c.numrestrictions[m.value-1] == 0) {
					c.candidates.set(m.value-1, true);
				}
			}

			for(int i=0; i<DIMENSION; ++i) {
				cell& c = grid[i][m.col];
				if(forward) {
					++c.numrestrictions[m.value-1];
					c.candidates.set(m.value-1, false);
				} else if(--c.numrestrictions[m.value-1] == 0) {
					c.candidates.set(m.value-1, true);
				}
			}

			for(int i=0; i<SUBGRID; ++i)
				for(int j=0; j<SUBGRID; ++j) {
					cell& c = grid[(m.row/SUBGRID)*SUBGRID + i][(m.col/SUBGRID)*SUBGRID + j];
					if(forward) {
						++c.numrestrictions[m.value-1];
						c.candidates.set(m.value-1, false);
					} else if(--c.numrestrictions[m.value-1] == 0) {
						c.candidates.set(m.value-1, true);
					}
				}
		}

		void make_move(const move& m) {
			process_move(m, true);
		}

		void unmake_move(const move& m) {
			process_move(m, false);
		}
		
		cell grid[DIMENSION][DIMENSION];
		vector<cell*> freeCells;
		vector<move> moves;
		bool solved;
		CellCompare compare;
};

ostream& operator<<(ostream& os, const Board& board) {
	tr(i, DIMENSION) {
		tr(j, DIMENSION)
			os << board.grid[i][j].state << " ";
		os << endl;
	}
	return os;
}

int main() {
	ifstream file("bigsudoku.txt");
	string line;

	int numgrids = 50;

	vector<boardgrid> puzzles(numgrids, boardgrid(DIMENSION, boardrow(DIMENSION)));
	
	if(file.is_open()) {
		for(int g=0; g<numgrids; ++g) {
			getline(file, line);

			for(int i=0; i<DIMENSION; ++i) {
				getline(file, line);

				for(int j=0; j<DIMENSION; ++j) {
					char c = line[j];
					puzzles[g][i][j] = atoi(&c);
				}

			}
		}
	}

	file.close();

	int sum = 0;
	for(int g=0; g<numgrids; ++g) {
		Board b(puzzles[g]);
		sum += b.upperleft();
	}
	cout << sum << endl;

	return 0;
}


