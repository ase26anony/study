/* test.c - Simple program to generate GCOV coverage data */
#include <stdio.h>
#include <stdlib.h>

int function1(int x) {
    if (x > 0) {
        printf("Positive: %d\n", x);
        return x * 2;
    } else {
        printf("Non-positive: %d\n", x);
        return x - 1;
    }
}

int function2(int y) {
    for (int i = 0; i < y; i++) {
        if (i % 2 == 0) {
            printf("Even: %d\n", i);
        } else {
            printf("Odd: %d\n", i);
        }
    }
    return y * y;
}

void function3() {
    printf("Function3 called\n");
    int z = 0;
    while (z < 3) {
        printf("Loop iteration: %d\n", z);
        z++;
    }
}

int main(int argc, char *argv[]) {
    printf("Test program for GCOV coverage\n");
    
    if (argc > 1) {
        int input = atoi(argv[1]);
        
        // Different execution paths based on input
        if (input == 1) {
            function1(5);
            function2(3);
        } else if (input == 2) {
            function1(-2);
            function3();
        } else if (input == 3) {
            function1(0);
            function2(1);
            function3();
        } else {
            function1(input);
            function2(2);
        }
    } else {
        // Default execution
        function1(10);
        function2(4);
        function3();
    }
    
    return 0;
}
