#include "val.cpp"

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

			if (type != v.type)
				throw DatatypeMismatchE(type, v.type);

			switch(v.type) {
				case INT:
					vInt.expand(size);
					vInt.push(v.vInt);
				break; case FORN:
					vForn.expand(size);
					vForn.push(v.vForn);
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
			case INT:
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

	Json::Value report(u32 size) {
		Json::Value ret = Json::objectValue;

		u64 bytes = isNil.size()/8;
		u64 total = isNil.capacity()/8;
		u32 records = 0;
		ret["type"] = Type2string[type];

		switch(type) {
			case INT: {
				u32 i = 1;
				for(auto &el: vInt.sizes()) {
					ret["sizes"].append(el);
					bytes += el*i++;
				}

				ret["overflows"] = vInt.overflows;
				records = vInt.size();
			} break; case FORN:
				ret["table"] = table->report();
			break; case MFORN: {
				ret["table"] = table->report();
			} break; case STR:
				records = vStr.size();
				ret["unique"] = lookup.usage();
				bytes += lookup.length();
			break; case REAL:
				records = vReal.size();
				bytes += records*sizeof(real);
				total += vReal.capacity()*sizeof(real);
			break; case BOOL:
				records = vBool.size();
				bytes += records/8;
				total += vBool.capacity()/8;
			break; default:
				;
		}
		u64 nulls = count(isNil.begin(), isNil.end(), true) + (size-records);
		ret["nulls"] = (Json::Value::UInt64)nulls;
		ret["bytes"] = (Json::Value::UInt64)bytes;
		ret["overhead"] = (Json::Value::UInt64)(total - bytes);

		return ret;
	}
};
