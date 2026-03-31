/* reload_test.c - Complex program to exercise GCC reload machinery */
#include <stdint.h>
#include <stdlib.h>

/* Volatile structure to prevent optimization */
struct VolatileData {
    volatile int64_t a;
    volatile int64_t b;
    volatile int64_t c[4];
    volatile int64_t d[2][3];
};

/* Global volatile data to force memory accesses */
volatile struct VolatileData vdata[10];
volatile int64_t global_array[100];

/* Explicit register variables - bind to specific registers */
register int64_t reg_a asm("r10");
register int64_t reg_b asm("r11");
register int64_t reg_c asm("r12");
register int64_t reg_d asm("r13");
register int64_t reg_e asm("r14");
register int64_t reg_f asm("r15");

/* Complex addressing mode with structure member access */
static inline void complex_address_op(int64_t idx1, int64_t idx2, int64_t idx3) {
    /* Force RELOAD_FOR_OTHER_ADDRESS and RELOAD_FOR_INPUT_ADDRESS */
    asm volatile (
        /* Complex memory addressing with multiple index registers */
        "lea (%[base], %[idx1], 8), %%rax\n\t"
        "add (%[base], %[idx2], 8), %%rax\n\t"
        "mov %%rax, (%[base], %[idx3], 8)\n\t"
        : 
        : [base] "r" (&global_array[0]), 
          [idx1] "r" (idx1), 
          [idx2] "r" (idx2), 
          [idx3] "r" (idx3)
        : "rax", "memory", "cc"
    );
}

/* Operation requiring RELOAD_FOR_OPERAND_ADDRESS */
static inline void operand_address_reload(int64_t *ptr, int64_t offset) {
    /* Mixed constraints forcing address reload */
    asm volatile (
        "mov (%[ptr], %[off], 8), %%rbx\n\t"
        "add $0x12345678, %%rbx\n\t"
        "mov %%rbx, (%[ptr], %[off], 8)\n\t"
        : 
        : [ptr] "r" (ptr), 
          [off] "r" (offset),
          "m" (*(volatile int64_t (*)[100])ptr)  /* Memory constraint forces address reload */
        : "rbx", "memory", "cc"
    );
}

/* Force RELOAD_FOR_INPADDR_ADDRESS and RELOAD_FOR_OUTADDR_ADDRESS */
static inline void inout_address_reloads(struct VolatileData *vptr, int64_t idx) {
    int64_t temp;
    
    /* Input address reload */
    asm volatile (
        "mov 0(%[vptr], %[idx], 1), %[temp]\n\t"
        : [temp] "=r" (temp)
        : [vptr] "r" (vptr),
          [idx] "r" (idx * sizeof(struct VolatileData)),
          "m" (vptr->a)  /* Memory constraint for structure member */
        : "cc"
    );
    
    /* Output address reload */
    asm volatile (
        "mov %[val], 8(%[vptr], %[idx], 1)\n\t"
        : 
        : [val] "r" (temp + 1),
          [vptr] "r" (vptr),
          [idx] "r" (idx * sizeof(struct VolatileData)),
          "m" (vptr->b)  /* Memory constraint forces address calculation */
        : "memory", "cc"
    );
}

/* Complex loop with nested addressing */
static void nested_addressing_loop(int64_t iterations) {
    /* Use explicit register variables to increase pressure */
    reg_a = 0;
    reg_b = 1;
    reg_c = 2;
    reg_d = 3;
    reg_e = 4;
    reg_f = 5;
    
    volatile int64_t * volatile ptr1 = &global_array[0];
    volatile int64_t * volatile ptr2 = &global_array[50];
    
    for (int64_t i = 0; i < iterations; i++) {
        /* Force multiple reload types in a single loop iteration */
        
        /* 1. RELOAD_FOR_INPUT_ADDRESS with complex index */
        asm volatile (
            "mov (%[base], %[idx], 8), %%rcx\n\t"
            "imul %%rcx, %[reg_a]\n\t"
            : [reg_a] "+r" (reg_a)
            : [base] "r" (ptr1),
              [idx] "r" (i),
              "m" (ptr1[i])  /* Memory constraint */
            : "rcx", "cc"
        );
        
        /* 2. RELOAD_FOR_OTHER_ADDRESS with structure access */
        int64_t struct_idx = (i * reg_b) % 10;
        asm volatile (
            "mov %[reg_c], (%[vptr], %[idx], 1)\n\t"
            : 
            : [vptr] "r" (&vdata[0]),
              [idx] "r" (struct_idx * sizeof(struct VolatileData)),
              [reg_c] "r" (reg_c),
              "m" (vdata[struct_idx])  /* Complex memory constraint */
            : "memory"
        );
        
        /* 3. RELOAD_FOR_OPERAND_ADDRESS with immediate */
        asm volatile (
            "add $0x7FFF, (%[ptr], %[reg_d], 8)\n\t"
            : 
            : [ptr] "r" (ptr2),
              [reg_d] "r" (reg_d),
              "m" (*(volatile int64_t (*)[100])ptr2)  /* Forces operand address reload */
            : "memory", "cc"
        );
        
        /* 4. Mix register variables to force spills/reloads */
        asm volatile (
            "mov %[reg_e], %[reg_f]\n\t"
            "add %[reg_a], %[reg_f]\n\t"
            : [reg_f] "=r" (reg_f)
            : [reg_e] "r" (reg_e),
              [reg_a] "r" (reg_a)
            : "cc"
        );
        
        /* 5. Complex addressing with multiple components */
        complex_address_op(reg_a % 20, reg_b % 20, reg_c % 20);
        
        /* 6. Force RELOAD_FOR_OPADDR_ADDR */
        operand_address_reload((int64_t *)ptr1, reg_d % 10);
        
        /* 7. Input/output address reloads */
        inout_address_reloads(&vdata[0], struct_idx);
        
        /* Update register variables to create dependencies */
        reg_a = (reg_a * 1103515245 + 12345) & 0x7FFFFFFF;
        reg_b = (reg_b * 1664525 + 1013904223) & 0x7FFFFFFF;
        reg_c = reg_c ^ reg_d;
        reg_d = reg_d + reg_e;
        reg_e = reg_e * reg_f;
    }
}

/* Additional test with multi-dimensional array */
static void multi_dim_addressing(void) {
    volatile int64_t md_array[5][7][3];
    
    /* Force complex address computations */
    for (int i = 0; i < 5; i++) {
        for (int j = 0; j < 7; j++) {
            for (int k = 0; k < 3; k++) {
                /* Address computation involving multiple registers */
                int64_t idx = i * 21 + j * 3 + k;
                
                /* RELOAD_FOR_OUTPUT_ADDRESS scenario */
                asm volatile (
                    "mov %[val], (%[base], %[idx], 8)\n\t"
                    : 
                    : [val] "r" (idx * idx),
                      [base] "r" (&md_array[0][0][0]),
                      [idx] "r" (idx),
                      "m" (md_array[i][j][k])  /* Memory constraint for specific element */
                    : "memory"
                );
                
                /* RELOAD_FOR_INPUT_ADDRESS with different index */
                int64_t idx2 = (i + j + k) % 100;
                asm volatile (
                    "mov (%[arr], %[idx2], 8), %%rdx\n\t"
                    "add %%rdx, %[reg]\n\t"
                    : [reg] "+r" (reg_a)
                    : [arr] "r" (&global_array[0]),
                      [idx2] "r" (idx2),
                      "m" (global_array[idx2])  /* Memory constraint */
                    : "rdx", "cc"
                );
            }
        }
    }
}

int main(void) {
    /* Initialize data */
    for (int i = 0; i < 100; i++) {
        global_array[i] = i * i;
    }
    
    for (int i = 0; i < 10; i++) {
        vdata[i].a = i;
        vdata[i].b = i * 2;
        for (int j = 0; j < 4; j++) {
            vdata[i].c[j] = i + j;
        }
        for (int j = 0; j < 2; j++) {
            for (int k = 0; k < 3; k++) {
                vdata[i].d[j][k] = i * j * k;
            }
        }
    }
    
    /* Execute tests designed to trigger specific reload types */
    
    /* Test 1: Nested addressing with register pressure */
    nested_addressing_loop(100);
    
    /* Test 2: Multi-dimensional array addressing */
    multi_dim_addressing();
    
    /* Test 3: Additional complex inline asm patterns */
    
    /* Force RELOAD_FOR_OTHER_ADDRESS with pointer arithmetic */
    volatile int64_t *ptr = &global_array[0];
    for (int i = 0; i < 50; i++) {
        asm volatile (
            /* Complex addressing with scaled index */
            "mov (%[ptr], %[idx], 8), %%r8\n\t"
            "mov 8(%[ptr], %[idx], 8), %%r9\n\t"
            "add %%r9, %%r8\n\t"
            "mov %%r8, 16(%[ptr], %[idx], 8)\n\t"
            : 
            : [ptr] "r" (ptr),
              [idx] "r" (i),
              "m" (ptr[i]),        /* Input address reload */
              "m" (ptr[i + 1]),    /* Another input address */
              "m" (ptr[i + 2])     /* Output address reload */
            : "r8", "r9", "memory", "cc"
        );
    }
    
    /* Use the results to prevent dead code elimination */
    asm volatile ("" : : "r" (reg_a), "r" (reg_b), "r" (reg_c), 
                       "r" (reg_d), "r" (reg_e), "r" (reg_f));
    
    return 0;
}
