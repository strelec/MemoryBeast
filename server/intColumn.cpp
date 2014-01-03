template<class T>
struct EcoIntStep {
	constexpr static int bits = sizeof(T)*8;
	constexpr static i64 maxValue = (1LL << bits) - 1;
	constexpr static i64 spare = (maxValue+1) >> 3;

	vector<T> v;
	i64 base;
	i64 max;

	bool push(int n) {
		if (v.empty()) {
			base = n-spare;
			v.push_back(spare);
			max = spare;
		} else {
			if(n < base) {
				i64 diff = maxValue - (base+max-n);
				if (diff < 0) // not enough space
					return false;

				i64 dec = base-n + min(diff, spare);
				cerr << "INFO: Moving array by " << dec << endl;
				base -= dec;
				max += dec;
				for(T &el: v)
					el += dec;
			} else {
				if(n-base > maxValue) // too little left
					return false;
			}
			v.push_back(n-base);
			max = std::max(n-base, max);
		}
		return true;
	}

	int operator[](int pos) {
		return base + v[pos];
	}

	void resize(int n) {
		if (v.empty()) {
			v.resize(n);
		} else {
			v.resize(n, v.back());
		}
	}

	u32 size() {
		return v.size();
	}

	void shrink_to_fit() {
		v.shrink_to_fit();
	}

	void debug() {
		cout << "Base: " << base << endl;
		cout << "Max:  " << max << endl;
		for(T &el: v)
			cout << (i64)el << ", ";
		cout << endl;
	}
};

template<class T>
struct IntStep {
	constexpr static int bits = sizeof(T)*8;
	constexpr static i64 maxValue = (1LL << bits) - 1;

	vector<T> v;

	bool push(int n) {
		if (n > maxValue)
			return false;
		v.push_back(n);
		return true;
	}

	int operator[](u32 pos) {
		return v[pos];
	}

	void resize(int n) {
		if (v.empty()) {
			v.resize(n);
		} else {
			v.resize(n, v.back());
		}
	}

	u32 size() {
		return v.size();
	}

	void shrink_to_fit() {
		v.shrink_to_fit();
	}

	void debug() {
		for(T &el: v)
			cout << (i64)el << ", ";
		cout << endl;
	}
};

template<template<class> class T>
struct BaseIntVector {
	T<u8> n8;
	T<u16> n16;
	T<u24> n24;
	T<u32> n32;

	int overflows = 0;

	bool push(int n) {
		int cur = current();
		if (cur == 0)
			if (!n8.push(n)) cur++;
		if (cur == 1)
			if (!n16.push(n)) cur++;
		if (cur == 2)
			if (!n24.push(n)) cur++;
		if (cur == 3)
			if (!n32.push(n)) cur++;
		if (cur == 4) {
			overflows++;
			cerr << "ERROR: IntColumn overflow (" << n << ")" << endl;
			return false;
		}
		return true;
	}

	int operator[](u32 pos) {
		return at(pos);
	}

	void expand(int n) {
		switch(current()) {
			       case 0:
				n8.resize(n);
			break; case 1:
				n16.resize(n - n8.size());
			break; case 2:
				n24.resize(n - n8.size() - n16.size());
			break; case 3:
				n32.resize(n - n8.size() - n16.size() - n24.size());
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
	}

	void debug() {
		u32 siz = size();
		for(u32 i=0; i<siz; ++i)
			cout << at(i) << ", ";
		cout << endl;
	}

private:

	inline array<u32, 4> sizes() {
		return {n8.size(), n16.size(), n24.size(), n32.size()};
	}

	inline int current() {
		if (n32.size())
			return 3;
		if (n24.size())
			return 2;
		if (n16.size())
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
		cerr << "ERROR: IntColumn index out of bounds" << endl;
		return -1;
	}
};

typedef BaseIntVector<EcoIntStep> EcoIntVector;
typedef BaseIntVector<IntStep> IntVector;
