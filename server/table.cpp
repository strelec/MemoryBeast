struct Table {
	map<path, Column> columns;
	map<string, u32> *index = nullptr;

	// current path stack
	path cur;
	u32 size = 0;

	Table() {
		index = new map<string, u32>;
	}

	~Table() {
		unindex();
	}

	void unindex() {
		delete index;
		index = nullptr;
	}

	bool dbg(bool a) {
		cout << a << endl;
		return a;
	}

	int current = 0;
	int simple = true;

	Json::Value select(Json::Value q) {
		Json::Value ret = Json::arrayValue;

		for(u32 i=0; i<size; ++i) {
			current = i;
			if (eval(q["where"]).truey()) {
				Json::Value o = Json::objectValue;
				for(auto it = q["what"].begin(); it != q["what"].end(); ++it) {
					o[ it.key().asString() ] = eval(*it).json();
				}
				ret.append(o);
			}
		}

		return ret;
	}

	Val eval(Json::Value expr) {
		switch(expr.type()) {
			case Json::arrayValue: {
				string call = expr[0u].asString();
				if (call == "get") {
					simple = false;

					path p;
					for(u32 i=1; i<expr.size(); ++i)
						p.push_back( expr[i].asString() );
					return Val(columns[p][current]);
				} else {
					switch(call[0]) {
						case '=':
							if (call == "==") {
								Val v;
								v.type = BOOL;
								v.vBool = eval(expr[1]) == eval(expr[2]);
								return v;
							}
						break; case '+':
							if (expr.size() >= 3)
								return plus(eval(expr[1]), eval(expr[2]));
							if (expr.size() >= 2)
								return eval(expr[1]);
						break; case '-':
							if (expr.size() >= 3)
								return minus(eval(expr[1]), eval(expr[2]));
							if (expr.size() >= 2)
								return minus(eval(expr[1]));
						break; case 'o':
							if (call == "or") {
								Val v;
								v.type = BOOL;
								v.vBool = dbg(eval(expr[1]).truey()) || dbg(eval(expr[2]).truey());
								return v;
							}
						break; case 'a':
							if (call == "and") {
								Val v;
								v.type = BOOL;
								v.vBool = eval(expr[1]).truey() && eval(expr[2]).truey();
								return v;
							}
						break; case 'n':
							if (call == "not") {
								Val v;
								v.type = BOOL;
								v.vBool = !eval(expr[1]).truey();
								return v;
							}
					}
				}
			} break;
			case Json::objectValue:
				cerr << "Invalid expression with Hash." << endl;
			break; default:
				return Val(expr);
		}
		return Val();
	}

	u32 insert(Json::Value root) {
		u32 temp = doInsert(root);
		size++;
		return temp;
	}

	void cleanup() {
		unindex();
		for(auto &c: columns) {
			c.second.lookup.unpopulate();
			c.second.cleanup();
			if (c.second.table)
				c.second.table->cleanup();
		}
	}

	void report() {
		for(auto &c: columns) {
			cout << endl << "COLUMN " << dispPath(c.first) << endl;
			c.second.report();
			if (c.second.table)
				c.second.table->report();
		}
	}

private:

	u32 doInsert(Json::Value root) {
		assert(root.isObject());

		u32 id = 0;
		if (false && index && root.isMember("id")) {
			cout << root["id"].toStyledString() << endl;
		}

		for(auto it = root.begin(); it != root.end(); ++it) {
			cur.push_back( it.key().asString() );

			switch((*it).type()) {
				case Json::objectValue:
					doInsert(*it);
				break;
				case Json::arrayValue: {
					Column &c = columns[cur];
					if (!c.table)
						c.table = new Table();

					auto *ids = new vector<u32>;
					for(auto el: *it)
						ids->push_back( c.table->insert(el) );
					c.push(	Val(ids), size );
				} break; default: {
					Column &c = columns[cur];
					c.push( Val(*it), size );
				}
			}
			cur.pop_back();
		}

		return id;
	}

private:

	Val plus(Val a, Val b) {
		Val v;
		v.type = INT;
		v.vInt = a.vInt + b.vInt;
		return v;
	}

	Val minus(Val a, Val b) {
		Val v;
		v.type = INT;
		v.vInt = a.vInt - b.vInt;
		return v;
	}

	Val minus(Val a) {
		Val v;
		v.type = INT;
		v.vInt = -a.vInt;
		return v;
	}
};