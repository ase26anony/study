/* early-remat-trigger.c
 * Designed to trigger early rematerialization with virtual register spills
 * Compile with: gcc -O2 -fearly-remat -fdump-rtl-early-remat -fdump-rtl-all -fno-omit-frame-pointer -fno-schedule-insns -fno-schedule-insns2 --param=max-early-remat-iterations=5 -o early-remat-test early-remat-trigger.c
 */

#include <stdio.h>
#include <stdlib.h>

/* Global variables to create rematerialization candidates */
volatile int global_counter = 0;
int global_array[256] = {0};
const long global_const = 0x123456789ABCDEF0LL;

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

/* Another noinline function to force register pressure */
static int __attribute__((noinline)) use_value(int x, long y, void *z) {
    volatile int sink = 0;
    sink = x + (int)(y >> 32) + (int)((long)z & 0xFFFF);
    return sink;
}

int main(int argc, char *argv[]) {
    int iterations = 1000;
    if (argc > 1) {
        iterations = atoi(argv[1]);
        if (iterations <= 0) iterations = 1000;
    }
    
    int base_param = (argc > 2) ? atoi(argv[2]) : 42;
    
    /* Volatile sink to prevent optimization */
    volatile int result_sink = 0;
    volatile long long_result_sink = 0;
    
    /* Outer loop to create many register pressures */
    for (int outer = 0; outer < 10; outer++) {
        /* Inner hot loop with many basic blocks */
        for (int i = 0; i < iterations; i++) {
            /* --- Basic Block 1: Create many remat candidates --- */
            /* Small integer constants (expensive to materialize) */
            const int const1 = 0x7FFFFFFF;  /* Large constant */
            const int const2 = 0x80000000;  /* Another large constant */
            const long const3 = 0x12345678; /* Different mode */
            
            /* Constants derived from function arguments */
            int derived1 = base_param + 1;          /* param + 1 */
            int derived2 = base_param << 2;         /* param << 2 */
            int derived3 = (base_param * 3) / 2;    /* More complex */
            long derived4 = (long)base_param * 1000L;
            
            /* Symbol addresses as remat candidates */
            int *addr1 = &global_array[i & 0xFF];
            int *addr2 = &global_array[(i + 1) & 0xFF];
            int *addr3 = &global_array[(i + base_param) & 0xFF];
            
            /* Constants with different modes */
            short short_const = 0x7FFF;
            char char_const = 0x7F;
            long long ll_const = 0x123456789ABCDEF0LL;
            
            /* --- Basic Block 2: Conditional branch --- */
            if (i & 1) {
                /* Use some values in this branch */
                derived1 = derived1 + const1;
                derived2 = derived2 - const2;
                *addr1 = derived1;
                
                /* More computations with different modes */
                long_result_sink = const3 + derived4;
                result_sink = short_const * char_const;
                
                /* Function call clobbers registers */
                clobber_registers();
            } else {
                /* Different computations in else branch */
                derived3 = derived3 * 2;
                derived4 = derived4 / 3;
                *addr2 = derived3;
                
                /* Use the long long constant */
                long_result_sink = ll_const - derived4;
                result_sink = const1 >> 1;
                
                /* Another function call */
                use_value(derived1, derived4, addr3);
            }
            
            /* --- Basic Block 3: More computations --- */
            /* Create many intermediate values that are simultaneously live */
            int temp1 = derived1 + derived2;
            int temp2 = derived3 - derived1;
            int temp3 = temp1 * temp2;
            int temp4 = temp3 / (base_param + 1);
            int temp5 = temp4 << (i & 3);
            int temp6 = temp5 ^ const1;
            int temp7 = temp6 | const2;
            int temp8 = temp7 & 0xFFFF;
            
            long ltemp1 = derived4 + const3;
            long ltemp2 = ltemp1 * 3;
            long ltemp3 = ltemp2 - ll_const;
            long ltemp4 = ltemp3 >> 4;
            
            /* Use pointers with different addressing modes */
            int *temp_addr1 = addr1 + (i & 7);
            int *temp_addr2 = addr2 + (base_param & 7);
            int *temp_addr3 = addr3 + (derived1 & 7);
            
            /* --- Basic Block 4: Another conditional --- */
            if (i & 2) {
                *temp_addr1 = temp1;
                *temp_addr2 = temp2;
                result_sink = temp3 + temp4;
                
                /* More register pressure */
                long_result_sink = ltemp1 + ltemp2;
                clobber_registers();
            } else {
                *temp_addr3 = temp5;
                global_array[(i + 2) & 0xFF] = temp6;
                result_sink = temp7 - temp8;
                
                long_result_sink = ltemp3 - ltemp4;
                use_value(temp1, ltemp1, temp_addr1);
            }
            
            /* --- Basic Block 5: Final computations --- */
            /* Mix all values together to keep them live */
            int final1 = temp1 + temp3 + temp5 + temp7;
            int final2 = temp2 + temp4 + temp6 + temp8;
            int final3 = final1 * final2;
            int final4 = final3 / (base_param ? base_param : 1);
            
            long lfinal1 = ltemp1 + ltemp3;
            long lfinal2 = ltemp2 + ltemp4;
            long lfinal3 = lfinal1 * lfinal2;
            
            /* Force use of all remat candidates one more time */
            result_sink = final4 + const1 + const2 + derived1 + derived2 + derived3;
            long_result_sink = lfinal3 + const3 + derived4 + ll_const;
            
            /* Store to global with address computation */
            global_array[i & 0xFF] = final4;
            global_counter = i;
            
            /* Final function call to clobber everything */
            clobber_registers();
        }
    }
    
    /* Use results to prevent dead code elimination */
    printf("Result: %d %lld\n", result_sink, long_result_sink);
    printf("Global[0] = %d\n", global_array[0]);
    
    return 0;
}
