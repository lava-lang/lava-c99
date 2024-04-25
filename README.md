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

Example Lava Code:

```c
//Global scope lambda function
str getName() -> "lava";

//Consumer Lambda
str[] keyboards = {"unsaver", "kishsaver", "f122", "mopar"};
void stringPrinter(str:(void) printFunc) {
    for (str k : keyboards) {
        printFunc(k);
    }
}
stringPrinter(str s -> print(s));

//Producer Lambda
void convertFloatAndPrint(f32 input, i32:(f32) convertFloatFunc) {
    //Execute passed lambda function
    i32 result = convertFloatFunc(input);
    //Print result
    print(result);
}
convertFloatAndPrint(5.5, (f32 a) -> cast(i32, a));
//This should output an i32 of 5

//Vector dot product lambda example
struct Vec {
    f32 x, y, z;
}

//Vector struct objects
Vec vecA = Vec(1.0, 1.0, 1.0), vecB = Vec(5.0, 5.0, 5.0);

//Lambda function defined in global scope
f32 vecDotFunc(Vec a, Vec b) -> a.x * b.x + a.y * b.y + a.z * b.z;

//Executing lambda function and saving output
f32 dot = vecDotFunc(vecA, vecB);

//Defining lambda within function parameter
void vecDotAndPrint(Vec inputA, Vec inputB, f32:(Vec a, Vec b) vecDotFunc) {
    f32 result = vecDotFunc(inputA, inputB);
    print(result);
}
//Call function with lambda input
vecDotAndPrint(vecA, vecB, (Vec a, Vec a) -> a.x * b.x + a.y * b.y + a.z * b.z);

//Calling function with previously defined lambda
vecDotAndPrint(vecA, vecB, vecDotFunc);

//Aliasing Lambda types (function pointers)
alias VecDotLambda = f32:(Vec a, Vec b);

void vecDot(Vec a, Vec b, VecDotLambda vecDot) {
    f32 result = vecDot(a, b);
    print(result);
}
```
