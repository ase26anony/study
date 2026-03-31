/* early-remat-test.c
 * Designed to trigger early rematerialization with virtual register spills
 * Targets uncovered lines 930-937 in early-remat.cc
 */

#include <stdio.h>
#include <stdlib.h>

/* Global variables to create rematerialization candidates */
volatile int global_counter = 0;
int global_array[256] = {0};
static const long global_const = 0x123456789ABCDEF0L;

/* Dummy function to clobber registers */
static void __attribute__((noinline)) clobber_registers(void) {
    __asm__ volatile ("" : : : "memory", "rax", "rbx", "rcx", "rdx", 
                      "rsi", "rdi", "r8", "r9", "r10", "r11", "r12", 
                      "r13", "r14", "r15");
}

/* Another noinline function to force register pressure */
static int __attribute__((noinline)) use_value(int x, long y, void *z) {
    __asm__ volatile ("# dummy asm" : : "r"(x), "r"(y), "r"(z));
    return x + (int)((long)z & 0xFF);
}

int main(int argc, char *argv[]) {
    /* Use arguments to create variant values */
    int base = argc > 1 ? atoi(argv[1]) : 1000;
    int iterations = argc > 2 ? atoi(argv[2]) : 100000;
    
    /* Volatile sink to prevent optimizations */
    volatile int sink = 0;
    
    /* Outer loop to create many intermediate values */
    for (int outer = 0; outer < 1000; outer++) {
        /* Inner computation with many basic blocks */
        for (int i = 0; i < iterations; i++) {
            /* --- Basic Block 1: Create many remat candidates --- */
            
            /* Small integer constants (require materialization) */
            int c1 = 0x7FFFFFFF;  /* Large immediate */
            int c2 = 0x12345678;  /* Another large constant */
            long c3 = 0xFEDCBA9876543210L;  /* 64-bit constant */
            
            /* Constants derived from function arguments */
            int derived1 = base + 1;      /* param + 1 */
            int derived2 = base << 2;     /* param << 2 */
            int derived3 = base * 3 + 7;  /* More complex */
            
            /* Symbol addresses (good remat candidates) */
            int *addr1 = &global_array[i & 0xFF];
            int *addr2 = &global_array[(i + 1) & 0xFF];
            long *addr3 = (long *)&global_array[(i + 2) & 0xFF];
            
            /* Loop-variant values */
            int variant1 = i * 2;
            int variant2 = i + outer;
            long variant3 = (long)i * (long)outer;
            
            /* --- Basic Block 2: Conditional branch --- */
            if (i & 1) {
                /* Use all values in this path */
                int temp1 = c1 + derived1;
                int temp2 = c2 + derived2;
                long temp3 = c3 + variant3;
                
                /* Pointer arithmetic */
                int *ptr1 = addr1 + (derived1 >> 2);
                int *ptr2 = addr2 + (derived2 >> 2);
                
                /* More computations */
                int sum1 = temp1 + *ptr1;
                int sum2 = temp2 + *ptr2;
                long sum3 = temp3 + (long)ptr1;
                
                /* Store to prevent elimination */
                sink += sum1 + sum2 + (int)sum3;
                
                /* Function call clobbers registers */
                clobber_registers();
                
                /* More values after call (must be rematerialized) */
                int revived1 = base + 3;  /* Must be remat after call */
                long revived2 = global_const + i;
                int *revived_addr = &global_array[(i + 3) & 0xFF];
                
                /* Use revived values */
                sink += revived1 + (int)revived2 + *revived_addr;
            } else {
                /* Alternative path with different computations */
                
                /* Different constants */
                int alt_c1 = 0x55555555;
                int alt_c2 = 0xAAAAAAAA;
                long alt_c3 = 0x3333333333333333L;
                
                /* Different derived values */
                int alt_derived1 = base - 1;
                int alt_derived2 = base >> 1;
                int alt_derived3 = base * 5 - 3;
                
                /* Complex expression chain */
                int chain1 = alt_c1 + alt_derived1;
                int chain2 = chain1 * alt_derived2;
                int chain3 = chain2 - alt_derived3;
                int chain4 = chain3 + variant1;
                int chain5 = chain4 * variant2;
                
                /* Pointer chain */
                int *chain_ptr = addr3 + (chain5 & 0xF);
                long chain_val = (long)chain_ptr + alt_c3;
                
                /* Store result */
                sink += chain5 + (int)chain_val;
                
                /* Another function call */
                use_value(chain5, chain_val, chain_ptr);
                
                /* More values after call */
                int post1 = base * 7 + 11;
                long post2 = global_const - i;
                int *post_addr = &global_array[(i + 4) & 0xFF];
                
                /* Final computation */
                sink += post1 + (int)post2 + *post_addr;
            }
            
            /* --- Basic Block 3: Another conditional --- */
            if (i & 2) {
                /* Mix data types */
                short s1 = base & 0xFFFF;
                short s2 = (base >> 16) & 0xFFFF;
                char c = (base >> 8) & 0xFF;
                
                /* Various operations */
                int mixed1 = s1 * s2;
                int mixed2 = mixed1 + c;
                long mixed3 = (long)mixed2 * variant3;
                
                sink += (int)mixed3;
            } else {
                /* More pointer arithmetic */
                int offset = (i * 3) & 0x3F;
                int *p1 = &global_array[offset];
                int *p2 = p1 + (base & 0xF);
                int *p3 = p2 + ((base >> 4) & 0xF);
                
                /* Chain of dependencies */
                int val1 = *p1 + derived1;
                int val2 = *p2 + derived2;
                int val3 = *p3 + derived3;
                int val4 = val1 + val2 + val3;
                
                sink += val4;
            }
            
            /* --- Basic Block 4: Final computations --- */
            
            /* Create more intermediate values */
            int final1 = sink & 0xFF;
            int final2 = (sink >> 8) & 0xFF;
            int final3 = (sink >> 16) & 0xFF;
            int final4 = (sink >> 24) & 0xFF;
            
            /* Use all final values */
            int combined = final1 + final2 + final3 + final4;
            
            /* Global store to create memory pressure */
            global_array[i & 0xFF] = combined;
            
            /* Periodic register clobbering */
            if ((i & 0x3F) == 0) {
                clobber_registers();
            }
        }
        
        /* Update global counter */
        global_counter += outer;
    }
    
    /* Prevent dead code elimination */
    printf("Result: %d (sink=%d)\n", global_array[0], sink);
    
    return 0;
}
