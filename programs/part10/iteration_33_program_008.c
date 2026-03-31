/* Simple C program to trigger GCC driver cleanup logic */
#include <stdio.h>

#define ARRAY_SIZE 100

int main(void) {
    int numbers[ARRAY_SIZE];
    int sum = 0;
    
    /* Fill array with values */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        numbers[i] = i * 2;
    }
    
    /* Compute sum to prevent optimization */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        sum += numbers[i];
    }
    
    /* Use result to prevent dead code elimination */
    printf("Sum of first %d even numbers: %d\n", ARRAY_SIZE, sum);
    
    return (sum > 0) ? 0 : 1;
}
