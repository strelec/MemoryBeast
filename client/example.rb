require 'awesome_print'

require_relative "memoryBeast"

comp = MemoryBeast.new({
	'127.0.0.1:5011' => 5000,
	'127.0.0.1:5012' => 5000,
})
comp.load '../../sets/tiny.json', 'data'

awesome_print comp.select(
	table: 'data',
	what: ['id', 'creative.id'],
	where: 'adServerLoadAvg == 0.0'
)