/* reload_trigger.c
 * Designed to trigger specific reload types in GCC's reload1.cc
 * Compile with: gcc -O2 -fno-omit-frame-pointer -fno-schedule-insns -fno-schedule-insns2 -c reload_trigger.c
 */

#include <stdint.h>

/* Complex structure to force various addressing modes */
struct ComplexStruct {
    int64_t a;
    int64_t b;
    int64_t c[4];
    volatile int64_t d[3];
    struct {
        int64_t x;
        volatile int64_t y;
    } nested;
};

/* Explicit register variables - using x86-64 registers */
register int64_t reg_a asm("r10");
register int64_t reg_b asm("r11");
register int64_t reg_c asm("r12");
register int64_t reg_d asm("r13");
register int64_t reg_e asm("r14");

/* Global arrays to create addressing complexity */
static struct ComplexStruct global_array[8];
static volatile int64_t volatile_buffer[256];
static int64_t large_buffer[512];

/* Function with complex addressing patterns */
void trigger_reloads(int iterations) {
    /* Local variables that will conflict with register variables */
    int64_t local_index;
    int64_t local_offset;
    struct ComplexStruct *ptr;
    volatile int64_t *volatile_ptr;
    
    /* Initialize some values */
    reg_a = 0x12345678;
    reg_b = 0x87654321;
    reg_c = (int64_t)&global_array[0];
    reg_d = (int64_t)&volatile_buffer[0];
    reg_e = (int64_t)&large_buffer[0];
    
    /* Loop with complex addressing computations */
    for (local_index = 0; local_index < iterations; local_index++) {
        /* Force RELOAD_FOR_INPUT_ADDRESS and RELOAD_FOR_OTHER_ADDRESS */
        /* Complex memory access with multiple register components */
        ptr = (struct ComplexStruct *)(reg_c + (local_index * sizeof(struct ComplexStruct)));
        
        /* Inline assembly with memory constraint requiring address reload */
        asm volatile (
            "movq (%[base], %[index], 8), %[temp]\n\t"
            "addq %[offset], %[temp]\n\t"
            "movq %[temp], (%[dest])"
            : [temp] "=&r" (local_offset)
            : [base] "r" (reg_e),           /* Base register - may need reload */
              [index] "r" (local_index),    /* Index register */
              [offset] "irm" (reg_a),       /* Mixed: immediate/register/memory */
              [dest] "r" (reg_d)            /* Destination register */
            : "memory"
        );
        
        /* Force RELOAD_FOR_INPADDR_ADDRESS */
        /* Access volatile member through pointer with offset */
        volatile_ptr = &ptr->d[0];
        
        asm volatile (
            "movq (%[addr]), %%r15\n\t"
            "addq %%r15, %[sum]"
            : [sum] "+r" (reg_b)
            : [addr] "r" (volatile_ptr),    /* Address that may need reload */
              "m" (*volatile_ptr)           /* Memory constraint forces address computation */
            : "r15", "memory"
        );
        
        /* Force RELOAD_FOR_OUTPUT_ADDRESS and RELOAD_FOR_OUTADDR_ADDRESS */
        /* Complex store operation with computed address */
        int64_t computed_addr = (int64_t)&large_buffer[local_index * 2] + reg_a;
        
        asm volatile (
            "leaq (%[base], %[scale], 4), %[addr]\n\t"
            "movq %[value], (%[addr])"
            : [addr] "=&r" (computed_addr)
            : [base] "r" (reg_e),
              [scale] "r" (local_index),
              [value] "r" (reg_b),
              "m" (*(int64_t*)computed_addr)  /* Memory output constraint */
            : "memory"
        );
        
        /* Force RELOAD_FOR_OPERAND_ADDRESS and RELOAD_FOR_OPADDR_ADDR */
        /* Nested addressing with multiple levels of indirection */
        struct ComplexStruct **ptr_ptr = &ptr;
        
        asm volatile (
            "movq (%[ptrptr]), %[temp]\n\t"
            "movq 32(%[temp]), %[val]\n\t"
            "imulq %[mult], %[val]"
            : [val] "+r" (reg_a),
              [temp] "=&r" (local_offset)
            : [ptrptr] "r" (ptr_ptr),       /* Pointer to pointer - needs address reload */
              [mult] "irm" (local_index),   /* Mixed constraint */
              "m" (**ptr_ptr)               /* Double memory dereference */
            : "memory"
        );
        
        /* Force RELOAD_FOR_OTHER_ADDRESS - complex address with multiple components */
        /* This is the primary target for the uncovered lines */
        int64_t complex_addr = (int64_t)&global_array[0].nested.y 
                               + (local_index * 16) 
                               + reg_a 
                               - reg_b;
        
        asm volatile (
            "movq (%[complex]), %%r15\n\t"
            "addq %%r15, %[accum]"
            : [accum] "+r" (reg_c)
            : [complex] "r" (&complex_addr),  /* Address of address variable */
              "m" (*(int64_t*)complex_addr),  /* Memory access through computed address */
              "m" (complex_addr)              /* The address itself in memory */
            : "r15", "memory"
        );
        
        /* Mix in more register pressure */
        asm volatile (
            "xchgq %[r1], %[r2]\n\t"
            "xchgq %[r2], %[r3]"
            : [r1] "+r" (reg_a),
              [r2] "+r" (reg_b),
              [r3] "+r" (reg_d)
            : 
            : "memory"
        );
    }
    
    /* Final operation to use all register variables */
    asm volatile (
        "addq %[a], %[b]\n\t"
        "addq %[c], %[d]\n\t"
        "subq %[e], %[b]"
        : [b] "+r" (reg_b),
          [d] "+r" (reg_d)
        : [a] "r" (reg_a),
          [c] "r" (reg_c),
          [e] "r" (reg_e)
        : "memory"
    );
}

/* Second function with different addressing patterns */
void trigger_more_reloads(void) {
    /* Multi-dimensional array access in loop */
    int64_t md_array[4][8][16];
    int64_t i, j, k;
    
    /* Explicit register variables for indices */
    register int64_t idx_i asm("r10");
    register int64_t idx_j asm("r11");
    register int64_t idx_k asm("r12");
    
    idx_i = 0;
    idx_j = 0;
    idx_k = 0;
    
    /* Triple nested loop with complex addressing */
    for (i = 0; i < 4; i++) {
        idx_i = i;
        for (j = 0; j < 8; j++) {
            idx_j = j;
            for (k = 0; k < 16; k++) {
                idx_k = k;
                
                /* Complex address computation involving multiple registers */
                int64_t *elem_ptr = &md_array[idx_i][idx_j][idx_k];
                
                /* Inline assembly that uses the address in a non-trivial way */
                asm volatile (
                    "movq (%[base], %[i], 64), %[temp]\n\t"    /* i * (8*8) */
                    "leaq (%[temp], %[j], 8), %[temp]\n\t"     /* j * 8 */
                    "leaq (%[temp], %[k]), %[temp]\n\t"        /* k * 1 */
                    "movq (%[temp]), %[val]"
                    : [temp] "=&r" (idx_i),  /* Reuse idx_i as temp */
                      [val] "=r" (idx_j)     /* Reuse idx_j as val */
                    : [base] "r" (md_array),
                      [i] "r" (idx_i),
                      [j] "r" (idx_j),
                      [k] "r" (idx_k),
                      "m" (md_array[i][j][k])  /* Memory constraint forces address reload */
                    : "memory"
                );
                
                /* Force RELOAD_FOR_OTHER_ADDRESS with different pattern */
                volatile int64_t *volatile_elem = (volatile int64_t *)elem_ptr;
                int64_t offset_addr = (int64_t)volatile_elem + 0x1000;
                
                asm volatile (
                    "movq %[offset], %%r15\n\t"
                    "movq (%%r15), %%r14\n\t"
                    "addq %%r14, %[sum]"
                    : [sum] "+r" (reg_e)
                    : [offset] "r" (&offset_addr),  /* Address of offset variable */
                      "m" (offset_addr),            /* The offset value itself */
                      "m" (*(int64_t*)offset_addr)  /* Memory at offset address */
                    : "r14", "r15", "memory"
                );
            }
        }
    }
}

/* Main function to set up and call the trigger functions */
int main(void) {
    /* Initialize global data */
    for (int i = 0; i < 8; i++) {
        global_array[i].a = i;
        global_array[i].b = i * 2;
        global_array[i].nested.x = i * 3;
        global_array[i].nested.y = i * 4;
    }
    
    /* Call functions with different iteration counts to trigger various reload scenarios */
    trigger_reloads(100);
    trigger_more_reloads();
    
    /* Additional complex pattern */
    {
        /* Array of function pointers to create indirect jumps needing reloads */
        void (*func_array[4])(void) = {
            trigger_reloads,
            trigger_more_reloads,
            trigger_reloads,
            trigger_more_reloads
        };
        
        /* Indirect call with address computation */
        int64_t idx = 2;
        asm volatile (
            "movq (%[array], %[idx], 8), %[temp]\n\t"
            "call *%[temp]"
            : 
            : [array] "r" (func_array),
              [idx] "r" (idx),
              [temp] "r" (reg_a)  /* Force temp into specific register */
            : "memory", "rax", "rcx", "rdx", "rsi", "rdi", "r8", "r9"
        );
    }
    
    return 0;
}
