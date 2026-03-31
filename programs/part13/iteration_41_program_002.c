/* Simple valid C code to ensure compilation succeeds */
#include <stdio.h>

int helper_function(void) {
    return 42;
}

int main(void) {
    printf("Result: %d\n", helper_function());
    return 0;
}
