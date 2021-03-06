#include <vector>
#include <stack>
#include <map>

#include <string>
#include <fstream>
#include <iostream>

#include <cassert>
#include <algorithm>

#include <json/json.h>
#include <json/writer.h>
using namespace std;

void info(string s) {
	cerr << "INFO: " << s << endl;
}

#include "typedef.cpp"
#include "exceptions.cpp"

#include "intColumn.cpp"
#include "lookup.cpp"

#include "table.h"

#include "column.cpp"
#include "ast.cpp"
#include "table.cpp"
#include "database.cpp"

#include "socket.cpp"

Database db;

void runServer(int port) {
	Socket s(port);
	cout << "Running server at port " << port << "." << endl;

	s.loop([&s] (string query) {
		Json::Value root;
		Json::Reader reader;
		reader.parse(query, root);

		Json::Value ret = Json::objectValue;
		try {
			string act = root["act"].asString();
			if (act == "insert") {
				string table = root["table"].asString();
				ret["count"] = db.load(root["data"], table);

			} else if (act == "select") {
				if (root["from"].isString()) {
					string table = root["from"].asString();
					ret["result"] = db.tables[table].select(root);
				} else {
					Table temp;
					temp.size = 1;
					ret["result"] = temp.select(root);
				}

			} else if (act == "tables") {
				for(auto &it: db.tables)
					ret[it.first] = it.second.size;

			} else if (act == "describe") {
				string table = root["table"].asString();
				ret["result"] = db.tables[table].report();

			} else if (act == "finalize") {
				string table = root["table"].asString();
				db.tables[table].cleanup();
				ret["result"] = "ok";

			} else {
				throw InvalidCommandE(act);
			}
		} catch(exception& e) {
			ret = Json::objectValue;
			ret["error"] = e.what();
		}

		Json::FastWriter writer;
		s.write( writer.write(ret) );
	});
}

void preloadFile(string file) {
	cout << "Preloading file " << file << "..." << endl;
	cout << "Done. Loaded " << db.load(file) << " rows." << endl;
}

int main(int argc, char* argv[]) {

	int port = 4567;

	if (argc == 1) {
		cout << "USAGE: ./" << argv[0] << " port [initial JSON]" << endl;
		cout << "Using default port " << port << " for now." << endl;
	}

	if (argc > 1)
		port = atoi(argv[1]);
	if (argc > 2)
		preloadFile(argv[2]);

	runServer(port);
}
