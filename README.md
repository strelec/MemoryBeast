MemoryBeast
===========

Amazingly fast in-memory distributed (map-reduce) SQL-like data storage. It consists of multiple cluster instances, written in C++ for speed and memory efficency and one Ruby instance connecting them all together, distributing tasks.

In terms of classic map-reduce paradigm, you could say the clusters are map and ruby frontend is the reduce. The ruby frontend is also in charge of dispatching the data to all the clusters.

Limitations
---

The only permamanent limitation of this engine is inability to join multiple JSON files together. You can have multiple tables, but you can only select from one at the time.

Server
===

To run the server, you have to compile `engine.cpp` first. After that is done, you can then run the server:

```
./engine 5001
```

Here, 5001 is the port number where the cluster listens for the input.
If you want to preload the cluster with the data (to avoid network traffic), you are allowed to specify the second argument, the file name:

```
./engine 5001 initial.json
```

The program then responds:

```
Preloading file initial.json...
Done. Loaded 276328 rows.

Socket created
Running server at port 5011.
Waiting for incoming connections...
```

What do we do to save the memory
---

This storage engine is very memory efficient, using only about 10% memory compared to the size of the `.json` file. This means you could store 40GB file to a laptop with 4GB of ram.

- Column-based data storage: Data is stored in column based fashion - this way we only save the schema once, not once for every record.
- String lookup tables: For each column, we allocate one string lookup table. This is great for columns that have sparse set of possible values, like `male` and `female` and many others. This feature is dynamic; during the insertion, if the engine determines uniqueness of more than 30%, this lookup table self-destructs reducing its overhead to zero.
- Dynamic integers: The engine dynamically determines the size of the integer column (1, 2, 3, 4, 8 byte) and then seamlessly switches to the bigger one, if needed.
- Range-based integer values: In case of timestamps, they are usually big numbers, but their range is small, it usually spans just a few days or maybe months. This allows us to save the base and report the number relative to it, using smaller integer to save it.
- Three-byte integers: The engine supports three byte integers, saving one byte per record in columns that range in value form 2¹⁶ to the 2²⁴.
- Packing boolean values: `True` and `False` only consume one bit as we pack eight of these into one byte. This is actually the default behaviour of the C++ `vector<bool>` data structure.

Notes
---

- The cluster uses only one processor core, so you should run as many instances per box as there are cores to ensure maximum efficency.

- The C++ compiler requires C++11 compatiblity. GCC is fine if you pass `-std=c++11` flag to it. We recommend compiling with the `-O3` flag for performance. `-funroll-loops -march=native` only for the crazy ones.

To-Do
---

- [ ] Currently, only filtration (`WHERE`) is implemented. Aggregation & Grouping is still to be done, but's not hard to do.
- [ ] `Val struct` leaks memory.

Client
===

How to use it
---

First, you have to specify all the clusters with their IP addreses:

```ruby
comp = MemoryBeast.new({
	'127.0.0.1:5011' => 2000,
	'127.0.0.1:5012' => 2000,
})
```

The second number beside the IP designates how should the client divide the future data amongst the clusters. Cluster with two times higher number should have twice as much RAM & CPU power.

Then you are free to load the data (if you didn't do that on server initialization). Specify the file and table name:

```ruby
comp.load '../../sets/tiny.json', 'data'
```

Then just make queries:

```ruby
result = comp.select(
	table: 'data',
	what: ['id', 'creative.id'],
	where: 'adServerLoadAvg == 0.0'
)
```



The JSON communication protocol
---

During the execution of the query, the communication to clusters and back is clearly visible. For the above query, the client first sends it in an appropriate form to all the clusters.

```json
127.0.0.1:5012 << {"act":"select","table":"data","what":{"id":["get","id"],"creative.id":["get","creative","id"]},"where":["==",["get","adServerLoadAvg"],0.0]}
127.0.0.1:5011 << {"act":"select","table":"data","what":{"id":["get","id"],"creative.id":["get","creative","id"]},"where":["==",["get","adServerLoadAvg"],0.0]}
```

It then gets the response and assembles (reduces) it into the final result:

```json
127.0.0.1:5012 0.0007s >> {"result":[]}
127.0.0.1:5011 0.0404s >> {"result":[{"creative.id":"5f34dfaf","id":"s1377995572x52228b3438a667x73933957"},{"creative.id":"28c05792","id":"s1378068738x5223a90253b100x73986990"},{"creative.id":"dbae1ef6","id":"s1378093114x5224083a3d77b0x52199543"}]}
```

To-Do
---

- [ ] Add support for classical (complex) SQL statements like:

```SQL
SELECT 2+MAX(a*b) AS max, COUNT() AS count, c
FROM data
WHERE 2*d = e AND ISNULL(f)
GROUP BY c, 10*b
```

This is easy to do as the expressions parsers are already made.

- [ ] Write it in the form of Ruby gem.
