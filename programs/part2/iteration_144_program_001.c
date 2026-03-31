/* reload_stress.c - Stress GCC's reload pass for uncovered switch cases */
#include <stdint.h>

/* Volatile structures with mixed types to prevent optimization */
typedef struct {
    volatile int a;
    volatile double b;
    volatile char c[7];
    volatile int *d;
} MixedType;

typedef struct {
    volatile MixedType arr[100];
    volatile long long big[50];
} Container;

/* Global volatile arrays to force memory accesses */
volatile Container glob_data[10];
volatile int global_index = 0;

/* Helper functions that take pointer-to-pointer arguments */
void modify_pptr(int ***ppp) {
    volatile int dummy = ***ppp;
    (void)dummy;
}

void compute_address(void **addr_ptr) {
    volatile int dummy = (int)(uintptr_t)*addr_ptr;
    (void)dummy;
}

/* Function with complex addressing patterns */
void stress_reloads(int iter) {
    /* Bind specific registers for address computation */
    register volatile MixedType *p1 asm ("r12");
    register volatile long long *p2 asm ("r13");
    register int *p3 asm ("r14");
    register void *p4 asm ("r15");
    
    /* Initialize pointers with complex offsets */
    p1 = &glob_data[iter % 10].arr[iter % 20];
    p2 = &glob_data[(iter + 1) % 10].big[iter % 25];
    p3 = (int *)&p1->a;
    p4 = (void *)&p2;
    
    /* Label for goto jumps */
    addr_computation:
    
    /* Complex address computation forcing RELOAD_FOR_INPUT_ADDRESS */
    volatile int *addr1 = &p1[(iter * 3) % 5].a + (iter & 0xF);
    volatile double *addr2 = &p1[(iter * 7) % 5].b - (iter % 3);
    
    /* Inline assembly with multiple memory operands and clobbers */
    asm volatile (
        "movl %[mem1], %%eax\n\t"
        "addl %[mem2], %%eax\n\t"
        "movl %%eax, %[mem3]\n\t"
        : [mem3] "=m" (p1->a)
        : [mem1] "m" (*addr1),
          [mem2] "m" (*p3),
          "m" (*addr2)  /* Additional memory input */
        : "eax", "r12", "r13", "r14", "memory"
    );
    
    /* Jump to create complex control flow */
    if (iter & 1) {
        goto second_block;
    }
    
    /* Nested function call with address-taken arguments - forcing RELOAD_FOR_INPADDR_ADDRESS */
    {
        int **pptr = &p3;
        int ***ppptr = &pptr;
        modify_pptr(&ppptr);
    }
    
    second_block:
    
    /* More complex addressing with different base */
    volatile char *char_ptr = &p1->c[(iter * 11) % 7];
    volatile long long *ll_ptr = p2 + (iter * 13) % 10;
    
    /* Another inline assembly with conflicting constraints */
    register int idx asm ("ebx") = iter * 17;
    asm volatile (
        "leal (%[base], %[index], 4), %%ecx\n\t"
        "movl (%%ecx), %%edx\n\t"
        "addl %%edx, %[out]\n\t"
        : [out] "+m" (p1->d[idx % 3])
        : [base] "r" (char_ptr),
          [index] "r" (idx)
        : "ecx", "edx", "ebx", "r12", "memory"
    );
    
    /* Compute address for output - forcing RELOAD_FOR_OUTPUT_ADDRESS */
    volatile int *out_addr = &p1[(iter + 2) % 5].a + (iter * 19) % 8;
    
    /* Assembly with output address reload */
    asm volatile (
        "movq %[src], %%rax\n\t"
        "movq %%rax, %[dst]\n\t"
        : [dst] "=m" (*out_addr)
        : [src] "r" (ll_ptr)
        : "rax", "r13", "memory"
    );
    
    /* For RELOAD_FOR_OPERAND_ADDRESS and RELOAD_FOR_OPADDR_ADDR */
    {
        void *addr_array[4];
        addr_array[0] = (void *)p1;
        addr_array[1] = (void *)p2;
        addr_array[2] = (void *)p3;
        addr_array[3] = p4;
        
        /* Complex address chain */
        void **chain_ptr = &addr_array[(iter & 3)];
        compute_address(chain_ptr);
        
        /* Inline asm using the chain */
        asm volatile (
            "movq (%[ptr]), %%r8\n\t"
            "movq %%r8, %[storage]\n\t"
            : [storage] "=m" (p1->d[0])
            : [ptr] "r" (chain_ptr)
            : "r8", "r12", "r15", "memory"
        );
    }
    
    /* Jump back to create loop-like flow */
    if (iter > 0) {
        iter--;
        goto addr_computation;
    }
}

/* Another stress function focusing on output address reloads */
void stress_output_address(void) {
    register volatile Container *cptr asm ("r12");
    register int *iptr asm ("r13");
    
    cptr = &glob_data[2];
    iptr = (int *)&cptr->arr[5].a;
    
    /* Multiple output addresses with complex computation */
    volatile int *out1 = &cptr->arr[global_index].a + global_index * 3;
    volatile double *out2 = &cptr->arr[global_index + 1].b - global_index;
    volatile char *out3 = &cptr->arr[global_index + 2].c[global_index % 7];
    
    /* Inline asm with multiple outputs */
    asm volatile (
        "movl %[in1], %%eax\n\t"
        "movl %%eax, %[out1]\n\t"
        "movsd %[in2], %%xmm0\n\t"
        "movsd %%xmm0, %[out2]\n\t"
        "movb %[in3], %%cl\n\t"
        "movb %%cl, %[out3]\n\t"
        : [out1] "=m" (*out1),
          [out2] "=m" (*out2),
          [out3] "=m" (*out3)
        : [in1] "m" (*iptr),
          [in2] "m" (cptr->arr[3].b),
          [in3] "m" (cptr->arr[4].c[2])
        : "eax", "ecx", "xmm0", "r12", "r13", "memory"
    );
    
    /* For RELOAD_FOR_OUTADDR_ADDRESS */
    {
        volatile int **outaddr_ptr = (volatile int **)&out1;
        asm volatile (
            "movq %[addr], %%rax\n\t"
            "addq $16, %%rax\n\t"
            "movq %%rax, %[ptr]\n\t"
            : [ptr] "=m" (*outaddr_ptr)
            : [addr] "r" (cptr)
            : "rax", "r12", "memory"
        );
    }
}

/* Main function that sets up and calls stress functions */
int main(void) {
    /* Initialize some data */
    for (int i = 0; i < 10; i++) {
        for (int j = 0; j < 100; j++) {
            glob_data[i].arr[j].a = i * 100 + j;
            glob_data[i].arr[j].b = (double)(i + j) * 1.5;
            for (int k = 0; k < 7; k++) {
                glob_data[i].arr[j].c[k] = (char)(i + j + k);
            }
        }
        for (int j = 0; j < 50; j++) {
            glob_data[i].big[j] = (long long)(i * 50 + j) * 1000LL;
        }
    }
    
    /* Call stress functions multiple times with different parameters */
    for (int i = 0; i < 5; i++) {
        stress_reloads(i);
        global_index = i;
        stress_output_address();
        
        /* Additional inline complex addressing in main */
        register volatile MixedType *mp asm ("r12");
        mp = &glob_data[i].arr[i * 3 % 20];
        
        /* Complex expression forcing address reloads */
        volatile int *complex_addr = &mp[(i * 7 + 3) % 10].a + 
                                     (i * 11) % 8 - 
                                     (i * 13) % 5;
        
        /* Final asm with mixed constraints */
        asm volatile (
            "movl (%[base], %[idx], 4), %%eax\n\t"
            "imull %%eax, %%eax\n\t"
            "movl %%eax, %[dest]\n\t"
            : [dest] "=m" (*complex_addr)
            : [base] "r" (mp),
              [idx] "r" (i * 17),
              "m" (mp->b)  /* Additional memory constraint */
            : "eax", "r12", "memory"
        );
    }
    
    return 0;
}
