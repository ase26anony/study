/* test.c - Simple program to generate coverage data */
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

int function2(int x, int y) {
    for (int i = 0; i < x; i++) {
        if (i % 2 == 0) {
            y += i;
        } else {
            y -= i;
        }
    }
    return y;
}

void function3() {
    int arr[5] = {1, 2, 3, 4, 5};
    for (int i = 0; i < 5; i++) {
        if (arr[i] % 2 == 0) {
            printf("Even: %d\n", arr[i]);
        } else {
            printf("Odd: %d\n", arr[i]);
        }
    }
}

int main(int argc, char *argv[]) {
    int input = 0;
    
    if (argc > 1) {
        input = atoi(argv[1]);
    }
    
    printf("Input value: %d\n", input);
    
    int result1 = function1(input);
    printf("function1 result: %d\n", result1);
    
    int result2 = function2(input, 10);
    printf("function2 result: %d\n", result2);
    
    function3();
    
    return 0;
}
