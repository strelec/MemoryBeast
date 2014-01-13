struct AST {

	enum Op {
		VAL,
		GET,
		CALL,
		AGG
	} op;

	Val val;
	Column* get;

	int func;
	vector<AST> params;

	const vector<string> calls = {
		"=", "==", "!=",
		"+", "-", "*", "/", "^",
		"or", "and", "not",
		"length", "position"
	};
	const vector<string> aggs = {"max", "min", "sum", "count"};

	AST(Json::Value& expr, map<path, Column>& columns) {
		op = VAL;
		val = Val();

		switch(expr.type()) {
			case Json::arrayValue: {
				string call = expr[0].asString();
				if (call == "get") {
					path p;
					for(u32 i=1; i<expr.size(); ++i)
						p.push_back( expr[i].asString() );

					if (columns.end() != columns.find(p)) {
						op = GET;
						get = &columns[p];
					}
				} else {
					if (!determineFunc(call))
						cerr << "Invalid function / operator " << call << "." << endl;
					for(u32 i=1; i<expr.size(); ++i)
						params.push_back(AST(expr[i], columns));
				}
			} break;
			case Json::objectValue:
				cerr << "Invalid expression with Hash." << endl;
			break; default:
				val = Val(expr);
		}
	}

	Val eval(u32 row) {
		if (op == VAL)
			return val;

		if (op == GET)
			return (*get)[row];

		Val first = params[0].eval(row);

		if (op == CALL) switch(func) {
			case 0: case 1: // ==
				return toVal( first == params[1].eval(row) );
			break; case 2: // !=
				return toVal( first != params[1].eval(row) );

			break; case 3: // +
				if (params.size() >= 2)
					first += params[1].eval(row);
			break; case 4: // -
				if (params.size() >= 2)
					first += params[1].eval(row);
				if (params.size() >= 1)
					first.inv();
			break; case 5: // *
				first *= params[1].eval(row);
			break; case 6: // /
				first /= params[1].eval(row);
			break; case 7: // ^
				first ^= params[1].eval(row);

			break; case 8: // or
				return toVal( first.truey() || params[1].eval(row).truey() );
			break; case 9: // and
				return toVal( first.truey() && params[1].eval(row).truey() );
			break; case 10: // not
				return toVal( !first.truey() );

			break; case 11: { // length
				if (first.type != STR)
					return Val();
				Val r(INT);
				r.vInt = first.vStr->size();
				return r;
			} break; case 12: { //position
				if (first.type != STR)
					return Val();
				Val needle = params[1].eval(row);
				if (needle.type != STR)
					return Val();

				Val r(INT);
				u32 pos = first.vStr->find( *needle.vStr );
				r.vInt = (pos == string::npos) ? 0 : pos+1;
				return r;
			}
		}

		if (op == AGG) switch(func) {
			case 0: case 1: case 2:
				// min & max & sum
				return first;
			case 3: // count
				Val r(INT);
				r.vInt = (first.type == NIL) ? 0 : 1;
				return r;
		}

		return first;
	}

	void aggregate(Val &prev, u32 row) {
		if (op != AGG)
			return;

		Val first = eval(row);

		switch(func) {
			case 0: // min
				if (prev > first)
					prev = first;
			break; case 1: // max
				if (prev < first)
					prev = first;
			break; case 2: // sum
				prev += first;
			break; case 3: // count
				if (first.type != NIL)
					++prev;
		}
	}

	bool isAgg() {
		if (op == AGG)
			return true;

		if (op == CALL)
			for(auto &it: params)
				if (it.isAgg())
					return true;

		return false;
	}

private:

	bool determineFunc(string call) {
		int pos = 0;
		for(auto &it: aggs) {
			if (it == call) {
				op = AGG;
				func = pos;
				return true;
			}
			pos++;
		}

		pos = 0;
		for(auto &it: calls) {
			if (it == call) {
				op = CALL;
				func = pos;
				return true;
			}
			pos++;
		}

		return false;
	}

	Val toVal(bool b) {
		Val v(BOOL);
		v.vBool = b;
		return v;
	}
};