/* reload1_trigger.c
 * Designed to trigger specific reload types in GCC's reload pass
 * Compile with: gcc -O2 -fno-omit-frame-pointer -fno-schedule-insns -fno-schedule-insns2 reload1_trigger.c -o reload1_trigger
 */

#include <stdint.h>
#include <stdlib.h>

/* Complex structure to force various addressing modes */
struct ComplexStruct {
    int64_t a;
    int64_t b;
    int64_t c[4];
    volatile int64_t d[8];
};

/* Multi-dimensional array with volatile elements */
volatile int64_t multi_array[8][8][8];

/* Explicit register variables - bind to specific registers */
register int64_t reg_a asm("r10");
register int64_t reg_b asm("r11");
register int64_t reg_c asm("r12");
register int64_t reg_d asm("r13");
register int64_t reg_e asm("r14");
register int64_t reg_f asm("r15");

/* Function with complex addressing patterns */
void complex_addressing(struct ComplexStruct *cs1, 
                        struct ComplexStruct *cs2,
                        int64_t *output,
                        int64_t iterations) {
    
    /* Force register variables to hold specific values */
    reg_a = (int64_t)cs1;
    reg_b = (int64_t)cs2;
    reg_c = 0;
    reg_d = 8;
    reg_e = 16;
    reg_f = 24;
    
    /* Loop with nested address computations */
    for (int64_t i = 0; i < iterations; i++) {
        /* Complex pattern 1: Mixed operand types with memory constraint */
        /* This should trigger RELOAD_FOR_INPUT_ADDRESS and RELOAD_FOR_OTHER_ADDRESS */
        asm volatile (
            "movq (%[ptr1], %[idx1], 8), %%rax\n\t"
            "addq %%rax, (%[ptr2], %[idx2], 8)\n\t"
            : /* no outputs */
            : [ptr1] "r" (reg_a),      /* register constraint */
              [idx1] "r" (reg_c),      /* register constraint */
              [ptr2] "r" (reg_b),      /* register constraint */
              [idx2] "r" (reg_d),      /* register constraint */
              "m" (*(struct ComplexStruct*)reg_a),  /* memory constraint forcing address reload */
              "m" (*(struct ComplexStruct*)reg_b)   /* another memory constraint */
            : "rax", "memory", "r10", "r11", "r12", "r13"
        );
        
        /* Complex pattern 2: Multiple address computations with clobbered registers */
        /* Should trigger RELOAD_FOR_INPADDR_ADDRESS and RELOAD_FOR_OPADDR_ADDR */
        int64_t offset1 = i * 16;
        int64_t offset2 = i * 32;
        
        asm volatile (
            "leaq (%[base], %[off1]), %%rbx\n\t"
            "movq (%%rbx), %%rcx\n\t"
            "leaq (%[base2], %[off2]), %%rdx\n\t"
            "addq %%rcx, (%%rdx)\n\t"
            : /* no outputs */
            : [base] "r" (reg_a),
              [off1] "r" (offset1),
              [base2] "r" (reg_b),
              [off2] "r" (offset2),
              "m" (cs1->d[0]),    /* volatile memory access */
              "m" (cs2->d[0])     /* another volatile access */
            : "rbx", "rcx", "rdx", "memory", "r10", "r11"
        );
        
        /* Complex pattern 3: Multi-dimensional array access with computed indices */
        /* Should trigger various address reload types */
        int64_t idx1 = (reg_c + i) & 7;
        int64_t idx2 = (reg_d + i * 2) & 7;
        int64_t idx3 = (reg_e + i * 3) & 7;
        
        /* Force address computation for multi-dimensional array */
        asm volatile (
            "imulq $64, %[i2], %%rax\n\t"      /* idx2 * 8 * 8 */
            "imulq $8, %[i3], %%rbx\n\t"       /* idx3 * 8 */
            "addq %%rbx, %%rax\n\t"
            "addq %[i1], %%rax\n\t"            /* + idx1 */
            "movq %[arr], %%rcx\n\t"
            "movq (%%rcx, %%rax, 8), %%rdx\n\t"
            "addq $1, %%rdx\n\t"
            "movq %%rdx, (%%rcx, %%rax, 8)\n\t"
            : /* no outputs */
            : [arr] "r" (multi_array),   /* array base address */
              [i1] "r" (idx1),           /* register constraint */
              [i2] "r" (idx2),           /* register constraint */
              [i3] "r" (idx3),           /* register constraint */
              "m" (multi_array[0][0][0]) /* memory constraint forcing address reload */
            : "rax", "rbx", "rcx", "rdx", "memory"
        );
        
        /* Pattern 4: Complex constraints with immediate and memory mix */
        /* Should trigger RELOAD_FOR_OPERAND_ADDRESS */
        int64_t immediate_val = 42;
        int64_t* volatile ptr = (int64_t*)reg_a + i;
        
        asm volatile (
            "addq %[imm], (%[mem])\n\t"
            : /* no outputs */
            : [imm] "i" (immediate_val),  /* immediate constraint */
              [mem] "m" (*ptr),           /* memory constraint with complex address */
              "r" (reg_a),                /* register tied to address computation */
              "r" (reg_b)
            : "memory", "r10", "r11"
        );
        
        /* Pattern 5: Output address reloads */
        /* Should trigger RELOAD_FOR_OUTPUT_ADDRESS and RELOAD_FOR_OUTADDR_ADDRESS */
        int64_t* out_addr = output + i;
        
        asm volatile (
            "movq (%[in]), %%rax\n\t"
            "movq %%rax, (%[out])\n\t"
            : "=m" (*out_addr)    /* memory output */
            : [in] "r" (cs1),     /* input register */
              [out] "r" (out_addr) /* output address register */
            : "rax", "memory"
        );
        
        /* Update register variables to create live value pressure */
        reg_c += 1;
        reg_d += 2;
        reg_e += 3;
        reg_f += 4;
        
        /* Force spill/reload of register variables */
        asm volatile (
            "addq $1, %[r12]\n\t"
            "subq $1, %[r13]\n\t"
            : [r12] "+r" (reg_c),
              [r13] "+r" (reg_d)
            : /* no inputs */
            : "cc"
        );
    }
}

/* Second function with different patterns */
void other_address_patterns(volatile int64_t* ptrs[8], int64_t n) {
    register int64_t idx_reg asm("r10");
    register int64_t base_reg asm("r11");
    register int64_t temp_reg asm("r12");
    
    idx_reg = 0;
    base_reg = (int64_t)ptrs;
    
    for (int64_t i = 0; i < n; i++) {
        /* Complex address chain */
        volatile int64_t* ptr1 = ptrs[i & 7] + (idx_reg * 2);
        volatile int64_t* ptr2 = ptrs[(i + 1) & 7] + (idx_reg * 3);
        
        /* Should trigger RELOAD_FOR_OTHER_ADDRESS specifically */
        asm volatile (
            "movq (%[addr1]), %%rax\n\t"
            "movq (%[addr2]), %%rbx\n\t"
            "addq %%rbx, %%rax\n\t"
            "movq %%rax, (%[addr3])\n\t"
            : /* no outputs */
            : [addr1] "r" (ptr1),      /* address that may need reloading */
              [addr2] "r" (ptr2),      /* another address */
              [addr3] "r" (ptrs[i & 7]), /* yet another address */
              "m" (*ptr1),             /* memory constraints */
              "m" (*ptr2),
              "m" (*(ptrs[i & 7]))
            : "rax", "rbx", "memory", "r10", "r11", "r12"
        );
        
        /* Nested address computation in loop */
        temp_reg = i * 16;
        asm volatile (
            "leaq (%[base], %[idx], 8), %%rcx\n\t"
            "movq (%%rcx), %%rdx\n\t"
            "addq %[inc], %%rdx\n\t"
            "movq %%rdx, (%%rcx)\n\t"
            : /* no outputs */
            : [base] "r" (base_reg),
              [idx] "r" (temp_reg),
              [inc] "i" (100),        /* immediate */
              "m" (*(int64_t*)(base_reg + temp_reg * 8)) /* complex memory constraint */
            : "rcx", "rdx", "memory", "cc"
        );
        
        idx_reg += i;
    }
}

int main() {
    /* Allocate and initialize complex structures */
    struct ComplexStruct cs1, cs2;
    int64_t output[100];
    volatile int64_t* ptr_array[8];
    
    /* Initialize structures */
    for (int i = 0; i < 4; i++) {
        cs1.c[i] = i * 10;
        cs2.c[i] = i * 20;
    }
    for (int i = 0; i < 8; i++) {
        cs1.d[i] = i * 100;
        cs2.d[i] = i * 200;
    }
    
    /* Initialize pointer array */
    for (int i = 0; i < 8; i++) {
        ptr_array[i] = (volatile int64_t*)malloc(64 * sizeof(int64_t));
        for (int j = 0; j < 64; j++) {
            ptr_array[i][j] = i * 1000 + j;
        }
    }
    
    /* Initialize multi-dimensional array */
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            for (int k = 0; k < 8; k++) {
                multi_array[i][j][k] = i * 100 + j * 10 + k;
            }
        }
    }
    
    /* Call functions with complex addressing patterns */
    complex_addressing(&cs1, &cs2, output, 50);
    other_address_patterns(ptr_array, 32);
    
    /* Cleanup */
    for (int i = 0; i < 8; i++) {
        free((void*)ptr_array[i]);
    }
    
    return 0;
}
