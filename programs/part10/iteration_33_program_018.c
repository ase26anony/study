/* test_cleanup.c - Minimal C program to trigger GCC driver cleanup logic */
#include <stdio.h>

int main(void) {
    /* Simple computation to prevent optimization */
    int values[5] = {1, 2, 3, 4, 5};
    int sum = 0;
    
    for (int i = 0; i < 5; i++) {
        sum += values[i];
    }
    
    printf("Result: %d\n", sum);
    return sum == 15 ? 0 : 1;  /* Return 0 on correct sum */
}
