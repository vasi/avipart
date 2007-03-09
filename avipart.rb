#!/usr/bin/ruby

# Convert a specification like "100M" to an integer number of bytes.
# Currently only allows the suffices K, M, G, or none. Uses powers of 1024.
def byte_size(spec)
	md = /^(\d+)\s*([kmg]?)b?$/i.match(spec) or raise "Bad size specification"
	num, suf = md.captures
	sufs = [''] + %w[k m g]
	p sufs
	return Integer(num) * 1024 ** sufs.index(suf.downcase)
end

class IO
	def read_four_char_code; self.read(4).unpack('a4')[0]; end
end

class Integer
	def align_to(i); return self.quo(i).ceil * i; end
end


module Avi
class Dumper
	def initialize(opts = {})
		@opts = opts
		@no_data = opts[:no_data]
		@indent = 0
		@pass = { }
		@pass_children = { }
		@out = opts[:out] || ''
		@lines = 0
	end
	
	def self.dump(obj, opts, more)
		self.new(opts.merge!(more)).dump(obj)
	end
	
	def indent
		@out << "\n" if @lines == 1
		@out << '  ' * @indent
		@lines += 1;
	end
	
	def dump(obj)
		indent
		obj.desc(@out)
		@out << "\n" if @lines > 1
		if obj.respond_to?(:each) && !@pass_children[obj.code]
			@indent += 1
			obj.each do |sub|
				if @pass[sub.code] || (@no_data && sub.codec_data?)
					@pass[sub.code] = true
					@pass_children[obj.code] = true
					break
				else
					dump(sub)
				end
			end
			@indent -= 1
		end
	end
end

class Chunk
	attr_reader :code, :parent
	
	def initialize(parent, io, cod)
		@parent = parent
		@io = io
		@code = cod
		@size = io.read(4).unpack('V')[0]
		@contents = io.pos
		yield io if block_given?
		@data = io.pos
		
		if self.damaged?; @size = @data - @contents; end
	end
	
	def data; @io.pos = @data; @io.read(data_size); end
	
	def total_size; @size + 8; end
	def data_size; @size - (@data - @contents); end
	
	def indent(out, lev); out << '  ' * lev; end
	
	def desc(out); out << @code << " - " << data_size; end
	def dump(out, opts = {}); Dumper.dump(self, opts, :out => out); end 
	def to_s(opts = {}); Dumper.dump(self, opts); end
	
	def damaged?; code == 255.chr * 4; end
	def codec_data?; /^\d\d(wb|d[bc])$/.match(@code); end
end

class List < Chunk
	include Enumerable
	
	CODE = 'LIST'
	REC_CODE = 'rec '
	
	attr_reader :list_code
	
	def self.list_code?(cod); cod == CODE || cod == File::CODE; end
	
	def initialize(parent, io, cod)
		super(parent, io, cod) { |i| @list_code = i.read_four_char_code }
	end
	
	def each
		offset = @data
		last = offset + data_size
		while offset < last
			child = File.chunk_factory(self, @io, offset)
			yield child
			break if child.damaged?
			offset = (offset + child.total_size).align_to(2)
		end
		raise "Children extend past end of parent" if offset > last
	end
	
	def desc(out); out << @code << ' (' << @list_code << ') - ' \
		<< data_size; end
end

class File < List
	CODE = 'RIFF'
	LIST_CODE = 'AVI '
	
	def initialize(obj)
		begin
			opened = !obj.respond_to?(:read)	
			io = opened ? ::File.new(obj) : obj
			cod = io.read_four_char_code
			super(nil, io, cod)
			raise "Not an AVI file" unless code == CODE &&
				list_code == LIST_CODE
			
			# Ack, multiple top-level RIFFs?
			filesize = io.stat.size
			if filesize >= total_size + 4
				io.pos = total_size
				other = io.read_four_char_code
				if other == CODE # Hack!
					@contents = @data = 0
					@size = filesize
				end
			end
			
			yield self if block_given?
		ensure
			io.close if io && opened && block_given?
		end
	end
		
	def self.chunk_factory(parent, io, offset)
		io.pos = offset
		code = io.read_four_char_code
		klass = List.list_code?(code) ? List : Chunk
		return klass.new(parent, io, code)
	end
end
end # module Avi


file = *ARGV

Avi::File.new(file) do |avi|
	avi.dump($stderr, :no_data => false)
end
