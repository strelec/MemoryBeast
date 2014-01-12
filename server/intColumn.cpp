#include "intStep.cpp"

template<template<class> class T>
struct BaseIntVector {
	T<u8> n8;
	T<u16> n16;
	T<u24> n24;
	T<u32> n32;
	T<u40> n40;
	T<u48> n48;

	int overflows = 0;

	bool push(i64 n) {
		int cur = current();
		if (cur == 0)
			if (!n8.push(n)) cur++;
		if (cur == 1)
			if (!n16.push(n)) cur++;
		if (cur == 2)
			if (!n24.push(n)) cur++;
		if (cur == 3)
			if (!n32.push(n)) cur++;
		if (cur == 4)
			if (!n40.push(n)) cur++;
		if (cur == 5)
			if (!n48.push(n)) cur++;
		if (cur == 6) {
			overflows++;
			cerr << "ERROR: IntColumn overflow (" << n << ")" << endl;
			return false;
		}
		return true;
	}

	int operator[](u32 pos) {
		return at(pos);
	}

	void expand(u32 n) {
		u32 siz = size();
		if (n < siz) {
			cerr << "ERROR: Column can only be expanded." << endl;
		} else if (n != siz) {
			switch(current()) {
					   case 0:
					n8.resize(n /* - siz + n8.size() */);
				break; case 1:
					n16.resize(n - siz + n16.size());
				break; case 2:
					n24.resize(n - siz + n24.size());
				break; case 3:
					n32.resize(n - siz + n32.size());
				break; case 4:
					n40.resize(n - siz + n40.size());
				break; case 5:
					n48.resize(n - siz + n48.size());
			}
		}
	}

	u32 size() {
		u32 sum = 0;
		for(auto sz: sizes())
			sum += sz;
		return sum;
	}

	void report() {
		int i = 0;
		for(auto sz: sizes())
			cout << ++i*8 << ": " << sz << ", ";
		cout << "++: " << overflows << endl;
	}

	void shrink_to_fit() {
		n8.shrink_to_fit();
		n16.shrink_to_fit();
		n24.shrink_to_fit();
		n32.shrink_to_fit();
		n40.shrink_to_fit();
		n48.shrink_to_fit();
	}

	void debug() {
		u32 siz = size();
		for(u32 i=0; i<siz; ++i)
			cout << at(i) << ", ";
		cout << endl;
	}

private:

	inline array<u32, 6> sizes() {
		return {n8.size(), n16.size(), n24.size(), n32.size(), n40.size(), n48.size()};
	}

	inline int current() {
		if (!n48.empty())
			return 5;
		if (!n40.empty())
			return 4;
		if (!n32.empty())
			return 3;
		if (!n24.empty())
			return 2;
		if (!n16.empty())
			return 1;
		return 0;
	}

	int at(u32 pos) {
		if (pos < n8.size())
			return n8[pos];
		pos -= n8.size();
		if (pos < n16.size())
			return n16[pos];
		pos -= n16.size();
		if (pos < n24.size())
			return n24[pos];
		pos -= n24.size();
		if (pos < n32.size())
			return n32[pos];
		pos -= n32.size();
		if (pos < n40.size())
			return n40[pos];
		pos -= n40.size();
		if (pos < n48.size())
			return n48[pos];
		cerr << "ERROR: IntColumn index out of bounds" << endl;
		return -1;
	}
};

typedef BaseIntVector<EcoIntStep> EcoIntVector;
typedef BaseIntVector<IntStep> IntVector;
