struct Lookup {
	// how big table has to get to start the checks
	constexpr static u32 treshold = 10000;
	// uniqueness percent to automatically stop
	constexpr static int percent = 30;
	// uniqueness percent to keep at all times
	constexpr static int threshold = 2;

	map<string, u32>* lookup;
	u32 records = 0;

	string data;
	IntVector table;

	Lookup() {
		table.push(0);
		populate();
	}

	~Lookup() {
		delete lookup;
	}

	u32 add(string s) {
		if(!(++records % treshold))
			maintain();

		if(lookup) {
			auto it = lookup->find(s);
			if (it != lookup->end())
				return it->second;
			return (*lookup)[s] = append(s);
		}
		return append(s);
	}

	string operator[](int pos) {
		return get(pos);
	}

	u32 size() {
		return table.size()-1;
	}

	u64 length() {
		return data.size();
	}

	void populate() {
		lookup = new map<string, u32>;
		for(u32 i=0; i<size(); ++i)
			(*lookup)[get(i)] = i;
	}

	void unpopulate() {
		if (usage() < threshold) {
			delete lookup;
			lookup = nullptr;
		}
	}

	double usage() {
		return 100.0*table.size()/records;
	}

private:

	string get(int pos) {
		int start = table[pos];
		return data.substr(start, table[pos+1]-start );
	}

	u32 append(string s) {
		data.append(s);
		table.push(data.size());
		return size()-1;
	}

	void maintain() {
		if (!lookup)
			return;

		info("Lookup check: " + to_string(usage()) + "%");
		if (usage() > percent) {
			info("Disabling lookup");
			unpopulate();
		}
	}
};
