#include <stdio.h>

void function1() {
    printf("Function 1 executed\n");
}

void function2() {
    printf("Function 2 executed\n");
}

int main() {
    printf("Test program for gcov-dump\n");
    
    function1();
    function2();
    
    int x = 1;
    if (x > 0) {
        printf("x is positive\n");
    } else {
        printf("x is non-positive\n");
    }
    
    for (int i = 0; i < 3; i++) {
        printf("Loop iteration %d\n", i);
    }
    
    return 0;
}
