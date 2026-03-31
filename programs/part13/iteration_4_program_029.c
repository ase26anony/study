/* test_source.c - Simple program to generate gcov data */
#include <stdio.h>

void func1() {
    printf("Function 1 called\n");
}

void func2() {
    printf("Function 2 called\n");
}

int main() {
    int i;
    
    for (i = 0; i < 10; i++) {
        if (i % 2 == 0) {
            func1();
        } else {
            func2();
        }
    }
    
    return 0;
}
