/* reload1_trigger.c
 * Designed to trigger complex reload scenarios in GCC's reload pass
 * Specifically targets RELOAD_FOR_OTHER_ADDRESS and related reload types
 */

#include <stdint.h>
#include <stdio.h>

/* Complex data structure to force interesting addressing */
struct MultiDim {
    int data[4][8][16];
    volatile int* volatile ptrs[8];
    long long padding[3];
};

/* Global volatile structures to prevent optimization */
volatile struct MultiDim global_struct;
volatile int global_array[256];

/* Explicit register variables - force specific register allocation */
register int reg_a asm("r10");
register int reg_b asm("r11"); 
register int reg_c asm("r12");
register int reg_d asm("r13");
register int reg_e asm("r14");
register int reg_f asm("r15");

/* Function with complex addressing patterns */
void complex_addressing_loop(int iterations) {
    /* Local arrays with different alignments */
    int aligned_array[128] __attribute__((aligned(64)));
    int unaligned_array[129]; /* Odd size for potential alignment issues */
    
    /* Pointers that will need address reloads */
    volatile int* volatile ptr1 = &aligned_array[0];
    volatile int* volatile ptr2 = &unaligned_array[1];
    volatile struct MultiDim* volatile struct_ptr = (volatile struct MultiDim*)&global_struct;
    
    /* Initialize some values */
    for (int i = 0; i < 128; i++) {
        aligned_array[i] = i * 3;
        if (i < 129) unaligned_array[i] = i * 5;
    }
    
    /* Complex loop with mixed addressing modes */
    for (int outer = 0; outer < iterations; outer++) {
        /* Force register pressure by using all explicit registers */
        reg_a = outer * 2;
        reg_b = outer * 3;
        reg_c = outer * 5;
        reg_d = outer * 7;
        reg_e = outer * 11;
        reg_f = outer * 13;
        
        /* Pattern 1: Memory operand with complex address computation
         * This should trigger RELOAD_FOR_INPUT_ADDRESS and possibly RELOAD_FOR_OTHER_ADDRESS
         */
        int index1 = (reg_a + reg_b) & 0x7F;
        int index2 = (reg_c + reg_d) & 0x7F;
        
        asm volatile (
            /* Complex operation with memory operand needing address reload */
            "imul %[idx1], %[idx2]\n\t"
            "add %[idx2], %[mem1]\n\t"
            "sub %[idx1], %[mem2]"
            : [mem1] "+m" (*(volatile int*)((char*)ptr1 + index1 * sizeof(int))),
              [mem2] "+m" (*(volatile int*)((char*)ptr2 + index2 * sizeof(int)))
            : [idx1] "r" (reg_e),
              [idx2] "r" (reg_f)
            : "cc", "memory"
        );
        
        /* Pattern 2: Nested structure access with multiple address computations
         * Should trigger RELOAD_FOR_INPADDR_ADDRESS and RELOAD_FOR_OTHER_ADDRESS
         */
        int x = (reg_a ^ reg_b) & 0x3;
        int y = (reg_c ^ reg_d) & 0x7;
        int z = (reg_e ^ reg_f) & 0xF;
        
        /* Complex address computation involving multiple registers */
        volatile int* elem_ptr = &struct_ptr->data[x][y][z];
        
        asm volatile (
            /* Operation requiring the computed address in a register */
            "mov %[val], (%[ptr])\n\t"
            "add $1, %[ptr]\n\t"
            "mov (%[ptr]), %[val]"
            : [val] "=r" (reg_a), [ptr] "+r" (elem_ptr)
            : "m" (*elem_ptr)
            : "memory"
        );
        
        /* Pattern 3: Multiple memory operands with conflicting constraints
         * Should trigger various RELOAD_FOR_*_ADDRESS types
         */
        int offset1 = reg_b * 4;
        int offset2 = reg_c * 8;
        
        asm volatile (
            /* Multiple memory accesses with different base registers */
            "lea (%[base1], %[off1]), %[temp]\n\t"
            "mov (%[temp]), %[out1]\n\t"
            "lea (%[base2], %[off2]), %[temp]\n\t"
            "add (%[temp]), %[out1]\n\t"
            "mov %[out1], (%[base3])"
            : [out1] "=r" (reg_b), [temp] "=&r" (reg_c)
            : [base1] "r" (ptr1), [off1] "r" (offset1),
              [base2] "r" (ptr2), [off2] "r" (offset2),
              [base3] "r" (&global_array[outer & 0xFF]),
              "m" (*(volatile int*)((char*)ptr1 + offset1)),
              "m" (*(volatile int*)((char*)ptr2 + offset2))
            : "cc", "memory"
        );
        
        /* Pattern 4: Immediate + memory + register mix
         * Should trigger RELOAD_FOR_OPERAND_ADDRESS
         */
        long long large_constant = 0x123456789ABCDEF0LL;
        
        asm volatile (
            /* Operation with immediate, memory, and register constraints */
            "mov %[imm], %%rax\n\t"
            "add %[mem], %%rax\n\t"
            "imul %[reg], %%rax\n\t"
            "mov %%rax, %[out]"
            : [out] "=m" (global_array[(outer + 1) & 0xFF])
            : [imm] "i" (0x1000),
              [mem] "m" (*(volatile long long*)(&struct_ptr->padding[outer & 0x1])),
              [reg] "r" (reg_d)
            : "rax", "cc", "memory"
        );
        
        /* Pattern 5: Address computation for output
         * Should trigger RELOAD_FOR_OUTPUT_ADDRESS and RELOAD_FOR_OUTADDR_ADDRESS
         */
        volatile int** addr_ptr = &struct_ptr->ptrs[outer & 0x7];
        
        asm volatile (
            /* Compute address and store result */
            "lea (%[idx1], %[idx2], 4), %[addr]\n\t"
            "add %[base], %[addr]\n\t"
            "mov %[addr], (%[out_addr])"
            : [addr] "=r" (reg_e), [out_addr] "+r" (addr_ptr)
            : [idx1] "r" (reg_a),
              [idx2] "r" (reg_b),
              [base] "r" (ptr1),
              "m" (*addr_ptr)
            : "cc", "memory"
        );
        
        /* Pattern 6: Complex chain of address computations
         * Should trigger RELOAD_FOR_OPADDR_ADDR and RELOAD_FOR_OTHER_ADDRESS
         */
        {
            register int* chain_ptr asm("r8") = (int*)&aligned_array[0];
            register int* chain_ptr2 asm("r9") = (int*)&unaligned_array[0];
            
            asm volatile (
                /* Multiple dependent address calculations */
                "mov %[chain1], %[tmp1]\n\t"
                "add %[off1], %[tmp1]\n\t"
                "mov (%[tmp1]), %[tmp2]\n\t"
                "lea (%[chain2], %[tmp2], 4), %[tmp1]\n\t"
                "mov (%[tmp1]), %[tmp2]\n\t"
                "mov %[tmp2], (%[chain1], %[off2], 4)"
                : [tmp1] "=&r" (reg_f), [tmp2] "=&r" (reg_a)
                : [chain1] "r" (chain_ptr),
                  [chain2] "r" (chain_ptr2),
                  [off1] "r" (reg_b * 4),
                  [off2] "r" (reg_c * 4),
                  "m" (*(volatile int*)((char*)chain_ptr + reg_b * 4)),
                  "m" (*(volatile int*)((char*)chain_ptr2 + reg_a * 4))
                : "cc", "memory"
            );
        }
    }
}

/* Secondary function with different patterns */
void secondary_patterns(void) {
    /* Use explicit register variables in different combinations */
    reg_a = 1;
    reg_b = 2;
    reg_c = 3;
    reg_d = 4;
    reg_e = 5;
    reg_f = 6;
    
    /* Array of pointers requiring address reloads */
    volatile int* ptr_array[8];
    for (int i = 0; i < 8; i++) {
        ptr_array[i] = &global_array[i * 32];
    }
    
    /* Complex addressing through pointer array */
    for (int i = 0; i < 100; i++) {
        int idx = (reg_a + i) & 0x7;
        volatile int** ptr_to_ptr = &ptr_array[idx];
        
        asm volatile (
            /* Load address from memory, then use it */
            "mov (%[ptrptr]), %[addr]\n\t"
            "mov (%[addr], %[idx], 4), %[val]\n\t"
            "add %[inc], %[val]\n\t"
            "mov %[val], (%[addr], %[idx], 4)"
            : [addr] "=&r" (reg_b), [val] "=&r" (reg_c)
            : [ptrptr] "r" (ptr_to_ptr),
              [idx] "r" (reg_d),
              [inc] "r" (reg_e),
              "m" (*ptr_to_ptr),
              "m" (*(volatile int*)(*ptr_to_ptr + reg_d))
            : "cc", "memory"
        );
        
        /* Rotate registers to force different reload decisions */
        asm volatile (
            "xchg %[a], %[b]\n\t"
            "xchg %[b], %[c]\n\t"
            "xchg %[c], %[d]"
            : [a] "+r" (reg_a), [b] "+r" (reg_b),
              [c] "+r" (reg_c), [d] "+r" (reg_d)
            :
            : "cc"
        );
    }
}

int main(void) {
    /* Initialize global data */
    for (int i = 0; i < 256; i++) {
        global_array[i] = i;
    }
    
    /* Initialize structure */
    for (int x = 0; x < 4; x++) {
        for (int y = 0; y < 8; y++) {
            for (int z = 0; z < 16; z++) {
                global_struct.data[x][y][z] = x * 1000 + y * 100 + z;
            }
        }
    }
    
    /* Call functions with complex addressing patterns */
    complex_addressing_loop(50);
    secondary_patterns();
    
    /* Final mixed pattern */
    {
        volatile int* final_ptr = &global_array[0];
        int final_offset = reg_a * 16 + reg_b * 8 + reg_c * 4;
        
        asm volatile (
            /* Final complex pattern combining everything */
            "lea (%[base], %[off], 2), %[addr1]\n\t"
            "mov (%[addr1]), %[val1]\n\t"
            "lea (%[val1], %[val1], 4), %[val2]\n\t"
            "mov %[val2], (%[base], %[off], 1)"
            : [addr1] "=&r" (reg_d), [val1] "=&r" (reg_e), [val2] "=&r" (reg_f)
            : [base] "r" (final_ptr),
              [off] "r" (final_offset),
              "m" (*(volatile int*)((char*)final_ptr + final_offset * 2)),
              "m" (*(volatile int*)((char*)final_ptr + final_offset))
            : "cc", "memory"
        );
    }
    
    return 0;
}
