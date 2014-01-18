require 'readline'
require 'awesome_print'

require_relative "memoryBeast"

@comp = MemoryBeast.new({
	'127.0.0.1:5000' => 3000,
	'127.0.0.1:5001' => 3000,
})

#@comp.load 'data.json', 'data'

def exec(sql)
	ap @comp.sql sql
end

puts 'Welcome, user. Please start typing SQL commands. Few examples:'
puts 'SELECT 1+2*3, 2^10, LENGTH("Rok"), POSITION("Slovenia", "love")'
puts
loop {
	exec Readline.readline('>> ', true)
}
