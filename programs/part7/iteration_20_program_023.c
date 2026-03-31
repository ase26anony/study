/* test.c - Simple program to generate coverage data */
#include <stdio.h>

int function1(int x) {
    if (x > 0) {
        return x * 2;
    } else if (x < 0) {
        return x * -1;
    } else {
        return 0;
    }
}

void function2() {
    for (int i = 0; i < 3; i++) {
        printf("Iteration %d\n", i);
    }
}

int main() {
    int result = function1(5);
    function2();
    result += function1(-3);
    result += function1(0);
    return result;
}
