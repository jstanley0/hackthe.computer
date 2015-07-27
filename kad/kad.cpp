#include <iostream>
#include <string>
#include <vector>
#include <stdlib.h>
#include <string.h>
#include <algorithm>

using namespace std;

#define HBITS 7
#define HSIZE (1 << HBITS)
#define HTAG(x) ((x) >> (64 - HBITS))

class SmartHashThingy
{
	vector<unsigned long long> teh_lists[HSIZE];
		
	struct index
	{
		size_t sublist;
		size_t xor_dist;
		bool operator<(const index& rhs) const { return xor_dist < rhs.xor_dist; }
	};
	
public:
	SmartHashThingy()
	{
	}

	void add(unsigned long long val)
	{
		vector<unsigned long long> &list = teh_lists[HTAG(val)];
		if (list.empty())
			list.reserve(500);
		list.push_back(val);
	}
	
	void query(unsigned long long num, int request_size)
	{
		unsigned long long tag = HTAG(num);
		index distances[HSIZE];
		for(size_t i = 0; i < HSIZE; i++)
		{
			distances[i].sublist = i;
			distances[i].xor_dist = tag ^ i;
		}
		sort(distances, distances + HSIZE);
		for(size_t i = 0; i < HSIZE; i++)
		{
			request_size = subquery(distances[i].sublist, num, request_size);
			if (request_size == 0)
				break;
		}
	}
	
	int subquery(int sublist, unsigned long num, int request_size)
	{
		//cerr << "querying sublist " << sublist << endl;
		size_t count = teh_lists[sublist].size();
		int sub_request_size = request_size;
		if (sub_request_size > count)
			sub_request_size = count;
		vector<unsigned long long> qlist(count);
		for(size_t i = 0; i < count; ++i) {
			qlist[i] = teh_lists[sublist][i] ^ num;
		}
		partial_sort(qlist.begin(), qlist.begin() + sub_request_size, qlist.end());
		for(int i = 0; i < sub_request_size; ++i) {
			cout << (qlist[i] ^  num) << "\n";
		}
		return request_size - sub_request_size;
	}
};

int main(int argc, char **argv)
{
	int request_size = 0;
	const string prefix = "--request_size=";
	if (argc >= 2 && 0 == strncmp(argv[1], prefix.c_str(), prefix.size())) 	{
		request_size = atoi(argv[1] + prefix.size());
	}
	if (request_size == 0) {
		cerr << "invalid or missing --request_size=N param" << endl;
		cerr << "argc=" << argc << endl;
		for(int i = 0; i < argc; ++i) {
			cerr << "argv[" << i << "]=" << argv[i] << endl;
		}
		exit(1);
	}
	cerr << "Request size: " << request_size << endl;
	
	bool query = false;
	SmartHashThingy thingy;
	char filebuf[65536];
	cin.rdbuf()->pubsetbuf(filebuf, sizeof(filebuf));
	char linebuf[80];
	while(cin.getline(linebuf, 80)) {
		if (linebuf[0] == '\0') {
			query = true;
			continue;
		}
		unsigned long long num = strtoull(linebuf, NULL, 10);
		if (query) {
			thingy.query(num, request_size);
		} else {
			thingy.add(num);
		}
	}
}

