/* reload1_trigger.c
 * Designed to trigger specific reload types in GCC's reload pass,
 * particularly RELOAD_FOR_OTHER_ADDRESS and related address reloads.
 */

#include <stdint.h>
#include <stdlib.h>

/* Complex data structure to force interesting addressing */
struct MultiDim {
    int data[4][8][16];
    volatile int* volatile ptrs[8];
    long offsets[16];
};

/* Global volatile structures to prevent optimization */
volatile struct MultiDim g_struct;
volatile int g_array[256];

/* Explicit register variables - force specific register allocation */
register int r10_var asm("r10");
register int r11_var asm("r11");
register int r12_var asm("r12");
register volatile int* r13_ptr asm("r13");
register long r14_idx asm("r14");

/* Function with complex addressing patterns */
void trigger_reloads(int iterations) {
    /* Local variables that will conflict with register variables */
    int local_array[128];
    volatile int* volatile local_ptrs[32];
    struct MultiDim local_struct;
    
    /* Initialize some data */
    for (int i = 0; i < 128; i++) {
        local_array[i] = i * 3;
    }
    
    for (int i = 0; i < 32; i++) {
        local_ptrs[i] = (volatile int*)&local_array[i % 128];
    }
    
    /* Initialize explicit register variables */
    r10_var = 42;
    r11_var = 73;
    r12_var = 99;
    r13_ptr = (volatile int*)&g_array[0];
    r14_idx = 0;
    
    /* Main loop with complex addressing patterns */
    for (int outer = 0; outer < iterations; outer++) {
        /* Pattern 1: Complex addressing with multiple operands
         * Forces RELOAD_FOR_INPUT_ADDRESS and RELOAD_FOR_OTHER_ADDRESS */
        asm volatile (
            /* Operation using complex memory addressing */
            "lea (%[base], %[idx], 4), %[temp]\n\t"
            "add (%[mem], %[temp], 4), %[out]\n\t"
            : [out] "+r" (r10_var)
            : [base] "r" (&local_array[0]),
              [idx] "r" (r11_var),
              [mem] "m" (*(struct MultiDim*)&local_struct),
              [temp] "r" (r12_var)
            : "memory", "cc"
        );
        
        /* Pattern 2: Nested address computation with explicit registers
         * Forces RELOAD_FOR_INPADDR_ADDRESS */
        int temp_idx = (outer * 17) & 0x7F;
        asm volatile (
            /* Complex address calculation requiring multiple reloads */
            "imul $12, %[idx], %[t1]\n\t"
            "mov (%[base], %[t1], 2), %[t2]\n\t"
            "add %[t2], %[out]\n\t"
            : [out] "+r" (r11_var),
              [t1] "=&r" (r12_var),
              [t2] "=&r" (r10_var)
            : [base] "r" (&g_struct.data[0][0][0]),
              [idx] "irm" (temp_idx)  /* Mixed: immediate, register, or memory */
            : "memory", "cc"
        );
        
        /* Pattern 3: Multiple memory accesses with conflicting constraints
         * Forces RELOAD_FOR_OUTPUT_ADDRESS and RELOAD_FOR_OUTADDR_ADDRESS */
        volatile int* addr1 = &local_array[outer % 64];
        volatile int* addr2 = &g_array[(outer * 13) % 256];
        
        asm volatile (
            /* Operation requiring address reloads for both source and destination */
            "mov %[src], %%r15\n\t"
            "mov (%[dst]), %%rax\n\t"
            "add (%%r15), %%rax\n\t"
            "mov %%rax, (%[dst])\n\t"
            : 
            : [dst] "r" (addr2),
              [src] "m" (*(void**)&addr1)  /* Memory constraint forces address reload */
            : "memory", "rax", "r15", "cc"
        );
        
        /* Pattern 4: Operand address reloads
         * Forces RELOAD_FOR_OPERAND_ADDRESS and RELOAD_FOR_OPADDR_ADDR */
        long complex_offset = (r14_idx * 8 + outer * 4) & 0xFF;
        
        asm volatile (
            /* Operation with complex offset calculation */
            "mov %[offset], %%rbx\n\t"
            "lea (%[base], %%rbx, 2), %%rcx\n\t"
            "mov (%%rcx), %%rdx\n\t"
            "add %%rdx, %[result]\n\t"
            : [result] "+r" (r10_var)
            : [base] "r" (&g_struct),
              [offset] "irm" (complex_offset)  /* Mixed constraint */
            : "memory", "rbx", "rcx", "rdx", "cc"
        );
        
        /* Pattern 5: Other address reloads with volatile accesses
         * Forces RELOAD_FOR_OTHER_ADDRESS specifically */
        volatile int** volatile ptr_ptr = &local_ptrs[outer % 16];
        
        asm volatile (
            /* Nested pointer dereference requiring address reload */
            "mov %[pptr], %%rsi\n\t"
            "mov (%%rsi), %%rdi\n\t"
            "mov (%%rdi), %%eax\n\t"
            "add %%eax, %[sum]\n\t"
            : [sum] "+r" (r12_var)
            : [pptr] "m" (*(void**)ptr_ptr)  /* Memory constraint on pointer-to-pointer */
            : "memory", "rax", "rsi", "rdi", "cc"
        );
        
        /* Pattern 6: Loop-carried dependencies with address computations */
        r14_idx = (r14_idx + r10_var + r11_var) & 0x3F;
        
        /* Complex array access with multiple index calculations */
        int idx1 = (r10_var + outer) & 0x3;
        int idx2 = (r11_var * 3) & 0x7;
        int idx3 = (r12_var >> 2) & 0xF;
        
        /* This access pattern forces various reload types */
        int value = local_struct.data[idx1][idx2][idx3];
        
        asm volatile (
            /* Use the value in another complex operation */
            "imul $7, %[val], %%rax\n\t"
            "add %%rax, %[acc]\n\t"
            : [acc] "+r" (r11_var)
            : [val] "irm" (value)
            : "rax", "cc"
        );
        
        /* Pattern 7: Address computation spanning multiple statements
         * Forces reloads for address components */
        {
            long base_offset = (long)&g_struct.offsets[0];
            long element_offset = idx1 * sizeof(g_struct.offsets[0]);
            volatile long* final_addr;
            
            asm volatile (
                /* Compute final address using multiple components */
                "lea (%[base], %[elem], 1), %[addr]\n\t"
                : [addr] "=r" (final_addr)
                : [base] "r" (base_offset),
                  [elem] "r" (element_offset)
                : "cc"
            );
            
            /* Use the computed address */
            asm volatile (
                "add (%[addr]), %[sum]\n\t"
                : [sum] "+r" (r10_var)
                : [addr] "m" (*(void**)&final_addr)
                : "memory", "cc"
            );
        }
    }
    
    /* Final operation to use all register variables */
    asm volatile (
        "add %[a], %[b]\n\t"
        "add %[b], %[c]\n\t"
        "mov %[c], (%[mem])\n\t"
        : 
        : [a] "r" (r10_var),
          [b] "r" (r11_var),
          [c] "r" (r12_var),
          [mem] "r" (r13_ptr)
        : "memory", "cc"
    );
}

/* Secondary function with different patterns */
void more_complex_addressing(int n) {
    /* Multi-dimensional volatile access */
    volatile int md_array[8][16][32];
    
    /* Initialize */
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 16; j++) {
            for (int k = 0; k < 32; k++) {
                md_array[i][j][k] = i * 1000 + j * 100 + k;
            }
        }
    }
    
    /* Complex loop with addressing that requires various reloads */
    for (int i = 0; i < n; i++) {
        /* Compute indices using complex expressions */
        int idx_a = (i * 11) % 8;
        int idx_b = (i * 13) % 16;
        int idx_c = (i * 17) % 32;
        
        /* Access with volatile qualifier prevents optimization */
        volatile int* elem_ptr = &md_array[idx_a][idx_b][idx_c];
        
        /* Operation requiring address reload */
        asm volatile (
            "mov (%[ptr]), %%eax\n\t"
            "add $1, %%eax\n\t"
            "mov %%eax, (%[ptr])\n\t"
            : 
            : [ptr] "m" (*(void**)&elem_ptr)  /* Forces address reload */
            : "memory", "eax", "cc"
        );
        
        /* Additional complexity with pointer arithmetic */
        volatile int** ptr_to_ptr = (volatile int**)&md_array[idx_a][idx_b];
        
        asm volatile (
            "mov %[pptr], %%rbx\n\t"
            "mov (%%rbx, %[off], 4), %%eax\n\t"
            "add %%eax, %[sum]\n\t"
            : [sum] "+r" (r10_var)
            : [pptr] "r" (ptr_to_ptr),
              [off] "r" (idx_c)
            : "memory", "rax", "rbx", "cc"
        );
    }
}

int main(int argc, char** argv) {
    /* Initialize global data */
    for (int i = 0; i < 256; i++) {
        g_array[i] = i;
    }
    
    /* Initialize struct offsets */
    for (int i = 0; i < 16; i++) {
        g_struct.offsets[i] = i * 8;
    }
    
    /* Call functions with complex addressing patterns */
    trigger_reloads(argc > 1 ? atoi(argv[1]) : 10);
    more_complex_addressing(argc > 1 ? atoi(argv[1]) : 5);
    
    /* Final volatile store to prevent dead code elimination */
    asm volatile ("" : : "r"(r10_var), "r"(r11_var), "r"(r12_var) : "memory");
    
    return r10_var + r11_var + r12_var;
}
