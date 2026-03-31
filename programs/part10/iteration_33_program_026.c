/* Minimal C program to trigger GCC driver cleanup logic */
#include <stdio.h>

#define ARRAY_SIZE 10

int main(void) {
    int numbers[ARRAY_SIZE];
    int sum = 0;
    
    /* Fill array with values */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        numbers[i] = i * 2 + 1;  /* Simple non-zero values */
        sum += numbers[i];
    }
    
    /* Use the result to prevent optimization */
    printf("Sum of first %d odd numbers: %d\n", ARRAY_SIZE, sum);
    
    return sum % 256;  /* Return non-zero but valid exit code */
}
