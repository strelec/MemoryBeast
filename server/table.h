struct Column;
struct Val;
struct AST;

struct Table {
	map<path, Column> columns;
	map<Val, u32> *index = nullptr;

	typedef map<vector<Val>, vector<vector<Val>>> rs;

	// current path stack
	path cur;
	u32 size = 0;

	Table();
	~Table();

	void unindex();

	Json::Value select(Json::Value q);
	u32 insert(Json::Value root);

	void cleanup();
	void report();

private:

	u32 doInsert(Json::Value& root);

	rs selectNormal(vector<AST> &what, AST &where, vector<AST> &group, i32 skip, u32 limit);

	rs selectAggregate(vector<AST> &what, AST &where, vector<AST> &group, i32 skip, u32 limit);

	Json::Value convert(vector<Val> vec);
};
