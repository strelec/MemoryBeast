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
	Val(const Val &b) {
		type = b.type;
		vStr = b.vStr; // copy whole union

		if (type == STR) {
			vStr = new string(*vStr);
		} else if (type == MFORN) {
			vTbl = new vector<u32>(*vTbl);
		}
	}
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

	Val& operator=(Val b) {
		destroy();

		type = b.type;
		b.type = NIL;
		vStr = b.vStr; // copy whole union
		return *this;
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
	real realize() const {
		if (type == REAL)
			return vReal;
		if (type == INT)
			return vInt;
		return 0;
	}

	bool operator<(const Val b) const {
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

	bool operator==(const Val b) const {
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

	bool operator>(const Val b) const {
		return !(*this < b) && !(*this == b);
	}

	void operator+=(const Val b) {
		if (type == INT) {
			if (b.type == INT) {
				vInt += b.vInt;
			} else {
				type = REAL;
				vReal = vInt + b.vReal;
			}
		} else if (type == REAL) {
			vReal += b.realize();
		} else {
			type = NIL;
		}
	}

	void operator-=(const Val b) {
		if (type == INT) {
			if (b.type == INT) {
				vInt -= b.vInt;
			} else {
				type = REAL;
				vReal = vInt - b.vReal;
			}
		} else if (type == REAL) {
			vReal -= b.realize();
		} else {
			type = NIL;
		}
	}

	void inv() {
		if (type == INT) {
			vInt *= -1;
		} else if (type == REAL) {
			vReal *= 1;
		} else {
			type = NIL;
		}
	}

	void operator*=(const Val b) {
		if (type == INT) {
			if (b.type == INT) {
				vInt *= b.vInt;
			} else {
				type = REAL;
				vReal = vInt * b.vReal;
			}
		} else if (type == REAL) {
			vReal *= b.realize();
		} else {
			type = NIL;
		}
	}

	void operator/=(const Val b) {
		if (b.realize() == 0) {
			type = NIL;
		} else if (type == INT) {
			if (b.type == INT) {
				vInt /= b.vInt;
			} else {
				type = REAL;
				vReal = vInt / b.vReal;
			}
		} else if (type == REAL) {
			vReal /= b.realize();
		} else {
			type = NIL;
		}
	}

	void operator^=(const Val b) {
		if (type == INT) {
			if (b.type == INT) {
				vInt = pow(vInt, b.vInt);
			} else {
				type = REAL;
				vReal = pow(vInt, b.vReal);
			}
		} else if (type == REAL) {
			vInt = pow(vReal, b.realize());
		} else {
			type = NIL;
		}
	}

	void operator++() {
		if (type == INT)
			vInt++;
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

	~Val() {
		destroy();
	}

private:

	void destroy() {
		if (type == STR) {
			delete vStr;
		} else if (type == MFORN) {
			delete vTbl;
		}
	}

	Val coerc(Val a, Val b,
	function<i64(i64 a, i64 b)> i, function<real(real a, real b)> r) {
		Val out;
		if (a.type == INT && b.type == INT) {
			out.type = INT;
			out.vInt = i(a.vInt, b.vInt);
		} else if (a.type == REAL || b.type == REAL) {
			out.type = REAL;
			out.vReal = r(a.realize(), b.realize());
		}
		return out;
	}
};