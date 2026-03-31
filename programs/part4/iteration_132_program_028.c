#include <stdio.h>

void function1() {
    printf("Function 1 called\n");
}

void function2() {
    printf("Function 2 called\n");
}

int main() {
    int condition = 1;
    
    function1();
    
    if (condition) {
        printf("Condition is true\n");
        function2();
    } else {
        printf("Condition is false\n");
    }
    
    for (int i = 0; i < 3; i++) {
        printf("Loop iteration %d\n", i);
    }
    
    return 0;
}
