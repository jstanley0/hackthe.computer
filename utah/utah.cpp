#include <iostream>
#include <string>
#include <vector>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

using namespace std;

void die(int code, string message)
{
	cerr << message << endl;
	exit(code);
}

struct cell
{
	char *row1[3];
	char *row2[3];
};


class Utah
{
public:
	// read the next game from standard in, solve it, and
	// write the solution to standard out.
	// return false if no more games exist
	bool solve_game()
	{
		if (!read())
			return false;
			
		solve();
		print();
		return true;
	}
	
	
private:
	vector<vector<char> > m_board;
	int m_size;
	char *m_block;	// square to play at to block opponent; for if there isn't a win

	inline char & cube(int plane, int row, int col)
	{
		return m_board[row][plane * (m_size + 1) + col];
	}

	void print()
	{
		for(int i = 0; i < m_size; ++i) {
			cout << &m_board[i][0] << "\n";
		}
		cout << endl;
	}

	bool read()
	{
		m_board.clear();
		string line;
		while (getline(cin, line)) {
			if (line.empty())
				break;
			vector<char> line_data;
			line_data.resize(line.size() + 1);
			strcpy(&line_data[0], line.c_str());
			m_board.push_back(line_data);
		}
		if (m_board.empty())
			return false;

		init_cube();			
		return true;
	}
	
	void init_cube()
	{
		m_size = 0;
		m_block = NULL;
		while(m_size < m_board[0].size() && m_board[0][m_size] != ' ')
			m_size++;
		if (m_size < 3 || m_board.size() != m_size) 
			die(1, "width/height mismatch");
		for(int i = 0; i < m_size; ++i)
			if (m_board[i].size() < ((m_size + 1) * m_size) - 1)
				die(2, "invalid board");
	}
	
	// check 5 spaces that form the shape of a pentomino
	bool check(char *spaces[5])
	{
		/*
		char orig[5];
		for(int i = 0; i < 5; i++)
		{
			orig[i] = *spaces[i];
			*spaces[i] = '*';
		}
		print();
		for(int i = 0; i < 5; i++)
		{
			*spaces[i] = orig[i];
		}
		return false;
		*/
				
		int countX = 0, countO = 0;
		for(int i = 0; i < 5; i++) {
			if ('X' == *(spaces[i])) countX++;
			if ('O' == *(spaces[i])) countO++;
		}
		if (countX == 4) {
			for(int i = 0; i < 5; i++) {
				if (*(spaces[i]) == '_') {
					*(spaces[i]) = 'X';
					return true;
				}
			}
		} 
		if (countO == 4) {
			for(int i = 0; i < 5; i++) {
				if (*(spaces[i]) == '_') {
					m_block = spaces[i];
				}
			}
		} 		
		return false;
	}
		
	// check a 3x2 cell	
	bool check(cell &pp)
	{
		/*
		char orig1[3], orig2[3];
		for(int i = 0; i < 3; i++)
		{
			orig1[i] = *pp.row1[i];
			orig2[i] = *pp.row2[i];
			*pp.row1[i] = '*';
			*pp.row2[i] = '*';
		}
		print();
		for(int i = 0; i < 3; i++)
		{
			*pp.row1[i] = orig1[i];
			*pp.row2[i] = orig2[i];
		}		
		return false;
		*/
	
		char *spaces[5];
		// row 1 filled pentominoes
		for(int i = 0; i < 3; i++)
			spaces[i] = pp.row1[i];
		spaces[3] = pp.row2[0];
		spaces[4] = pp.row2[1];
		if (check(spaces))
			return true;
		spaces[3] = pp.row2[1];
		spaces[4] = pp.row2[2];
		if (check(spaces))
			return true;
			
		// row 2 filled pentominoes
		for(int i = 0; i < 3; i++)
			spaces[i] = pp.row2[i];
		spaces[3] = pp.row1[0];
		spaces[4] = pp.row1[1];
		if (check(spaces))
			return true;
		spaces[3] = pp.row1[1];
		spaces[4] = pp.row1[2];
		if (check(spaces))
			return true;
		
		return false;
	}
	
	void solve()
	{
		cell pp;
		
		// iterate all possible 3x2 slices of our NxNxN cube
		for(int p = 0; p < m_size; ++p) {
			// 1x2x3
			for(int sr = 0; sr < m_size - 1; sr++) {
				for(int sc = 0; sc < m_size - 2; sc++) {
					for(int i = 0; i < 3; i++) {
						pp.row1[i] = &cube(p,sr,sc+i);
						pp.row2[i] = &cube(p,sr+1,sc+i);
					}
					if (check(pp))
						return;
				}
			}
			
			// 1x3x2
			for(int sr = 0; sr < m_size - 2; sr++) {
				for(int sc = 0; sc < m_size - 1; sc++) {
					for(int i = 0; i < 3; i++) {
						pp.row1[i] = &cube(p,sr+i,sc);
						pp.row2[i] = &cube(p,sr+i,sc+1);
					}
					if (check(pp))
						return;
				}
			}
		}

		for(int r = 0; r < m_size; ++r)
		{
			// 2x1x3
			for(int sp = 0; sp < m_size - 1; sp++) {
				for(int sc = 0; sc < m_size - 2; sc++) {
					for(int i = 0; i < 3; i++) {
						pp.row1[i] = &cube(sp,r,sc+i);
						pp.row2[i] = &cube(sp+1,r,sc+i);
					}
					if (check(pp))
						return;
				}
			}

			// 3x1x2
			for(int sp = 0; sp < m_size - 2; sp++) {
				for(int sc = 0; sc < m_size - 1; sc++) {
					for(int i = 0; i < 3; i++) {
						pp.row1[i] = &cube(sp+i,r,sc);
						pp.row2[i] = &cube(sp+i,r,sc+1);
					}
					if (check(pp))
						return;
				}
			}
		}

		for(int c = 0; c < m_size; c++)
		{
			// 2x3x1
			for(int sp = 0; sp < m_size - 1; sp++) {
				for(int sr = 0; sr < m_size - 2; sr++) {
					for(int i = 0; i < 3; i++) {
						pp.row1[i] = &cube(sp,sr+i,c);
						pp.row2[i] = &cube(sp+1,sr+i,c);
					}
					if (check(pp))
						return;
				}
			}

			// 3x2x1
			for(int sp = 0; sp < m_size - 2; sp++) {
				for(int sr = 0; sr < m_size - 1; sr++) {
					for(int i = 0; i < 3; i++) {
						pp.row1[i] = &cube(sp+i,sr,c);
						pp.row2[i] = &cube(sp+i,sr+1,c);
					}
					if (check(pp))
						return;
				}
			}		
		}
		
		if (m_block) {
			*m_block = 'X';
			return;
		}
		
		die(1, "no solution");
	}

};


int main(int argc, char **argv)
{
	Utah utah;
	while (utah.solve_game());
	return 0;
}
