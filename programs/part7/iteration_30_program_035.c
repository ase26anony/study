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
volatile long global_counter = 0;
volatile int global_index = 0;

int main() {
    /* Declare diverse variables with different storage characteristics */
    register int reg_var asm("ebx") = 123;  /* Try to bind to specific reg */
    volatile int vol_var = 456;
    int normal_var = 789;
    long long_var = 101112;
    char char_array[256];
    int int_array[128];
    long long_array[64];
    
    /* Misaligned pointer through packed struct */
    struct misaligned_data mdata;
    char *misaligned_ptr = (char*)&mdata.i;
    
    /* Address-taken variables to force spills */
    int *ptr1 = &normal_var;
    int *ptr2 = &vol_var;
    long *ptr3 = &long_var;
    
    /* Initialize arrays */
    for (int i = 0; i < 256; i++) char_array[i] = i % 128;
    for (int i = 0; i < 128; i++) int_array[i] = i * 3;
    for (int i = 0; i < 64; i++) long_array[i] = i * 5;
    
    /* Loop with varying constraints to trigger different reload types */
    for (int iter = 0; iter < 10; iter++) {
        /* Change base/index registers each iteration */
        int base_idx = iter * 7 % 64;
        int index_idx = iter * 11 % 64;
        int scale = 1 << (iter % 4);  /* 1, 2, 4, 8 */
        int displacement = iter * 16;
        
        /* --- Test 1: Complex addressing modes for input addresses --- */
        /* RELOAD_FOR_INPUT_ADDRESS, RELOAD_FOR_INPADDR_ADDRESS */
        asm volatile (
            "mov %[val1], %[out1]\n\t"
            "add %[val2], %[out1]\n\t"
            : [out1] "=r" (normal_var)
            : [val1] "m" (long_array[base_idx + index_idx * scale + displacement]),
              [val2] "r" (reg_var)
            : "cc"
        );
        
        /* --- Test 2: Output address reloads --- */
        /* RELOAD_FOR_OUTPUT_ADDRESS, RELOAD_FOR_OUTADDR_ADDRESS */
        asm volatile (
            "lea (%[base], %[index], %[scale]), %[out]\n\t"
            "mov %[in], (%[out])\n\t"
            : [out] "=&r" (ptr1), "=m" (int_array[base_idx])
            : [base] "r" (long_array),
              [index] "r" (index_idx * sizeof(long)),
              [scale] "i" (1),
              [in] "r" (iter)
            : "memory"
        );
        
        /* --- Test 3: Multiple operand constraints with early clobber --- */
        /* RELOAD_FOR_INPUT, RELOAD_FOR_OUTPUT, RELOAD_OTHER */
        asm volatile (
            "imul %[a], %[b]\n\t"
            "add %[c], %[b]\n\t"
            "mov %[b], %[d]\n\t"
            : [b] "=&r" (reg_var), [d] "=m" (char_array[iter])
            : [a] "r" (normal_var), 
              [c] "rm" (vol_var),  /* Mix register/memory constraint */
              [d] "2" (char_array[iter])  /* Matching constraint */
            : "cc", "memory"
        );
        
        /* --- Test 4: Nested address computation --- */
        /* RELOAD_FOR_OPERAND_ADDRESS, RELOAD_FOR_OPADDR_ADDR */
        void *nested_addr;
        asm volatile (
            "mov %[array], %[addr]\n\t"
            "add %[offset], %[addr]\n\t"
            : [addr] "=r" (nested_addr)
            : [array] "m" (int_array),  /* Memory constraint forces address reload */
              [offset] "ri" (iter * sizeof(int) * scale)  /* Register/immediate mix */
            : "cc"
        );
        
        /* Use the computed address */
        asm volatile (
            "mov (%[addr]), %[val]\n\t"
            : [val] "=r" (vol_var)
            : [addr] "r" (nested_addr)
            : "memory"
        );
        
        /* --- Test 5: Other address reloads with memory clobber --- */
        /* RELOAD_FOR_OTHER_ADDRESS, RELOAD_OTHER */
        asm volatile (
            "mov %[src], %%eax\n\t"
            "add $1, %%eax\n\t"
            "mov %%eax, %[dst]\n\t"
            "lea (%[base], %%eax, %[scale]), %[ptr]\n\t"
            : [dst] "=m" (long_var),
              [ptr] "=r" (ptr2)
            : [src] "rm" (global_counter),  /* Global volatile forces memory */
              [base] "r" (char_array),
              [scale] "i" (2)
            : "eax", "cc", "memory"
        );
        
        /* --- Test 6: Maximum register pressure --- */
        /* Force spills and reloads of all types */
        int r1, r2, r3, r4, r5, r6, r7, r8;
        asm volatile (
            "mov %[in1], %[out1]\n\t"
            "mov %[in2], %[out2]\n\t"
            "add %[in3], %[out3]\n\t"
            "imul %[in4], %[out4]\n\t"
            "lea (%[in5], %[in6], 4), %[out5]\n\t"
            "mov (%[in7]), %[out6]\n\t"
            "mov %[in8], (%[out7])\n\t"
            "lea (%[in9], %[out1], 2), %[out8]\n\t"
            : [out1] "=&r" (r1), [out2] "=&r" (r2), 
              [out3] "=&r" (r3), [out4] "=&r" (r4),
              [out5] "=&r" (r5), [out6] "=&r" (r6),
              [out7] "=r" (r7), [out8] "=r" (r8)
            : [in1] "r" (normal_var), [in2] "m" (vol_var),
              [in3] "ri" (iter), [in4] "rm" (reg_var),
              [in5] "r" (int_array), [in6] "r" (base_idx),
              [in7] "r" (misaligned_ptr), [in8] "r" (iter),
              [in9] "m" (char_array[displacement])
            : "cc", "memory"
        );
        
        /* Update globals to prevent dead code elimination */
        global_counter += r1 + r2 + r3 + r4;
        global_index = iter;
    }
    
    /* Compute checksum to ensure all operations have effect */
    unsigned long checksum = 0;
    checksum += normal_var;
    checksum += vol_var;
    checksum += long_var;
    checksum += reg_var;
    
    for (int i = 0; i < 128; i++) {
        checksum += int_array[i];
    }
    
    for (int i = 0; i < 64; i++) {
        checksum += long_array[i] & 0xFF;
    }
    
    printf("Checksum: %lu\n", checksum);
    printf("Global counter: %ld\n", (long)global_counter);
    
    return (int)(checksum % 256);
}
