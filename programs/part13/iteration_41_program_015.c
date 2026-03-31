/* test_gcc_driver.c - Minimal test program to exercise GCC driver reinitialization */
#include <stdio.h>

int simple_function(void) {
    return 42;
}

int main(void) {
    printf("Result: %d\n", simple_function());
    return 0;
}
