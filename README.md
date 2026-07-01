# mandoc for Ruby

This is a parser library for mdoc/man formats, which bases on roff
format.  This is implemented as a [mandoc][m] binding for Ruby.

Please not that libmandoc interface is [*not* particularly
recommended][lm] officially.  That said, as far as I consider, mandoc
is the best for this purpose.

Project links: [repository][cb], [mirror][gh]

[lm]: https://mandoc.bsd.lv/libmandoc.html
[cb]: https://codeberg.org/gemmaro/ruby-mandoc
[gh]: https://github.com/gemmaro/ruby-mandoc
[m]: https://mandoc.bsd.lv/

## Installation

Install the gem and add to the application's `Gemfile` by executing:

```bash
bundle add mandoc
```

If bundler is not being used to manage dependencies, install the gem by executing:

```bash
gem install mandoc
```

## Usage

This gem can parse manpages in files and examine metadata, walk
abstract syntax tree (AST), and converts into some formats.  Please
refer to the API document for details.

```ruby
require "mandoc"

parser = Mandoc::Parser.new

# "meta" is something like "document".
meta = parser.parse_file(path_to_mandoc_1)
meta.section  #=> "1"
meta.title    #=> "MANDOC"

meta.markdown #=> rendered string in Markdown

# Walk AST
root = meta.first_node
root.first_child.next_sibling.name #=> "Dt"
```

## Development

First, you have to download mandoc sources.  The `bin/download` script
downloads the tarball.  Then extract into, e.g. `mandoc-1.14.6/`.

Then generate `config.h` file of mandoc.  Please refer to
`bin/configure` script to do this.

Run `bin/setup` to install dependencies. Then, run `rake test` to run
the tests. You can also run `bin/console` for an interactive prompt
that will allow you to experiment.

To install this gem onto your local machine, run `bundle exec rake
install`. To release a new version, update the version number in
`version.rb`, and then run `bundle exec rake release`, which will
create a git tag for the version, push git commits and the created
tag, and push the `.gem` file to [rubygems.org](https://rubygems.org).

On OpenBSD, and if you are using packaged `ruby`, native compilation
can fail because `install` command has `-o root -g bin` flags.  You
can overwrite the behavior; please refer to `bin/compile` script.

On Guix, edit `CC=` line in `mandoc-1.14.6/configure` script.  For
example, if using GCC, change to `CC=gcc`.

References: [my past project which uses similar mkmf strategy][re]

[re]: https://github.com/gemmaro/ruby-eldc

## Contributing

Bug reports and pull requests are welcome.

## Prior Works

There are few libraries which parses manpages in mdoc/man/roff
formats.  [man\_parser][mp] gem is a very concise parser
implementation written in pure Ruby.

The ronn gem is for converting Markdown-like format into roff/HTML,
but not for parsing existing man pages.  The md2man gem and the
kramdown-man also have a same idea.

[mp]: https://github.com/grosser/man_parser

## License

The gem archive contains some files from the mandoc sources, which is
distributed under the ISC license.  Please refer to the
`mandoc-*.*.*/LICENSE` file in the archive.

This mandoc gem itself is distributed under the following license:

Copyright (C) 2026  gemmaro

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <https://www.gnu.org/licenses/>.
