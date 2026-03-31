#include <stdio.h>

void function1() {
    printf("Function 1 executed\n");
}

void function2() {
    printf("Function 2 executed\n");
}

int main() {
    int x = 1;
    
    function1();
    
    if (x > 0) {
        function2();
    } else {
        printf("Else branch\n");
    }
    
    for (int i = 0; i < 3; i++) {
        printf("Loop iteration %d\n", i);
    }
    
    return 0;
}
