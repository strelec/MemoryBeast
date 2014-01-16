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

		# PREPARE

		group = (params[:group] || []).map { |el|
			Expression.new(el).to_a
		}
		what = []

		find = -> ex {
			pos = group.index ex
			if pos
				[:key, pos]
			else
				pos = what.index ex
				if pos
					[:val, pos]
				else
					what << ex
					[:val, what.size-1]
				end
			end
		}

		result = {}
		params[:select].each { |k, v|
			ex = Expression.new(v).to_a

			result[k] = if Array === ex
				if c ex[0], :avg
					[ :avg, find[['sum', ex.last]], find[['count', ex.last]] ]
				elsif [:min, :max, :sum, :count].any? {|e| c ex[0], e}
					[ ex[0].to_sym, find[ex] ]
				else
					find[ex]
				end
			else
				find[ex]
			end
		}

		# EXECUTE

		params = {
			from: params[:from],
			select: what,
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

		# REDUCE

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

private

	def subtable(key, rows, result)
		agg = result.any? { |t|
			not [:key, :val].include? t
		}

		ev = -> cell, row {
			case cell.first
			when :key
				key[cell.last]
			when :val
				row[cell.last]
			when :avg
				[ ev[cell[1], row], ev[cell[2], row] ]
			else
				ev[cell.last, row]
			end
		}

		out = []
		rows.map { |row|
			current = Hash[ result.map { |k, v|
				[ k, ev[v, row] ]
			} ]

			if agg && !out.empty?
				out.last.merge!(current) { |ky, a, b|
					case result[ky].first
					when :count, :sum
						a+b
					when :min
						[a,b].min
					when :max
						[a,b].max
					when :avg
						[a[0]+b[0], a[1]+b[1]]
					else
						a
					end
				}
			else
				out << current
			end
		}

		out.map { |o|
			Hash[ o.map { |k, v|
				[ k.to_sym, Array === v ? v[0]/v[1] : v ]
			} ]
		}
	end


	def c(a, b)
		a.to_s.casecmp(b.to_s) == 0
	end

end