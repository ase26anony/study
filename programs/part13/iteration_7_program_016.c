/* early-remat-trigger.c
 * Designed to trigger early rematerialization with virtual register spills
 * Compile with: gcc -O2 -fearly-remat -fdump-rtl-early-remat -fdump-rtl-all -fno-omit-frame-pointer -fno-schedule-insns -fno-schedule-insns2 -o early_remat_test early-remat-trigger.c
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Global variables to create rematerialization candidates */
volatile int global_base = 1000;
volatile long global_offset = 2000;
int global_array[256] = {0};
static volatile int sink; /* Prevent optimization */

/* Dummy function to clobber registers */
static void __attribute__((noinline)) clobber_registers(void) {
    __asm__ volatile ("" : : : "memory", "rax", "rbx", "rcx", "rdx", "rsi", "rdi",
                      "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15");
}

/* Another noinline function to force spills */
static int __attribute__((noinline)) compute_offset(int x, int y) {
    __asm__ volatile ("" : "+r"(x), "+r"(y));
    return x + y;
}

int main(int argc, char *argv[]) {
    int iterations = 1000;
    if (argc > 1) iterations = atoi(argv[1]);
    
    int param1 = (argc > 2) ? atoi(argv[2]) : 42;
    long param2 = (argc > 3) ? atol(argv[3]) : 123456789;
    
    volatile int result = 0;
    
    /* Outer loop to create many virtual registers */
    for (int outer = 0; outer < 10; outer++) {
        /* Inner hot loop with high register pressure */
        for (int i = 0; i < iterations; i++) {
            /* Create many rematerialization candidates with different modes */
            
            /* SImode constants (expensive to materialize) */
            int const1 = 0x12345678;  /* Large constant requiring multiple insns */
            int const2 = 0x87654321;
            int const3 = 0xDEADBEEF;
            int const4 = 0xCAFEBABE;
            
            /* DImode constants */
            long const5 = 0x123456789ABCDEF0L;
            long const6 = 0xFEDCBA9876543210L;
            
            /* Constants derived from function arguments (remat candidates) */
            int derived1 = param1 + 1;          /* param + 1 */
            int derived2 = param1 << 2;         /* param << 2 */
            int derived3 = param1 * 3;          /* param * 3 */
            long derived4 = param2 + 0x1000;    /* param + offset */
            long derived5 = param2 << 3;        /* param << 3 */
            
            /* Symbol addresses (Pmode remat candidates) */
            int *addr1 = &global_array[i & 0xFF];
            int *addr2 = &global_array[(i + 1) & 0xFF];
            int *addr3 = &global_array[(i + param1) & 0xFF];
            
            /* Loop-variant values */
            int variant1 = i * 7;
            int variant2 = i + param1;
            long variant3 = i * param2;
            
            /* Complex expressions creating many intermediate values */
            int temp1 = const1 + derived1;
            int temp2 = const2 + derived2;
            int temp3 = const3 + derived3;
            long temp4 = const5 + derived4;
            long temp5 = const6 + derived5;
            
            /* Use in conditional branches to create multiple basic blocks */
            if (i & 1) {
                /* Branch 1 */
                temp1 = temp1 * 2;
                temp4 = temp4 >> 1;
                *addr1 = variant1 + temp1;
                
                /* More computations */
                int temp6 = temp1 + temp2;
                long temp7 = temp4 + temp5;
                temp6 = temp6 * 3;
                temp7 = temp7 / 2;
                
                /* Store to prevent elimination */
                sink = temp6;
                global_array[(i + 2) & 0xFF] = (int)temp7;
            } else {
                /* Branch 2 */
                temp2 = temp2 / 2;
                temp5 = temp5 << 1;
                *addr2 = variant2 + temp2;
                
                /* Different computations */
                int temp8 = temp2 + temp3;
                long temp9 = temp5 + variant3;
                temp8 = temp8 ^ 0xFF;
                temp9 = temp9 | 0xFFFF;
                
                sink = temp8;
                global_array[(i + 3) & 0xFF] = (int)temp9;
            }
            
            /* Third basic block (always executed) */
            if (i & 2) {
                /* More computations mixing types */
                int temp10 = temp1 + temp2 + temp3;
                long temp11 = temp4 + temp5 + variant3;
                temp10 = temp10 ^ const4;
                temp11 = temp11 & const6;
                
                /* Pointer arithmetic */
                int *addr4 = addr3 + (temp10 & 0xF);
                *addr4 = temp10 + (int)temp11;
                
                sink = *addr4;
            } else {
                /* Alternative computations */
                long temp12 = (long)temp1 * (long)temp2;
                int temp13 = (int)(temp12 >> 16);
                temp13 = temp13 + derived1 + derived2;
                
                global_array[i & 0xFF] = temp13;
                sink = temp13;
            }
            
            /* Function call that clobbers registers */
            clobber_registers();
            
            /* More computations after call (forces reloads) */
            int temp14 = derived1 + variant1;
            long temp15 = derived4 + variant3;
            temp14 = compute_offset(temp14, const1);
            temp15 = temp15 + global_offset;
            
            /* Use all created values to keep them live */
            result += temp14 + (int)temp15 + *addr1 + *addr2;
            
            /* Another conditional with more computations */
            if (i % 3 == 0) {
                int temp16 = global_base + i;
                long temp17 = global_offset + param2;
                temp16 = temp16 * 5;
                temp17 = temp17 / 4;
                
                /* Mix pointer and integer operations */
                int *addr5 = &global_array[(temp16 >> 2) & 0xFF];
                *addr5 = (int)temp17;
                sink = *addr5;
            }
            
            /* Final use of many values to extend live ranges */
            int final1 = const1 + const2 + const3 + const4;
            long final2 = const5 + const6 + derived4 + derived5;
            final1 = final1 ^ variant1;
            final2 = final2 | variant3;
            
            result += final1 + (int)final2;
        }
    }
    
    printf("Result: %d\n", result);
    return 0;
}
