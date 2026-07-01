# frozen_string_literal: true

#--
# Copyright (C) 2026  gemmaro
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#++

require_relative "mandoc/version"
require "mandoc/mandoc"

# The main entrypoint should be Mandoc::Parser.new.
module Mandoc
  class Parser
    ##
    # :method: new
  end

  #--
  # TODO: Converters for HTML, PDF, ASCII, UTF8, LOCALE, PS
  # (PostScript) are tricky, since it expects outdata (void*) of
  # struct outstate pointer.
  #++
  class MdocMeta
  end
end
