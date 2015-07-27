#include <stdio.h>
#include <stdlib.h>
#include <map>
#include <vector>
#include <unordered_map>
#include <algorithm>

using namespace std;

#define MAX_PEOPLE 3456
#define BS_ELEMENT_SIZE (sizeof(unsigned long long))
#define BS_BITS (BS_ELEMENT_SIZE * 8)
#define BS_ELEMENTS (MAX_PEOPLE / BS_ELEMENT_SIZE)

// okay this is insane, but hey
class BigBitSet
{
	unsigned long long elements[BS_ELEMENTS];
	
	inline int location(int elem, int &bit_offset) const
	{
		int byte_offset = elem / BS_BITS;
		bit_offset = elem % BS_BITS;
		return byte_offset;
	}

public:	
	BigBitSet()
	{
		fill(elements, elements + BS_ELEMENTS, 0);
	}
	
	BigBitSet(const BigBitSet &other)
	{
		copy(other.elements, other.elements + BS_ELEMENTS, elements);
	}
	
	// performs an intersection (yes, this isn't obvious,
	// but this whole thing is crazy and I wanted to avoid unnecessarily zeroing)
	BigBitSet(const BigBitSet &lhs, const BigBitSet &rhs)
	{
		for(int i = 0; i < BS_ELEMENTS; ++i)
			elements[i] = lhs.elements[i] & rhs.elements[i];
	}
	
	void add(int elem)
	{
		int bit_offset;
		int byte_offset = location(elem, bit_offset);
		elements[byte_offset] |= (1ULL << bit_offset);
	}
	
	void remove(int elem)
	{
		int bit_offset;
		int byte_offset = location(elem, bit_offset);
		elements[byte_offset] &= ~(1ULL << bit_offset);	
	}
	
	bool include(int elem) const
	{
		int bit_offset;
		int byte_offset = location(elem, bit_offset);
		return 0 != (elements[byte_offset] & (1ULL << bit_offset));
	}
	
	bool empty() const
	{
		for(int i = 0; i < BS_ELEMENTS; ++i)
			if (elements[i] != 0)
				return false;
		return true;
	}
	
	class enumerator
	{
		enumerator(const BigBitSet &bitset) :
			m_bitset(bitset)
		{
			byte = 0;
			bit = 0;
		}
	
		const BigBitSet &m_bitset;
		int byte;
		int bit;
		
		friend class BigBitSet;
		
	public:
		int next()
		{
			while (byte < BS_ELEMENTS) {
				unsigned long long el = m_bitset.elements[byte];
				if (el != 0) {
					for(int i = bit; i < BS_BITS; ++i) {
						if (el & (1ULL << i)) {
							bit = i + 1;
							if (bit >= BS_BITS) {
								++byte;
								bit = 0;
							}
							return (byte * BS_BITS) + i;
						}
					}
				}
				bit = 0;
				++byte;
			}
			return -1;
		}
	};
	friend class enumerator;	
	
	enumerator enumerate() const
	{
		return enumerator(*this);
	}
	
	bool size_at_least(int size) const
	{
		int count = 0;
		enumerator e = enumerate();
		while(e.next() != -1)
		{
			if (++count >= size) 
				return true;
		}
		return false;
	}
	
};

#define LINE_LEN 80

typedef BigBitSet id_set;
struct Person
{
	id_set friends;
};
unordered_map<int, Person> g_people;

int min_group_size = 1;

void report_clique(const id_set &clique)
{
	if (!clique.size_at_least(min_group_size))
		return;
	id_set::enumerator e = clique.enumerate();
	for(;;) {
		int id = e.next();
		if (id < 0) break;
		printf("%d\n", id);
	}
	putchar('\n');
}

const id_set & find_pivot(const id_set &p, const id_set &x)
{
	int id = p.enumerate().next();
	if (id < 0) {
		id = x.enumerate().next();
	}
	return g_people[id].friends;
}

void find_cliques(id_set &p, id_set &r, id_set &x)
{
	if (p.empty() && x.empty()) {
		report_clique(r);
	} else {
		const id_set &skip = find_pivot(p, x);
		auto e = p.enumerate();
		for(;;) {
			int v = e.next();
			if (v < 0) break;
			if (skip.include(v))
				continue;

			const id_set &n = g_people[v].friends;
			
			id_set sub_p(p, n); // P intersect N(v)
			id_set sub_r(r); sub_r.add(v); // R union {v}
			id_set sub_x(x, n); // X intersect N (v)
			
			find_cliques(sub_p, sub_r, sub_x);
			
			x.add(v);
			p.remove(v);
		}
	}
}

int main(int argc, char **argv)
{
	char linebuf[LINE_LEN];
	
	fgets(linebuf, LINE_LEN, stdin);
	min_group_size = atoi(linebuf);
	id_set p;
	while(fgets(linebuf, LINE_LEN, stdin)) {
		int id0, id1;
		if (2 != sscanf(linebuf, "%d %d", &id0, &id1))
			break;
		p.add(id0);		
		g_people[id0].friends.add(id1);
		p.add(id1);
		g_people[id1].friends.add(id0);
	}
	id_set r, x;
	find_cliques(p, r, x);
	
	return 0;
}