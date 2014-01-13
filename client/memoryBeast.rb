require 'json'

require_relative 'client'
require_relative 'query'

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
		group = (params[:group] || []).map { |el|
			Expression.new(el).to_a
		}
		what = []

		find = -> ex {
			pos = group.index ex
			if pos
				[:g, pos]
			else
				pos = what.index ex
				if pos
					[:w, pos]
				else
					what << ex
					[:w, what.size-1]
				end
			end
		}

		result = {}
		params[:select].each { |k, v|
			ex = Expression.new(v).to_a

			result[k] = if Array === ex && ex[0] == 'avg'
				[ :a, find[['sum', ex.last]], find[['count', ex.last]] ]
			else
				find[ex]
			end
		}


		params = {
			table: params[:from],
			what: what,
			where: if params.has_key? :where
				Expression.new(params[:where]).to_a
			else true; end,

			group: group,
			limit: params[:limit] || 100,
			offset: params[:offset] || 0,
		}

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

		Hash[ table.map { |key, rows|
			[ key, subtable(key, rows, result) ]
		} ]
	end

	def sql(query)
		q = Query.new query
		select q
	end

	def subtable(key, rows, result)
		rows.map { |row|
			Hash[ result.map { |kr, vr|
				[ kr.to_sym, case vr.first
				when :g
					key[vr.last]
				when :w
					row[vr.last]
				end ]
			} ]
		}
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