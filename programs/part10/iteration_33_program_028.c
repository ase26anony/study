/* Simple C program to trigger GCC driver cleanup logic */
#include <stdio.h>

#define ARRAY_SIZE 100

int main(void) {
    int array[ARRAY_SIZE];
    int sum = 0;
    
    /* Fill array with values to prevent optimization */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        array[i] = i * 2;
    }
    
    /* Compute sum to ensure code isn't optimized away */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        sum += array[i];
    }
    
    /* Use the result to prevent dead code elimination */
    printf("Sum: %d\n", sum);
    
    return (sum == (ARRAY_SIZE - 1) * ARRAY_SIZE) ? 0 : 1;
}
