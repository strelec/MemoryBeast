#include <unistd.h>

struct Database {
	map<string, Table> tables;

	u32 load(Json::Value& array, string name="data") {
		Table &t = tables[name];
		t.unindex();

		u32 i = 0;
		for(auto line: array) {
			i++;
			t.insert(line);
		}
		return i;
	}

	u32 load(string file, string name="data") {
		ifstream f(file);
		if (!f.is_open())
			return -1;

		Table &t = tables[name];
		t.unindex();

		u32 i = 0;
		string line;
		while(getline(f, line)) {
			processLine(line, t);
			if (!(++i % 10000)) cout << i << endl;
		}

		f.close();
		return i;
	}

	void processLine(string line, Table& table) {
		Json::Value root;

		Json::Reader reader;
		reader.parse(line, root);
		table.insert(root);
	}
};