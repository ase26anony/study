/* This program is designed to trigger GCC's caller-save optimization pass
   to move an instruction that is currently marked as BB_END, covering
   lines 905-913 in caller-save.cc. It creates register pressure around
   function calls and specific control flow to force the pass to update
   basic block metadata when moving save/restore instructions. */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* Force register pressure by using multiple call-clobbered registers */
#define USE_REGS() do { \
    asm volatile("" : : : "rax", "rcx", "rdx", "rsi", "rdi", "r8", "r9", "r10", "r11", "cc", "memory"); \
} while(0)

/* Function that creates the specific pattern needed to trigger the BB_END update */
__attribute__((noinline))
int process_data(volatile int *arr, int size) {
    int sum = 0;
    volatile int trigger = 1; /* Prevent optimization of control flow */
    
    /* Outer loop to increase chances of hitting the scenario */
    for (int i = 0; i < size; i++) {
        int temp1, temp2, temp3, temp4;
        
        /* Create register pressure with computations in call-clobbered registers */
        temp1 = arr[i] * 3;
        temp2 = arr[i] + 7;
        temp3 = arr[i] - 2;
        temp4 = arr[i] / 5;
        
        /* Critical conditional block - the 'then' block will end with a jump */
        if (trigger || (arr[i] % 3 == 0)) {
            /* More register pressure before the call */
            int pre1 = temp1 * temp2;
            int pre2 = temp3 + temp4;
            
            /* Clobber call-clobbered registers to force save/restore */
            USE_REGS();
            
            /* Function call that clobbers registers - rand() is a good candidate */
            int r = rand();
            
            /* More register pressure after the call */
            int post1 = pre1 ^ r;
            int post2 = pre2 & r;
            
            /* Clobber again to ensure register pressure */
            USE_REGS();
            
            /* Complex computation using multiple temporaries */
            sum += post1 + post2 + (r % 256);
            
            /* This block ends here - the BB_END might be the jump to merge point */
        } else {
            /* Alternate path to maintain block structure */
            sum += temp1 + temp2;
        }
        
        /* Additional computations to increase register pressure across iterations */
        int mix = (sum * 1103515245 + 12345) & 0x7fffffff;
        if (mix % 7 == 0) {
            USE_REGS();
            sum ^= rand();
        }
    }
    
    return sum;
}

/* Another function with similar pattern but different structure */
__attribute__((noinline))
int process_data2(volatile int *arr, int size) {
    int total = 0;
    volatile int flag = 0;
    
    for (int j = 0; j < size; j += 2) {
        int a = arr[j];
        int b = arr[j + 1];
        
        /* Nested conditionals to create complex control flow */
        if (flag || (a > b)) {
            int x = a * a;
            int y = b * b;
            
            USE_REGS();
            
            /* Multiple calls in sequence */
            int r1 = rand();
            int r2 = rand();
            
            USE_REGS();
            
            if (r1 > r2) {
                total += x - y;
            } else {
                total += y - x;
            }
            
            /* More register-intensive computations */
            for (int k = 0; k < 3; k++) {
                int t = (r1 << k) | (r2 >> k);
                USE_REGS();
                total ^= t;
            }
        } else {
            total += a + b;
        }
    }
    
    return total;
}

int main(void) {
    const int SIZE = 1000;
    volatile int data[SIZE];
    int result1, result2;
    
    /* Initialize with volatile to prevent optimization */
    srand(time(NULL));
    for (int i = 0; i < SIZE; i++) {
        data[i] = rand() % 100;
    }
    
    /* Call the functions multiple times to increase coverage chances */
    result1 = process_data(data, SIZE);
    result2 = process_data2(data, SIZE);
    
    /* Use results to prevent dead code elimination */
    printf("Result 1: %d\n", result1);
    printf("Result 2: %d\n", result2);
    printf("Combined: %d\n", result1 + result2);
    
    return 0;
}
