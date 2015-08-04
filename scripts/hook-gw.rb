#!/usr/bin/env ruby
require "sinatra"
require "json"

REP_FULL_NAME="enukane/c88-harvey-os-book"
ORIG_FILE="c88book.pdf"
STORE_PREFIX="~/public_html/c88-harvey-os-book"
FILE_PREFIX="c88-"

def generate_filename
  return "#{FILE_PREFIX}#{Time.now.strftime('%Y%m%d_%H%M%S')}.pdf"
end

def do_job
  # first do gitpull
  result = system("git pull")
  unless result
    raise "failed to git pull"
  end

  # if Makefile is there
  unless File.exists?("Makefile")
    raise "no makefile"
  end

  # do make
  result = system("make")
  unless result
    raise "failed to make"
  end

  # copy output into public_html with name 
  filename = generate_filename()
  result = system("cp #{ORIG_FILE} #{STORE_PREFIX}/#{filename}")
  unless result
    raise "failed to copy #{ORIG_FILE} to #{STORE_PREFIX}/#{filename}"
  end
end

post '/' do
  begin
    print ">>>> POST request\n"
    json = JSON.parse(request.body.read)
    full_name = json["repository"]["full_name"]
    print "full_name = #{full_name}\n\n"
    if full_name != REP_FULL_NAME
      raise "repository is not what expected (#{full_name})"
    end

    do_job

    print "OK: done\n\n"

    "ok."
  rescue => e
    print "NG: error detected (#{e})\n\n"

    "error. (#{e})"
  end
end

set :bind, "0.0.0.0"
set :port, 11185
