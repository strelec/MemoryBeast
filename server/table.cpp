struct Table {
	map<path, Column> columns;
	map<string, u32> *index = nullptr;

	typedef map<vector<Val>, vector<vector<Val>>> rs;

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

	Json::Value select(Json::Value q) {
		bool agg = false;

		vector<AST> what;
		for(auto &it: q["select"]) {
			what.push_back( AST(it, columns) );
			if (what.back().isAgg())
				agg = true;
		}

		AST where(q["where"], columns);

		vector<AST> group;
		for(auto &it: q["group"])
			group.push_back( AST(it, columns) );

		i32 skip = q["offset"].asUInt();
		u32 limit = q["limit"].asUInt();

		const rs& res = agg ?
			selectAggregate(what, where, group, skip, limit) :
			selectNormal(what, where, group, skip, limit);

		// output json fron data structure
		Json::Value ret = Json::arrayValue;
		for(auto &it: res) if (!it.second.empty()) {
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

	u32 doInsert(Json::Value& root) {
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

	rs selectNormal(vector<AST> &what, AST &where, vector<AST> &group, i32 skip, u32 limit) {
		rs res;
		vector<Val> key;
		vector<Val> vals;

		for(u32 i=0; i<size; ++i) {
			if (where.eval(i).truey()) {
				key.clear();
				for(auto it: group)
					key.push_back( it.eval(i) );

				if (--skip < 0) {
					if (limit-- == 0)
						break;

					vals.clear();
					for(auto it: what)
						vals.push_back( it.eval(i) );

					res[key].push_back(vals);
				}
			}
		}
		return res;
	}

	rs selectAggregate(vector<AST> &what, AST &where, vector<AST> &group, i32 skip, u32 limit) {
		rs res;
		vector<Val> key;
		vector<Val> vals;

		for(u32 i=0; i<size; ++i) {
			if (where.eval(i).truey()) {
				key.clear();
				for(auto it: group)
					key.push_back( it.eval(i) );

				auto pos = res.find(key);
				if (pos == res.end()) {
					if (--skip < 0 && limit > 0) {
						limit--;

						vals.clear();
						for(auto it: what)
							vals.push_back( it.eval(i) );

						res[key].push_back(vals);
					} else {
						res[key];
					}
				} else if (!pos->second.empty()) { // update previous aggregate value
					vector<Val> &prev = pos->second.back();

					int col = 0;
					for(auto it: what)
						it.aggregate(prev[col++], i);
				}
			}
		}
		return res;
	}

	Json::Value convert(vector<Val> vec) {
		Json::Value v = Json::arrayValue;
		for(auto &el: vec)
			v.append( el.json() );
		return v;
	}
};
