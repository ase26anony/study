/* Minimal C program to trigger GCC driver cleanup logic */
#include <stdio.h>

int main(void) {
    /* Simple computation to prevent optimization */
    int array[5] = {1, 2, 3, 4, 5};
    int sum = 0;
    
    for (int i = 0; i < 5; i++) {
        sum += array[i];
    }
    
    printf("Result: %d\n", sum);
    return sum % 256;  /* Ensure non-zero return for coverage */
}
