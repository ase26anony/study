/* early-remat-trigger.c
 * Designed to trigger early rematerialization with virtual register spills
 * Compile with: gcc -O2 -fearly-remat -fdump-rtl-early-remat -fdump-rtl-all -fno-omit-frame-pointer -fno-schedule-insns -fno-schedule-insns2 --param=max-early-remat-iterations=5 -o early-remat-test early-remat-trigger.c
 */

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

/* Global variables to create rematerialization candidates */
volatile int global_base = 1000;
volatile long global_offset = 2000;
int global_array[256] = {0};
static volatile int sink; /* Prevent dead code elimination */

/* Function to clobber registers - forces spills */
static void __attribute__((noinline)) clobber_registers(void) {
    __asm__ volatile ("" : : : "memory", "rax", "rbx", "rcx", "rdx", "rsi", "rdi",
                     "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15");
}

/* Another clobbering function with different signature */
static void __attribute__((noinline)) clobber_more(void* p1, void* p2) {
    __asm__ volatile ("# clobber %0, %1" : : "r"(p1), "r"(p2) : "memory");
}

/* Complex computation that generates many intermediate values */
static int __attribute__((noinline)) 
compute_value(int base, int idx, int param1, int param2) {
    /* Create many rematerialization candidates */
    const int small_const = 42;          /* Likely remat candidate */
    const long large_const = 0x7FFF0000; /* Requires multiple instructions */
    const int shift_const = 3;           /* Simple constant */
    
    /* Derived constants from parameters - good remat candidates */
    int derived1 = param1 + 1;           /* param + constant */
    int derived2 = param2 << 2;          /* param << constant */
    int derived3 = param1 * 3;           /* param * constant */
    long derived4 = (long)param2 * 5L;   /* Different type */
    
    /* Address computations - likely rematerialized */
    int* addr1 = &global_array[idx % 256];
    int* addr2 = &global_array[(idx + 1) % 256];
    int* addr3 = &global_array[(idx + small_const) % 256];
    
    /* Force register pressure with many live values */
    volatile int temp1 = base + small_const;
    volatile int temp2 = base - shift_const;
    volatile long temp3 = (long)base + large_const;
    volatile int temp4 = derived1 + derived2;
    volatile int temp5 = derived3 * 2;
    volatile long temp6 = derived4 >> 1;
    
    /* Use the addresses */
    volatile int load1 = *addr1;
    volatile int load2 = *addr2;
    volatile int load3 = *addr3;
    
    /* More computations creating data dependencies */
    int result = temp1;
    result += temp2;
    result += (int)(temp3 & 0xFFFF);
    result += temp4;
    result += temp5;
    result += (int)temp6;
    result += load1;
    result += load2;
    result += load3;
    
    return result;
}

/* Main hot loop with multiple basic blocks */
void __attribute__((noinline))
hot_loop(int iterations, int param1, int param2) {
    volatile int accumulator = 0;
    volatile long long_accumulator = 0;
    
    /* Outer loop to increase pressure */
    for (int outer = 0; outer < 10; outer++) {
        /* Inner loop with many intermediate values */
        for (int i = 0; i < iterations; i++) {
            /* Basic block 1: Compute many values */
            int val1 = compute_value(i, i, param1, param2);
            int val2 = compute_value(i + 1, i, param2, param1);
            long val3 = (long)val1 * (long)val2;
            int val4 = val1 ^ val2;
            int val5 = val1 & 0xFF;
            int val6 = val2 | 0x7F;
            
            /* Force spill point with clobber */
            clobber_registers();
            
            /* Basic block 2: Conditional branch creating new BB */
            if (val1 > val2) {
                /* Branch block with its own computations */
                int branch_val1 = val1 + global_base;
                int branch_val2 = val2 + (int)global_offset;
                long branch_val3 = val3 >> 2;
                
                /* More remat candidates in branch */
                const int branch_const = 0xABCD;
                int* branch_addr = &global_array[(i + branch_const) % 256];
                volatile int branch_load = *branch_addr;
                
                accumulator += branch_val1 + branch_val2 + branch_load;
                long_accumulator += branch_val3;
                
                /* Another clobber in branch */
                clobber_more((void*)branch_addr, (void*)&global_base);
            } else {
                /* Else block with different computations */
                int else_val1 = val1 - global_base;
                int else_val2 = val2 - (int)global_offset;
                long else_val3 = val3 << 1;
                
                /* Different constants in else block */
                const int else_const = 0x1234;
                int* else_addr = &global_array[(i + else_const) % 256];
                volatile int else_load = *else_addr;
                
                accumulator += else_val1 * else_val2 + else_load;
                long_accumulator += else_val3;
                
                /* Clobber in else path */
                clobber_registers();
            }
            
            /* Basic block 3: More computations after branch */
            int val7 = val4 + val5;
            int val8 = val6 - val7;
            long val9 = (long)val7 * (long)val8;
            
            /* Pointer arithmetic with different modes */
            int* ptr1 = global_array + (val7 & 0xFF);
            int* ptr2 = global_array + (val8 & 0xFF);
            volatile int diff = ptr2 - ptr1;  /* Pointer difference */
            
            /* Mixed mode operations */
            long val10 = val9 + (long)diff;
            int val11 = (int)(val10 >> 16);
            
            /* Store results to prevent elimination */
            sink = val11;
            accumulator += val11;
            long_accumulator += val10;
            
            /* Final clobber to force spills */
            clobber_more((void*)ptr1, (void*)ptr2);
            
            /* Another conditional to create more BBs */
            if ((i & 0x3F) == 0) {
                /* Rare path with more computations */
                int rare_val = compute_value(i, i ^ 0xFF, param1 ^ param2, param2 ^ param1);
                accumulator ^= rare_val;
                clobber_registers();
            }
        }
    }
    
    /* Use results */
    printf("Result: %d (long: %ld)\n", accumulator, long_accumulator);
}

int main(int argc, char* argv[]) {
    int iterations = 1000;
    int param1 = 12345;
    int param2 = 67890;
    
    /* Use command line arguments to vary inputs */
    if (argc > 1) {
        iterations = atoi(argv[1]);
        if (iterations <= 0) iterations = 1000;
    }
    if (argc > 2) {
        param1 = atoi(argv[2]);
    }
    if (argc > 3) {
        param2 = atoi(argv[3]);
    }
    
    /* Initialize global array */
    for (int i = 0; i < 256; i++) {
        global_array[i] = i * 3;
    }
    
    /* Run the hot loop */
    hot_loop(iterations, param1, param2);
    
    return 0;
}
