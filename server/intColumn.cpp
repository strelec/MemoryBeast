#include "intStep.cpp"

template<template<class> class T>
class BaseIntVector {
	T<u8> n8;
	T<u16> n16;
	T<u24> n24;
	T<u32> n32;
	T<u40> n40;
	T<u48> n48;

	u8 cur = 0;

	u32 _size = 0;
	u32 _overflows = 0;

public:

	bool push(i64 n) {
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

		if (cur == 5 && !n48.push(n)) {
			info("IntColumn overflow: " + to_string(n));
			_overflows++;
			push( size() ? (*this)[0] : 0 );
			return false;
		}
		_size++;

		// DEVELOPMENT:
		// assert(n == (*this)[size()-1]);
		return true;
	}

	i64 operator[](u32 pos) {
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

		assert(pos >= size()); // Row number out of bounds
		return -1;
	}

	void expand(u32 n) {
		assert(n >= size()); // Column can only be expanded.

		if (n != size()) {
			switch(cur) {
					   case 0:
					n8.resize(n /* - size() + n8.size() */);
				break; case 1:
					n16.resize(n - size() + n16.size());
				break; case 2:
					n24.resize(n - size() + n24.size());
				break; case 3:
					n32.resize(n - size() + n32.size());
				break; case 4:
					n40.resize(n - size() + n40.size());
				break; case 5:
					n48.resize(n - size() + n48.size());
			}
			_size = n;
		}
	}

	inline u32 overflows() {
		return _overflows;
	}

	inline u32 size() {
		return _size;
	}

	inline array<u32, 6> sizes() {
		return {n8.size(), n16.size(), n24.size(), n32.size(), n40.size(), n48.size()};
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
		for(u32 i=0; i<size(); ++i)
			cout << (*this)[i] << ", ";
		cout << endl;
	}
};

typedef BaseIntVector<EcoIntStep> EcoIntVector;
typedef BaseIntVector<IntStep> IntVector;
