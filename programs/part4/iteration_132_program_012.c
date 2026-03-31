/* Simple program to generate GCOV data */
#include <stdio.h>

void function1() {
    printf("Function 1 executed\n");
}

void function2() {
    printf("Function 2 executed\n");
}

int main() {
    int i;
    
    function1();
    
    for (i = 0; i < 3; i++) {
        if (i % 2 == 0) {
            printf("Even iteration: %d\n", i);
        } else {
            printf("Odd iteration: %d\n", i);
        }
    }
    
    function2();
    
    return 0;
}
