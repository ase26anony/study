/* early-remat-test.c
 * Designed to trigger early rematerialization with virtual register spills
 * Compile with: gcc -O2 -fearly-remat -fdump-rtl-early-remat -fdump-rtl-all -fno-omit-frame-pointer -fno-schedule-insns -fno-schedule-insns2 -o early-remat-test early-remat-test.c
 */

#include <stdio.h>
#include <stdlib.h>

/* Global variables to create rematerialization candidates */
volatile int global_base = 100;
volatile long global_offset = 1000;
int global_array[256] = {0};
static const int static_const_array[] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};

/* Dummy function to clobber registers */
static void __attribute__((noinline)) clobber_registers(void) {
    /* Inline asm to clobber many registers */
    __asm__ volatile (
        "nop\n\t"
        "nop\n\t"
        "nop\n\t"
        : : : "memory", "rax", "rbx", "rcx", "rdx", "rsi", "rdi", 
              "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15"
    );
}

/* Another noinline function to force spills */
static int __attribute__((noinline)) compute_offset(int x, int y) {
    return (x << 2) + (y >> 1);
}

/* Main test function with high register pressure */
void __attribute__((noinline)) test_early_remat(int param1, int param2, int iterations) {
    volatile int sink = 0;  /* Prevent optimizations */
    int i, j;
    
    /* Outer loop to create many live ranges */
    for (i = 0; i < iterations; i++) {
        /* Create many short-lived intermediate values with different modes */
        
        /* Integer constants (SImode candidates) */
        int const1 = 0x7FFFFFFF;  /* Large constant requiring multiple insns */
        int const2 = -0x80000000;
        int const3 = 0x12345678;
        int const4 = 0xABCDEF01;
        
        /* Derived constants from parameters (remat candidates) */
        int derived1 = param1 + 1;          /* param + constant */
        int derived2 = param2 << 2;         /* param shifted */
        int derived3 = param1 * 3;          /* param multiplied */
        int derived4 = (param1 & 0xFF) | 0x100;
        
        /* Long constants (DImode candidates on 64-bit) */
        long long_const1 = 0x7FFFFFFFFFFFFFFFLL;
        long long_const2 = global_offset + i;
        long long_const3 = (long)param1 * 1000LL;
        
        /* Pointer constants (Pmode candidates) */
        int *ptr1 = &global_array[i & 0xFF];
        int *ptr2 = &global_array[(i + 1) & 0xFF];
        const int *ptr3 = &static_const_array[i % 10];
        
        /* Complex expressions that are rematerialization candidates */
        int expr1 = (param1 * 17) + 42;
        int expr2 = (param2 << 3) | 0xF;
        int expr3 = (i * 13) % 256;
        int expr4 = (param1 ^ param2) + global_base;
        
        /* Use in conditional branches to create multiple basic blocks */
        if (i & 1) {
            /* Branch 1 - use some values */
            int temp1 = derived1 + const1;
            int temp2 = derived2 * const2;
            long temp3 = long_const1 - long_const2;
            
            /* Force register pressure with many operations */
            temp1 = temp1 + expr1;
            temp2 = temp2 ^ expr2;
            temp3 = temp3 + (long)expr3;
            
            /* Use pointers */
            *ptr1 = temp1;
            sink = temp2;
            
            /* Call to clobber registers */
            clobber_registers();
            
            /* More computations after clobber */
            int temp4 = compute_offset(temp1, temp2);
            long temp5 = temp3 + (long)temp4;
            
            sink = (int)temp5;
        } else {
            /* Branch 2 - use different values */
            int temp1 = derived3 + const3;
            int temp2 = derived4 * const4;
            long temp3 = long_const3 + (long)const1;
            
            /* Different operations */
            temp1 = temp1 - expr3;
            temp2 = temp2 | expr4;
            temp3 = temp3 - (long)expr1;
            
            /* Use different pointer */
            *ptr2 = temp1;
            sink = temp2;
            
            /* Call to clobber registers */
            clobber_registers();
            
            /* More computations */
            int temp4 = compute_offset(temp2, temp1);
            long temp5 = (long)temp4 * temp3;
            
            sink = (int)temp5;
        }
        
        /* Third basic block (always executed) */
        if (i & 2) {
            /* Mix all types and modes */
            int mix1 = expr1 + expr2;
            long mix2 = (long)expr3 * long_const1;
            int *mix3 = ptr1 + (expr4 & 0xF);
            
            /* Complex expression chain */
            for (j = 0; j < 4; j++) {
                mix1 = (mix1 << 1) + j;
                mix2 = mix2 - (long)(derived1 + j);
                *mix3 = mix1 + (int)mix2;
                
                /* More register pressure */
                int tmp = compute_offset(mix1, (int)mix2);
                sink = tmp;
            }
            
            clobber_registers();
        }
        
        /* Fourth basic block with more remat candidates */
        {
            /* Create new constants that might be rematerialized */
            int new_const1 = 0x55555555;
            int new_const2 = 0xAAAAAAAA;
            long new_long1 = 0x3333333333333333LL;
            
            /* Use them in computations */
            int result1 = new_const1 & derived1;
            int result2 = new_const2 | derived2;
            long result3 = new_long1 ^ long_const3;
            
            /* Chain computations to extend live ranges */
            result1 = result1 + (param1 << 1);
            result2 = result2 - (param2 >> 1);
            result3 = result3 + (long)(param1 * param2);
            
            /* Force spills with many simultaneous live values */
            sink = result1 + result2 + (int)result3;
            
            /* Address computation that's a remat candidate */
            int *addr1 = &global_array[(result1 & 0xFF)];
            int *addr2 = addr1 + (result2 & 0x7);
            *addr2 = result1 ^ result2;
        }
        
        /* Final computation block */
        {
            /* One more set of rematerializable values */
            int final1 = global_base + i;
            int final2 = (param1 * i) + (param2 << (i & 3));
            long final3 = (long)final1 * final2;
            
            /* Use in volatile asm to prevent optimization */
            __asm__ volatile (
                "addl %1, %0\n\t"
                : "+r" (sink)
                : "r" (final1)
                : "cc"
            );
            
            sink += (int)final3;
        }
    }
}

int main(int argc, char **argv) {
    int param1, param2, iterations;
    
    /* Use command line arguments to create variant values */
    if (argc >= 3) {
        param1 = atoi(argv[1]);
        param2 = atoi(argv[2]);
        iterations = (argc >= 4) ? atoi(argv[3]) : 1000;
    } else {
        param1 = 12345;
        param2 = 67890;
        iterations = 1000;
    }
    
    /* Initialize global array */
    for (int i = 0; i < 256; i++) {
        global_array[i] = i;
    }
    
    /* Run the test */
    test_early_remat(param1, param2, iterations);
    
    printf("Test completed with params: %d, %d, iterations: %d\n", 
           param1, param2, iterations);
    printf("Final sink value (ignored): %d\n", global_array[0]);
    
    return 0;
}
