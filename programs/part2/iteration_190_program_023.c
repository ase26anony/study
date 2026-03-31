/* caller-save-test.c
 * 
 * This program is designed to trigger GCC's caller-save optimization pass
 * to move a save instruction across a basic block boundary, specifically
 * testing the BB_END update logic in caller-save.cc lines 905-913.
 *
 * Compile with: gcc -O2 -fno-optimize-sibling-calls -march=x86-64 -fdump-rtl-caller_save -o caller-save-test caller-save-test.c
 * Additional flags for debugging: -da -fdump-rtl-all
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* Force register pressure by using multiple call-clobbered registers */
#define USE_REGS() do { \
    volatile unsigned long r1, r2, r3, r4, r5; \
    asm volatile("" : "=r"(r1), "=r"(r2), "=r"(r3), "=r"(r4), "=r"(r5) \
                 : : "rax", "rcx", "rdx", "rsi", "rdi", "r8", "r9", "r10", "r11", "cc"); \
} while(0)

/* Function that creates the specific pattern needed to trigger the BB_END update */
static unsigned long __attribute__((noinline))
caller_save_hot_path(volatile int *arr, int size) {
    unsigned long sum = 0;
    volatile int condition = 1; /* Prevent optimization of control flow */
    
    /* Outer loop to increase caller-save pass activity */
    for (int i = 0; i < size; i++) {
        /* Use call-clobbered registers in calculations */
        unsigned long temp1 = arr[i];
        unsigned long temp2 = i * 3;
        unsigned long temp3 = temp1 ^ temp2;
        unsigned long temp4 = temp3 << 2;
        
        /* Critical conditional block that will create basic block boundaries */
        if (condition | (i & 1)) { /* Always true but compiler doesn't know */
            /* More register pressure before call */
            unsigned long pre_call = temp4 + temp2;
            
            /* Inline assembly that clobbers multiple call-clobbered registers */
            asm volatile("" : : : "rax", "rcx", "rdx", "rsi", "rdi", "r8", "r9", "r10", "r11", "cc");
            
            /* Function call that clobbers call-clobbered registers */
            int r = rand();  /* rand() is a library call */
            
            /* Inline assembly after call to ensure registers are considered clobbered */
            asm volatile("" : : : "rax", "rcx", "rdx", "rsi", "rdi", "r8", "r9", "r10", "r11", "cc");
            
            /* Use the result and more call-clobbered registers */
            unsigned long post_call = pre_call ^ r;
            unsigned long temp5 = post_call * 7;
            unsigned long temp6 = temp5 + temp3;
            
            /* This computation uses multiple temporaries to increase register pressure */
            sum += temp6;
            
            /* Create a value that depends on condition to prevent dead code elimination */
            arr[i] = (sum & 0xFF) | (r & 0xFF00);
        } else {
            /* This path is never taken but needed for block structure */
            sum += 1;
        }
        
        /* Additional computation between iterations to create more save/restore opportunities */
        USE_REGS();
        
        /* Another conditional to potentially split blocks further */
        if ((i % 16) == 0) {
            /* More register-intensive operations */
            unsigned long extra1 = sum * 3;
            unsigned long extra2 = extra1 ^ i;
            sum = extra2;
            
            /* Another function call to create more caller-save opportunities */
            int r2 = rand();
            sum ^= r2;
        }
    }
    
    return sum;
}

int main(void) {
    const int SIZE = 1000;
    volatile int *array = malloc(SIZE * sizeof(int));
    
    if (!array) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with volatile to prevent optimization */
    for (int i = 0; i < SIZE; i++) {
        array[i] = i * 3 + 1;
    }
    
    srand(time(NULL));
    
    /* Call the hot path multiple times to increase coverage chances */
    unsigned long total = 0;
    for (int iter = 0; iter < 10; iter++) {
        total += caller_save_hot_path(array, SIZE);
        
        /* Modify array slightly between iterations */
        for (int i = 0; i < SIZE; i += 7) {
            array[i] ^= iter;
        }
    }
    
    printf("Result: %lu\n", total);
    
    /* Use the array to prevent optimization */
    volatile int check = 0;
    for (int i = 0; i < SIZE; i++) {
        check += array[i];
    }
    printf("Array checksum: %d\n", check);
    
    free((void*)array);
    return 0;
}
