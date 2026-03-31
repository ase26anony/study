/* test_cleanup.c - Minimal C program to trigger GCC driver cleanup logic */
#include <stdio.h>

/* Simple computation to prevent optimization */
static int compute_sum(void) {
    int array[10];
    int sum = 0;
    
    for (int i = 0; i < 10; i++) {
        array[i] = i * 2;
        sum += array[i];
    }
    
    /* Use volatile to prevent dead code elimination */
    volatile int result = sum;
    return result;
}

/* Another function to ensure multiple compilation units if split */
static void dummy_function(void) {
    static int counter = 0;
    counter++;
}

int main(void) {
    int total = compute_sum();
    dummy_function();
    
    /* Print to ensure code isn't optimized away */
    printf("Result: %d\n", total);
    
    return (total == 90) ? 0 : 1;
}
