# Tiny Toy Language

A mixture of mathematic calculation and a subset of C-like expression language.

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

> if ( 2 > 3) { return 4; } else if ( 5 > 6) { return 7; } else if ( 7 < (8 && 1)) { return 9; } else if ( 9 < 10) { return 11;}

If you have a file a.txt, in which it contains:

> a = 3;
> b = 4;
> a += b;
> return a;

You can input following sentence after ttlc prompt:

> include(a.txt)

or

> return include(a.txt);

# TODO
1. add mathematic functions
2. fix bug

