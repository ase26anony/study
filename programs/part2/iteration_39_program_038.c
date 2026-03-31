/* test.c - Simple program with multiple functions and branches for coverage testing */
#include <stdio.h>
#include <stdlib.h>

int function1(int x) {
    if (x > 0) {
        printf("Positive: %d\n", x);
        return x * 2;
    } else if (x < 0) {
        printf("Negative: %d\n", x);
        return x * -1;
    } else {
        printf("Zero\n");
        return 0;
    }
}

void function2(int a, int b) {
    for (int i = 0; i < a; i++) {
        if (i % 2 == 0) {
            printf("Even: %d\n", i);
        } else {
            printf("Odd: %d\n", i);
        }
    }
    
    switch (b) {
        case 1:
            printf("Case 1\n");
            break;
        case 2:
            printf("Case 2\n");
            break;
        default:
            printf("Default case\n");
    }
}

int main(int argc, char *argv[]) {
    int input = 0;
    
    if (argc > 1) {
        input = atoi(argv[1]);
    }
    
    printf("Input value: %d\n", input);
    
    int result1 = function1(input);
    function2(input, input % 3);
    
    return result1;
}
