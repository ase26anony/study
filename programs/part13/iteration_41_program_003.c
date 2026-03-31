/* test_gcc_driver.c - Minimal C program for GCC driver testing */
#include <stdio.h>

int helper_function(void) {
    return 42;
}

int main(void) {
    printf("Test program compiled successfully\n");
    return helper_function() - 42;
}
