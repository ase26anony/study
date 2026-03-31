/* early-remat-test.c
 * Designed to trigger early rematerialization with virtual register spills
 * Compile with: gcc -O2 -fearly-remat -fdump-rtl-early-remat -fdump-rtl-all -fno-omit-frame-pointer -fno-schedule-insns -fno-schedule-insns2 --param=max-early-remat-iterations=5
 */

#include <stdint.h>
#include <stdlib.h>

/* Global variables to create rematerialization candidates */
volatile int global_counter = 0;
int global_array[256] = {0};
const long global_const = 0x7FFFFFFFFFFFFFFF;
static volatile int sink;

/* Dummy function to clobber registers */
static void __attribute__((noinline)) clobber_registers(void) {
    __asm__ volatile ("" : : : "memory", "rax", "rbx", "rcx", "rdx", "rsi", "rdi", 
                      "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15");
}

/* Another dummy function to prevent optimization */
static int __attribute__((noinline)) use_value(int val) {
    __asm__ volatile ("" : "+r" (val));
    return val;
}

/* Main function with complex loop structure */
int main(int argc, char *argv[]) {
    /* Use arguments to create variant values */
    int base = argc > 1 ? atoi(argv[1]) : 1000;
    int offset = argc > 2 ? atoi(argv[2]) : 17;
    
    /* Force many simultaneously live values */
    volatile int result = 0;
    
    /* Outer loop to create pressure */
    for (int outer = 0; outer < 1000; outer++) {
        /* Many basic blocks with different conditions */
        if (outer & 1) {
            /* Block 1: Create many integer constants (remat candidates) */
            int c1 = 0x12345678;          /* Large constant requiring multiple insns */
            int c2 = -1;                  /* All-ones constant */
            int c3 = 0x80000000;          /* High-bit set constant */
            long c4 = global_const;       /* Global constant load */
            int c5 = base + 1;            /* Derived from argument */
            int c6 = base << 2;           /* Shifted argument */
            int c7 = base * 3;            /* Multiplied argument */
            
            /* Use them in computations */
            int t1 = c1 + c2;
            int t2 = c3 - c5;
            long t3 = c4 / (c6 + 1);
            int t4 = c7 | c1;
            
            /* Force register pressure by using all values */
            sink = t1 + t2 + t3 + t4;
            
            /* Address computations (pointer remat candidates) */
            int *ptr1 = &global_array[base % 256];
            int *ptr2 = &global_array[offset % 256];
            int *ptr3 = &global_array[(base + offset) % 256];
            
            /* Use pointers */
            *ptr1 = t1;
            *ptr2 = t2;
            ptr3[0] = t3;
            
            /* Clobber registers between uses */
            clobber_registers();
            
            /* More computations with same values */
            t1 = c1 * c5;
            t2 = c2 & c6;
            t3 = c4 ^ c7;
            t4 = c3 >> 3;
            
            result += t1 + t2 + t3 + t4;
        } else {
            /* Block 2: Different set of constants and operations */
            int d1 = 0x55555555;          /* Alternating bits */
            int d2 = 0xAAAAAAAA;          /* Inverse pattern */
            long d3 = 0xFFFFFFFF00000000;
            int d4 = base - offset;
            int d5 = offset * 5;
            int d6 = base / 2;
            
            /* Mixed-type operations */
            long e1 = d1 * (long)d4;
            int e2 = d2 + d5;
            long e3 = d3 | (long)d6;
            int e4 = d4 ^ d5;
            
            /* More address computations */
            char *cptr1 = (char*)&global_array[d4 % 256];
            char *cptr2 = (char*)&global_array[d5 % 256];
            
            cptr1[0] = e1;
            cptr2[1] = e2;
            
            clobber_registers();
            
            /* Reuse constants in different ways */
            e1 = d1 << d6;
            e2 = d2 >> (d4 & 3);
            e3 = d3 + (long)d5;
            e4 = d4 * d6;
            
            result += e1 + e2 + e3 + e4;
        }
        
        /* Third block (always executed) */
        {
            /* More constants and derived values */
            int f1 = 0xDEADBEEF;
            int f2 = 0xCAFEBABE;
            int f3 = base + outer;
            int f4 = offset - outer;
            long f5 = (long)global_array;  /* Global address */
            int f6 = f3 * f4;
            
            /* Complex expression tree */
            int g1 = (f1 & f2) | (f3 ^ f4);
            long g2 = f5 + (f6 * 8);
            int g3 = (f1 >> 4) + (f2 << 4);
            int g4 = f3 % (f4 ? f4 : 1);
            
            /* Use inline asm to prevent optimization */
            __asm__ volatile ("# dummy asm" : : "r"(g1), "r"(g2), "r"(g3), "r"(g4));
            
            /* Conditional that creates another basic block */
            if (g1 > g3) {
                /* Nested block with more values */
                int h1 = g1 - g3;
                long h2 = g2 / 2;
                int h3 = g4 * 3;
                
                result += h1 + h2 + h3;
                
                /* Another function call to clobber */
                clobber_registers();
                
                /* Re-materialize constants */
                h1 = f1 + f3;
                h2 = f5 - (long)f6;
                h3 = f2 & f4;
                
                sink = h1 + h2 + h3;
            } else {
                /* Alternative path */
                int h1 = g3 - g1;
                long h2 = g2 * 2;
                int h3 = g4 / 3;
                
                result += h1 + h2 + h3;
                
                clobber_registers();
                
                /* Different constant usage */
                h1 = f1 | f3;
                h2 = f5 ^ (long)f6;
                h3 = f2 + f4;
                
                sink = h1 + h2 + h3;
            }
        }
        
        /* Fourth block with switch-like structure */
        switch (outer & 3) {
            case 0: {
                int k1 = 0x11111111;
                int k2 = base * base;
                long k3 = (long)&global_counter;
                result += k1 + k2 + k3;
                break;
            }
            case 1: {
                int k1 = 0x22222222;
                int k2 = offset * offset;
                long k3 = (long)&global_array[0];
                result += k1 + k2 + k3;
                break;
            }
            case 2: {
                int k1 = 0x33333333;
                int k2 = (base + offset) * (base - offset);
                long k3 = (long)&sink;
                result += k1 + k2 + k3;
                break;
            }
            default: {
                int k1 = 0x44444444;
                int k2 = base % offset;
                long k3 = (long)main;  /* Function address */
                result += k1 + k2 + k3;
                break;
            }
        }
        
        /* Final computations using all types of values */
        {
            /* Force many live ranges to overlap */
            int final1 = use_value(result);
            int final2 = use_value(base);
            int final3 = use_value(offset);
            long final4 = use_value(outer);
            
            /* Complex expression forcing register pressure */
            result = final1 + final2 + final3 + final4 + 
                    ((final1 * final2) >> (final3 & 7)) +
                    ((final3 * final4) & 0xFF);
        }
    }
    
    /* Prevent elimination of entire computation */
    global_counter = result;
    
    return result != 0;
}
