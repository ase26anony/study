/* Simple C program to trigger GCC driver cleanup logic */
#include <stdio.h>

int main(void) {
    /* Use a static array to prevent optimization */
    static int values[10];
    int sum = 0;
    
    /* Initialize array with values */
    for (int i = 0; i < 10; i++) {
        values[i] = i * 2;
    }
    
    /* Compute sum to ensure code isn't optimized away */
    for (int i = 0; i < 10; i++) {
        sum += values[i];
    }
    
    printf("Result: %d\n", sum);
    return sum == 90 ? 0 : 1;  /* 0+2+4+...+18 = 90 */
}
