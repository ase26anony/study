/* Simple C program to trigger GCC driver cleanup logic */
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
    
    /* Use result to avoid dead code elimination */
    if (sum > 0) {
        printf("Array sum: %d\n", sum);
    }
    
    return sum % 256;  /* Ensure non-zero exit if sum > 255 */
}
