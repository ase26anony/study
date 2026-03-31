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
    __asm__ volatile ("" : "+r" (x));
    return x;
}

/* Main function with complex loop structure */
int main(int argc, char *argv[]) {
    /* Use arguments to create variant values */
    int base = argc > 1 ? atoi(argv[1]) : 1000;
    int offset = argc > 2 ? atoi(argv[2]) : 42;
    
    /* Volatile sink to prevent optimization */
    volatile int sink = 0;
    
    /* Outer loop - creates many iterations */
    for (int outer = 0; outer < 1000; outer++) {
        /* Multiple basic blocks with different conditions */
        if (outer & 1) {
            /* Block A: Many rematerializable constants */
            
            /* Small integer constants (remat candidates) */
            int c1 = 0x7FFFFFFF;  /* Large immediate that might need multiple insns */
            int c2 = -123456;     /* Another constant */
            long c3 = 0x8000000000000000LL;  /* Large 64-bit constant */
            
            /* Symbol addresses (remat candidates) */
            int *addr1 = &global_array[outer & 0xFF];
            const long *addr2 = &global_const;
            
            /* Constants derived from function arguments */
            int derived1 = base + 1;          /* param + 1 */
            int derived2 = base << 2;         /* param << 2 */
            int derived3 = (base * 3) / 2;    /* More complex derivation */
            
            /* Use them in calculations to create dependencies */
            int val1 = c1 + derived1;
            int val2 = c2 * derived2;
            long val3 = c3 | (long)derived3;
            int *val4 = addr1 + (derived1 & 0xF);
            
            /* Force register pressure with many live values */
            int temp1 = val1 * 2;
            int temp2 = val2 / 3;
            long temp3 = val3 >> 4;
            int temp4 = *val4 + temp1;
            int temp5 = temp2 - (int)(temp3 & 0xFFFF);
            
            /* Conditional branch creating another basic block */
            if (temp4 > temp5) {
                /* Use different data types */
                uintptr_t ptr_val = (uintptr_t)addr2;
                int ptr_low = (int)(ptr_val & 0xFFFFFFFF);
                int ptr_high = (int)(ptr_val >> 32);
                
                temp5 = ptr_low + ptr_high + temp4;
            } else {
                /* Alternative computation path */
                long double_cast = (long)temp4 * (long)temp5;
                temp5 = (int)(double_cast & 0x7FFFFFFF);
            }
            
            /* Clobber registers between uses */
            clobber_registers();
            
            /* Use values to prevent elimination */
            sink += use_value(temp1) + use_value(temp2) + temp5;
            
        } else {
            /* Block B: Different set of rematerializable values */
            
            /* More constants with different modes */
            short c4 = -32768;        /* HImode */
            char c5 = 127;            /* QImode */
            int64_t c6 = 0xFFFFFFFF00000000LL;  /* DImode */
            
            /* Pointer arithmetic with different scales */
            int *addr3 = global_array + (offset * 2);
            int *addr4 = &global_array[(outer * 3) & 0xFF];
            
            /* Complex derived constants */
            int derived4 = (base << 3) | 0xF;
            int derived5 = (base + offset) * 7;
            int derived6 = ~base;
            
            /* Create web of dependencies */
            int val5 = c4 * derived4;
            int val6 = c5 + derived5;
            int64_t val7 = c6 ^ (int64_t)derived6;
            int val8 = addr3[outer & 0x7] - addr4[0];
            
            /* More intermediate values */
            int temp6 = val5 | val6;
            int temp7 = (int)(val7 & 0xFFFFFFFF);
            int temp8 = val8 * 3;
            int temp9 = temp6 + temp7;
            int temp10 = temp8 - temp9;
            
            /* Nested conditionals for more blocks */
            if (temp10 > 0) {
                /* Pointer-intensive block */
                int *p1 = addr3 + (temp6 & 0x3);
                int *p2 = addr4 + (temp7 & 0x3);
                temp10 = *p1 + *p2;
            } else if (temp10 < -1000) {
                /* 64-bit operations */
                int64_t big_val = (int64_t)temp6 * (int64_t)temp7;
                temp10 = (int)(big_val >> 16);
            } else {
                /* Mixed operations */
                temp10 = (temp6 << 4) | (temp7 & 0xF);
            }
            
            /* Another register clobber */
            clobber_registers();
            
            /* Force all values to be used */
            sink += temp6 + temp7 + temp8 + temp9 + temp10;
        }
        
        /* Common block with more computations */
        {
            /* Additional rematerializable constants */
            int c7 = 0x55555555;      /* Bit pattern constant */
            int c8 = 0xAAAAAAAA;
            int c9 = 0x12345678;
            
            /* More derived values */
            int derived7 = base * base;
            int derived8 = (base << 5) - (offset << 3);
            int derived9 = ~derived7;
            
            /* Final computations mixing everything */
            int final1 = c7 & derived7;
            int final2 = c8 | derived8;
            int final3 = c9 ^ derived9;
            
            /* Use in volatile operations */
            __asm__ volatile ("# Force use %0" : : "r" (final1));
            __asm__ volatile ("# Force use %0" : : "r" (final2));
            __asm__ volatile ("# Force use %0" : : "r" (final3));
            
            /* Update volatile sink */
            sink += final1 + final2 + final3;
        }
        
        /* Loop-variant update to prevent optimization */
        global_counter += outer;
    }
    
    return sink;
}

/* Additional function to create more compilation unit complexity */
void __attribute__((noinline)) extra_pressure(int iterations) {
    volatile int local_sink = 0;
    
    for (int i = 0; i < iterations; i++) {
        /* Many short-lived temporaries */
        int t1 = i * 0x11111111;
        int t2 = i + 0x22222222;
        int t3 = t1 ^ t2;
        int t4 = t3 << 3;
        int t5 = t4 >> 1;
        int t6 = t5 | 0x33333333;
        int t7 = t6 & 0x44444444;
        int t8 = t7 + t6;
        int t9 = t8 - t5;
        int t10 = t9 * 2;
        
        local_sink += t10;
        
        /* Frequent clobbering */
        __asm__ volatile ("" : : : "rax", "rbx", "rcx", "rdx");
    }
}
