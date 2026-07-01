# frozen_string_literal: true

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

require_relative "lib/mandoc/version"

Gem::Specification.new do |spec|
  spec.name = "mandoc"
  spec.version = Mandoc::VERSION
  spec.authors = ["gemmaro"]
  spec.email = ["gemmaro.dev@gmail.com"]

  spec.summary = "mdoc/man parsing library"
  spec.description = "This is a mandoc binding for Ruby.  This can parse manpages in mdoc/man formats, get metadata, traverse abstract syntax tree, and convert into some formats."
  spec.homepage = "https://codeberg.org/gemmaro/ruby-mandoc"
  spec.license = "GPL-3.0-or-later"
  spec.required_ruby_version = ">= 3.2.0"

  spec.metadata["homepage_uri"] = spec.homepage
  spec.metadata["source_code_uri"] = "https://codeberg.org/gemmaro/ruby-mandoc.git"

  spec.files = Dir[
    'mandoc-1.14.6/*.{c,h}',
    'mandoc-1.14.6/LICENSE',
    'ext/**/*.{c,h,rb}',
    'lib/**/*.rb',
    'COPYING',
    '*.md',
  ].reject { |file| file.match?(%r{\Amandoc-.+?/test-}) }
  spec.require_paths = ["lib"]
  spec.extensions = ["ext/mandoc/extconf.rb"]
end
