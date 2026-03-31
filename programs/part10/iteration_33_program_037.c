/* Minimal C program to trigger GCC driver cleanup logic */
#include <stdio.h>

int main(void) {
    /* Simple computation to prevent optimization */
    int values[10];
    int sum = 0;
    
    for (int i = 0; i < 10; i++) {
        values[i] = i * 2;
        sum += values[i];
    }
    
    /* Use result to prevent dead code elimination */
    printf("Result: %d\n", sum);
    return sum % 256;
}
