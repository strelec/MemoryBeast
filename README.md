MemoryBeast
===========

Amazingly fast in-memory distributed SQL-like data storage. It consists of multiple cluster instances, written in C++ for speed and memory efficency and one Ruby instance connecting them all together, distributing tasks.

In terms of classic map-reduce paradigm, you could say the clusters are map and ruby frontend is the reduce. The ruby frontend is also in charge of dispatching the data to all the clusters.

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
Socket created
bind done
Running server at port 5011.
Waiting for incoming connections...
```

Notes
---

- The cluster uses only one processor core, so you should run as many instances per box as there are cores to ensure maximum efficency.

- The C++ compiler requires C++11 compatiblity. GCC is fine.

To-Do
---

Currently, only filtration (`WHERE`) is implemented. Aggregation & Grouping is still to be done.

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

Then you are free to load the data (if you didn't do that before). Specify the file and table name:

comp.load '../../sets/tiny.json', 'data'

Then just make queries:

```ruby
result = comp.select(
	table: 'data',
	what: ['id', 'creative.id'],
	where: 'adServerLoadAvg == 0.0'
)
```

The second number designates how should the client divide the future data amongst the clusters. Cluster with two times higher number should have twice as much RAM & CPU power.

The communication between the client and servers
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

Write it in the form of Ruby gem.
