//File Extension is "C" to enable basic GitHub syntax highlighting

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
