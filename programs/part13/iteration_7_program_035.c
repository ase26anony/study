/* early-remat-trigger.c
 * Compile with: gcc -O2 -fearly-remat -fdump-rtl-early-remat -fdump-rtl-all -fno-omit-frame-pointer -fno-schedule-insns -fno-schedule-insns2 -o early_remat_test early-remat-trigger.c
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Global variables to create symbol address remat candidates */
volatile int global_array[256] = {0};
volatile long global_long = 0x123456789ABCDEF0L;
static const int const_array[] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};

/* Dummy function to clobber registers */
static void __attribute__((noinline)) clobber_registers(void) {
    __asm__ volatile ("" : : : "memory", "rax", "rbx", "rcx", "rdx", "rsi", "rdi",
                      "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15");
}

/* Another dummy function to prevent optimization */
static void __attribute__((noinline)) use_value(volatile int *ptr) {
    __asm__ volatile ("" : : "r"(ptr) : "memory");
}

/* Main function with high register pressure loop */
int main(int argc, char *argv[]) {
    volatile int sink = 0;
    int iterations = 1000;
    
    if (argc > 1) {
        iterations = atoi(argv[1]);
        if (iterations <= 0) iterations = 1000;
    }
    
    int base_param = (argc > 2) ? atoi(argv[2]) : 42;
    
    /* Outer loop to create many virtual registers */
    for (int outer = 0; outer < iterations; outer++) {
        /* Create many rematerialization candidates with different modes */
        
        /* SImode constants (expensive to materialize) */
        int const1 = 0x7FFFFFFF;  /* Large immediate might need multiple insns */
        int const2 = 0x12345678;
        int const3 = 0x89ABCDEF;
        int const4 = 0xFEDCBA98;
        
        /* DImode constants */
        long const5 = 0x123456789ABCDEF0L;
        long const6 = 0xFEDCBA9876543210L;
        
        /* Constants derived from function arguments (remat candidates) */
        int derived1 = base_param + 1;
        int derived2 = base_param << 2;
        int derived3 = base_param * 3;
        int derived4 = (base_param & 0xFF) | 0x100;
        
        /* Symbol addresses (Pmode remat candidates) */
        int *addr1 = &global_array[outer % 256];
        long *addr2 = (long*)&global_long;
        const int *addr3 = &const_array[outer % 10];
        
        /* Loop-variant derived values */
        int loop_derived1 = outer + const1;
        int loop_derived2 = outer * derived1;
        long loop_derived3 = (long)outer * const5;
        
        /* Force multiple basic blocks with conditional branches */
        if (outer & 1) {
            /* Branch 1: Use many values */
            int temp1 = const1 + const2;
            int temp2 = derived1 * derived2;
            long temp3 = const5 + const6;
            int *temp_addr = addr1 + (derived3 >> 2);
            
            /* Complex expression chain */
            for (int i = 0; i < 3; i++) {
                int chain1 = temp1 + i;
                int chain2 = temp2 * chain1;
                long chain3 = temp3 + chain2;
                
                /* Use volatile to prevent optimization */
                sink += chain1;
                sink ^= (int)chain3;
                
                /* Clobber registers periodically */
                if (i == 1) {
                    clobber_registers();
                }
            }
            
            /* Store to global to prevent DCE */
            global_array[outer % 256] = temp1 + temp2;
            use_value(&global_array[outer % 256]);
            
        } else {
            /* Branch 2: Different computation pattern */
            int temp4 = const3 - const4;
            int temp5 = derived3 / (derived4 ? derived4 : 1);
            long temp6 = const6 - const5;
            const int *temp_addr2 = addr3 + (derived2 % 5);
            
            /* Another expression chain */
            for (int j = 0; j < 4; j++) {
                int chain4 = temp4 * j;
                int chain5 = temp5 + chain4;
                long chain6 = temp6 - chain5;
                
                /* More volatile operations */
                sink -= chain4;
                sink |= (int)chain6;
                
                /* Clobber registers */
                if (j == 2) {
                    clobber_registers();
                }
            }
            
            /* Pointer arithmetic that creates Pmode values */
            int offset = (outer * 7) & 0xF;
            int *ptr_calc = (int*)((uintptr_t)addr1 + offset * sizeof(int));
            *ptr_calc = temp4 + temp5;
            use_value(ptr_calc);
        }
        
        /* Third basic block (always executed) */
        {
            /* Mix all types and modes */
            int mix1 = loop_derived1 + derived1;
            long mix2 = loop_derived3 + const5;
            int *mix3 = addr1 + (mix1 % 64);
            const int *mix4 = addr3 + (mix1 % 8);
            
            /* Complex computation using all values */
            int result1 = mix1 * (int)mix2;
            int result2 = *mix3 + *mix4;
            long result3 = mix2 + result1 + result2;
            
            /* Force register pressure with many simultaneous values */
            int t1 = result1 + const1;
            int t2 = result2 + const2;
            int t3 = t1 * t2;
            long t4 = result3 + const5;
            int t5 = t3 + (int)t4;
            int t6 = t5 - derived2;
            long t7 = t4 - const6;
            int t8 = t6 * derived3;
            int t9 = t8 + (int)t7;
            
            /* Final sink to prevent optimization */
            sink += t9;
            
            /* One more clobber */
            clobber_registers();
        }
        
        /* Additional basic block with switch for more complexity */
        switch (outer & 3) {
            case 0: {
                int sw1 = const1 + 1;
                int sw2 = const2 - 1;
                sink += sw1 * sw2;
                break;
            }
            case 1: {
                long sw3 = const5 >> 8;
                int sw4 = (int)sw3 + derived1;
                sink ^= sw4;
                break;
            }
            case 2: {
                int *sw5 = addr1 + (derived4 % 16);
                int sw6 = *sw5 + const3;
                sink |= sw6;
                break;
            }
            case 3: {
                const int *sw7 = addr3 + (derived2 % 5);
                long sw8 = (long)*sw7 * const6;
                sink += (int)sw8;
                break;
            }
        }
    }
    
    /* Prevent the entire loop from being optimized away */
    printf("Result: %d (iterations: %d)\n", sink, iterations);
    
    return sink != 0;
}
