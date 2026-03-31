/* early-remat-test.c
 * Designed to trigger early rematerialization with virtual register spills
 * Specifically targets lines 930-937 in early-remat.cc
 */

#include <stdint.h>
#include <stdlib.h>

/* Global variables to create rematerialization candidates */
volatile int global_counter = 0;
int global_array[256] = {0};
static const long global_const = 0x123456789ABCDEF0ULL;

/* Dummy function to clobber registers */
static void __attribute__((noinline)) clobber_registers(void) {
    __asm__ volatile ("" : : : "memory", "rax", "rbx", "rcx", "rdx", 
                      "rsi", "rdi", "r8", "r9", "r10", "r11", "r12", 
                      "r13", "r14", "r15");
}

/* Another noinline function to force spills */
static int __attribute__((noinline)) use_value(int x) {
    __asm__ volatile ("" : "+r" (x));
    return x;
}

/* Main test function with high register pressure */
int main(int argc, char *argv[]) {
    /* Use arguments to create variant values */
    int base = argc > 1 ? atoi(argv[1]) : 1000;
    int modifier = argc > 2 ? atoi(argv[2]) : 7;
    
    volatile int sink = 0;  /* Prevent optimizations */
    
    /* Outer loop to create hot region */
    for (int outer = 0; outer < 1000; outer++) {
        /* Create many short-lived intermediate values with different modes */
        
        /* Integer constants (SImode candidates) */
        int c1 = 0x7FFFFFFF;          /* Large constant requiring multiple insns */
        int c2 = -123456789;          /* Another large constant */
        int c3 = base * 2;            /* Derived from argument */
        int c4 = modifier << 4;       /* Shifted constant */
        int c5 = global_counter + 1;  /* Global-based constant */
        
        /* Long constants (DImode candidates) */
        long l1 = global_const;       /* 64-bit constant */
        long l2 = (long)base * 1000000L;
        long l3 = 0xFEDCBA9876543210LL;
        long l4 = (long)modifier << 32;
        
        /* Pointer/address constants (Pmode candidates) */
        int *p1 = &global_array[0];
        int *p2 = &global_array[modifier];
        int *p3 = &global_array[base & 0xFF];
        int *p4 = (int*)((uintptr_t)&global_array[0] + 0x1000);
        
        /* Complex derived values */
        int d1 = (base * 3) / 2;
        int d2 = (modifier << 3) | 0xF;
        int d3 = (c1 + c2) >> 1;
        long d4 = (l1 + l2) * 2;
        
        /* Force use in conditional branches to create multiple basic blocks */
        if (outer & 1) {
            /* Branch 1: Use integer constants */
            int t1 = c1 + c3;
            int t2 = c2 * modifier;
            int t3 = t1 ^ t2;
            sink = use_value(t3);
            
            /* More computations */
            long t4 = l1 + l3;
            long t5 = l2 - l4;
            sink = (int)(t4 ^ t5);
            
            /* Use pointers */
            *p1 = t1;
            *p2 = t2;
        } else {
            /* Branch 2: Different computation pattern */
            int t1 = c4 - c5;
            int t2 = d1 * d2;
            int t3 = t1 | t2;
            sink = use_value(t3);
            
            /* More long computations */
            long t4 = d4 / 3;
            long t5 = l3 & l4;
            sink = (int)(t4 + t5);
            
            /* Different pointer usage */
            *p3 = t1;
            *p4 = t2;
        }
        
        /* Another conditional block */
        if (outer & 2) {
            /* Mix all types */
            int r1 = c1 + (int)l1;
            long r2 = l2 + (long)c2;
            int r3 = (int)((uintptr_t)p1 + (uintptr_t)p2);
            sink = r1 + r2 + r3;
        }
        
        /* Third conditional with nested if */
        if (outer & 4) {
            int x = base + modifier;
            if (x & 1) {
                int y = x * 3;
                int z = y + c1;
                sink = z;
            } else {
                long y = (long)x * 5;
                long z = y - l1;
                sink = (int)z;
            }
        }
        
        /* Force register clobbering */
        clobber_registers();
        
        /* More computations after clobber (forces reloads/remat) */
        int f1 = c3 + modifier;
        long f2 = l2 + outer;
        int *f3 = &global_array[(f1 + f2) & 0xFF];
        *f3 = f1;
        
        /* Use all remaining values to keep them live */
        sink += d3 + (int)d4 + (int)(p4 - p1);
        
        /* Update global to prevent loop elimination */
        global_counter += sink & 1;
    }
    
    return sink > 0 ? 0 : 1;
}
