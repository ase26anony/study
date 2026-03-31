/* early-remat-trigger.c
 * Designed to trigger early rematerialization with virtual register spills
 * Compile with: gcc -O2 -fearly-remat -fdump-rtl-early-remat -fdump-rtl-all
 */

#include <stdint.h>
#include <stdlib.h>

/* Global variables to create rematerialization candidates */
volatile int global_counter = 0;
int global_array[256] = {0};
const long global_const = 0x123456789ABCDEF0LL;

/* Dummy function to clobber registers */
static void __attribute__((noinline)) clobber_registers(void) {
    __asm__ volatile ("" : : : "memory", "rax", "rbx", "rcx", "rdx", 
                      "rsi", "rdi", "r8", "r9", "r10", "r11", "r12", 
                      "r13", "r14", "r15");
}

/* Another noinline function to force register pressure */
static int __attribute__((noinline)) use_value(int x, long y, void *z) {
    __asm__ volatile ("# dummy asm" : : "r"(x), "r"(y), "r"(z));
    return x + (int)((intptr_t)z & 0xFF);
}

/* Main function with complex loop structure */
int main(int argc, char *argv[]) {
    /* Use arguments to create variant values */
    int base = argc > 1 ? atoi(argv[1]) : 1000;
    int iterations = argc > 2 ? atoi(argv[2]) : 1000000;
    
    /* Volatile sink to prevent optimizations */
    volatile int sink = 0;
    
    /* Outer loop to create many virtual registers */
    for (int outer = 0; outer < 1000; outer++) {
        /* Inner loop with high register pressure */
        for (int i = 0; i < iterations; i++) {
            /* Create many rematerialization candidates with different modes */
            
            /* SImode constants (expensive to materialize) */
            int const1 = 0x12345678;  /* Large constant requiring multiple insns */
            int const2 = 0x87654321;
            int const3 = 0xDEADBEEF;
            int const4 = 0xCAFEBABE;
            
            /* DImode constants */
            long const5 = 0x123456789ABCDEF0LL;
            long const6 = 0xFEDCBA9876543210LL;
            
            /* Pmode addresses (symbol addresses) */
            int *addr1 = &global_array[i & 0xFF];
            int *addr2 = &global_array[(i + 1) & 0xFF];
            int *addr3 = &global_array[(i + 2) & 0xFF];
            
            /* Constants derived from function arguments */
            int derived1 = base + 1;
            int derived2 = base << 2;
            int derived3 = base * 3;
            long derived4 = (long)base * 1000LL;
            
            /* Loop-variant derived values */
            int var1 = i + const1;
            int var2 = i * const2;
            long var3 = (long)i * const5;
            int *var4 = &global_array[i & 0x7F];
            
            /* Complex conditional to create multiple basic blocks */
            if (i & 1) {
                /* Branch 1: Use many values */
                int temp1 = const1 + const2;
                int temp2 = const3 - const4;
                long temp3 = const5 + const6;
                int *temp4 = addr1 + (derived1 >> 2);
                
                /* More computations */
                temp1 = temp1 * var1;
                temp2 = temp2 / (derived2 | 1);
                temp3 = temp3 + var3;
                
                /* Use in function call */
                sink += use_value(temp1, temp3, temp4);
                
                /* More register pressure */
                int temp5 = temp1 ^ temp2;
                long temp6 = temp3 * 2;
                int *temp7 = var4 + (temp5 & 0xF);
                
                /* Store to global to prevent elimination */
                global_array[i & 0xFF] = temp5;
                
                /* Another conditional inside */
                if (temp5 > 1000) {
                    sink += temp6;
                    clobber_registers();
                } else {
                    sink -= temp6;
                    clobber_registers();
                }
                
                /* Use all addresses */
                *addr1 = temp1;
                *addr2 = temp2;
                addr3[0] = temp5;
                
            } else {
                /* Branch 2: Different computation pattern */
                int temp1 = const2 - const1;
                int temp2 = const4 * const3;
                long temp3 = const6 / (const5 | 1);
                int *temp4 = addr2 + (derived3 >> 3);
                
                /* Different operations */
                temp1 = temp1 ^ var2;
                temp2 = temp2 & 0xFFFF;
                temp3 = temp3 - derived4;
                
                /* Function call with different args */
                sink += use_value(temp2, temp3, temp4);
                
                /* More computations */
                int temp5 = temp1 | temp2;
                long temp6 = temp3 << 2;
                int *temp7 = var4 + (temp5 & 0x7);
                
                /* Store to different global location */
                global_array[(i + 128) & 0xFF] = temp5;
                
                /* Another conditional */
                if (temp5 < 500) {
                    sink += temp6 >> 1;
                    clobber_registers();
                } else {
                    sink -= temp6 >> 1;
                    clobber_registers();
                }
                
                /* Use addresses differently */
                *addr2 = temp1;
                *addr3 = temp2;
                addr1[0] = temp5;
            }
            
            /* Third branch for more complexity */
            if (i % 3 == 0) {
                /* Mix all types and modes */
                int mix1 = const1 + i;
                long mix2 = const5 * i;
                int *mix3 = &global_array[i % 64];
                int mix4 = derived1 * derived2;
                
                /* Complex expression chain */
                for (int j = 0; j < 4; j++) {
                    mix1 = (mix1 << j) + const3;
                    mix2 = mix2 + const6 * j;
                    mix4 = mix4 ^ (derived3 + j);
                    
                    /* Small inner loop increases pressure */
                    sink += mix1 + (int)mix2 + mix4;
                }
                
                /* Function call with mixed types */
                use_value(mix1, mix2, mix3);
            }
            
            /* Force use of all derived constants */
            sink += derived1 + derived2 + derived3 + (int)derived4;
            
            /* Use all addresses to keep them live */
            sink += *addr1 + *addr2 + *addr3[0];
            
            /* Periodic clobbering */
            if (i % 100 == 0) {
                clobber_registers();
            }
        }
        
        /* Modify base to create different remat patterns */
        base = (base * 13 + 7) & 0xFFFF;
    }
    
    return sink & 0xFF;
}

/* Additional global to create more symbol references */
static volatile int extra_global[100];

/* Helper function to create more register pressure */
static void __attribute__((noinline)) 
complex_helper(int a, long b, int *c, int d, long e, int *f) {
    /* Use all parameters to force them into registers */
    __asm__ volatile ("# complex helper" 
                     : : "r"(a), "r"(b), "r"(c), "r"(d), "r"(e), "r"(f));
    
    /* Create local computations */
    int t1 = a + d;
    long t2 = b * e;
    int t3 = *c + *f;
    
    /* Store to extra global */
    extra_global[t1 % 100] = t3;
    
    /* More asm to prevent optimization */
    __asm__ volatile ("# more computation" : "+r"(t1), "+r"(t2), "+r"(t3));
}
