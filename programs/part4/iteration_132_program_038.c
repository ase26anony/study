/* test_coverage.c - Simple program to generate GCOV data */
#include <stdio.h>

void function_a(void) {
    printf("Function A executed\n");
}

void function_b(void) {
    printf("Function B executed\n");
}

int main(void) {
    int condition = 1;
    
    function_a();
    
    if (condition) {
        printf("Condition was true\n");
        function_b();
    } else {
        printf("Condition was false\n");
    }
    
    for (int i = 0; i < 3; i++) {
        printf("Loop iteration %d\n", i);
    }
    
    return 0;
}
