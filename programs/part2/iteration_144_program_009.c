/* reload_stress.c - Stress GCC's reload pass for uncovered cases */
#include <stdint.h>

/* Volatile structures with mixed types to prevent optimization */
typedef struct {
    volatile int a;
    volatile double b;
    volatile char c[7];
    volatile int64_t d;
} MixedType;

typedef struct {
    volatile MixedType *ptr;
    volatile int offset;
} AddrHolder;

/* Global volatile arrays to force memory accesses */
volatile MixedType global_array[256];
volatile AddrHolder addr_holders[16];

/* Helper functions that take pointer-to-pointer arguments */
void update_pointer(volatile MixedType ***ppp) {
    volatile MixedType **temp = *ppp;
    if (temp) {
        /* Force a memory barrier */
        asm volatile("" ::: "memory");
    }
}

void compute_address(volatile int64_t **addr_ptr, int offset) {
    volatile int64_t *temp = *addr_ptr;
    if (temp) {
        /* Complex address computation */
        *addr_ptr = temp + offset * 3 - 7;
    }
}

/* Function with complex addressing patterns */
void stress_reloads(int iterations) {
    /* Bind specific variables to registers */
    register volatile MixedType *p1 asm ("r12") = &global_array[0];
    register volatile MixedType *p2 asm ("r13") = &global_array[128];
    register int idx asm ("r14") = 0;
    
    volatile int64_t *addr_chain = (volatile int64_t *)&global_array[0].d;
    volatile MixedType **ptr_to_ptr = (volatile MixedType **)&addr_holders[0].ptr;
    
    /* Label for goto jumps */
    compute_again:
    
    for (int i = 0; i < iterations; i++) {
        /* Complex address computation with register-bound variables */
        volatile MixedType *temp1 = p1 + idx * 3 - (i % 4);
        volatile MixedType *temp2 = p2 - idx * 2 + (i % 3);
        
        /* Inline assembly with multiple memory operands and clobbers */
        asm volatile (
            "movq %[src], %%r15\n\t"
            "addq $16, %%r15\n\t"
            "movq %%r15, %[dst]\n\t"
            : [dst] "=m" (temp1->d)  /* Memory output */
            : [src] "r" (&temp2->b),  /* Register input */
              "0" (temp1->d)          /* Same as output */
            : "r15", "memory", "r12", "r13"  /* Clobber address registers */
        );
        
        /* Another asm with conflicting constraints */
        volatile int64_t offset_result;
        asm volatile (
            "leaq (%[base], %[index], 8), %%rbx\n\t"
            "movq (%%rbx), %%rcx\n\t"
            "addq $1, %%rcx\n\t"
            "movq %%rcx, %[out]\n\t"
            : [out] "=r" (offset_result)      /* Register output */
            : [base] "r" (addr_chain),        /* Register input */
              [index] "r" (idx)               /* Register input */
            : "rbx", "rcx", "memory", "r12", "r13", "r14"
        );
        
        /* Use computed address in memory operation */
        temp1->a = (int)offset_result;
        
        /* Nested function call with address-taken argument */
        update_pointer(&ptr_to_ptr);
        
        /* Compute new address chain */
        compute_address(&addr_chain, idx);
        
        /* Scattered, non-sequential access */
        volatile MixedType *scattered = &global_array[
            ((idx * 17) % 256) ^ ((i * 13) % 256)
        ];
        
        /* Mixed type access with alignment challenges */
        scattered->b = (double)scattered->a;
        scattered->c[idx % 7] = (char)(scattered->d & 0xFF);
        
        /* Update register-bound index with complex expression */
        idx = (idx * 3 + i * 5 - 7) % 64;
        
        /* Jump to create complex control flow */
        if (i % 8 == 3) {
            goto switch_blocks;
        }
        
        continue_block:
        /* More address computation */
        p1 = p1 + ((i * 2) % 16);
        p2 = p2 - ((i * 3) % 16);
    }
    
    goto done;
    
    switch_blocks:
    {
        /* Different addressing mode in this block */
        volatile double *dbl_ptr = (volatile double *)&global_array[idx].b;
        
        /* Inline asm that uses the same registers differently */
        asm volatile (
            "movq %[addr], %%r12\n\t"
            "movsd (%%r12), %%xmm0\n\t"
            "addsd %%xmm0, %%xmm0\n\t"
            "movsd %%xmm0, %[result]\n\t"
            : [result] "=m" (*dbl_ptr)
            : [addr] "r" (dbl_ptr)
            : "r12", "xmm0", "memory"
        );
        
        /* Call function with pointer-to-pointer */
        volatile MixedType ***triple_ptr = &ptr_to_ptr;
        update_pointer(triple_ptr);
        
        goto continue_block;
    }
    
    done:
    /* Final asm with output address reload */
    volatile int64_t final_result;
    asm volatile (
        "movq %[in1], %%rax\n\t"
        "imulq %[in2], %%rax\n\t"
        "movq %%rax, %[out]\n\t"
        : [out] "=m" (final_result)          /* Memory output */
        : [in1] "r" (addr_chain),            /* Register input */
          [in2] "r" ((int64_t)idx)           /* Register input */
        : "rax", "memory", "r12", "r13", "r14"
    );
}

/* Another stress function focusing on output addresses */
void stress_output_addresses(void) {
    register volatile int *out1 asm ("r12") = (volatile int *)&global_array[0].a;
    register volatile double *out2 asm ("r13") = (volatile double *)&global_array[64].b;
    
    volatile int input_val = 42;
    
    /* Inline asm with output address computation */
    asm volatile (
        "movl %[in], %%eax\n\t"
        "addl $100, %%eax\n\t"
        "movl %%eax, (%[out1])\n\t"
        "cvtsi2sd %%eax, %%xmm0\n\t"
        "movsd %%xmm0, (%[out2], %[idx], 8)\n\t"
        : 
        : [in] "r" (input_val),
          [out1] "r" (out1),
          [out2] "r" (out2),
          [idx] "r" (2)
        : "rax", "xmm0", "memory", "r12", "r13"
    );
    
    /* Complex output address chain */
    volatile int **out_addr_ptr = (volatile int **)&addr_holders[0].ptr;
    volatile int *temp_out;
    
    asm volatile (
        "movq %[ptr], %%r15\n\t"
        "movq (%%r15), %%rbx\n\t"
        "movq %%rbx, %[temp]\n\t"
        : [temp] "=r" (temp_out)
        : [ptr] "r" (out_addr_ptr)
        : "r15", "rbx", "memory"
    );
    
    /* Use the computed output address */
    if (temp_out) {
        *temp_out = 999;
    }
}

/* Function with inpaddr/outaddr address reloads */
void stress_inpaddr_outaddr(void) {
    volatile MixedType *base = &global_array[0];
    volatile int offsets[4] = {1, 3, 7, 15};
    
    /* Multiple levels of address computation */
    volatile MixedType ***ptr_chain = (volatile MixedType ***)&addr_holders[0].ptr;
    volatile MixedType **ptr2 = (volatile MixedType **)&addr_holders[1].ptr;
    volatile MixedType *ptr3;
    
    /* Chain of address computations */
    *ptr_chain = &ptr2;
    ptr2 = &ptr3;
    
    for (int i = 0; i < 4; i++) {
        /* Complex input address computation */
        ptr3 = base + offsets[i] * 2 - (i % 2);
        
        /* Inline asm that uses both input and output addresses */
        volatile int64_t result;
        asm volatile (
            "movq (%[in_addr]), %%rax\n\t"
            "movq 16(%[in_addr]), %%rbx\n\t"
            "addq %%rbx, %%rax\n\t"
            "movq %%rax, %[out_addr]\n\t"
            : [out_addr] "=m" (result)
            : [in_addr] "r" (&ptr3->d)
            : "rax", "rbx", "memory", "r12", "r13"
        );
        
        /* Update using the result */
        ptr3->a = (int)result;
        
        /* Jump to create control flow complexity */
        if (i == 2) {
            goto recalc_address;
        }
        
        back_here:
        /* Modify the base for next iteration */
        base = base + offsets[i];
    }
    
    goto finished;
    
    recalc_address:
    {
        /* Recompute addresses with different pattern */
        volatile int *alt_base = (volatile int *)&global_array[32].a;
        
        asm volatile (
            "movq %[base], %%r12\n\t"
            "leaq (%%r12, %[off], 4), %%r13\n\t"
            "movl (%%r13), %%eax\n\t"
            "movl %%eax, %[result]\n\t"
            : [result] "=m" (ptr3->a)
            : [base] "r" (alt_base),
              [off] "r" (8)
            : "r12", "r13", "rax", "memory"
        );
        
        goto back_here;
    }
    
    finished:
    return;
}

int main(void) {
    /* Initialize data */
    for (int i = 0; i < 256; i++) {
        global_array[i].a = i;
        global_array[i].b = (double)i;
        global_array[i].d = (int64_t)i * 1000;
        for (int j = 0; j < 7; j++) {
            global_array[i].c[j] = (char)(i + j);
        }
    }
    
    for (int i = 0; i < 16; i++) {
        addr_holders[i].ptr = &global_array[i * 16];
        addr_holders[i].offset = i * 4;
    }
    
    /* Call stress functions multiple times with small iterations */
    stress_reloads(4);
    stress_output_addresses();
    stress_inpaddr_outaddr();
    
    /* More stress with different parameters */
    for (int k = 0; k < 3; k++) {
        stress_reloads(2);
    }
    
    return 0;
}
