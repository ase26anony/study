/* early-remat-trigger.c
 * Designed to trigger early rematerialization with virtual register spills
 * Compile with: gcc -O2 -fearly-remat -fdump-rtl-early-remat -fdump-rtl-all -fno-omit-frame-pointer -fno-schedule-insns -fno-schedule-insns2 --param=max-early-remat-iterations=5
 */

#include <stdio.h>
#include <stdlib.h>

/* Global variables to create rematerialization candidates */
volatile int global_base = 42;
volatile long global_offset = 1000;
int global_array[256] = {0};
static const int const_array[] = {1, 2, 3, 5, 7, 11, 13, 17, 19, 23};

/* Dummy function to clobber registers */
static void __attribute__((noinline)) clobber_registers(void) {
    __asm__ volatile ("" : : : "memory", "rax", "rbx", "rcx", "rdx", "rsi", "rdi",
                      "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15");
}

/* Another noinline function to force spills */
static int __attribute__((noinline)) compute_offset(int idx) {
    return idx * 7 + 3;
}

/* Main function with complex loop structure */
int main(int argc, char *argv[]) {
    int iterations = 1000;
    if (argc > 1) {
        iterations = atoi(argv[1]);
        if (iterations <= 0) iterations = 1000;
    }
    
    int base_param = (argc > 2) ? atoi(argv[2]) : 73;
    volatile int sink = 0; /* Prevent optimization */
    
    /* Outer loop to create pressure */
    for (int outer = 0; outer < 10; outer++) {
        /* Inner hot loop with many basic blocks */
        for (int i = 0; i < iterations; i++) {
            /* --- Basic Block 1: Create many remat candidates --- */
            /* Small integer constants (expensive to materialize) */
            int const1 = 0x12345678;      /* Large constant */
            int const2 = 0x87654321;      /* Another large constant */
            long const3 = 0xFEDCBA9876543210LL; /* 64-bit constant */
            
            /* Constants derived from function arguments */
            int derived1 = base_param + 1;
            int derived2 = base_param << 2;
            int derived3 = base_param * 3 + 7;
            
            /* Symbol addresses as remat candidates */
            int *addr1 = &global_array[i & 255];
            int *addr2 = &global_array[(i + 1) & 255];
            const int *addr3 = &const_array[i % 10];
            
            /* Constants from globals */
            int global_derived1 = global_base + i;
            long global_derived2 = global_offset * i;
            
            /* --- Basic Block 2: Conditional branch --- */
            if (i & 1) {
                /* Use all values in arithmetic */
                int temp1 = const1 + derived1;
                int temp2 = const2 * derived2;
                long temp3 = const3 + global_derived2;
                
                /* Pointer arithmetic */
                int *temp_addr = addr1 + (derived3 >> 2);
                int load1 = *temp_addr;
                
                /* Mix operations */
                temp1 = (temp1 << 3) | (temp2 & 0xFF);
                temp3 = (temp3 >> 4) + global_derived2;
                
                /* Store to volatile to prevent elimination */
                sink = temp1 + load1 + (int)(temp3 & 0xFFFFFFFF);
                
                /* Call to clobber registers */
                clobber_registers();
                
                /* More computations with different modes */
                long double_result = (long)temp1 * (long)temp2;
                int *ptr_result = addr2 + (temp1 % 16);
                
                /* Use in condition */
                if (double_result > 1000000) {
                    *ptr_result = temp1;
                } else {
                    *ptr_result = temp2;
                }
            } else {
                /* --- Basic Block 3: Alternative path --- */
                /* Different set of operations */
                int temp4 = derived1 * derived3;
                int temp5 = global_derived1 - const2;
                long temp6 = (long)const1 * (long)global_derived1;
                
                /* More pointer arithmetic */
                const int *temp_addr2 = addr3 + (i % 5);
                int load2 = *temp_addr2;
                
                /* Complex expression */
                temp4 = (temp4 ^ temp5) + load2;
                temp6 = temp6 / (global_derived1 + 1);
                
                /* Another register-clobbering call */
                clobber_registers();
                
                /* Function call that returns value */
                int offset = compute_offset(i);
                temp4 = temp4 + offset;
                
                /* Store results */
                sink = temp4 + (int)(temp6 & 0xFFFF);
                
                /* More mode mixing */
                if (temp6 > 0x7FFFFFFF) {
                    long big_temp = temp6 + const3;
                    *addr2 = (int)(big_temp >> 32);
                }
            }
            
            /* --- Basic Block 4: Merge point --- */
            /* Create more intermediate values */
            int merge1 = sink + i;
            int merge2 = merge1 * base_param;
            long merge3 = (long)merge1 * (long)merge2;
            
            /* Use all addressing modes */
            int *final_addr = &global_array[(merge1 + merge2) & 255];
            *final_addr = merge1;
            
            /* Final clobber */
            clobber_registers();
            
            /* Force use of all created values */
            if ((merge3 & 1) && (merge1 != merge2)) {
                global_array[0] += merge1;
                global_array[1] += merge2;
            }
        }
    }
    
    /* Use results to prevent elimination */
    printf("Result: %d (sink=%d)\n", global_array[0], sink);
    return 0;
}
