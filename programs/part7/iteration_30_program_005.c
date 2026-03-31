/* reload_coverage.c - Comprehensive test for GCC reload pass coverage */
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

/* Packed struct to force misalignment */
struct __attribute__((packed)) misaligned_data {
    char c;
    int i;
    long l;
};

/* Volatile globals to prevent optimization */
volatile int global_counter = 0;
volatile long global_array[32] = {0};

int main(void) {
    /* Declare diverse variables with different storage characteristics */
    register int reg_var asm("r12") = 1;  /* Try to bind to specific reg */
    volatile int vol_var = 2;
    int auto_var = 3;
    long large_array[64];
    char *char_ptr = (char *)large_array;
    struct misaligned_data packed = {.c = 'A', .i = 42, .l = 999};
    
    /* Initialize arrays with pattern */
    for (int i = 0; i < 64; i++) {
        large_array[i] = i * 3 + 1;
    }
    
    /* Loop with varying constraints to trigger different reload types */
    unsigned long checksum = 0;
    
    for (int iteration = 0; iteration < 10; iteration++) {
        int offset = iteration * 7;
        long index = iteration * 11;
        
        /* VARYING CONSTRAINT 1: Complex addressing with multiple components */
        /* Should trigger RELOAD_FOR_INPUT_ADDRESS, RELOAD_FOR_INPADDR_ADDRESS */
        asm volatile (
            "add %[out1], %[in1], %[in2], lsl #2\n\t"
            "ldr %[out2], [%[base], %[index], lsl #3]\n\t"
            : [out1] "=&r" (auto_var), 
              [out2] "=r" (reg_var)
            : [in1] "r" (auto_var),
              [in2] "r" (offset),
              [base] "r" (large_array),
              [index] "r" (index),
              "m" (large_array[0])  /* Memory constraint forces address reloads */
            : "cc", "memory"
        );
        
        /* VARYING CONSTRAINT 2: Early clobber with overlapping operands */
        /* Should trigger RELOAD_FOR_INPUT, RELOAD_FOR_OUTPUT, RELOAD_OTHER */
        long temp1, temp2, temp3;
        asm volatile (
            "mov %[t1], %[a]\n\t"
            "add %[t2], %[t1], %[b]\n\t"
            "mul %[t3], %[t2], %[c]\n\t"
            : [t1] "=&r" (temp1),
              [t2] "=&r" (temp2),
              [t3] "=r" (temp3)
            : [a] "r" (reg_var),
              [b] "r" (auto_var),
              [c] "r" (vol_var),
              "m" (packed)  /* Complex memory operand */
            : "cc"
        );
        
        /* VARYING CONSTRAINT 3: Address of address computation */
        /* Should trigger RELOAD_FOR_OPERAND_ADDRESS, RELOAD_FOR_OPADDR_ADDR */
        long *addr_of_addr;
        asm volatile (
            "adr %[addr], %[mem]\n\t"
            "str %[val], [%[addr]]\n\t"
            : [addr] "=r" (addr_of_addr),
              "=m" (*(volatile long *)char_ptr)
            : [mem] "m" (*(volatile long *)(char_ptr + offset)),
              [val] "r" (iteration)
            : "cc"
        );
        
        /* VARYING CONSTRAINT 4: Multiple outputs with memory inputs */
        /* Should trigger RELOAD_FOR_OUTPUT_ADDRESS, RELOAD_FOR_OUTADDR_ADDRESS */
        long result1, result2;
        asm volatile (
            "ldp %[r1], %[r2], [%[ptr1]]\n\t"
            "stp %[r1], %[r2], [%[ptr2], %[off]]\n\t"
            : [r1] "=&r" (result1),
              [r2] "=&r" (result2),
              "=m" (*(volatile long (*)[2])(large_array + iteration * 2))
            : [ptr1] "r" (&large_array[iteration * 4]),
              [ptr2] "r" (large_array),
              [off] "r" (iteration * 16),
              "m" (*(volatile long (*)[2])(large_array + iteration * 4))
            : "cc", "memory"
        );
        
        /* VARYING CONSTRAINT 5: Mixed size operands with clobbered registers */
        /* Should trigger RELOAD_FOR_OTHER_ADDRESS */
        char char_result;
        int int_result;
        asm volatile (
            "ldrb %w[cr], [%[cp], %[off1]]\n\t"
            "ldr %w[ir], [%[cp], %[off2]]\n\t"
            "add %w[cr], %w[cr], %w[ir]\n\t"
            : [cr] "=&r" (char_result),
              [ir] "=&r" (int_result)
            : [cp] "r" (char_ptr),
              [off1] "r" (offset),
              [off2] "r" (offset * 2),
              "m" (*(volatile char *)char_ptr),
              "m" (*(volatile int *)(char_ptr + offset * 2))
            : "cc"
        );
        
        /* Update checksum with all modified values */
        checksum += auto_var + reg_var + temp3 + (long)addr_of_addr + 
                   result1 + result2 + char_result + int_result;
        
        /* Modify pointer to change addressing patterns */
        char_ptr += iteration;
        vol_var += iteration;
        
        /* Force spill/reload by using many temporary variables */
        {
            long t1 = checksum, t2 = checksum * 2, t3 = checksum * 3;
            long t4 = t1 + t2, t5 = t2 + t3, t6 = t3 + t1;
            asm volatile (
                "add %[sum], %[sum], %[t4]\n\t"
                "add %[sum], %[sum], %[t5]\n\t"
                "add %[sum], %[sum], %[t6]\n\t"
                : [sum] "+r" (checksum)
                : [t4] "r" (t4),
                  [t5] "r" (t5),
                  [t6] "r" (t6)
                : "cc"
            );
        }
    }
    
    /* Final computation to ensure all asm statements have effect */
    checksum += packed.i + packed.l;
    
    /* Use checksum to prevent dead code elimination */
    printf("Final checksum: %lu\n", checksum);
    
    /* Additional forced reload scenarios */
    {
        /* Force RELOAD_OTHER through register pressure */
        long r1, r2, r3, r4, r5, r6, r7, r8, r9, r10;
        asm volatile (
            "mov %[r1], #1\n\t"
            "mov %[r2], #2\n\t"
            "add %[r3], %[r1], %[r2]\n\t"
            "mov %[r4], %[r3]\n\t"
            "mov %[r5], %[r4]\n\t"
            "mov %[r6], %[r5]\n\t"
            "mov %[r7], %[r6]\n\t"
            "mov %[r8], %[r7]\n\t"
            "mov %[r9], %[r8]\n\t"
            "mov %[r10], %[r9]\n\t"
            : [r1] "=&r" (r1), [r2] "=&r" (r2), [r3] "=&r" (r3),
              [r4] "=&r" (r4), [r5] "=&r" (r5), [r6] "=&r" (r6),
              [r7] "=&r" (r7), [r8] "=&r" (r8), [r9] "=&r" (r9),
              [r10] "=r" (r10)
            : 
            : "cc"
        );
        checksum += r10;
    }
    
    printf("Final result: %lu\n", checksum);
    return (checksum > 1000) ? 0 : 1;
}
