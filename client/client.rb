require 'socket'

require_relative "expression"

class NilThread
	def method_missing(*); end
end

class Client

	def initialize(host='127.0.0.1', port=5000)
		@host = [host, port]
		@thread = NilThread.new
	end

	attr_accessor :capability, :preserve
	attr_reader :thread, :host

	def result
		thread.join
		@result
	end

	def run(name, *params)
		result
		connect
		@thread = Thread.new {
			send name, *params
			close unless @preserve
			@preserve = false
		}
	end

private

	def connect
		@sock = TCPSocket.new(*host) unless @sock
	end

	def close
		@sock.close
		@sock = nil
	end

	def out(query)
		query = query.to_json
		debug "#{host * ':'} << #{query}"
		@start = Time.now
		@sock.puts query
	end

	def recv
		message = @sock.gets
		@result = JSON::parse message

		elapsed = Time.now - @start
		debug "#{host * ':'} #{elapsed.round 2}s >> #{message}"
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
		}.merge params
		out query
		recv
	end

	def evaluate(expr)
		e = Expression.new expr
		query = {
			act: 'eval',
			expr: e.to_a
		}
		out query
		recv
	end

	def cleanup(table)
		query = {act: 'cleanup', table: table}
		out query
		recv
	end

	def tables
		query = {act: 'tables'}
		out query
		recv
	end

end