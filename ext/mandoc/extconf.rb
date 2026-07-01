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

require "mkmf"

# Suppress following:
#
#   compiling ../../../../ext/mandoc/../../mandoc-1.14.6/mandoc.c
#   [...]/mandoc.c:321:14: warning: implicit conversion loses
#         integer precision: 'long' to 'int' [-Wshorten-64-to-32]
#     321 |                 *sz = *end - *start;
#         |                     ~ ~~~~~^~~~~~~~
append_cflags("-Wno-shorten-64-to-32")

vendor = File.join('../../mandoc-1.14.6')
$INCFLAGS << " -I$(srcdir)/#{vendor}"
$VPATH << "$(srcdir)/#{vendor}"
mandoc_srcs = [
  'chars.c',
  'compat_ohash.c',
  'eqn.c',
  'mandoc.c',
  'mandoc_aux.c',
  'mandoc_msg.c',
  'mandoc_ohash.c',
  'mandoc_xr.c',
  'mdoc.c',
  'mdoc_argv.c',
  'mdoc_macro.c',
  'mdoc_markdown.c',
  'mdoc_state.c',
  'mdoc_validate.c',
  'msec.c',
  'read.c',
  'roff.c',
  'tag.c',
  'tbl.c',
  'tree.c',
  'mdoc_man.c',
]
$srcs = ['rb_mandoc.c', *mandoc_srcs]

# Makes all symbols private by default to avoid unintended conflict
# with other gems. To explicitly export symbols you can use RUBY_FUNC_EXPORTED
# selectively, or entirely remove this flag.
append_cflags("-fvisibility=hidden")

create_makefile("mandoc/mandoc")
