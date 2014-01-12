template<class T>
struct BaseIntStep {
	constexpr static int bits = sizeof(T)*8;
	constexpr static i64 maxValue = (1LL << bits) - 1;

	vector<T> v;

	void resize(u32 n) {
		if (v.empty()) {
			v.resize(n);
		} else {
			v.resize(n, v.back());
		}
	}

	u32 size() {
		return v.size();
	}

	bool empty() {
		return v.empty();
	}

	void shrink_to_fit() {
		v.shrink_to_fit();
	}

	void debug() {
		for(T &el: this->v)
			cout << (i64)el << ", ";
		cout << endl;
	}
};

template<class T>
struct EcoIntStep : public BaseIntStep<T> {
	constexpr static i64 spare = (BaseIntStep<T>::maxValue+1) >> 3;

	i64 base;
	i64 max;

	bool push(i64 n) {
		if (this->v.empty()) {
			base = n-spare;
			this->v.push_back(spare);
			max = spare;
		} else {
			const i64 &mValue = BaseIntStep<T>::maxValue;
			if(n < base) {
				i64 diff = mValue - (base+max-n);
				if (diff < 0) // not enough space
					return false;

				i64 dec = base-n + min(diff, spare);
				cerr << "INFO: Moving array by " << dec << endl;

				base -= dec;
				max += dec;
				for(T &el: this->v)
					el += dec;
			} else {
				if(n-base > mValue) // too little left
					return false;
			}
			this->v.push_back(n-base);
			max = ::max(n-base, max);
		}
		return true;
	}

	int operator[](u32 pos) {
		return base + this->v[pos];
	}

	void debug() {
		cout << "Base: " << base << endl;
		cout << "Max:  " << max << endl;
		BaseIntStep<T>::debug();
	}
};

template<class T>
struct IntStep : public BaseIntStep<T> {
	constexpr static int bits = sizeof(T)*8;
	constexpr static i64 maxValue = (1LL << bits) - 1;

	bool push(i64 n) {
		if (n > maxValue)
			return false;
		this->v.push_back(n);
		return true;
	}

	int operator[](u32 pos) {
		return this->v[pos];
	}
};