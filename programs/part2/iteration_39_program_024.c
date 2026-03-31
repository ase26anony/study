/* test.c - Simple program to generate coverage data */
#include <stdio.h>
#include <stdlib.h>

int func1(int x) {
    if (x > 0) {
        printf("Positive: %d\n", x);
        return x * 2;
    } else {
        printf("Non-positive: %d\n", x);
        return x - 1;
    }
}

int func2(int x, int y) {
    for (int i = 0; i < x; i++) {
        if (i % 2 == 0) {
            y += i;
        } else {
            y -= i;
        }
    }
    return y;
}

void func3(int threshold) {
    int sum = 0;
    for (int i = 1; i <= 10; i++) {
        if (i > threshold) {
            sum += i * i;
        } else {
            sum += i;
        }
    }
    printf("Sum: %d\n", sum);
}

int main(int argc, char *argv[]) {
    int input = 1;
    if (argc > 1) {
        input = atoi(argv[1]);
    }
    
    printf("Running with input: %d\n", input);
    
    int result1 = func1(input);
    int result2 = func2(input, 10);
    func3(input);
    
    printf("Results: %d, %d\n", result1, result2);
    
    return 0;
}
