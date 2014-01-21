#include <exception>

struct HashExpressionE: public exception {
	const char* what() const noexcept {
		return "JSON Hash found in AST.";
	}
};

struct InvalidCommandE: public exception {
	string comm;

	InvalidCommandE(string comm) {
		this->comm = comm;
	}

	const char* what() const noexcept {
		return ("Invalid command: ." + comm).c_str();
	}
};


struct UnknownOperationE: public exception {
	string op;

	UnknownOperationE(string op) {
		this->op = op;
	}

	const char* what() const noexcept {
		return ("Unknown operation: " + op + ".").c_str();
	}
};

struct DatatypeMismatchE: public exception {
	int expected;
	int given;

	DatatypeMismatchE(int expected, int given) {
;		this->expected = expected;
		this->given = given;
	}

	const char* what() const noexcept {
		return ("Type mismatch on insertion, " + to_string(expected) + " expected, " + to_string(given) + " given.").c_str();
	}
};

struct ParameterMisnumberE: public exception {
	string op;
	string expe;
	int given;

	ParameterMisnumberE(string op, vector<int> expected, int given) {
		this->op = op;
		this->given = given;

		assert(expected.size() != 0);
		for(u8 i=0; i<expected.size()-1; ++i)
			expe.append(' ' + to_string(expected[i]) + ',');

		if (!expe.empty()) {
			expe.pop_back();
			expe.append(" or");
		}

		expe.append(' ' + to_string(expected.back()));
		expe.append(" parameter");
		if (expected.back() != 1)
			expe.append("s");
	}

	const char* what() const noexcept {
		return ("Operation " + op + " expects" + expe + ", " + to_string(given) + " given.").c_str();
	}
};