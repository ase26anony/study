/* early-remat-trigger.c
 * Designed to trigger early rematerialization with virtual register spills
 * Specifically targets lines 930-937 in early-remat.cc
 */

#include <stdint.h>
#include <stdlib.h>

/* Global variables to create rematerialization candidates */
volatile int global_base = 100;
volatile long global_offset = 1000;
int global_array[256] = {0};
static volatile int sink; /* Prevent optimization */

/* Dummy function to clobber registers */
static void __attribute__((noinline)) clobber_registers(void) {
    __asm__ volatile ("" : : : "memory", "rax", "rbx", "rcx", "rdx", 
                      "rsi", "rdi", "r8", "r9", "r10", "r11", "r12", 
                      "r13", "r14", "r15");
}

/* Another clobbering function with different signature */
static int __attribute__((noinline)) use_registers(int x, long y) {
    __asm__ volatile ("# dummy asm" : "+r"(x), "+r"(y) : : "memory");
    return x + (int)y;
}

/* Complex function with many basic blocks */
void __attribute__((noinline)) 
process_values(int param1, int param2, long param3, int iterations) {
    volatile int result = 0;
    
    /* Outer loop to create register pressure */
    for (int outer = 0; outer < iterations; outer++) {
        /* Create many short-lived intermediate values */
        int c1 = param1 + 1;           /* Constant derived from argument */
        int c2 = param2 << 2;          /* Shifted constant */
        long c3 = param3 * 3;          /* Multiplied constant */
        int c4 = global_base + outer;  /* Global + loop variant */
        long c5 = global_offset * 2;   /* Global constant */
        
        /* Pointer arithmetic - creates Pmode REGs */
        int *ptr1 = &global_array[outer % 256];
        int *ptr2 = ptr1 + (param1 >> 1);
        long *ptr3 = (long *)&global_array[(outer + 1) % 256];
        
        /* Force multiple basic blocks with conditional branches */
        if (outer & 1) {
            /* Branch 1: More computations */
            int t1 = c1 * c2;
            long t2 = c3 + c5;
            int t3 = t1 ^ param1;
            long t4 = t2 - param3;
            
            /* Use pointers */
            *ptr1 = t1;
            ptr3[0] = t4;
            
            /* Call to clobber registers */
            clobber_registers();
            
            /* More computations after call */
            int t5 = t3 + *ptr1;
            long t6 = t4 + (long)ptr2;
            result += t5 + (int)t6;
        } else {
            /* Branch 2: Different computations */
            int t1 = c1 | c2;
            long t2 = c3 & c5;
            int t3 = t1 + c4;
            long t4 = t2 * 2;
            
            /* Different pointer usage */
            ptr2[0] = t3;
            *ptr3 = t4;
            
            /* Function call with arguments - more register pressure */
            int t5 = use_registers(t3, t4);
            long t6 = (long)t5 * c3;
            
            /* Complex expression with many operands */
            result += (t5 << 3) + (int)(t6 >> 2) + *ptr2;
        }
        
        /* Third basic block (executed after if/else) */
        {
            /* Create more intermediate values */
            int d1 = result & 0xFF;
            long d2 = (long)result * 7;
            int d3 = d1 ^ param2;
            long d4 = d2 + global_offset;
            
            /* Mixed mode operations */
            int *ptr4 = &global_array[d1 % 256];
            long lval = d4 + (long)ptr4;
            
            /* Another register-clobbering call */
            clobber_registers();
            
            /* Final computation using all types */
            int final1 = d3 + *ptr4;
            long final2 = lval - d2;
            result = final1 + (int)final2;
        }
        
        /* Additional loop with more register pressure */
        for (int inner = 0; inner < 4; inner++) {
            /* Constants that are rematerialization candidates */
            int k1 = 0x12345678;      /* Large constant */
            long k2 = 0x87654321ABCDEFLL; /* Very large constant */
            int k3 = param1 + inner;   /* Argument-derived */
            long k4 = param3 << inner; /* Shifted argument */
            
            /* Address computations */
            int *addr1 = &global_array[(outer + inner) % 256];
            long *addr2 = (long *)&global_array[(outer - inner + 256) % 256];
            
            /* Complex expression tree */
            int val1 = (k1 * k3) >> (inner + 1);
            long val2 = (k2 + k4) * (inner + 1);
            int val3 = val1 + (int)(val2 & 0xFFFFFFFF);
            long val4 = val2 + (long)val3;
            
            /* Store results */
            addr1[0] = val3;
            addr2[0] = val4;
            
            /* Occasional function call */
            if ((inner & 1) == 0) {
                use_registers(val3, val4);
            }
            
            /* Update result */
            result ^= val3 + (int)(val4 >> 32);
        }
        
        /* Prevent everything from being optimized away */
        sink = result;
    }
}

/* Main function with command line arguments */
int main(int argc, char *argv[]) {
    int iterations = 1000;
    int param1 = 42;
    int param2 = 73;
    long param3 = 123456789L;
    
    /* Use command line arguments if provided */
    if (argc > 1) {
        iterations = atoi(argv[1]);
        if (iterations < 1) iterations = 1000;
    }
    if (argc > 2) {
        param1 = atoi(argv[2]);
    }
    if (argc > 3) {
        param2 = atoi(argv[3]);
    }
    if (argc > 4) {
        param3 = atol(argv[4]);
    }
    
    /* Initialize global array */
    for (int i = 0; i < 256; i++) {
        global_array[i] = i;
    }
    
    /* Call the processing function */
    process_values(param1, param2, param3, iterations);
    
    /* Return something based on results */
    return sink & 1;
}
