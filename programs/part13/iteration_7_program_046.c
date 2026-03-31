/* early-remat-trigger.c
 * Compile with: gcc -O2 -fearly-remat -fdump-rtl-early-remat -fdump-rtl-all -fno-omit-frame-pointer -fno-schedule-insns -fno-schedule-insns2 --param=max-early-remat-iterations=5 -o early-remat-test early-remat-trigger.c
 */

#include <stdint.h>
#include <stdlib.h>

/* Global variables to create symbol address remat candidates */
volatile int global_array[256] = {0};
volatile long global_long = 42;
static volatile double static_double = 3.14159;

/* Dummy function to clobber registers */
static void __attribute__((noinline)) clobber_registers(void) {
    __asm__ volatile ("" : : : "memory", "rax", "rbx", "rcx", "rdx", "rsi", "rdi", 
                      "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15");
}

/* Another dummy function with arguments */
static int __attribute__((noinline, noclone)) 
dummy_compute(int a, long b, void *c) {
    __asm__ volatile ("# dummy compute" : : "r"(a), "r"(b), "r"(c));
    return 0;
}

/* Main function designed to create high register pressure */
int main(int argc, char *argv[]) {
    volatile int sink = 0;
    int loop_count = (argc > 1) ? atoi(argv[1]) : 1000;
    int base_val = (argc > 2) ? atoi(argv[2]) : 12345;
    
    /* Outer loop to increase pressure */
    for (int outer = 0; outer < 10; outer++) {
        /* Inner hot loop with many basic blocks */
        for (int i = 0; i < loop_count; i++) {
            /* --- Basic Block 1: Create many remat candidates --- */
            /* Small integer constants (expensive to materialize) */
            int const1 = 0x7FFFFFFF;  /* Large constant */
            int const2 = 0x80000000;  /* Another large constant */
            long const3 = 0x123456789ABCDEFLL; /* 64-bit constant */
            
            /* Constants derived from function arguments */
            int derived1 = base_val + 1;
            int derived2 = base_val << 2;
            int derived3 = (base_val * 3) / 2;
            long derived4 = (long)base_val * 1000L;
            
            /* Symbol addresses (good remat candidates) */
            int *addr1 = &global_array[i & 0xFF];
            long *addr2 = &global_long;
            double *addr3 = &static_double;
            
            /* Complex expressions with loop variant */
            int expr1 = i * 7 + 3;
            int expr2 = (i << 4) | 0xF;
            long expr3 = (long)i * 0x100000001LL;
            
            /* Pointer arithmetic */
            int *ptr1 = &global_array[(i + 1) & 0xFF];
            int *ptr2 = &global_array[(i + 2) & 0xFF];
            int *ptr3 = &global_array[(i + 3) & 0xFF];
            
            /* --- Basic Block 2: Conditional branch --- */
            if (i & 1) {
                /* Use some values in this branch */
                int temp1 = derived1 + expr1;
                int temp2 = derived2 - expr2;
                long temp3 = derived4 + expr3;
                
                /* More computations */
                temp1 = temp1 * const1;
                temp2 = temp2 / 2;
                temp3 = temp3 >> 4;
                
                /* Store to prevent elimination */
                *addr1 = temp1;
                global_long = temp3;
                
                /* Function call clobbers registers */
                clobber_registers();
                
                /* More computations after clobber */
                int temp4 = temp2 + *addr1;
                long temp5 = temp3 + global_long;
                
                sink += temp4;
                sink += (int)temp5;
            } else {
                /* Alternative branch with different computations */
                int temp1 = const2 - expr1;
                long temp2 = const3 ^ expr3;
                int temp3 = derived3 * expr2;
                
                /* Pointer operations */
                int val1 = *ptr1;
                int val2 = *ptr2;
                int val3 = *ptr3;
                
                temp1 = temp1 + val1;
                temp2 = temp2 - val2;
                temp3 = temp3 * val3;
                
                /* Another function call */
                dummy_compute(temp1, temp2, ptr3);
                
                /* More computations mixing types */
                double dtemp = *addr3 + (double)temp1;
                long ltemp = (long)dtemp + temp2;
                int itemp = (int)ltemp + temp3;
                
                sink += itemp;
            }
            
            /* --- Basic Block 3: More computations --- */
            /* Create more intermediate values */
            int extra1 = (derived1 << 3) + 0xABCD;
            int extra2 = (derived2 >> 1) - 0x1234;
            long extra3 = derived4 * 0x987654321LL;
            
            /* Use all address calculations */
            int sum1 = *addr1 + *ptr1;
            int sum2 = *ptr2 + *ptr3;
            long sum3 = extra3 + global_long;
            
            /* Complex expression with many operands */
            int final1 = extra1 + extra2 + sum1 + sum2 + (int)sum3;
            
            /* Another conditional */
            if (i & 2) {
                int final2 = final1 * 3;
                long final3 = (long)final2 * 0x10001;
                double final4 = (double)final3 * 2.71828;
                
                sink += (int)final4;
                *addr2 = final3;
            } else {
                int final2 = final1 / 5;
                long final3 = (long)final2 * 0x20002;
                
                /* Force spill/reload with volatile */
                volatile int vtemp = final2;
                volatile long vltemp = final3;
                
                sink += vtemp + (int)vltemp;
            }
            
            /* Final function call to clobber everything */
            clobber_registers();
        }
    }
    
    /* Use sink to prevent dead code elimination */
    return sink & 0xFF;
}

/* Additional global to increase symbol address variety */
volatile uint64_t global_64bit_array[128] = {0};

/* Helper function to create more register pressure */
static void __attribute__((noinline)) 
create_pressure(int iterations) {
    volatile int local_sink = 0;
    
    for (int i = 0; i < iterations; i++) {
        /* Many constants with different modes */
        int8_t c1 = 127;
        int16_t c2 = -32768;
        int32_t c3 = 0xDEADBEEF;
        int64_t c4 = 0xCAFEBABEDEADBEEFLL;
        
        /* Address computations */
        uint64_t *addr4 = &global_64bit_array[i & 0x7F];
        
        /* Mixed operations */
        int32_t t1 = c3 + (int32_t)c4;
        int64_t t2 = (int64_t)c2 * c4;
        int16_t t3 = c1 * c2;
        
        /* Use in complex expression */
        int64_t result = t2 + (int64_t)t1 * (int64_t)t3;
        
        /* Store to global */
        *addr4 = result;
        local_sink += (int)result;
        
        /* Periodic clobber */
        if (i % 8 == 0) {
            clobber_registers();
        }
    }
    
    /* Call from main to increase complexity */
    if (loop_count > 500) {
        create_pressure(loop_count / 10);
    }
}
