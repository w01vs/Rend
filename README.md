# Intro

This is my attempt at making a compiled programming language. It will compile directly to nasm assembly x86-64. It will run on Linux. To get to a working programming language faster, I will likely use the C runtime internally to handle things suchs as heap allocation and file IO, at least for the foreseeable future. I might write my own versions of these functionalities at some point.

This branch is a complete rework of lexer, parser and code generator with new internal structure. This branch will be merged and removed when I reach feature parity with main.

See issues for what I am currently working on.

Strongly inspired by:
[Hydrogen](https://github.com/orosmatthew/hydrogen-cpp/tree/master)\
[Hydrogen Youtube](https://www.youtube.com/playlist?list=PLUDlas_Zy_qC7c5tCgTMYq2idyyT241qs)

