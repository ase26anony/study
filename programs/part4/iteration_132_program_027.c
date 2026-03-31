#include <stdio.h>

void function1() {
    printf("Function 1 called\n");
}

void function2() {
    printf("Function 2 called\n");
}

int main() {
    printf("Starting test program\n");
    
    function1();
    function2();
    
    int x = 5;
    if (x > 0) {
        printf("x is positive\n");
    } else {
        printf("x is non-positive\n");
    }
    
    for (int i = 0; i < 3; i++) {
        printf("Loop iteration %d\n", i);
    }
    
    printf("Test program completed\n");
    return 0;
}
