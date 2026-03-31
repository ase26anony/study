/* Simple C program to trigger GCC driver cleanup logic */
#include <stdio.h>

#define ARRAY_SIZE 10

int main(void) {
    int numbers[ARRAY_SIZE];
    int sum = 0;
    
    /* Fill array with values */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        numbers[i] = i * 2 + 1;  /* Odd numbers: 1, 3, 5, ... */
    }
    
    /* Compute sum - prevents optimization from removing everything */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        sum += numbers[i];
    }
    
    /* Use the result to prevent dead code elimination */
    printf("Sum of first %d odd numbers: %d\n", ARRAY_SIZE, sum);
    
    return (sum == ARRAY_SIZE * ARRAY_SIZE) ? 0 : 1;
}
