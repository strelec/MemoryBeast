class Array
	def break(sep)
		result = [[]]
		each { |el|
			if el == sep
				result << []
			else
				result.last << el
			end
		}
		result
	end
end

class Query < Hash

	STAGES = [
		[:select],
		[:from],
		[:where],
		[:group, :by],
		[:limit],
		[:offset],
	]

	def initialize(sql)
		super()

		toks = tokenize(sql).flat_map { |tok|
			case tok
			when Array
				tok.first
			when String
				tok.split(/(?=,)|\s+/).reject(&:empty?)
			end
		}

		unless c toks.first, :select
			raise "Query has to start with SELECT, not #{tokens.first}."
		end

		cur = 0
		i = -1
		while (i+=1) < toks.size
			cur.upto(STAGES.size()-1) { |j|
				size = STAGES[j].size
				if bigc STAGES[j], toks[i, size]
					cur = j
					i += size
				end
			}
			( self[ STAGES[cur].first ] ||= [] ) << toks[i]
		end

		prepare!

	end

private

	def prepare!
		each { |key, val|
			self[key] = case key
			when :select
				Hash[ val.break(',').map { |el|
					parts = el.break 'AS'
					if parts.size <= 1
						expr = expr parts.first
						[ parts[0].first, expr ]
					else
						[ parts[1].first, expr(parts.first) ]
					end.map(&:to_s)
				} ]
			when :from
				val.first.to_s
			when :where
				expr val
			when :group
				val.break(',').map { |el|
					expr el
				}
			when :limit, :offset
				val.first.to_i
			end
		}
	end

	def expr(parts)
		parts.join
	end

	def tokenize(str)
		inString = false;
		parenDepth = 0;

		buffer = ''
		result = []
		str.each_char { |c|
			if inString
				case c
				when ?', ?"
					if inString == c
						result << [buffer+c]
						buffer.clear
						inString = false
					else
						buffer << c
					end
				else
					buffer << c
				end
			else
				case c
				when ?', ?"
					if parenDepth == 0
						result << buffer
						buffer = c
						inString = c.dup
					else
						buffer << c
					end
				when ?(
					if parenDepth == 0
						result << buffer
						buffer = ''
					end
					buffer << c
					parenDepth += 1
				when ?)
					buffer << c
					parenDepth -= 1
					if parenDepth == 0
						result << [buffer]
						buffer = ''
					end
				else
					buffer << c
				end
			end
		}
		result << buffer unless buffer.empty?

		result
	end

	def c(a, b)
		a.to_s.casecmp(b.to_s) == 0
	end

	def bigc(a, b)
		a.zip(b) { |x, y|
			return true if c x, y
		}
	end
end