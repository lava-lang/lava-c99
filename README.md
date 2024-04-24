Lava Masters Goal: High-level language features with low-level performance, backed by language features to improve manual memory
handling and additional safety to various common pitfalls. This should be possible without making a complex and verbose language like Rust or C++.

- Using higher-level features should produce more concise, convenient and more readable code, overall allowing faster development
- Are various language features targeting manual memory management easier? Does it reduce bugs and exploits within software?
- Is the program safer using safe techniques like bounds checking, nullable variables and more robust manual memory management?
- With these improvements in place, is the language still concise and on the level of complexity such as a language like Java?
- Challenge the status quo of the gap between high-level language features and low-level language performance.

Features To Explore
- Pointer ownership
- Reference Counting
- Array Bounds Checking

Version 0.0 (Before Masters Start Date):
- 1:1 Support for all basic C features (excluding macros)
- Compiling multiple files Lava files into a single C file
- Allow the use of standard C malloc, free and print with "c." notation

Version 1.0 (Masters Project Goals):
- Type Aliasing
- Non-capturing lambdas
- Expression Evaluator
- Alternate allocators (arena, stack, etc) (allow different memory strategies across the program, avoiding many head allocations)
- Safer NULL handling by requiring nullable variable status (Helps to avoid bugs and crashes by requiring explicit NULL handling)
- Defer Statement (Helps with cleaning up heap allocation, avoiding leaks)

Version 2.0 (Final Goals For Lava):
- Struct member functions
- Capturing Lambdas
- Function overloading and operator overloading
- Rust-style interface system, supported by structs (and maybe classes?)
- Compile time generics
- Language-wide string intern cache for string literals
- General Lava standard library
- Basic local folder package system that imports other .lava files
- Importing C headers along with adding Lava bindings for C functions and structures
- SDL and/or RayLib bindings (along with other valuable packages)
