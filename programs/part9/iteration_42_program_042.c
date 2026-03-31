// Using function attributes (cleaner, more portable across compilers)
void foo() __attribute__((optimize("O0")));
void foo() { /* complex code */ }

// Using Clang's equivalent
#pragma clang optimize off
void foo() { /* complex code */ }
#pragma clang optimize on
