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
	real realize() {
		if (type == REAL)
			return vReal;
		if (type == INT)
			return vInt;
		return 0;
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