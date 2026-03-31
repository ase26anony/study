/* reload_trigger.c
 * Designed to trigger various reload types in GCC's reload1.cc
 * Specifically targets lines 7146-7174 (switch case for RELOAD_OTHER through RELOAD_FOR_OTHER_ADDRESS)
 */

#include <stdint.h>
#include <stdlib.h>

/* Complex structure to force non-trivial addressing */
struct MultiDim {
    int data[4][8][16];
    volatile int* volatile ptrs[8];
    long long padding[3];
};

/* Global volatile structures to prevent optimization */
volatile struct MultiDim global_struct;
volatile int global_array[256];

/* Explicit register variables - will force specific register usage */
register int reg_a asm("r10");
register int reg_b asm("r11");
register int reg_c asm("r12");
register int reg_d asm("r13");
register int reg_e asm("r14");
register int reg_f asm("r15");

/* Function with complex addressing patterns */
void complex_addressing(int iterations) {
    /* Local arrays with different alignments/sizes */
    int local_array[128] __attribute__((aligned(64)));
    volatile int* volatile local_ptrs[32];
    
    /* Initialize some data */
    for (int i = 0; i < 128; i++) {
        local_array[i] = i * 3;
    }
    
    for (int i = 0; i < 32; i++) {
        local_ptrs[i] = &local_array[i * 4];
    }
    
    /* Set up explicit register variables */
    reg_a = (int)(uintptr_t)&global_struct;
    reg_b = (int)(uintptr_t)local_array;
    reg_c = 42;
    reg_d = 17;
    reg_e = 8;
    reg_f = 23;
    
    /* Main loop with complex addressing modes */
    for (int i = 0; i < iterations; i++) {
        /* Pattern 1: Mixed operand types with memory constraint */
        /* Forces RELOAD_FOR_INPUT_ADDRESS and RELOAD_FOR_OTHER_ADDRESS */
        asm volatile (
            "/* Complex addressing pattern 1 */\n\t"
            "movl %[idx], %%eax\n\t"
            "leal (%%eax, %%eax, 2), %%ecx\n\t"
            "addl %[const1], %%ecx\n\t"
            "movl (%%r12, %%rcx, 4), %%edx\n\t"
            "addl %%edx, %[out1]\n\t"
            : [out1] "+r" (reg_a)
            : [idx] "irm" (i),           /* Immediate, register, or memory */
              [const1] "irm" (reg_c),    /* Mix of immediate and register */
              "m" (*(struct MultiDim*)(uintptr_t)reg_b),  /* Memory constraint forcing address reload */
              "r" (reg_d), "r" (reg_e)
            : "eax", "ecx", "edx", "memory"
        );
        
        /* Pattern 2: Nested address computation with explicit registers */
        /* Forces RELOAD_FOR_INPADDR_ADDRESS and RELOAD_FOR_OPADDR_ADDR */
        int temp_idx = (i * reg_d + reg_e) & 0x7F;
        asm volatile (
            "/* Complex addressing pattern 2 */\n\t"
            "movl %[base], %%ebx\n\t"
            "movl %[offset], %%esi\n\t"
            "leal (%%ebx, %%esi, 4), %%edi\n\t"
            "movl (%%edi), %%r8d\n\t"
            "imull %[scale], %%r8d\n\t"
            "addl %%r8d, %[accum]\n\t"
            : [accum] "+r" (reg_f)
            : [base] "r" (reg_b),
              [offset] "r" (temp_idx),
              [scale] "irm" (reg_c),     /* Mixed constraint */
              "m" (local_array[temp_idx]) /* Memory operand forcing address reload */
            : "ebx", "esi", "edi", "r8", "memory"
        );
        
        /* Pattern 3: Multiple memory operands with different addressing */
        /* Forces RELOAD_FOR_OUTPUT_ADDRESS and RELOAD_FOR_OUTADDR_ADDRESS */
        int idx1 = (i + reg_a) & 0x7;
        int idx2 = (i * 3 + reg_b) & 0xF;
        int idx3 = (i * 5 + reg_c) & 0xF;
        
        asm volatile (
            "/* Complex addressing pattern 3 */\n\t"
            "movl %[val1], %%r9d\n\t"
            "addl %[val2], %%r9d\n\t"
            "movl %%r9d, %[dest]\n\t"
            : [dest] "=m" (global_struct.data[idx1][idx2][idx3])  /* Complex memory destination */
            : [val1] "r" (reg_d),
              [val2] "irm" (reg_e),      /* Mixed constraint */
              "m" (global_array[idx1 * 32 + idx2 * 2 + idx3])  /* Memory source */
            : "r9", "memory"
        );
        
        /* Pattern 4: Operand address reload with volatile pointers */
        /* Forces RELOAD_FOR_OPERAND_ADDRESS */
        volatile int* volatile ptr1 = &global_array[i & 0xFF];
        volatile int* volatile ptr2 = local_ptrs[(i * 7) & 0x1F];
        
        asm volatile (
            "/* Complex addressing pattern 4 */\n\t"
            "movl (%[ptr1]), %%r10d\n\t"
            "movl (%[ptr2]), %%r11d\n\t"
            "subl %%r11d, %%r10d\n\t"
            "movl %%r10d, %[result]\n\t"
            : [result] "=r" (reg_a)
            : [ptr1] "r" (ptr1),         /* Register containing address */
              [ptr2] "r" (ptr2),         /* Another address in register */
              "m" (*ptr1), "m" (*ptr2)   /* Memory constraints forcing reloads */
            : "r10", "r11", "memory"
        );
        
        /* Pattern 5: Other address reload scenarios */
        /* Forces RELOAD_OTHER and RELOAD_FOR_OTHER_ADDRESS */
        struct MultiDim* local_struct_ptr = (struct MultiDim*)&local_array[0];
        
        asm volatile (
            "/* Complex addressing pattern 5 */\n\t"
            "movq %[struct_ptr], %%rax\n\t"
            "movl %[idx_a], %%ebx\n\t"
            "movl %[idx_b], %%ecx\n\t"
            "movl %[idx_c], %%edx\n\t"
            "imull $128, %%ebx\n\t"
            "imull $16, %%ecx\n\t"
            "leal (%%rax, %%rbx), %%rsi\n\t"
            "leal (%%rsi, %%rcx), %%rdi\n\t"
            "leal (%%rdi, %%rdx, 4), %%r8\n\t"
            "movl (%%r8), %%r9d\n\t"
            "addl %%r9d, %[sum]\n\t"
            : [sum] "+r" (reg_b)
            : [struct_ptr] "r" (local_struct_ptr),
              [idx_a] "irm" (i & 0x3),
              [idx_b] "irm" ((i >> 2) & 0x7),
              [idx_c] "irm" ((i >> 5) & 0xF),
              "m" (local_struct_ptr->data[0][0][0])  /* Forces address reload */
            : "rax", "rbx", "rcx", "rdx", "rsi", "rdi", "r8", "r9", "memory"
        );
        
        /* Rotate register values to create different pressure patterns */
        int temp = reg_a;
        reg_a = reg_b;
        reg_b = reg_c;
        reg_c = reg_d;
        reg_d = reg_e;
        reg_e = reg_f;
        reg_f = temp;
    }
}

/* Secondary function with different patterns */
void secondary_patterns(int count) {
    /* Array of pointers with complex indexing */
    void* ptr_array[64];
    volatile long long results[16];
    
    /* Initialize */
    for (int i = 0; i < 64; i++) {
        ptr_array[i] = (void*)(uintptr_t)(i * 16);
    }
    
    register void* reg_ptr1 asm("r10") = ptr_array;
    register void* reg_ptr2 asm("r11") = &results[0];
    register int reg_idx asm("r12") = count;
    
    for (int j = 0; j < count; j++) {
        /* Complex address chain */
        asm volatile (
            "/* Secondary pattern - address chain */\n\t"
            "movq %[base], %%rax\n\t"
            "movl %[index], %%ebx\n\t"
            "shll $3, %%ebx\n\t"
            "addq %%rbx, %%rax\n\t"
            "movq (%%rax), %%rcx\n\t"
            "movq %%rcx, %[dest]\n\t"
            : [dest] "=m" (results[j & 0xF])
            : [base] "r" (reg_ptr1),
              [index] "irm" (reg_idx + j * 3),
              "m" (*(char**)reg_ptr1)    /* Forces address reload */
            : "rax", "rbx", "rcx", "memory"
        );
        
        reg_idx += 7;
    }
}

int main() {
    /* Initialize global data */
    for (int i = 0; i < 256; i++) {
        global_array[i] = i;
    }
    
    /* Call functions with complex addressing patterns */
    complex_addressing(100);
    secondary_patterns(50);
    
    /* Final mixed pattern in main */
    register int final_acc asm("r10") = 0;
    volatile int* volatile final_ptr = &global_array[0];
    
    for (int k = 0; k < 20; k++) {
        asm volatile (
            "/* Final mixed pattern */\n\t"
            "movl (%[ptr]), %%eax\n\t"
            "addl %%eax, %[acc]\n\t"
            "leal 4(%[ptr]), %[ptr]\n\t"
            : [acc] "+r" (final_acc),
              [ptr] "+r" (final_ptr)
            : "m" (*final_ptr)           /* Will force various reload types */
            : "eax", "memory"
        );
    }
    
    /* Prevent dead code elimination */
    asm volatile ("" : : "r"(final_acc) : "memory");
    
    return final_acc != 0;
}
