#include <vector>
#include <stack>
#include <map>

#include <string>
#include <fstream>
#include <iostream>

#include <cassert>
#include <algorithm>

#include <jsoncpp/json/json.h>
#include <jsoncpp/json/writer.h>
using namespace std;

typedef vector<string> path;

string dispPath(path p) {
	string s;
	for(string &px: p)
		s += px + ".";
	s.pop_back();
	return s;
}

#include "typedef.cpp"
#include "intColumn.cpp"
#include "lookup.cpp"

#include "column.cpp"
#include "table.cpp"
#include "database.cpp"

#include "socket.cpp"

void debug(string s) {
	cout << s << endl;
}

void pause(string s) {
	debug(s);
	int y;
	cin >> y;
}


Database db;

void runServer(int port) {
	Socket s(port);
	cout << "Running server at port " << port << "." << endl;

	s.loop([&s] (string query) {
		Json::Value root;
		Json::Reader reader;
		reader.parse(query, root);

		Json::Value ret = Json::objectValue;
		Json::FastWriter writer;

		string act = root["act"].asString();
		if (act == "insert") {
			string table = root["table"].asString();
			ret["ok"] = db.load(root["data"], table);

		} else if (act == "select") {
			string table = root["table"].asString();

			ret["result"] = db.tables[table].select(root);

		} else if (act == "eval") {
			ret["ok"] = db.tables.begin()->second.eval(root["expr"]).json();

		} else if (act == "tables") {
			for(auto &it: db.tables)
				ret[it.first] = it.second.size;

		} else if (act == "cleanup") {
			string table = root["table"].asString();
			db.tables[table].cleanup();
			ret["ok"] = true;
		} else {

			ret["error"] = "Invalid command.";
		}

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
		cout << "USAGE: ./" << argv[0] << " port [initial JSON]";
		cout << "Using default port " << port << " for now." << endl;
	}

	if (argc > 1)
		port = atoi(argv[1]);
	if (argc > 2)
		preloadFile(argv[2]);

	runServer(port);

	vector<Val> x;

	Val e1;
	e1.type = STR;
	e1.vStr = new string("asas");

	x.push_back(e1);
}