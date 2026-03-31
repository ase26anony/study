/* Minimal valid C program to test GCC driver re-initialization */
#include <stdio.h>

int simple_function(void) {
    return 42;
}

int main(void) {
    printf("Result: %d\n", simple_function());
    return 0;
}
