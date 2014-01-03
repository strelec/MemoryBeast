enum Type {
	NIL = 0,
	INT,
	STR,
	REAL,
	BOOL,

	//foreign keys
	FORN,
	MFORN
};

struct Val {
	Type type = NIL;

	union {
		i64 vInt;
		u32 vForn;
		string *vStr;
		real vReal;
		bool vBool;
		vector<u32> *vTbl;
	};

	Val() {
		type = NIL;
	}
	/*Val(const Val &b) {
		type = b.type;
		if (type == STR) {
			vStr = new string(*b.vStr);
		} else if (type == MFORN) {
			vTbl = new vector<u32>(*b.vTbl);
		}
	}*/
	Val(u32 val) {
		type = FORN;
		vForn = val;
	}
	Val(vector<u32> *val) {
		type = MFORN;
		vTbl = val;
	}
	Val(Json::Value val) {
		switch(val.type()) {
			case Json::intValue:
				type = INT;
				vInt = val.asInt();
			break; case Json::uintValue:
				type = INT;
				vInt = val.asUInt();
			break; case Json::realValue:
				type = REAL;
				vReal = val.asDouble();
			break; case Json::stringValue:
				type = STR;
				vStr = new string(val.asString());
			break; case Json::booleanValue:
				type = BOOL;
				vBool = val.asBool();
			default:
				type = NIL;
		}
	}

	bool truey() {
		switch(type) {
			case NIL:   return false;
			case INT:   return vInt;
			case STR:   return !vStr->empty();
			case BOOL:  return vBool;
			default:    return true;
		}
	}

	bool operator<(const Val& b) const {
		assert(type == b.type);
		assert(type != MFORN);

		if (type < b.type)
			return true;

		switch(type) {
			case INT:   return vInt < b.vInt;
			case FORN:  return vForn < b.vForn;
			case MFORN: return false;
			case REAL:  return vReal < b.vReal;
			case STR:   return *vStr < *b.vStr;
			case BOOL:  return vBool < b.vBool;
			default:    return false;
		}
	}

	bool operator==(const Val& b) const {
		assert(type != MFORN);

		if (type != b.type)
			return false;

		switch(type) {
			case INT:   return vInt == b.vInt;
			case FORN:  return vForn == b.vForn;
			case MFORN: return false;
			case REAL:  return vReal == b.vReal;
			case STR:   return *vStr == *b.vStr;
			case BOOL:  return vBool == b.vBool;
			default:    return true;
		}
	}

	Json::Value json() {
		Json::Value val;
		switch(type) {
			case INT:
				val = (Json::Value::Int)vInt;
			break; case FORN:
				val = (Json::Value::UInt)vForn;
			break; case MFORN:
				val = Json::arrayValue;
				for(u32 el: *vTbl)
					val.append(el);
			break; case REAL:
				val = vReal;
			break; case STR:
				val = *vStr;
			break; case BOOL:
				val = vBool;
			break; default:
				val = Json::Value::null;
		}
		return val;
	}

	void debug() {
		cout << type << ": " << json().toStyledString();
	}

	~Val() {
		return;
		if (type == STR) {
			delete vStr;
		} else if (type == MFORN) {
			delete vTbl;
		}
	}
};

struct Table;

struct Column {
	Type type = NIL;
	vector<bool> isNil;

	EcoIntVector vInt;

	Lookup lookup;
	IntVector vStr;

	vector<real> vReal;
	vector<bool> vBool;

	// Foreign tables
	Table *table = nullptr;
	IntVector vForn;

	IntVector vTbl;
	IntVector vTblStarts;

	Column() {
		vTblStarts.push(0);
	}

	void push(Val v, u32 size) {
		if (v.type == NIL) {
			isNil.resize(size);
			isNil.push_back(true);
		}

		if (v.type != NIL) {
			if (type == NIL)
				type = v.type;

			if (type != v.type) {
				cerr << "Type mismatch: " << type << " " << v.type << endl;
				cout << v.json().toStyledString() << endl;
			} else switch(v.type) {
				case INT:
					vInt.expand(size);
					vInt.push(v.vInt);
				break; case FORN:
					vForn.expand(size);
					vForn.push(v.vInt);
				break; case MFORN:
					if (v.vTbl->size()) {
						vTblStarts.expand(size+1);
						for(auto el: *v.vTbl)
							vTbl.push(el);
						vTblStarts.push(vTbl.size());
					}
				break; case STR:
					vStr.expand(size);
					vStr.push( lookup.add(*v.vStr) );
				break; case REAL:
					vReal.resize(size);
					vReal.push_back(v.vReal);
				break; case BOOL:
					vBool.resize(size);
					vBool.push_back(v.vBool);
				case NIL:;
			}
		}
	}

	Val operator[](u32 pos) {
		if (isNil.size() > pos && isNil[pos])
			return Val();

		Val v;
		v.type = type;
		switch(type) {
			break; case INT:
				v.vInt = vInt[pos];
			break; case FORN:
				v.vInt = vForn[pos];
			break; case MFORN: {
				auto *els = new vector<u32>;
				if (vTblStarts.size() > pos+1) {
					u32 end = vTblStarts[pos+1];
					for(u32 i=vTblStarts[pos]; i<end; ++i)
						els->push_back(vTbl[i]);
				}
				v.vTbl = els;
			} break; case STR:
				v.vStr = new string( lookup[ vStr[pos] ] );
			break; case REAL:
				v.vReal = vReal.at(pos);
			break; case BOOL:
				v.vBool = vBool.at(pos);
			break; default:
				v.type = NIL;
		}
		return v;
	}

	void cleanup() {
		lookup.unpopulate();

		isNil.shrink_to_fit();

		vInt.shrink_to_fit();
		vStr.shrink_to_fit();
		vReal.shrink_to_fit();
		vBool.shrink_to_fit();

		vForn.shrink_to_fit();
		vTbl.shrink_to_fit();
		vTblStarts.shrink_to_fit();
	}

	void report() {
		int nulls = count(isNil.begin(), isNil.end(), true);
		if (nulls)
			cout << "Null: " << nulls << " / " << isNil.size() << endl;

		if (vInt.size() != 0) {
			cout << "Int: " << endl;
			vInt.report();
		}
		if (vForn.size() != 0) {
			cout << "Foreign key: " << endl;
			vForn.report();
		}
		if (vStr.size() != 0) {
			cout << "String: " << lookup.size() << " / " << lookup.length() << endl;
			vStr.report();
		}
		if (!vReal.empty())
			cout << "Real: " << vReal.size() << endl;
		if (!vBool.empty())
			cout << "Bool: " << vBool.size() << endl;
		if (vTblStarts.size() > 1)
			cout << "Table: " << vTblStarts.size()-1 << " / " << vTbl.size() << endl;
	}
};
