/* reload1_trigger.c
 * Designed to trigger specific reload types in GCC's reload pass
 * Compile with: gcc -O2 -fno-omit-frame-pointer -fno-schedule-insns -fno-schedule-insns2 reload1_trigger.c -o reload1_trigger
 */

#include <stdint.h>
#include <stdlib.h>

/* Complex structure to force various addressing modes */
struct ComplexStruct {
    int64_t data[8];
    int32_t indices[16];
    volatile int32_t volatile_field;
    char padding[64];
};

/* Global arrays to create addressing complexity */
static struct ComplexStruct global_array[32];
static volatile int64_t volatile_global[100];

/* Explicit register variables - force specific register allocation */
register int64_t reg_a asm("r10");
register int64_t reg_b asm("r11");
register int64_t reg_c asm("r12");
register int64_t reg_d asm("r13");
register int64_t reg_e asm("r14");

/* Function with complex addressing patterns */
void complex_addressing(int iterations) {
    /* Local variables with different storage classes */
    struct ComplexStruct local_struct;
    int64_t local_array[64];
    volatile int64_t* volatile_ptr = &volatile_global[0];
    
    /* Initialize explicit register variables */
    reg_a = (int64_t)&local_struct;
    reg_b = (int64_t)&global_array[0];
    reg_c = (int64_t)local_array;
    reg_d = iterations;
    reg_e = 0;
    
    /* Loop with nested address computations */
    for (int i = 0; i < iterations; i++) {
        /* Complex pattern 1: Multiple memory operands with different addressing modes */
        asm volatile (
            /* Force RELOAD_FOR_INPUT_ADDRESS and RELOAD_FOR_OTHER_ADDRESS */
            "movq (%[base1], %[idx1], 8), %%rax\n\t"
            "addq %%rax, (%[base2], %[idx2], 4)\n\t"
            : /* no outputs */
            : [base1] "r" (reg_b),          /* register constraint */
              [idx1] "r" (reg_d),           /* register constraint */
              [base2] "r" (reg_a),          /* register constraint */
              [idx2] "r" (i),               /* immediate/register constraint */
              "m" (*(struct ComplexStruct*)reg_a),  /* memory constraint forcing address reload */
              "m" (global_array[0].data[0])         /* another memory constraint */
            : "rax", "memory", "cc"
        );
        
        /* Complex pattern 2: Mixed operand types with explicit register clobbering */
        int64_t temp;
        asm volatile (
            /* Force RELOAD_FOR_INPADDR_ADDRESS and RELOAD_FOR_OPADDR_ADDR */
            "leaq (%[ptr], %[offset], 1), %%r15\n\t"
            "movq (%%r15), %[temp]\n\t"
            "imulq %[imm], %[temp]\n\t"
            : [temp] "=r" (temp)
            : [ptr] "r" (volatile_ptr),     /* pointer in register */
              [offset] "irm" (i * 8),       /* immediate, register, or memory - creates conflicts */
              [imm] "irm" (42),             /* immediate that might need reloading */
              "m" (*volatile_ptr)           /* memory constraint on volatile */
            : "r15", "memory"
        );
        
        /* Complex pattern 3: Address computation for structure field access */
        int32_t index = i & 0xF;
        asm volatile (
            /* Force RELOAD_FOR_OUTPUT_ADDRESS and RELOAD_FOR_OUTADDR_ADDRESS */
            "movl %[idx], %%ebx\n\t"
            "leaq (%[base], %%rbx, 4), %%rcx\n\t"
            "movl (%%rcx), %%edx\n\t"
            "addl %%edx, %[out]\n\t"
            : [out] "+m" (local_struct.indices[index])  /* memory output with complex address */
            : [base] "r" (&local_struct.indices[0]),    /* base address in register */
              [idx] "irm" (index)                       /* mixed constraint */
            : "rbx", "rcx", "rdx", "memory", "cc"
        );
        
        /* Complex pattern 4: Multiple explicit register variables in addressing */
        asm volatile (
            /* Force RELOAD_FOR_OPERAND_ADDRESS and RELOAD_FOR_OTHER_ADDRESS */
            "addq %[val1], %[val2]\n\t"
            "movq %[val2], (%[addr])\n\t"
            : 
            : [val1] "r" (reg_c),           /* explicit register variable */
              [val2] "r" (reg_e),           /* explicit register variable */
              [addr] "r" (reg_a),           /* address in explicit register */
              "m" (*(int64_t*)reg_a)        /* memory constraint forcing address reload */
            : "memory"
        );
        
        /* Update explicit register variables - forces spilling/reloading */
        reg_c += 8;
        reg_e += temp;
        
        /* Complex pattern 5: Volatile access with address computation */
        asm volatile (
            /* Force various address reload types */
            "movq %[offset], %%r8\n\t"
            "addq %[base], %%r8\n\t"
            "movq (%%r8), %%r9\n\t"
            "addq %%r9, %[sum]\n\t"
            : [sum] "+r" (reg_d)
            : [base] "r" (&volatile_global[0]),
              [offset] "irm" (i * 8),       /* creates addressing mode conflicts */
              "m" (volatile_global[0])      /* volatile memory constraint */
            : "r8", "r9", "memory"
        );
        
        /* Nested addressing with multiple steps */
        int64_t* ptr1 = &local_array[i];
        int64_t* ptr2 = &global_array[i % 32].data[0];
        
        asm volatile (
            /* Complex addressing chain */
            "movq (%[src]), %%rax\n\t"
            "movq %%rax, (%[dst], %[idx], 8)\n\t"
            "addq $1, (%[dst], %[idx], 8)\n\t"
            : 
            : [src] "r" (ptr2),             /* pointer in register */
              [dst] "r" (ptr1),             /* pointer in register */
              [idx] "r" (i & 7),            /* register index */
              "m" (*ptr2),                  /* memory input */
              "m" (ptr1[0])                 /* memory output with complex address */
            : "rax", "memory", "cc"
        );
    }
    
    /* Final pattern: Complex return value computation */
    int64_t result;
    asm volatile (
        /* Multiple address computations in one asm */
        "leaq (%[a], %[b], 2), %%rax\n\t"
        "addq (%[c]), %%rax\n\t"
        "movq %%rax, %[res]\n\t"
        : [res] "=r" (result)
        : [a] "r" (reg_a),
          [b] "r" (reg_b),
          [c] "r" (&reg_c),                 /* address of register variable */
          "m" (reg_c)                       /* forces address reload */
        : "rax", "memory"
    );
    
    /* Use result to prevent optimization */
    volatile_global[0] = result;
}

/* Second function with different addressing patterns */
void another_complex_function(void) {
    /* Array of pointers for indirection */
    int64_t* ptr_array[16];
    for (int i = 0; i < 16; i++) {
        ptr_array[i] = &global_array[i].data[0];
    }
    
    /* Use explicit register variables differently */
    register int64_t reg_x asm("r10");
    register int64_t reg_y asm("r11");
    
    reg_x = (int64_t)ptr_array;
    reg_y = 0;
    
    for (int i = 0; i < 100; i++) {
        /* Pattern forcing RELOAD_FOR_INPUT and RELOAD_FOR_OTHER_ADDRESS */
        int64_t* current_ptr;
        asm volatile (
            "movq (%[base], %[idx], 8), %[ptr]\n\t"
            "movq (%[ptr]), %%rax\n\t"
            "addq %%rax, %[sum]\n\t"
            : [ptr] "=r" (current_ptr),
              [sum] "+r" (reg_y)
            : [base] "r" (reg_x),
              [idx] "irm" ((i % 16) * 8),   /* mixed constraint */
              "m" (*(int64_t**)reg_x)       /* memory constraint on pointer array */
            : "rax", "memory"
        );
        
        /* Additional computation on the loaded pointer */
        asm volatile (
            "addq $8, %[ptr]\n\t"
            "movq %[ptr], (%[base], %[idx], 8)\n\t"
            : 
            : [ptr] "r" (current_ptr),
              [base] "r" (reg_x),
              [idx] "r" (i % 16),
              "m" (ptr_array[0])            /* memory constraint */
            : "memory"
        );
    }
    
    volatile_global[1] = reg_y;
}

/* Main function to drive everything */
int main(int argc, char** argv) {
    int iterations = 100;
    if (argc > 1) {
        iterations = atoi(argv[1]);
        if (iterations <= 0) iterations = 100;
        if (iterations > 1000) iterations = 1000;
    }
    
    /* Initialize global data */
    for (int i = 0; i < 32; i++) {
        for (int j = 0; j < 8; j++) {
            global_array[i].data[j] = i * 100 + j;
        }
        for (int j = 0; j < 16; j++) {
            global_array[i].indices[j] = i + j;
        }
        global_array[i].volatile_field = i;
    }
    
    for (int i = 0; i < 100; i++) {
        volatile_global[i] = i * 2;
    }
    
    /* Call functions with complex addressing */
    complex_addressing(iterations);
    another_complex_function();
    
    /* Final volatile store to prevent dead code elimination */
    volatile int64_t final_result = 0;
    for (int i = 0; i < iterations; i++) {
        final_result += volatile_global[i % 100];
    }
    
    volatile_global[99] = final_result;
    
    return (int)(final_result & 0x7FFFFFFF);
}
