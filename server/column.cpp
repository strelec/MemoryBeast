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

			if (type != v.type) {
				cerr << "Type mismatch: " << type << " " << v.type << endl;
				cout << v.json().toStyledString() << endl;
			} else switch(v.type) {
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
