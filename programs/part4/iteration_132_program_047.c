#include <stdio.h>

void function1() {
    printf("Function 1 executed\n");
}

void function2() {
    printf("Function 2 executed\n");
}

int main() {
    int condition = 1;
    
    function1();
    
    if (condition) {
        printf("Condition was true\n");
        function2();
    } else {
        printf("Condition was false\n");
    }
    
    // Loop to generate some execution counts
    for (int i = 0; i < 3; i++) {
        printf("Loop iteration %d\n", i);
    }
    
    return 0;
}
