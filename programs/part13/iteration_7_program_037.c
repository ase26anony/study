/* early-remat-trigger.c
 * Designed to trigger early rematerialization with virtual register spills
 * Compile with: gcc -O2 -fearly-remat -fdump-rtl-early-remat -S early-remat-trigger.c
 */

#include <stdint.h>
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

/* Another noinline function to force spills */
static int __attribute__((noinline)) use_value(int x) {
    __asm__ volatile ("# use_value %0" : : "r"(x));
    return x + 1;
}

/* Main function with complex loop structure */
int main(int argc, char *argv[]) {
    /* Use arguments to create variant values */
    int base = argc > 1 ? atoi(argv[1]) : 1000;
    int offset = argc > 2 ? atoi(argv[2]) : 42;
    
    volatile int sink = 0;  /* Prevent elimination of computations */
    
    /* Outer loop to create pressure */
    for (int outer = 0; outer < 1000; outer++) {
        /* Many distinct integer constants (remat candidates) */
        const int c1 = 0x7FFFFFFF;      /* Large constant requiring multiple insns */
        const int c2 = -0x80000000;     /* Another large constant */
        const long c3 = 0x12345678;     /* Different size constant */
        const long c4 = global_const;   /* Global constant address */
        
        /* Derived constants from arguments (remat candidates) */
        int derived1 = base + 1;        /* param + 1 */
        int derived2 = base << 2;       /* param << 2 */
        int derived3 = base * 3;        /* param * 3 */
        long derived4 = (long)base * 5; /* Different type */
        
        /* Symbol addresses (remat candidates) */
        int *addr1 = &global_array[0];
        int *addr2 = &global_array[16];
        int *addr3 = &global_array[32];
        int *addr4 = &global_array[48];
        
        /* Loop-variant values */
        for (int i = 0; i < 100; i++) {
            /* Create many intermediate values with different modes */
            int val1 = c1 + i;              /* SImode */
            long val2 = c3 + i;             /* DImode */
            int *val3 = addr1 + i;          /* Pmode */
            long val4 = derived4 + i * 2;   /* DImode */
            
            /* Complex conditional branching creating multiple basic blocks */
            if (i & 1) {
                /* Branch 1: More computations */
                int tmp1 = val1 * 2;
                long tmp2 = val2 >> 1;
                int *tmp3 = val3 + offset;
                
                /* Use derived constants */
                tmp1 += derived1;
                tmp2 += derived2;
                
                /* Force register pressure with many live values */
                int tmp4 = tmp1 + c2;
                long tmp5 = tmp2 + (long)derived3;
                int *tmp6 = tmp3 - offset;
                
                /* Store to prevent elimination */
                sink = tmp4;
                global_array[i & 255] = tmp5;
                
                /* Call to clobber registers */
                clobber_registers();
                
                /* Use the values again after call */
                tmp1 = use_value(tmp4);
                tmp2 = tmp5 + global_counter;
                
            } else {
                /* Branch 2: Different computations */
                int tmp1 = val1 / 2;
                long tmp2 = val2 << 1;
                int *tmp3 = addr2 + i;
                
                /* Use different derived constants */
                tmp1 += derived2;
                tmp2 += derived3;
                
                /* More intermediate values */
                int tmp4 = tmp1 - c1;
                long tmp5 = tmp2 - derived4;
                int *tmp6 = addr3 + (i * 3);
                
                /* Store results */
                sink = tmp4;
                global_array[(i + 1) & 255] = tmp5;
                
                /* Another register-clobbering call */
                clobber_registers();
                
                /* Reuse values */
                tmp1 = use_value(tmp4);
                tmp2 = tmp5 - global_counter;
            }
            
            /* Common code after branches with more computations */
            int common1 = val1 + outer;
            long common2 = val2 * 2;
            int *common3 = addr4 + i;
            
            /* Mix operations with different modes */
            common1 += (int)val4;
            common2 += (long)common1;
            
            /* Use all address constants */
            int load1 = *addr1;
            int load2 = *addr2;
            int load3 = *addr3;
            int load4 = *addr4;
            
            /* More arithmetic to keep values live */
            common1 += load1 + load2 + load3 + load4;
            common2 += common1;
            
            /* Final store */
            sink = common1;
            global_counter += common2;
            
            /* One more clobber call */
            clobber_registers();
        }
        
        /* Modify base to create varying derived constants */
        base += outer;
        offset ^= outer;
    }
    
    return sink;
}

/* Additional function to create more register pressure */
void __attribute__((noinline)) extra_pressure(int iterations) {
    volatile int sink = 0;
    
    for (int i = 0; i < iterations; i++) {
        /* Many constants in a tight loop */
        const int k1 = 0x55555555;
        const int k2 = 0xAAAAAAAA;
        const long k3 = 0x3333333333333333LL;
        const long k4 = 0xCCCCCCCCCCCCCCCCLL;
        
        /* Computations with all constants live */
        int v1 = k1 + i;
        int v2 = k2 - i;
        long v3 = k3 + (long)i;
        long v4 = k4 - (long)i;
        
        /* Use them in complex expressions */
        v1 = (v1 * v2) + (int)v3;
        v2 = (v2 / (i + 1)) + (int)v4;
        v3 = v3 ^ v4;
        v4 = v4 | (long)v1;
        
        /* Force spills with many simultaneous values */
        sink = v1 + v2 + (int)v3 + (int)v4;
        
        clobber_registers();
    }
}
