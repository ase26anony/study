// Without volatile, compiler might optimize this:
int flag = 0;
if (flag) {  // Compiler knows flag is 0, might remove entire if block
    do_something();
}

// With volatile, compiler must check the actual memory location:
volatile int flag = 0;
if (flag) {  // Compiler MUST read from memory here
    do_something();  // This code remains in the executable
}
