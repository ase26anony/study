/* Minimal valid C program to test GCC driver reinitialization */
#include <stdio.h>

int simple_function(int x) {
    return x * 2;
}

int main(void) {
    printf("Result: %d\n", simple_function(21));
    return 0;
}
