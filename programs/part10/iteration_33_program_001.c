/* Minimal C program to trigger GCC driver cleanup logic */
#include <stdio.h>

#define ARRAY_SIZE 10

int main(void) {
    int numbers[ARRAY_SIZE];
    int sum = 0;
    
    /* Fill array with values */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        numbers[i] = i * 2 + 1;  /* Odd numbers: 1, 3, 5, ... */
        sum += numbers[i];
    }
    
    /* Use the result to prevent optimization */
    if (sum > 0) {
        printf("Sum of first %d odd numbers: %d\n", ARRAY_SIZE, sum);
    }
    
    return sum % 100;  /* Non-zero return to ensure computation matters */
}
