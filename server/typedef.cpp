typedef double real;

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef int8_t i8;
typedef int16_t i16;
typedef int32_t i32;
typedef int64_t i64;

template<int size, class B>
struct baseU {
	u8 nums[size];

	baseU() {
		set(0);
	}
	baseU(B n) {
		set(n);
	}

	operator B() {
		B ret = 0;
		for(int i=0; i<size; ++i) {
			ret <<= 8;
			ret |= nums[i];
		}
		return ret;
	}

	void set(B n) {
		for(int i=size-1; i>=0; --i) {
			nums[i] = n;
			n >>= 8;
		}
	}

	void operator+=(B n) {
		set(B() + n);
	}
};

typedef baseU<3, u32> u24;
typedef baseU<5, u64> u40;
typedef baseU<6, u64> u48;

typedef vector<string> path;

string dispPath(path p) {
	string s;
	for(string &px: p)
		s += px + ".";
	if (!p.empty())
		s.pop_back();
	return s;
}