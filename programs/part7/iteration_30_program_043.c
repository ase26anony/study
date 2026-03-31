/* reload_coverage.c - Comprehensive test for GCC reload pass coverage */
#include <stdio.h>
#include <stdint.h>

/* Packed struct to force misaligned accesses */
struct __attribute__((packed)) misaligned_data {
    char c;
    int i;
    long l;
    char pad[3];
};

/* Volatile globals to prevent optimization */
volatile int global_counter = 0;
volatile long global_array[32] = {0};

int main() {
    /* Declare variables with different types and storage */
    register int reg_var1 asm("r12") = 1;
    register int reg_var2 asm("r13") = 2;
    int auto_var1 = 3, auto_var2 = 4, auto_var3 = 5;
    long stack_array[8] = {10, 20, 30, 40, 50, 60, 70, 80};
    volatile int volatile_var = 100;
    struct misaligned_data packed = {.c = 'A', .i = 123, .l = 456};
    
    /* Take addresses to force spill/reload */
    int *ptr1 = &auto_var1;
    int *ptr2 = &auto_var2;
    long *ptr3 = &stack_array[2];
    char *cptr = &packed.c;
    
    /* Loop to vary constraints and trigger different reload types */
    for (int iter = 0; iter < 4; iter++) {
        /* Vary constraints based on iteration */
        const char *constraint1 = (iter & 1) ? "r" : "m";
        const char *constraint2 = (iter & 2) ? "=&r" : "r";
        
        /* Complex addressing mode with multiple components */
        asm volatile (
            /* Output operands with early clobber to force RELOAD_FOR_OUTPUT */
            "mov %[out1], %[in1]\n\t"
            "lea (%[base], %[index], %[scale]), %[out2]\n\t"
            /* Complex memory operand forcing RELOAD_FOR_INPUT_ADDRESS */
            "add (%[mem1], %[idx1], 4), %[out1]\n\t"
            /* Nested address computation for RELOAD_FOR_OPERAND_ADDRESS */
            "mov %[addr], (%[mem2])\n\t"
            /* Multiple constraints to create register pressure */
            "imul %[in2], %[out1]\n\t"
            : [out1] "=&r" (auto_var1), 
              [out2] "=r" (auto_var2),
              [addr] "=m" (stack_array[iter])
            : [in1] "r" (reg_var1),
              [in2] "r" (reg_var2),
              [base] "r" ((long)stack_array),
              [index] "r" ((long)iter * 8),
              [scale] "i" (1),
              [mem1] "r" (ptr3),
              [idx1] "r" (iter),
              [mem2] "r" (&ptr3)
            : "memory", "cc"
        );
        
        /* Second asm to trigger RELOAD_FOR_INPADDR_ADDRESS and RELOAD_FOR_OUTADDR_ADDRESS */
        asm volatile (
            /* Take address of memory operand itself */
            "mov %[val], (%[addr1], %[offset1])\n\t"
            /* Output with address that needs reload */
            "mov (%[addr2], %[offset2]), %[result]\n\t"
            : [result] "=r" (volatile_var)
            : [val] "r" (iter + 100),
              [addr1] "r" (stack_array),
              [offset1] "r" (iter * sizeof(long)),
              [addr2] "r" (global_array),
              [offset2] "r" (iter * sizeof(long))
            : "memory"
        );
        
        /* Third asm with many operands to force RELOAD_OTHER */
        asm volatile (
            "add %[a], %[b]\n\t"
            "sub %[c], %[d]\n\t"
            "xor %[e], %[f]\n\t"
            "or %[g], %[h]\n\t"
            : [b] "+r" (auto_var1),
              [d] "+r" (auto_var2),
              [f] "+r" (auto_var3),
              [h] "+r" (reg_var1)
            : [a] "r" (iter),
              [c] "r" (iter * 2),
              [e] "m" (stack_array[iter % 8]),
              [g] "m" (packed.i)
            : "cc"
        );
        
        /* Force RELOAD_FOR_OTHER_ADDRESS with volatile and complex constraints */
        asm volatile (
            "movq (%[complex]), %%rax\n\t"
            "addq $1, %%rax\n\t"
            "movq %%rax, (%[complex])\n\t"
            : 
            : [complex] "r" (&global_array[iter * 2 + 1])
            : "rax", "memory"
        );
        
        /* Update register variables to change constraints in next iteration */
        reg_var1 += iter;
        reg_var2 -= iter;
    }
    
    /* Compute checksum to ensure all operations have effect */
    long checksum = 0;
    checksum += auto_var1;
    checksum += auto_var2;
    checksum += auto_var3;
    checksum += volatile_var;
    checksum += reg_var1;
    checksum += reg_var2;
    
    for (int i = 0; i < 8; i++) {
        checksum += stack_array[i];
    }
    
    checksum += packed.i;
    checksum += packed.l;
    
    printf("Checksum: %ld\n", checksum);
    printf("Global counter: %d\n", global_counter);
    
    return 0;
}
