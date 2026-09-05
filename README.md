# libvb

## About
libvb is a library in C to develop desktop applications. libvb is published on <https://github.com/vbsw/libvb>.

## Copyright
Copyright 2026, Vitali Baumtrok (vbsw@mailbox.org).

libvb is distributed under the Boost Software License, version 1.0. (See accompanying file LICENSE or copy at http://www.boost.org/LICENSE_1_0.txt)

libvb is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the Boost Software License for more details.

## Compile
Go into project directory and execute

	make

You find the static labrary (libvb.a) and dynamic labrary (libvb.so) in

	./build/release.linux

**Tests**  
To compile tests go into project directory and checkout the "development" branch with

	git switch development

Then compile tests with

	make debug

Then run tests with

	./build/debug.linux/vbtests
