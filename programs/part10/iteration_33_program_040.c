/* Minimal C program to trigger GCC driver cleanup logic */
#include <stdio.h>

int main(void) {
    /* Simple computation to prevent optimization */
    int array[10];
    int sum = 0;
    
    for (int i = 0; i < 10; i++) {
        array[i] = i * 2;
        sum += array[i];
    }
    
    /* Use result to prevent dead code elimination */
    printf("Result: %d\n", sum);
    
    return (sum == 90) ? 0 : 1;  /* 0+2+4+...+18 = 90 */
}
