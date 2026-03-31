/* early-remat-test.c
 * Designed to trigger early rematerialization with virtual register spills
 * Compile with: gcc -O2 -fearly-remat -fdump-rtl-early-remat -fdump-rtl-all -fno-omit-frame-pointer -fno-schedule-insns -fno-schedule-insns2 --param=max-early-remat-iterations=5 -o test test.c
 */

#include <stdint.h>
#include <stdlib.h>

/* Global variables to create rematerialization candidates */
volatile int global_counter = 0;
int global_array[256] = {0};
const long global_const = 0x123456789ABCDEF0L;

/* Dummy function to clobber registers */
static void __attribute__((noinline)) clobber_registers(void) {
    __asm__ volatile ("" : : : "memory", "rax", "rbx", "rcx", "rdx", "rsi", "rdi", 
                      "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15");
}

/* Another noinline function to force register pressure */
static int __attribute__((noinline)) use_value(int x, long y, void *p) {
    __asm__ volatile ("# dummy asm" : : "r"(x), "r"(y), "r"(p));
    return x + (int)(y >> 32);
}

/* Main test function with high register pressure */
int main(int argc, char *argv[]) {
    /* Use arguments to create variant values */
    int base = argc > 1 ? atoi(argv[1]) : 1000;
    int iterations = argc > 2 ? atoi(argv[2]) : 1000000;
    
    /* Volatile sink to prevent optimizations */
    volatile int sink = 0;
    
    /* Outer loop to create many register pressures */
    for (int outer = 0; outer < 1000; outer++) {
        /* Inner loop with multiple basic blocks */
        for (int i = 0; i < iterations; i++) {
            /* Create many rematerialization candidates with different modes */
            
            /* SImode constants (expensive to materialize) */
            int const1 = 0x7FFFFFFF;  /* Large immediate */
            int const2 = 0x80000000;  /* Another large immediate */
            int const3 = 0x12345678;  /* Constant requiring multiple insns */
            int const4 = 0x89ABCDEF;  /* Another multi-insn constant */
            
            /* DImode constants */
            long const5 = 0x123456789ABCDEF0L;  /* 64-bit constant */
            long const6 = global_const + i;     /* Global + loop variant */
            
            /* Pmode address calculations (likely remat candidates) */
            int *ptr1 = &global_array[i & 0xFF];
            int *ptr2 = &global_array[(i + 1) & 0xFF];
            int *ptr3 = &global_array[(i * 2) & 0xFF];
            
            /* Derived constants from function arguments */
            int derived1 = base + 1;      /* param + 1 */
            int derived2 = base << 2;     /* param << 2 */
            int derived3 = base * 3;      /* param * 3 */
            int derived4 = (base << 4) | 0xF;  /* Complex expression */
            
            /* More variants with different operations */
            long derived5 = (long)base * 0x100000001L;
            int derived6 = (base & 0xFFF) | 0x1000;
            int derived7 = ~base + i;
            
            /* Create conditional branches to form multiple basic blocks */
            if (i & 1) {
                /* Branch 1: Use some values */
                int temp1 = const1 + const2;
                int temp2 = derived1 * derived2;
                long temp3 = const5 + const6;
                
                /* Use in arithmetic */
                temp1 = temp1 * 2 + (int)(temp3 >> 32);
                temp2 = temp2 - derived3;
                
                /* Store to prevent elimination */
                sink = temp1 + temp2;
                
                /* Use pointers */
                *ptr1 = temp1;
                *ptr2 = temp2;
                
                /* Function call clobbers registers */
                clobber_registers();
                
                /* More computations after clobber */
                int temp4 = const3 + const4;
                long temp5 = (long)derived4 * derived5;
                temp4 = temp4 ^ derived6;
                
                sink = temp4 + (int)temp5;
                *ptr3 = temp4;
            } else {
                /* Branch 2: Different computation pattern */
                int temp6 = const2 - const1;
                int temp7 = derived3 / (derived2 + 1);
                long temp8 = const6 - const5;
                
                /* Different operations */
                temp6 = temp6 << (i & 3);
                temp7 = temp7 | derived7;
                
                sink = temp6 - temp7;
                
                /* Different pointer usage */
                global_array[(i + 3) & 0xFF] = temp6;
                global_array[(i + 4) & 0xFF] = temp7;
                
                /* Another register-clobbering call */
                clobber_registers();
                
                /* Recompute some values (forces remat) */
                int temp9 = const4 - const3;
                long temp10 = (long)derived5 >> (derived6 & 0x1F);
                temp9 = temp9 & derived4;
                
                sink = temp9 ^ (int)temp10;
                global_counter = temp9;
            }
            
            /* Third basic block (always executed) */
            if (i & 2) {
                /* Mix all types of values */
                int temp11 = const1 + derived1 + (int)(const5 >> 32);
                long temp12 = (long)const2 * derived5;
                int *temp13 = ptr1 + (derived2 >> 2);
                
                /* Complex expression with many operands */
                int result = temp11 + (int)temp12 + *temp13 + derived3 + derived6;
                
                /* Use function that takes multiple arguments */
                result = use_value(result, temp12, temp13);
                
                sink = result;
                
                /* Another clobber */
                clobber_registers();
                
                /* Force reuse of constants */
                int temp14 = const3 + const4 + derived4;
                long temp15 = const5 + const6 + derived5;
                temp14 = temp14 * 2 - (int)(temp15 & 0xFFFFFFFF);
                
                global_array[i & 0xFF] = temp14;
            } else {
                /* Alternative computation path */
                long temp16 = (long)const5 * const6;
                int temp17 = derived1 + derived2 + derived3 + derived4;
                int *temp18 = &global_array[(i * 3) & 0xFF];
                
                int result2 = (int)(temp16 >> 16) + temp17 + *temp18;
                result2 = use_value(result2, temp16, temp18);
                
                sink = result2;
                
                clobber_registers();
                
                /* More constant reuse */
                int temp19 = const1 - const2 + const3 - const4;
                long temp20 = const6 - const5 + derived5;
                temp19 = temp19 ^ (int)temp20;
                
                global_counter = temp19;
            }
            
            /* Final computations using all live values */
            int final1 = const1 + derived1;
            int final2 = const2 + derived2;
            long final3 = const5 + derived5;
            int final4 = const3 + derived3;
            int final5 = const4 + derived4;
            
            /* Force all to be live simultaneously */
            sink = final1 + final2 + (int)final3 + final4 + final5 + 
                   (int)((long)ptr1 - (long)ptr2) + derived6 + derived7;
            
            /* One more clobber to increase pressure */
            clobber_registers();
        }
    }
    
    return sink;
}
