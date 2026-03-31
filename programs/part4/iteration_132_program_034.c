#include <stdio.h>

void function1() {
    printf("Function 1 called\n");
}

void function2() {
    printf("Function 2 called\n");
}

int main() {
    int x = 1;
    
    function1();
    
    if (x > 0) {
        function2();
    } else {
        printf("This won't execute\n");
    }
    
    for (int i = 0; i < 3; i++) {
        printf("Loop iteration %d\n", i);
    }
    
    return 0;
}
