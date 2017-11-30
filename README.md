# Tiny Toy Language

A mixture of mathematics calculation and a subset of C-like expression language.

# dependency

0. cmake 3.0
1. libreadline.so.7

# build

> mkdir build

> cd build

> cmake ..

> make

# demo

Try run the executable file build/ttlc and input following sentences:

> a = 3; b = 4; return a + b;

> a = 3; b = 4; a += b; return a;

> (((333)))

> return 2 + (-2) * 4 && 0;

> 0 || -2 + (-2) / 2

# TODO
1. add 'if-else', 'for' sentences
2. fix bugs('include')
3. add mathematics functions

