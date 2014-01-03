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

Notes
---

The cluster uses only one processor core, so you should run as many instances as there are cores to ensure maximum efficency.

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
awesome_print comp.select(
	table: 'data',
	what: ['id', 'creative.id'],
	where: 'adServerLoadAvg == 0.0'
)
```

The second number designates how should the client divide the future data amongst the clusters. Cluster with two times higher number should have twice as much RAM & CPU power.


To-Do
---

Write it in the form of Ruby gem.
