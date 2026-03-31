#include <stdio.h>
#include <stdint.h>

#define SIZE 1000

int main(void) {
    // Declare source and destination arrays
    int src[SIZE];
    int dst[SIZE];
    
    // Initialize source array with pattern
    for (int i = 0; i < SIZE; i++) {
        src[i] = i * 3 + 1;
    }
    
    // Use pointer arithmetic to create post-increment patterns
    int *psrc = src;
    int *pdst = dst;
    
    // Loop with clear post-increment pattern
    // This should generate RTL with memory access followed by register increment
    for (int i = 0; i < SIZE; i++) {
        // These operations should create mem_insn structures
        // with reg1_val = 0 for the initial memory reference
        *pdst++ = *psrc++;
    }
    
    // Add another independent access stream to increase chances
    // of hitting the uncovered initialization block
    int sum1 = 0, sum2 = 0;
    int *p1 = dst;
    int *p2 = dst + SIZE/2;
    
    // Second loop with different pointer variables
    // Creates more candidate mem_insn structures
    for (int i = 0; i < SIZE/2; i++) {
        sum1 += *p1++;
        sum2 += *p2++;
    }
    
    // Use volatile to prevent dead code elimination
    // but keep the increment patterns clear
    volatile int result = sum1 + sum2;
    
    printf("Result: %d\n", result);
    
    return 0;
}
