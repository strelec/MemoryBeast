require 'ripper'

class Expression

	def initialize(expr)
		expr = '' unless String === expr
		@tree = Ripper.sexp(expr).last.first
	end

	def to_a
		get @tree
	end

private

	def get(expr)
		case expr.first
		when :void_stmt
			nil
		when :vcall
			return nil if expr[1][1] == 'null'
			[ 'get', expr[1][1] ]
		when :call
			[ 'get', get(expr[1]).drop(1), expr[3][1] ].flatten
		when :command
			[ expr[1][1].downcase ] + expr[2][1].map { |e|
				get e
			}
		when :method_add_arg
			get [ :command, expr[1][1], expr[2][1] ]
		when :binary
			[ expr[2].to_s, get(expr[1]), get(expr[3]) ]
		when :unary
			[ expr[1].to_s.chomp('@'), get(expr[2]) ]
		when :paren
			get expr[1].first
		when :var_ref
			{
				'true' => true,
				'false' => false,
				'nil' => nil,
			}[ expr[1][1] ]
		when :@int
			expr[1].to_i
		when :@float
			expr[1].to_f
		when :string_literal
			expr[1][1][1]
		else
			throw "Unknown subexpression: #{expr.first}"
		end
	end
end