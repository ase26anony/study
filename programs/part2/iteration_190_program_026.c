/* caller-save-test.c
 * 
 * This program is designed to trigger GCC's caller-save optimization pass
 * to move a save/restore instruction across a basic block boundary,
 * specifically testing the BB_END update logic in caller-save.cc lines 905-913.
 * 
 * Compile with: gcc -O2 -fno-optimize-sibling-calls -march=x86-64 -fdump-rtl-caller_save -o caller-save-test caller-save-test.c
 * 
 * The generated RTL dumps will show caller-save activity. For coverage instrumentation,
 * compile with coverage flags appropriate for your testing framework.
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* Helper function that creates register pressure around a volatile call */
static int __attribute__((noinline))
pressure_function(volatile int *arr, int idx) {
    /* Use multiple call-clobbered registers in calculations */
    register long r1 asm("rax") = arr[idx] * 3;
    register long r2 asm("rcx") = arr[idx + 1] * 5;
    register long r3 asm("rdx") = arr[idx + 2] * 7;
    register long r4 asm("rsi") = arr[idx + 3] * 11;
    
    /* Volatile function call that clobbers registers */
    int r = rand();
    
    /* More calculations using the same registers */
    r1 ^= r;
    r2 += r1;
    r3 -= r2;
    r4 *= r3;
    
    /* Inline assembly that explicitly clobbers call-clobbered registers */
    asm volatile("" : : "r"(r1), "r"(r2), "r"(r3), "r"(r4) : "rax", "rcx", "rdx", "rsi", "cc");
    
    return (int)(r1 + r2 + r3 + r4);
}

/* Main test function with complex control flow */
static int __attribute__((noinline))
test_caller_save_scenario(void) {
    volatile int seed = time(NULL);  /* Prevent optimization */
    srand(seed);
    
    volatile int array[256];
    for (int i = 0; i < 256; i++) {
        array[i] = i * 3;
    }
    
    int sum = 0;
    volatile int threshold = 128;  /* Volatile to preserve control flow */
    
    /* Outer loop to increase caller-save pass activity */
    for (int outer = 0; outer < 100; outer++) {
        /* Inner loop with conditional that creates block boundaries */
        for (int i = 0; i < 250; i++) {
            /* This condition creates a basic block ending with a conditional jump */
            if (array[i] > threshold) {  /* Block boundary here */
                /* This block contains the function call and will have
                 * caller-save instructions inserted around it */
                int result = pressure_function(array, i);
                
                /* Additional register pressure */
                register long t1 asm("rax") = result;
                register long t2 asm("rcx") = result * 2;
                register long t3 asm("rdx") = result * 3;
                
                /* Another volatile call to force more save/restore */
                int r2 = rand();
                
                t1 ^= r2;
                t2 += t1;
                t3 -= t2;
                
                /* Clobber registers again */
                asm volatile("" : : "r"(t1), "r"(t2), "r"(t3) : "rax", "rcx", "rdx", "cc");
                
                sum += (int)(t1 + t2 + t3);
            } else {
                /* Alternative path to maintain control flow complexity */
                sum -= array[i];
            }
            
            /* Mix in another conditional to split blocks further */
            if (i % 3 == 0) {
                /* Another call site with different register usage */
                register long a1 asm("rax") = sum;
                register long a2 asm("r10") = sum * 2;
                
                int r3 = rand();
                
                a1 |= r3;
                a2 &= r3;
                
                asm volatile("" : : "r"(a1), "r"(a2) : "rax", "r10", "cc");
                
                sum = (int)(a1 ^ a2);
            }
        }
        
        /* Modify threshold to change control flow pattern */
        threshold = (threshold + 1) % 256;
    }
    
    return sum;
}

int main(void) {
    int result = test_caller_save_scenario();
    printf("Result: %d\n", result);
    
    /* Additional test with different optimization pressure */
    volatile int quick_test = 1;
    if (quick_test) {
        /* Small test case that might trigger different block patterns */
        volatile int arr[10] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
        int quick_sum = 0;
        
        for (int i = 0; i < 10; i++) {
            if (arr[i] > 5) {
                register long x asm("rax") = arr[i];
                register long y asm("rcx") = arr[i] * 2;
                
                int r = rand();
                
                x += r;
                y -= r;
                
                asm volatile("" : : "r"(x), "r"(y) : "rax", "rcx", "cc");
                
                quick_sum += (int)(x + y);
            }
        }
        printf("Quick sum: %d\n", quick_sum);
    }
    
    return 0;
}
