MemoryBeast
===========

Amazingly fast in-memory distributed (map-reduce) SQL-like data storage. It consists of multiple server instances, written in C++ for speed and memory efficency and one Ruby instance connecting them all together, distributing tasks.

In terms of classic map-reduce paradigm, you could say the clusters are map and ruby frontend is the reduce. The ruby frontend is also in charge of dispatching the data to all the instances.

Limitations
---

- Inability to join multiple JSON files together. You can have multiple tables (for multiple users / datasets), but you can only select from one at the time.
- Insert only (warehousing). No deletions or updates.

Server
===

To run the server, you have to compile `engine.cpp` first. After that is done, you can then run the server:

```
./engine 5001
```

Here, 5001 is the port number where the instance listens for the input.
If you want to preload the data (to avoid network traffic), you are allowed to specify the second argument, the file name:

```
./engine 5001 initial.json
```

The program then responds:

```
Preloading file initial.json...
Done. Loaded 276328 rows.

Socket created
Running server at port 5001.
Waiting for incoming connections...
```

What do we do to save the memory
---

This storage engine is very memory efficient, using only about 10% memory compared to the size of the `.json` file. This means you could store 40GB file to a laptop with 4GB of ram. The engine is able to perform all the optimisations below *without the user intervention!*

- **Column-based data storage:** Data is stored in column based fashion - this way we only save the schema once, not once for every record.
- **String lookup tables:** For each column, we allocate one string lookup table. This is great for columns that have sparse set of possible values, like `male` and `female` and many others. This feature is dynamic; during the insertion, if the engine determines uniqueness of more than 30%, this lookup table self-destructs reducing its overhead to zero.
- **Dynamic integers:** The engine dynamically determines the size of the integer column (1, 2, 3, 4, 5, 6 byte) and then seamlessly switches to the bigger one, if needed.
- **Range-based integer values:** In case of timestamps, they are usually big numbers, but their range is small, it usually spans just a few days or maybe months. This allows us to save the base and report the number relative to it, using smaller integer to save it.
- **Three-byte integers:** The engine supports three byte integers, saving one byte per record in columns that range in value form 2¹⁶ to the 2²⁴.
- **Packing boolean values:** `True` and `False` only consume one bit as we pack eight of these into one byte. This is actually the default behaviour of the C++ `vector<bool>` data structure.

- **Auto normalisation** (planned for 1.1): When you insert the json, you typically have to denormalise 1:n and n:n relations by storing the same data multiple times. If you supply the `id` field with the subobject, it is stored only once per unique id.

Notes
---

- The instance uses only one processor core, so you should run as many instances per box as there are cores to ensure maximum efficiency.

- You have to have jsoncpp 0.6.0 or higher installed. It is currently a release candidate, so make sure you got the correct version.

Client
===

How to use it
---

First, you have to specify all the instances with their IP addreses:

```ruby
comp = MemoryBeast.new({
	'127.0.0.1:5011' => 2000,
	'127.0.0.1:5012' => 2000,
})
```

The second number beside the IP designates how should the client divide the future data amongst the instances. Instance with two times higher number should have twice as much RAM & CPU power.

Then you are free to load the data (if you didn't do that on server initialization). Specify the file and table name:

```ruby
comp.load '../../sets/tiny.json', 'data'
```

Then just make queries:

```ruby
result = comp.select(
	select: {average: 'avg(adServerLoadAvg)', max: 'max(adServerLoadAvg)'},
	from: 'data',
	where: 'adServerLoadAvg != 0.0',
	group: ['sdk', 'creativeVersion'],
)
```

or in plain SQL:

```ruby
result = comp.sql <<-SQL
	SELECT
		AVG(adServerLoadAvg) AS average, MAX(adServerLoadAvg)
	FROM data
	WHERE adServerLoadAvg != 0.0
	GROUP BY sdk, creativeVersion
SQL
```

Operations & functions
===

Example of a query:

```sql
SELECT SUM(LENGTH(id)), AVG(load), (3*10-2)^2 AS squared
FROM data
WHERE public AND POSITION(userAgent, "Firefox")
GROUP BY 10 * version
LIMIT 100, 30
```

Operations
---

- Aggregations: `MIN`, `MAX`, `SUM`, `COUNT`, `AVG`
- Comparison: `=` (alias `==`), `!=`
- Arithmetic: `+`, `-`, `*`, `/`, `^` (exponentation)
- Logic: `OR`, `AND`, `NOT`
- String: `LENGTH`, `POSITION(haystack, needle)` (substring search, 0 if not found)

Easy to add new ones.

The JSON communication protocol
---

```sql
SELECT
	COUNT(adServerLoadAvg) AS total,
	AVG(adServerLoadAvg) AS expected
FROM data
GROUP BY sdk
```

During the execution of the query, the communication to servers and back is clearly visible. For the above query, the client first sends it in an agreed-upon form to all the instances.
Notice how `AVG` gets separated into `COUNT` and `SUM`.

```json
127.0.0.1:5000 << {"act":"select","table":"data","what":[["count",["get","adServerLoadAvg"]],["sum",["get","adServerLoadAvg"]]],"where":true,"group":[["get","sdk"]],"limit":100,"offset":0}
127.0.0.1:5001 << {"act":"select","table":"data","what":[["count",["get","adServerLoadAvg"]],["sum",["get","adServerLoadAvg"]]],"where":true,"group":[["get","sdk"]],"limit":100,"offset":0}
```

It then waits for the response from all of them:

```json
127.0.0.1:5000 2.06s >> {"result":[[["AdMarvel"],[[168315,2960.050000000717]]],[["Mobclix"],[[13306,220.2700000000012]]],[["MobileWeb"],[[65003,1042.479999999818]]]]}
127.0.0.1:5001 2.19s >> {"result":[[["AdMarvel"],[[168054,2954.320000000776]]],[["Mobclix"],[[13433,247.7700000000005]]],[["MobileWeb"],[[64513,1072.909999999805]]]]}
```

... and assembles (reduces) it into the final result:
```json
{
     [ "AdMarvel" ] => [
        [0] {
               :total => 336369,
            :expected => 0.017582981784889488
        }
    ],
      [ "Mobclix" ] => [
        [0] {
               :total => 26739,
            :expected => 0.017504020344814752
        }
    ],
    [ "MobileWeb" ] => [
        [0] {
               :total => 129516,
            :expected => 0.016333039933287184
        }
    ]
}
```