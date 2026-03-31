/* reload_trigger.c
 * Designed to trigger specific reload types in GCC's reload1.cc
 * Compile with: gcc -O2 -fno-omit-frame-pointer -fno-schedule-insns -fno-schedule-insns2 reload_trigger.c -o reload_trigger
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

/* Global arrays to create addressing pressure */
static struct ComplexStruct global_array[64];
static volatile int64_t volatile_buffer[256];

/* Explicit register variables - force specific register allocation */
register int64_t reg_a asm("r10");
register int64_t reg_b asm("r11");
register int64_t reg_c asm("r12");
register int64_t reg_d asm("r13");
register int64_t reg_e asm("r14");
register int64_t reg_f asm("r15");

/* Function with complex addressing patterns */
void trigger_reloads(struct ComplexStruct *base, int64_t iterations) {
    /* Local variables that will conflict with register variables */
    int64_t index1, index2, offset;
    struct ComplexStruct *ptr1, *ptr2;
    volatile int64_t *volatile_ptr;
    
    /* Initialize explicit register variables */
    reg_a = (int64_t)&global_array[0];
    reg_b = (int64_t)&volatile_buffer[0];
    reg_c = 0;
    reg_d = 8;  /* Structure stride */
    reg_e = 16; /* Additional offset */
    reg_f = iterations;
    
    /* Loop with complex addressing computations */
    for (int64_t i = 0; i < iterations; i++) {
        /* Complex address computation involving multiple registers */
        index1 = (i * reg_d) & 63;
        index2 = (i * 3 + reg_e) & 255;
        offset = (reg_c + i * 16) & 511;
        
        /* Force RELOAD_FOR_INPUT_ADDRESS and RELOAD_FOR_OTHER_ADDRESS */
        /* Pattern 1: Memory operand with complex addressing */
        asm volatile (
            "movq (%[base], %[idx1], 8), %[temp]\n\t"
            "addq %[offset], %[temp]\n\t"
            "movq %[temp], (%[regb], %[idx2], 8)"
            : [temp] "=&r" (reg_c)  /* Early clobber to force reload */
            : [base] "r" (base),    /* Base pointer - may need address reload */
              [idx1] "r" (index1),  /* Index - in register */
              [offset] "irm" (offset), /* Mixed: immediate, register, or memory */
              [regb] "r" (reg_b),   /* Fixed register variable */
              [idx2] "r" (index2)   /* Another index */
            : "memory", "cc"
        );
        
        /* Force RELOAD_FOR_INPADDR_ADDRESS and RELOAD_FOR_OUTPUT_ADDRESS */
        /* Pattern 2: Nested addressing with explicit register constraints */
        ptr1 = (struct ComplexStruct *)((char *)base + index1 * sizeof(struct ComplexStruct));
        ptr2 = (struct ComplexStruct *)(reg_a + i * sizeof(struct ComplexStruct));
        
        asm volatile (
            "leaq (%[ptr1], %[regd], 4), %[addr]\n\t"
            "movq 16(%[addr]), %[val]\n\t"
            "movq %[val], 32(%[ptr2])"
            : [addr] "=&r" (reg_e),  /* Address computation result */
              [val] "=&r" (reg_f)    /* Value loaded */
            : [ptr1] "r" (ptr1),     /* Base pointer needing address reload */
              [regd] "r" (reg_d),    /* Register variable */
              [ptr2] "r" (ptr2)      /* Another base pointer */
            : "memory", "cc"
        );
        
        /* Force RELOAD_FOR_OPERAND_ADDRESS and RELOAD_FOR_OPADDR_ADDR */
        /* Pattern 3: Complex constraints with immediate and memory mix */
        volatile_ptr = &volatile_buffer[index2];
        
        asm volatile (
            "imulq $7, %[imm], %[res]\n\t"
            "addq (%[volptr]), %[res]\n\t"
            "movq %[res], (%[rega], %[idx1], 2)"
            : [res] "=&r" (reg_a)    /* Early clobber on register variable */
            : [imm] "irm" (i),       /* Mixed constraint - could be immediate */
              [volptr] "m" (*volatile_ptr), /* Memory constraint forcing address reload */
              [rega] "r" (reg_a),    /* Base register (self-modifying) */
              [idx1] "r" (index1)    /* Index register */
            : "memory", "cc"
        );
        
        /* Force RELOAD_FOR_OUTADDR_ADDRESS */
        /* Pattern 4: Output address computation */
        int64_t *output_addr = (int64_t *)(reg_b + index2 * 8 + 64);
        
        asm volatile (
            "movq %[input], (%[outaddr])\n\t"
            "mfence"
            : 
            : [input] "r" (reg_c),   /* Input in register */
              [outaddr] "m" (*output_addr) /* Output memory address needing reload */
            : "memory"
        );
        
        /* Force RELOAD_FOR_INPUT and RELOAD_OTHER */
        /* Pattern 5: Multiple constraints that conflict */
        asm volatile (
            "addq %[a], %[b]\n\t"
            "subq %[c], %[d]\n\t"
            "movq %[b], %[result]"
            : [result] "=r" (reg_d)
            : [a] "r" (reg_a),
              [b] "0" (reg_b),      /* Same as result - creates conflict */
              [c] "irm" (i),        /* Mixed constraint */
              [d] "r" (reg_d)       /* Another register variable */
            : "cc"
        );
        
        /* Update register variables to create live range conflicts */
        reg_b = reg_b + 8;
        reg_c = reg_c ^ i;
        reg_d = reg_d * 3 + 1;
    }
}

/* Secondary function with different patterns */
void nested_addressing(int64_t depth) {
    /* Multi-dimensional array access with complex indexing */
    int64_t matrix[8][8][8];
    volatile int64_t *volatile_ptrs[4];
    
    /* Initialize volatile pointers */
    for (int i = 0; i < 4; i++) {
        volatile_ptrs[i] = &volatile_buffer[i * 16];
    }
    
    /* Complex loop with nested addressing */
    for (int64_t x = 0; x < depth; x++) {
        for (int64_t y = 0; y < 8; y++) {
            /* Compute complex index using multiple registers */
            int64_t idx = (x * reg_d + y * reg_e) & 7;
            int64_t idy = (y * reg_f + reg_a) & 7;
            int64_t idz = (reg_b + reg_c) & 7;
            
            /* Access with complex addressing - may need RELOAD_FOR_OTHER_ADDRESS */
            asm volatile (
                "movq (%[mat], %[idx], 64), %[temp1]\n\t"      /* idx * 64 */
                "leaq (%[temp1], %[idy], 8), %[temp2]\n\t"     /* + idy * 8 */
                "movq (%[temp2], %[idz], 8), %[val]\n\t"       /* + idz * 8 */
                "addq %[inc], %[val]\n\t"
                "movq %[val], (%[vptr], %[idx], 8)"
                : [temp1] "=&r" (reg_a),
                  [temp2] "=&r" (reg_b),
                  [val] "=&r" (reg_c)
                : [mat] "r" (&matrix[0][0][0]),
                  [idx] "r" (idx),
                  [idy] "r" (idy),
                  [idz] "r" (idz),
                  [inc] "irm" (x + y),
                  [vptr] "r" (volatile_ptrs[x & 3])
                : "memory", "cc"
            );
            
            /* Additional pressure with fixed register constraints */
            register int64_t fixed1 asm("rbx");
            register int64_t fixed2 asm("rbp");
            
            fixed1 = matrix[idx][idy][idz];
            fixed2 = volatile_ptrs[y & 3][idx];
            
            asm volatile (
                "xchgq %[f1], %[f2]"
                : [f1] "+r" (fixed1),
                  [f2] "+r" (fixed2)
                :
                : "cc"
            );
            
            matrix[idx][idy][idz] = fixed1;
            volatile_ptrs[y & 3][idx] = fixed2;
        }
    }
}

int main(int argc, char *argv[]) {
    int64_t iterations = 100;
    
    if (argc > 1) {
        iterations = atoi(argv[1]);
        if (iterations <= 0) iterations = 100;
        if (iterations > 1000) iterations = 1000;
    }
    
    /* Initialize global data */
    for (int i = 0; i < 64; i++) {
        global_array[i].a = i;
        global_array[i].b = i * 2;
        for (int j = 0; j < 8; j++) {
            global_array[i].c[j] = i + j;
        }
        for (int j = 0; j < 4; j++) {
            global_array[i].d[j] = i * j;
        }
        global_array[i].next = &global_array[(i + 1) & 63];
    }
    
    for (int i = 0; i < 256; i++) {
        volatile_buffer[i] = i;
    }
    
    /* Trigger the reload patterns */
    trigger_reloads(&global_array[0], iterations);
    
    /* Trigger nested addressing patterns */
    nested_addressing(iterations / 10);
    
    /* Final barrier */
    asm volatile ("" ::: "memory");
    
    return 0;
}
