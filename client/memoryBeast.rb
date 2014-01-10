require 'json'

require_relative 'client'

def debug(str)
	$stderr.puts str
end

class MemoryBeast
	def initialize(clients)
		@clients = clients.map { |k,v|
			Client.new(*k.split(':')).tap { |c|
				c.capability = v
			}
		}
	end

	attr_reader :clients

	def load(file, table='data')
		data = []
		stage = 0

		send = -> stage, data {
			clients[stage].preserve = true
			clients[stage].run :load, data, table
		}

		File.open(file, 'r') { |f|
			f.each { |line|
				data << line.chomp
				if data.size >= clients[stage].capability
					send[stage, data]
					data = Array.new
					stage = (stage + 1) % clients.size
				end
			}
		}
		send[stage, data]

		clients.each { |c|
			c.result
		}
		debug 'Load done.'
	end

	def evaluate(expr)
		clients.first.run :evaluate, expr
		clients.first.result
	end

	def select(params)
		clients.each { |c|
			c.run :select, params
		}

		table = {}
		clients.each { |c|
			results = Hash[ c.result['result'] ]
			table.merge!(results) { |k, a, b|
				a+b
			}
		}
		table
	end

	def cleanup(table='data')
		clients.each { |c|
			c.run :cleanup, table
		}
		clients.each { |c|
			c.result
		}
	end

	def tables
		clients.each { |c|
			c.run :tables
		}
		clients.reduce({}) { |res, c|
			res.merge(c.result) { |key, a, b|
				a+b
			}
		}
	end
end