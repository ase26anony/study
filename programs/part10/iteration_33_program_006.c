/* Simple C program to trigger GCC driver cleanup logic */
#include <stdio.h>

int main(void) {
    /* Use a trivial computation to prevent optimization */
    int values[5] = {1, 2, 3, 4, 5};
    int sum = 0;
    
    for (int i = 0; i < 5; i++) {
        sum += values[i];
    }
    
    printf("Sum: %d\n", sum);
    return sum % 256;  /* Return non-zero to ensure computation isn't optimized away */
}
