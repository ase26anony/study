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
    
    int x = 1;
    if (x > 0) {
        function2();
    } else {
        printf("This branch won't execute\n");
    }
    
    return 0;
}
