/* early-remat-trigger.c */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Global variables to create rematerialization candidates */
volatile int global_base = 1000;
volatile long global_offset = 2000;
int global_array[256] = {0};
static volatile int sink;

/* Dummy function to clobber registers */
static void __attribute__((noinline)) clobber_registers(void) {
    __asm__ volatile ("" : : : "memory", "rax", "rbx", "rcx", "rdx", 
                      "rsi", "rdi", "r8", "r9", "r10", "r11", "r12", 
                      "r13", "r14", "r15");
}

/* Another dummy function with arguments */
static int __attribute__((noinline)) dummy_compute(int a, long b) {
    __asm__ volatile ("# dummy compute %0 %1" : : "r"(a), "r"(b));
    return a + (int)b;
}

/* Complex loop with many basic blocks and register pressure */
void stress_early_remat(int param1, int param2) {
    volatile int outer_sink = 0;
    long loop_counter;
    
    /* Outer loop to increase pressure */
    for (loop_counter = 0; loop_counter < 1000; loop_counter++) {
        /* Many short-lived intermediate values - potential remat candidates */
        int const1 = 0x7FFFFFFF;  /* Large constant requiring multiple insns */
        int const2 = -0x80000000; /* Another large constant */
        long const3 = 0x123456789ABCDEF0LL; /* 64-bit constant */
        int const4 = param1 + 1;  /* Derived from argument */
        int const5 = param2 << 2; /* Shifted argument */
        int const6 = param1 * 3;  /* Multiplied argument */
        
        /* Address computations - good remat candidates */
        int *addr1 = &global_array[param1 & 0xFF];
        int *addr2 = &global_array[param2 & 0xFF];
        long *addr3 = (long*)&global_array[loop_counter & 0xFF];
        
        /* More derived values */
        int val1 = const1 + const4;
        int val2 = const2 - const5;
        long val3 = const3 + loop_counter;
        int val4 = const6 * 2;
        int val5 = param1 ^ param2;
        int val6 = ~param1;
        int val7 = param2 | 0xFF00;
        long val8 = (long)param1 << 32 | param2;
        int val9 = (param1 + 7) & ~7;
        int val10 = param2 * param1;
        
        /* First basic block - use some values */
        if (val1 > val2) {
            /* Branch creates new basic block */
            int branch_val1 = val3 & 0xFFFFFFFF;
            int branch_val2 = val4 + branch_val1;
            long branch_val3 = val8 >> 16;
            
            /* More computations */
            int temp1 = branch_val1 * branch_val2;
            int temp2 = temp1 + const5;
            long temp3 = branch_val3 - const3;
            
            /* Store to volatile to prevent elimination */
            sink = temp1 + temp2;
            outer_sink += (int)temp3;
            
            /* Call to clobber registers */
            clobber_registers();
        } else {
            /* Alternative branch - different computations */
            int branch_val4 = val5 ^ val6;
            int branch_val5 = val7 & 0xFFFF;
            long branch_val6 = (long)val9 * val10;
            
            /* Pointer arithmetic */
            int *ptr1 = addr1 + (branch_val4 & 0xF);
            int *ptr2 = addr2 + (branch_val5 & 0xF);
            
            /* More intermediate values */
            int temp4 = *ptr1 + branch_val4;
            int temp5 = *ptr2 - branch_val5;
            long temp6 = branch_val6 / (val10 ? val10 : 1);
            
            /* Another clobber call */
            clobber_registers();
            
            sink = temp4 * temp5;
            outer_sink += (int)temp6;
        }
        
        /* Third basic block - more computations */
        {
            /* Mix different data types */
            int mix1 = const1 >> 2;
            long mix2 = const3 << 1;
            int mix3 = mix1 + (int)mix2;
            int mix4 = const4 * const5;
            
            /* More address computations */
            uintptr_t addr_val1 = (uintptr_t)addr1;
            uintptr_t addr_val2 = (uintptr_t)addr2;
            uintptr_t addr_val3 = addr_val1 ^ addr_val2;
            
            /* Function call with multiple arguments */
            int call_result = dummy_compute(mix3, mix2);
            
            /* Complex expression with many operands */
            int complex1 = call_result + mix4 + (int)addr_val3;
            int complex2 = complex1 * param1 / (param2 ? param2 : 1);
            long complex3 = (long)complex2 * loop_counter;
            
            /* Final store */
            sink = complex1 ^ complex2;
            outer_sink += (int)(complex3 & 0xFFFFFFFF);
        }
        
        /* Fourth basic block - nested conditionals */
        if (loop_counter & 1) {
            if (param1 > param2) {
                int nested1 = global_base + param1;
                int nested2 = global_offset - param2;
                int nested3 = nested1 * nested2;
                
                /* More remat candidates */
                int const7 = 0x55555555;
                int const8 = 0xAAAAAAAA;
                int const9 = const7 ^ const8;
                
                sink = nested3 + const9;
                clobber_registers();
            } else {
                long nested4 = (long)global_base * global_offset;
                int nested5 = (int)(nested4 >> 16);
                int nested6 = nested5 & 0xFFFF;
                
                sink = nested6;
            }
        }
        
        /* Update parameters to create varying values */
        param1 = (param1 * 1103515245 + 12345) & 0x7FFFFFFF;
        param2 = (param2 * 1664525 + 1013904223) & 0x7FFFFFFF;
    }
    
    /* Prevent elimination of entire function */
    printf("Result: %d\n", outer_sink);
}

/* Main function with argument handling */
int main(int argc, char *argv[]) {
    int param1 = 123456;
    int param2 = 654321;
    
    /* Use command line arguments if provided */
    if (argc > 1) {
        param1 = atoi(argv[1]);
    }
    if (argc > 2) {
        param2 = atoi(argv[2]);
    }
    
    /* Multiple calls to increase overall pressure */
    for (int i = 0; i < 10; i++) {
        stress_early_remat(param1 + i, param2 - i);
    }
    
    return 0;
}
