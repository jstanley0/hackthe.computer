#include <iostream>
#include <string>
#include <vector>
#include <stdlib.h>
#include <math.h>
#include <algorithm>
#include <queue>

using namespace std;

void die(const char *message)
{
	cerr << message << endl;
	exit(1);
}

struct Node
{
	int row;
	int col;
	int length;
	int estimated_cost;
	char dir;
	
	// since priority_queue takes the _highest_ value, this function
	// needs to identify whether this node is *worse* than rhs
	bool operator<(const Node& rhs) const
	{
		return estimated_cost >= rhs.estimated_cost;
	}
};

class CostMatrix
{
	vector<int> data;
	int width;
	
public:
	CostMatrix() : width(0) {}
	
	void initialize(int _width, int _height)
	{
		width = _width;
		data.resize(_width * _height);
	}
	
	int at(int row, int col)
	{
		return data[row * width + col];
	}
	
	void set(int row, int col, int val)
	{
		data[row * width + col] = val;
	}
};

class Maze
{
public:
	vector<string> rows;
	vector<int> dupes;
	priority_queue<Node> search;
	CostMatrix cost_matrix;
	int start_row, start_col;
	int end_row, end_col;
	int width, height;

	Maze() :
		start_row(-1), start_col(-1),
		end_row(-1), end_col(-1),
		width(0), height(0)
	{
	}
	
	void read(istream &infile)
	{
		int dupe_count = 0;
		string line, lastline;
		while(getline(infile, line)) {
			if (line.size() == 0)
				continue;
			if (width == 0) {
				width = line.size();
				rows.reserve(width * 2);	// a heuristic to reduce reallocations
			}
			string::size_type pos;
			if (string::npos != (pos = line.find('S'))) {
				start_row = height;
				start_col = pos;
			}
			if (string::npos != (pos = line.find('F'))) {
				end_row = height;
				end_col = pos;
			}
			if (line == lastline) {
				dupes[height - 1]++;
				dupe_count++;
			}
			else {
				rows.push_back(line);
				dupes.push_back(1);
				++height;
			}
			lastline = line;
		}
		cerr << "width: " << width << "; height: " << height << " + " << dupe_count << endl;
		cost_matrix.initialize(width, height);
	}
	
	void solve()
	{
		if (start_row == -1)
			die("Maze start not found");
		if (end_row == -1)
			die("Maze finish not found");
			
		push_node(start_row, start_col, 0, '*');
		while(!search.empty())
		{
			Node node = search.top();
			search.pop();
			mark_node(node);
			
			if (node.row > 0 && investigate_node(node.row - 1, node.col, node.length + 1, 'N'))
				break;
			if (node.row < height - 1 && investigate_node(node.row + 1, node.col, node.length + 1, 'S'))
				break;
			if (node.col > 0 && investigate_node(node.row, node.col - 1, node.length + 1, 'W'))
				break;
			if (node.col < width - 1 && investigate_node(node.row, node.col + 1, node.length + 1, 'E'))
				break;
		}

		print_solution();
	}
	
private:
		
	void mark_node(const Node &node)
	{
		rows[node.row][node.col] = node.dir;
	}
		
	void push_node(int row, int col, int length, char dir)
	{
		Node n;
		n.row = row;
		n.col = col;
		n.length = length;
		n.dir = dir;
		n.estimated_cost = estimate_cost(n);
		search.push(n);
	}
	
	int estimate_cost(const Node &node)
	{
		int dr = node.row - end_row;
		int dc = node.col - end_col;
		return node.length + abs(dr) + abs(dc);
	}
	
	bool investigate_node(int row, int col, int length, char dir)
	{
		char c = rows[row][col];
		if (c == 'F')
		{
			rows[row][col] = dir;
			return true;
		}
		if (c == '-')
		{
			// only push the node if we haven't been here before
			// or this path is shorter
			int prior_cost = cost_matrix.at(row, col);
			if (prior_cost == 0 || prior_cost > length)
			{
				cost_matrix.set(row, col, length);
				push_node(row, col, length, dir);
			}
		}
		return false;
	}
	
	void print_maze()
	{
		for(size_t i = 0; i < rows.size(); ++i)
		{
			cerr << rows[i] << endl;
		}
	}
	
	struct dir_node {
		char c;
		int repeat;
		dir_node(char _c, int _repeat) : c(_c), repeat(_repeat) {}
	};
	
	void print_solution()
	{
		// backtrack from the finish to the start
		vector<dir_node> directions;
		int row = end_row, col = end_col;
		while(row != start_row || col != start_col)
		{
			char c = rows[row][col];
			directions.push_back(dir_node(c, dupes[row]));
			switch(c)
			{
				case 'N': row += 1; break;
				case 'S': row -= 1; break;
				case 'W': col += 1; break;
				case 'E': col -= 1; break;
				default: die("I don't know where I am, Sam.");
			}
		}
		
		for(vector<dir_node>::reverse_iterator i = directions.rbegin(); i != directions.rend(); ++i)
		{
			for(int x = 0; x < i->repeat; x++)
				cout << i->c << '\n';
		}
	}
};

int main(int argc, char **argv)
{
	Maze maze;
	maze.read(cin);
	maze.solve();
	return 0;
}
