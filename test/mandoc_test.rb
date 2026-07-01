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

require "test_helper"

class MandocTest < Test::Unit::TestCase
  test "VERSION" do
    assert do
      ::Mandoc.const_defined?(:VERSION)
    end
  end

  test "parser new" do
    assert_kind_of Mandoc::Parser, Mandoc::Parser.new
  end

  SAMPLE1 = File.join(__dir__, "sample.1")

  test "parser parse file" do
    assert_kind_of Mandoc::Meta, Mandoc::Parser.new.parse_file(SAMPLE1)
  end

  test "meta deroff" do
    assert_equal "July 1 2026 SAMPLE 1 NAME sample sample program SYNOPSIS sample DESCRIPTION The sample command prints a simple greeting to standard output. EXAMPLES $ sample Hello, world! AUTHOR gemmaro", Mandoc::Parser.new.parse_file(SAMPLE1).deroff
  end

  test "many parses to check reset works" do
    parser = Mandoc::Parser.new
    meta = parser.parse_file(SAMPLE1)
    parser.parse_file(SAMPLE1)
    meta.deroff
  end

  MANDOC1 = File.join(__dir__, "../mandoc-1.14.6/mandoc.1")

  test "section" do
    assert_equal "1", Mandoc::Parser.new.parse_file(MANDOC1).section
  end

  test "volume" do
    assert_equal "General Commands Manual", Mandoc::Parser.new.parse_file(MANDOC1).volume
  end

  test "os" do
    assert_kind_of String, Mandoc::Parser.new.parse_file(MANDOC1).os
    # e.g. "OpenBSD 7.9"
  end

  test "arch" do
    assert_nil Mandoc::Parser.new.parse_file(MANDOC1).arch
  end

  test "title" do
    assert_equal "MANDOC", Mandoc::Parser.new.parse_file(MANDOC1).title
  end

  test "name" do
    assert_equal "mandoc", Mandoc::Parser.new.parse_file(MANDOC1).name
  end

  test "date" do
    assert_equal "August 14, 2021", Mandoc::Parser.new.parse_file(MANDOC1).date
  end

  test ".so target" do
    assert_nil Mandoc::Parser.new.parse_file(MANDOC1).so_target
  end

  test "has body?" do
    assert_false Mandoc::Parser.new.parse_file(MANDOC1).has_body?
  end

  test "markdown" do
    # path = File.join(__dir__, "../mandoc-1.14.6/mandoc.1") # this takes too long, what's mandoc wrong?
    assert_match "**sample** - sample program", Mandoc::Parser.new.parse_file(SAMPLE1).markdown
  end

  test "tree" do
    assert_match(/title = \"SAMPLE\"/, Mandoc::Parser.new.parse_file(SAMPLE1).tree)
  end

  test "man" do
    assert_match(/Automatically generated from an mdoc input file/, Mandoc::Parser.new.parse_file(SAMPLE1).man)
  end

  test "first node" do
    assert_kind_of Mandoc::Node, Mandoc::Parser.new.parse_file(SAMPLE1).first_node
  end

  test "node name" do
    assert_equal "Dt", Mandoc::Parser.new.parse_file(SAMPLE1).first_node.first_child.next_sibling.name
  end
end
