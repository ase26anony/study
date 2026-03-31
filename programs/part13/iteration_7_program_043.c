/* early-remat-test.c
 * Designed to trigger early rematerialization with virtual register spills
 * Compile with: gcc -O2 -fearly-remat -fdump-rtl-early-remat -fdump-rtl-all -fno-omit-frame-pointer -fno-schedule-insns -fno-schedule-insns2 -o early-remat-test early-remat-test.c
 */

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

/* Global variables to create rematerialization candidates */
volatile int global_counter = 0;
int global_array[256] = {0};
const long global_const = 0x123456789ABCDEF0LL;

/* Dummy function to clobber registers */
static void __attribute__((noinline)) clobber_registers(void) {
    __asm__ volatile ("" : : : "memory", "rax", "rbx", "rcx", "rdx", "rsi", "rdi", 
                      "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15");
}

/* Another dummy function with arguments */
static int __attribute__((noinline)) dummy_compute(int a, int b) {
    __asm__ volatile ("" : : "r"(a), "r"(b) : "memory");
    return 0;
}

/* Main test function with high register pressure */
void __attribute__((noinline)) 
stress_early_remat(int param1, int param2, int iterations) {
    volatile int sink = 0;  /* Prevent optimizations */
    long loop_variant;
    int i, j;
    
    /* Outer loop to increase pressure */
    for (i = 0; i < iterations; i++) {
        /* Create many short-lived intermediate values with different modes */
        
        /* SImode constants (expensive to materialize) */
        int const1 = 0x7FFFFFFF;          /* Large 32-bit constant */
        int const2 = 0x80000000;          /* Another large constant */
        int const3 = param1 + 0x1234;     /* Derived from parameter */
        int const4 = param2 << 3;         /* Shifted parameter */
        int const5 = ~param1;             /* Bitwise operation */
        
        /* DImode constants */
        long const6 = global_const + i;   /* Global + loop variant */
        long const7 = (long)param1 * 0x100000000LL;  /* 64-bit constant */
        long const8 = (long)&global_array[i & 0xFF]; /* Address computation */
        
        /* Pointer arithmetic (Pmode) */
        int* ptr1 = &global_array[param1 & 0xFF];
        int* ptr2 = &global_array[param2 & 0xFF];
        int* ptr3 = ptr1 + (i & 0xF);
        
        /* Use in arithmetic to create dependencies */
        int val1 = const1 + const3;
        int val2 = const2 - const4;
        int val3 = const5 * param1;
        long val4 = const6 + const7;
        long val5 = (long)ptr3 - (long)ptr1;
        
        /* Multiple basic blocks to split live ranges */
        if (val1 > 0) {
            /* Branch 1: More computations */
            int val6 = val1 * 2;
            long val7 = val4 >> 4;
            int* ptr4 = ptr2 + (val6 & 0x7);
            
            /* Use in dummy call */
            dummy_compute(val6, (int)val7);
            
            /* Store to volatile to prevent elimination */
            sink = val6 + *ptr4;
        } else {
            /* Branch 2: Different computations */
            int val8 = val2 / 2;
            long val9 = val5 * 3;
            int* ptr5 = ptr1 - (val8 & 0x7);
            
            /* More arithmetic */
            int val10 = val8 ^ const5;
            long val11 = val9 + const6;
            
            /* Another dummy call */
            dummy_compute(val10, (int)val11);
            
            sink = val10 + *ptr5;
        }
        
        /* Second conditional with more values */
        if (val3 < param2) {
            /* Create more intermediate values */
            int val12 = const1 - const4;
            int val13 = val12 * 3;
            long val14 = (long)val13 * const7;
            int* ptr6 = (int*)((char*)ptr3 + val13);
            
            /* Complex expression */
            int val15 = (val13 << 2) | (val12 & 0xFF);
            long val16 = val14 / (const6 + 1);
            
            sink += val15 + *ptr6;
        } else {
            /* Alternative computations */
            int val17 = const2 | const3;
            int val18 = val17 ^ param1;
            long val19 = const8 - const6;
            int* ptr7 = (int*)((uintptr_t)ptr2 + val18);
            
            /* More operations */
            int val20 = val18 * 5;
            long val21 = val19 & 0xFFFFFFFF;
            
            sink += val20 + *ptr7;
        }
        
        /* Third basic block - always executed */
        {
            /* Even more values to increase pressure */
            int val22 = sink + const1;
            int val23 = val22 * param2;
            long val24 = (long)val23 + const6;
            int* ptr8 = &global_array[(i + param1) & 0xFF];
            
            int val25 = val23 / (param1 ? param1 : 1);
            long val26 = val24 - const7;
            
            /* Force use of all created values */
            __asm__ volatile ("# Force use: %0 %1 %2 %3 %4 %5" 
                            : : "r"(val22), "r"(val23), "r"(val24), 
                                "r"(val25), "r"(val26), "r"(ptr8) : "memory");
        }
        
        /* Call to clobber registers - forces spills/reloads */
        clobber_registers();
        
        /* Use results to prevent elimination */
        global_array[i & 0xFF] = sink + i;
    }
}

/* Additional nested loops for more pressure */
void __attribute__((noinline))
nested_loop_stress(int base, int count) {
    volatile int sink = 0;
    int i, j, k;
    
    for (i = 0; i < count; i++) {
        /* Many constants with different derivations */
        int c1 = base + 0x11111111;
        int c2 = base * 0x22222222;
        int c3 = base & 0x33333333;
        int c4 = base | 0x44444444;
        int c5 = base ^ 0x55555555;
        
        for (j = 0; j < 8; j++) {
            /* More derived values */
            int v1 = c1 + (j << 2);
            int v2 = c2 - (j * 3);
            int v3 = c3 ^ j;
            int v4 = c4 | (j << 8);
            int v5 = c5 & ~j;
            
            for (k = 0; k < 4; k++) {
                /* Complex expressions using all values */
                int r1 = v1 * v2 + k;
                int r2 = v3 / (v4 + 1) - k;
                int r3 = (v5 << k) | r1;
                int r4 = r2 ^ r3;
                int r5 = r4 * base;
                
                /* Address computations */
                int* addr1 = &global_array[(r1 + i) & 0xFF];
                int* addr2 = &global_array[(r2 + j) & 0xFF];
                int* addr3 = &global_array[(r3 + k) & 0xFF];
                
                /* Use in asm to prevent optimization */
                __asm__ volatile ("# Nested: %0 %1 %2 %3" 
                                : : "r"(r1), "r"(r2), "r"(r3), "r"(r5) : "memory");
                
                sink += *addr1 + *addr2 + *addr3;
            }
            
            /* Clobber between inner loops */
            clobber_registers();
        }
        
        /* Store result */
        global_counter += sink;
    }
}

int main(int argc, char *argv[]) {
    int param1, param2, iterations;
    
    /* Use command line arguments for variability */
    if (argc > 2) {
        param1 = atoi(argv[1]);
        param2 = atoi(argv[2]);
        iterations = argc > 3 ? atoi(argv[3]) : 1000;
    } else {
        param1 = 0x12345678;
        param2 = 0x9ABCDEF0;
        iterations = 1000;
    }
    
    /* Initialize global array */
    for (int i = 0; i < 256; i++) {
        global_array[i] = i;
    }
    
    /* Run the stress tests */
    stress_early_remat(param1, param2, iterations);
    nested_loop_stress(param1, iterations / 10);
    
    /* Use results to prevent dead code elimination */
    printf("Result: %d %d\n", global_counter, global_array[0]);
    
    return 0;
}
