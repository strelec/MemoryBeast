	Table::Table() {
		index = new map<Val, u32>;
	}

	Table::~Table() {
		unindex();
	}

	void Table::unindex() {
		delete index;
		index = nullptr;
	}

	Json::Value Table::select(Json::Value q) {
		bool agg = false;

		vector<AST> what;
		for(auto &it: q["select"]) {
			what.push_back( AST(it, this) );
			if (what.back().isAgg())
				agg = true;
		}

		AST where(q["where"], this);

		vector<AST> group;
		for(auto &it: q["group"])
			group.push_back( AST(it, this) );

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

	u32 Table::insert(Json::Value root) {
		assert(root.isObject());

		// auto normalisation feature
		if (index && root.isMember("id")) {
			auto pos = index->find(Val(root["id"]));
			if (pos == index->end()) {
				(*index)[Val(root["id"])] = size;
			} else {
				return pos->second;
			}
		}

		doInsert(root);
		return size++;
	}

	vector<Column*> Table::findColumn(path p) {
		auto pos = columns.find(p);
		if (pos != columns.end())
			return {&pos->second};

		u8 maxMatch = 0;
		Column *current = nullptr;
		for(auto &it: columns) {
			u8 match = 0;
			while(it.first[match] == p[match])
				match++;
			if (match > maxMatch) {
				maxMatch = match;
				current = &it.second;
			}
		}
		if (!current)
			return {nullptr};

		assert(p.size() != maxMatch);

		path subpath = path(p.begin()+maxMatch, p.end());
		auto ret = current->table->findColumn(subpath);
		ret.push_back(current);
		return ret;
	}

	void Table::cleanup() {
		unindex();
		for(auto &c: columns) {
			c.second.lookup.unpopulate();
			c.second.cleanup();
			if (c.second.table)
				c.second.table->cleanup();
		}
	}

	Json::Value Table::report() {
		Json::Value ret;
		ret["type"] = "table";
		ret["rows"] = size;
		for(auto &c: columns)
			ret["columns"][dispPath(c.first)] = c.second.report(size);
		return ret;
	}

	void Table::doInsert(Json::Value& root) {
		for(auto it = root.begin(); it != root.end(); ++it) {
			cur.push_back( it.key().asString() );

			switch((*it).type()) {
				case Json::objectValue:
					// auto normalisation
					if ((*it).isMember("id")) {
						Column &c = columns[cur];
						if (!c.table)
							c.table = new Table();

						u32 id = c.table->insert(*it);
						c.push( Val(id), size );
					} else {
						doInsert(*it);
					}
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
	}

	Table::rs Table::selectNormal(vector<AST> &what, AST &where, vector<AST> &group, i32 skip, u32 limit) {
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

	Table::rs Table::selectAggregate(vector<AST> &what, AST &where, vector<AST> &group, i32 skip, u32 limit) {
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

	Json::Value Table::convert(vector<Val> vec) {
		Json::Value v = Json::arrayValue;
		for(auto &el: vec)
			v.append( el.json() );
		return v;
	}