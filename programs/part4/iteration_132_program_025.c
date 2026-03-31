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
    
    int x = 5;
    if (x > 3) {
        printf("x is greater than 3\n");
    } else {
        printf("x is not greater than 3\n");
    }
    
    for (int i = 0; i < 3; i++) {
        printf("Loop iteration %d\n", i);
    }
    
    return 0;
}
