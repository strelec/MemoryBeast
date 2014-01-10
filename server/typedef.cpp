typedef double real;

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef int8_t i8;
typedef int16_t i16;
typedef int32_t i32;
typedef int64_t i64;

struct u24 {
	u8 nums[3];

	u24() {
		set(0);
	}
	u24(u32 n) {
		set(n);
	}

	operator u32() {
		return (nums[0] << 16) | (nums[1] << 8) | nums[2];
	}

	void set(u32 n) {
		nums[0] = (n >> 16) & 0xFF;
		nums[1] = (n >> 8) & 0xFF;
		nums[2] = n & 0xFF;
	}

	void operator+=(u32 n) {
		set(u32() + n);
	}
};