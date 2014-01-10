require 'socket'

require_relative "expression"

class NilThread
	def method_missing(*); end
end

class Client

	def initialize(host='127.0.0.1', port=5000)
		@host = host
		@port = port

		@sock = TCPSocket.new host, port
		@thread = NilThread.new
	end

	attr_accessor :thread, :capability
	attr_reader :host, :port

	def result
		thread.join
		@result
	end

	def run(name, *params)
		result
		@thread = Thread.new {
			send name, *params
		}
	end

	def close
		@sock.close
	end

private

	def out(query)
		debug "#{host}:#{port} << #{query}"
		@start = Time.now
		@sock.puts query
	end

	def recv
		message = @sock.gets
		@result = JSON::parse message

		elapsed = Time.now - @start
		debug "#{host}:#{port} #{elapsed.round 4}s >> #{message}"
	end

	def load(data, table)
		size = data.size
		query = {
			act: 'insert',
			table: table,
			data: [size]
		}.to_json
		query.sub! size.to_s, data * ','

		@start = Time.now
		@sock.puts query
		recv
	end

	def select(params)
		query = {
			act: 'select',
			table: params[:table],
			what: params[:what].values.map { |el|
				Expression.new(el).to_a
			},
			where: Expression.new(params[:where]).to_a,
			group: params[:group].map { |el|
				Expression.new(el).to_a
			},
		}
		out query.to_json
		recv
	end

	def evaluate(expr)
		e = Expression.new expr
		query = {
			act: 'eval',
			expr: e.to_a
		}
		out query.to_json
		recv
	end

	def cleanup(table)
		query = {act: 'cleanup', table: table}
		out query.to_json
		recv
	end

	def tables
		query = {act: 'tables'}
		out query.to_json
		recv
	end

end