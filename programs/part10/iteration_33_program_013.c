/* test_cleanup.c - Minimal C program to trigger GCC driver cleanup */
#include <stdio.h>

#define ARRAY_SIZE 10

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
    printf("Sum of array elements: %d\n", sum);
    
    return (sum == 90) ? 0 : 1;  /* Expected sum: 0+2+4+...+18 = 90 */
}
