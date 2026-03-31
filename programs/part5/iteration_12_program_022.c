/* reload1_trigger.c
 * Designed to trigger specific reload types in GCC's reload pass
 * Compile with: gcc -O2 -fno-omit-frame-pointer -fno-schedule-insns -fno-schedule-insns2 -c reload1_trigger.c
 */

#include <stdint.h>
#include <stdlib.h>

/* Complex structure to force various addressing modes */
struct ComplexStruct {
    int64_t a;
    int64_t b;
    int64_t c[8];
    volatile int64_t d[4];
    struct ComplexStruct *next;
};

/* Global arrays to create addressing complexity */
static struct ComplexStruct global_array[16];
static volatile int64_t volatile_buffer[256];

/* Explicit register variables - force specific register allocation */
register int64_t reg_a asm("r10");
register int64_t reg_b asm("r11");
register int64_t reg_c asm("r12");
register int64_t reg_d asm("r13");
register int64_t reg_e asm("r14");
register int64_t reg_f asm("r15");

/* Function with complex addressing patterns */
void trigger_reloads(int iterations) {
    /* Local variables with different storage classes */
    struct ComplexStruct local_struct;
    struct ComplexStruct *ptr_array[8];
    int64_t index_array[32];
    volatile int64_t *volatile_ptr = volatile_buffer;
    
    /* Initialize data structures */
    for (int i = 0; i < 8; i++) {
        ptr_array[i] = &global_array[i];
    }
    
    for (int i = 0; i < 32; i++) {
        index_array[i] = (i * 3) % 32;
    }
    
    /* Main loop with complex addressing */
    for (int iter = 0; iter < iterations; iter++) {
        /* Force RELOAD_FOR_INPUT and RELOAD_FOR_INPUT_ADDRESS */
        /* Complex inline asm with memory operand and register constraints */
        asm volatile (
            "/* Complex operation with memory addressing */\n\t"
            "mov %[mem1], %[tmp1]\n\t"
            "add %[idx1], %[tmp1]\n\t"
            "mov %[tmp1], %[out1]"
            : [out1] "=r" (reg_a)          /* Output in forced register */
            : [mem1] "m" (local_struct.c[iter % 8]),  /* Memory input */
              [idx1] "r" (index_array[iter % 32]),    /* Register input */
              [tmp1] "r" (reg_b)           /* Temporary in forced register */
            : "memory"
        );
        
        /* Force RELOAD_FOR_OTHER_ADDRESS */
        /* Nested address computation with multiple registers */
        int64_t offset = iter * 16;
        asm volatile (
            "/* Nested address computation */\n\t"
            "lea (%[base], %[idx], 8), %[addr]\n\t"
            "add %[offset], %[addr]\n\t"
            "mov (%[addr]), %[result]"
            : [result] "=r" (reg_c),
              [addr] "=&r" (reg_d)         /* Early clobber - forces reload */
            : [base] "r" (&global_array[0]),
              [idx] "r" (reg_a),           /* From previous operation */
              [offset] "irm" (offset)      /* Immediate, register, or memory */
            : "memory"
        );
        
        /* Force RELOAD_FOR_OUTPUT_ADDRESS and RELOAD_FOR_OUTADDR_ADDRESS */
        /* Store with complex address calculation */
        int64_t store_val = iter * 7;
        asm volatile (
            "/* Complex store operation */\n\t"
            "mov %[val], (%[base], %[scale], %[idx])"
            : 
            : [val] "r" (store_val),
              [base] "r" (volatile_ptr),
              [scale] "r" (reg_c),         /* Dynamic scale factor */
              [idx] "r" (reg_d)            /* Index register */
            : "memory"
        );
        
        /* Force RELOAD_FOR_OPERAND_ADDRESS and RELOAD_FOR_OPADDR_ADDR */
        /* Multiple memory operands with different addressing modes */
        struct ComplexStruct *current = ptr_array[iter % 8];
        asm volatile (
            "/* Multiple memory accesses */\n\t"
            "mov (%[ptr1]), %[tmp1]\n\t"
            "add (%[ptr2], %[idx], 8), %[tmp1]\n\t"
            "mov %[tmp1], (%[ptr3])"
            : 
            : [ptr1] "r" (&current->a),
              [ptr2] "r" (&current->c[0]),
              [ptr3] "r" (&current->d[iter % 4]),
              [idx] "r" (reg_a),
              [tmp1] "r" (reg_e)
            : "memory", "r10", "r11", "r12", "r13", "r14", "r15"
        );
        
        /* Force RELOAD_FOR_INPADDR_ADDRESS */
        /* Memory operand with address that needs reloading */
        int64_t complex_index = (reg_a + reg_b + reg_c) % 16;
        asm volatile (
            "/* Memory operand with complex address */\n\t"
            "imul %[src], %[dst]\n\t"
            "add %%gs:%p[mem], %[dst]"     /* Segment prefix adds complexity */
            : [dst] "+r" (reg_f)
            : [src] "r" (reg_d),
              [mem] "m" (global_array[complex_index].b)
            : "memory", "cc"
        );
        
        /* Mix in immediate values to force different reload types */
        if (iter % 3 == 0) {
            /* Immediate value combined with memory access */
            asm volatile (
                "/* Immediate with memory */\n\t"
                "cmp $0x7FFF, %[mem]\n\t"
                "setg %[out]"
                : [out] "=r" (reg_b)
                : [mem] "m" (local_struct.d[iter % 4])
                : "memory", "cc"
            );
        }
        
        /* Pointer chasing to create address dependency chain */
        if (current->next) {
            asm volatile (
                "/* Pointer chasing */\n\t"
                "mov (%[ptr]), %[tmp]\n\t"
                "add $8, %[tmp]\n\t"
                "mov %[tmp], (%[ptr])"
                : [tmp] "=&r" (reg_e)      /* Early clobber */
                : [ptr] "r" (&current->next->a)
                : "memory"
            );
        }
        
        /* Prevent optimization of loop variables */
        asm volatile ("" : : "r" (reg_a), "r" (reg_b), "r" (reg_c),
                           "r" (reg_d), "r" (reg_e), "r" (reg_f) : );
    }
}

/* Secondary function with different pattern */
void secondary_reload_pattern(int count) {
    /* Multi-dimensional array access */
    int64_t matrix[8][8][8];
    volatile int64_t *volatile matrix_ptr = (volatile int64_t *)matrix;
    
    /* Complex index calculations using explicit register variables */
    for (int i = 0; i < count; i++) {
        for (int j = 0; j < 8; j++) {
            /* Force address reloads with multi-dimensional indexing */
            int64_t *base_ptr = &matrix[i % 8][j][0];
            
            asm volatile (
                "/* Multi-dimensional access */\n\t"
                "mov (%[base], %[idx], 8), %[val]\n\t"
                "add %[val], %[acc]"
                : [acc] "+r" (reg_a)
                : [base] "r" (base_ptr),
                  [idx] "r" (reg_b),       /* Dynamic index */
                  [val] "r" (reg_c)
                : "memory"
            );
            
            /* Alternate between different addressing modes */
            if (j % 2 == 0) {
                asm volatile (
                    "/* Scaled index addressing */\n\t"
                    "mov %[offset], (%[base], %[scale], 4)"
                    : 
                    : [base] "r" (matrix_ptr),
                      [scale] "r" (reg_d),
                      [offset] "r" (reg_e)
                    : "memory"
                );
            }
            
            /* Update index registers to force spills/reloads */
            asm volatile (
                "/* Update multiple registers */\n\t"
                "add $1, %0\n\t"
                "add $2, %1\n\t"
                "add $3, %2"
                : "+r" (reg_b), "+r" (reg_c), "+r" (reg_d)
                : 
                : "cc"
            );
        }
    }
}

/* Main function to set up and call the trigger functions */
int main(int argc, char **argv) {
    /* Initialize global data */
    for (int i = 0; i < 16; i++) {
        global_array[i].a = i;
        global_array[i].b = i * 2;
        for (int j = 0; j < 8; j++) {
            global_array[i].c[j] = i + j;
        }
        global_array[i].next = (i < 15) ? &global_array[i + 1] : NULL;
    }
    
    /* Initialize explicit register variables */
    reg_a = 1;
    reg_b = 2;
    reg_c = 3;
    reg_d = 4;
    reg_e = 5;
    reg_f = 6;
    
    /* Call functions with different complexities */
    trigger_reloads(100);
    secondary_reload_pattern(50);
    
    /* Final barrier to prevent dead code elimination */
    asm volatile ("" : : : "memory");
    
    /* Use results to prevent optimization */
    volatile int64_t result = reg_a + reg_b + reg_c + reg_d + reg_e + reg_f;
    (void)result;  /* Suppress unused warning */
    
    return 0;
}
