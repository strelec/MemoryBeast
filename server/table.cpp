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

	u32 current = 0;
	bool simple = true;
	bool agg = false;

	Json::Value select(Json::Value q) {
		simple = true;
		agg = false;

		map<vector<Val>, vector<vector<Val>>> res;
		vector<Val> key;
		vector<Val> vals;

		bool trueFilter = q["where"].type() == Json::nullValue;
		for(current=0; current<size; ++current) {
			if (trueFilter || eval(q["where"]).truey()) {
				key.clear();
				for(auto it: q["group"]) {
					key.push_back( eval(it) );
				}

				if (!agg || res[key].empty()) {

					vals.clear();
					for(auto it: q["what"]) {
						vals.push_back( eval(it) );
					}
					res[key].push_back(vals);

				} else { // correct previous row to aggregate
					vector<Val> &prev = res[key].back();

					int i = 0;
					for(auto it: q["what"]) {
						if (it.type() == Json::arrayValue) {
							string call = it[0u].asString();
							switch(call[0]) {
								case 'm':
									if (call == "max") {
										Val first = eval(it[1]);
										if (prev[i] < first)
											prev[i] = first;
									}
									if (call == "min") {
										Val first = eval(it[1]);
										if (prev[i] > first)
											prev[i] = first;
									}
								break; case 's':
									if (call == "sum") {
										prev[i] += eval(it[1]);
									}
								break; case 'c':
									if (call == "count") {
										if (eval(it[1]).type != NIL)
											++prev[i];
									}
							}
						}
						i++;
					}
				}
			}
		}

		// output json fron data structure
		Json::Value ret = Json::arrayValue;
		for(auto &it: res) {
			Json::Value tbl = Json::arrayValue;
			for(auto &vl: it.second)
				tbl.append(convert(vl));

			Json::Value group = Json::arrayValue;
			group.append(convert(it.first));
			group.append(tbl);

			ret.append(group);
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
					Val first = eval(expr[1]);
					switch(call[0]) {
						case '=':
							if (call == "=" || call == "==")
								return toVal( eval(expr[1]) == eval(expr[2]) );

						break; case '+':
							if (expr.size() >= 3)
								first += eval(expr[2]);
						break; case '-':
							if (expr.size() >= 3)
								first += eval(expr[2]);
							if (expr.size() >= 2)
								first.inv();
						break; case '*':
							if (expr.size() >= 3)
								first *= eval(expr[2]);
						break; case '/':
							if (expr.size() >= 3)
								first /= eval(expr[2]);
						break; case '^':
							if (expr.size() >= 3)
								first ^= eval(expr[2]);

						break; case 'o':
							if (call == "or")
								return toVal( eval(expr[1]).truey() || eval(expr[2]).truey() );
						break; case 'a':
							if (call == "and")
								return toVal( eval(expr[1]).truey() && eval(expr[2]).truey() );
						break; case 'n':
							if (call == "not")
								return toVal( !eval(expr[1]).truey() );

						break; case 'm':
							if (call == "max" || call == "min") {
								agg = true;
								return first;
							}
						break; case 's':
							if (call == "sum") {
								agg = true;
								return first;
							}
						break; case 'c':
							if (call == "count") {
								agg = true;
								Val r;
								r.type = INT;
								r.vInt = (first.type == NIL) ? 0 : 1;
								return r;
							}
					}
					return first;
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

	Val toVal(bool b) {
		Val v;
		v.type = BOOL;
		v.vBool = b;
		return v;
	}

	Json::Value convert(vector<Val> vec) {
		Json::Value v = Json::arrayValue;
		for(auto &el: vec)
			v.append( el.json() );
		return v;
	}
};
