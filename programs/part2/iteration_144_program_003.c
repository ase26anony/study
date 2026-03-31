/* reload_stress.c - Stress GCC's reload pass for coverage testing */
#include <stdint.h>

/* Volatile structures with mixed types to prevent optimization */
struct MixedData {
    volatile int32_t i;
    volatile double d;
    volatile char c[7];
    volatile int64_t l;
};

struct PointerChain {
    volatile struct MixedData *ptr;
    volatile int32_t offset;
    volatile void *next;
};

/* Global volatile arrays to force memory accesses */
volatile struct MixedData global_array[100];
volatile struct PointerChain chain_array[50];

/* Helper functions that take pointer-to-pointer arguments */
void modify_pointer(volatile struct MixedData ***ppp) {
    volatile struct MixedData **temp = *ppp;
    if (temp) {
        /* Force memory access */
        asm volatile("" : : "r"(temp) : "memory");
    }
}

void compute_address(volatile void **addr_ptr, int32_t offset) {
    if (*addr_ptr) {
        /* Complex address computation */
        uintptr_t base = (uintptr_t)*addr_ptr;
        base += offset * sizeof(struct MixedData);
        *addr_ptr = (volatile void*)base;
    }
}

/* Main stress function */
void stress_reloads(void) {
    /* Bind specific pointers to explicit registers */
    register volatile struct MixedData *p1 asm ("r12") = &global_array[0];
    register volatile struct MixedData *p2 asm ("r13") = &global_array[50];
    register volatile struct PointerChain *chain asm ("r14") = &chain_array[0];
    
    volatile int32_t index = 0;
    volatile int32_t offset = 0;
    volatile struct MixedData *temp_ptr = NULL;
    
    /* Label for goto jumps */
    compute_addr:
    
    /* Complex addressing mode 1: array indexing with register base */
    offset = (index * 3 + 7) % 20;
    
    /* This should trigger RELOAD_FOR_INPUT_ADDRESS */
    temp_ptr = p1 + offset;
    
    /* Inline assembly with memory operand and clobbered address register */
    asm volatile (
        "movl (%[ptr]), %%eax\n\t"
        "addl $1, %%eax\n\t"
        "movl %%eax, (%[ptr])"
        : 
        : [ptr] "m" (temp_ptr->i)
        : "eax", "r12", "memory"
    );
    
    /* Jump to create control flow complexity */
    if (index++ < 3) goto compute_addr;
    
    /* Reset for next pattern */
    index = 0;
    p1 = &global_array[10];  /* Change base */
    
    /* Label for address computation jumps */
    addr_chain:
    
    /* Nested pointer access - should trigger RELOAD_FOR_INPADDR_ADDRESS */
    volatile struct MixedData **pp = &temp_ptr;
    modify_pointer(&pp);
    
    /* Complex offset calculation using multiple registers */
    int32_t complex_offset = (int32_t)((uintptr_t)p2 - (uintptr_t)p1);
    complex_offset = complex_offset / sizeof(struct MixedData);
    
    /* Inline assembly with multiple memory constraints */
    asm volatile (
        "leal (%[base], %[idx], 4), %%ecx\n\t"
        "movl %%ecx, %[out]"
        : [out] "=m" (chain->offset)
        : [base] "r" (complex_offset), [idx] "r" (index)
        : "ecx", "r13", "memory"
    );
    
    /* Should trigger RELOAD_FOR_OUTPUT_ADDRESS */
    chain->ptr = p2 + chain->offset;
    
    /* Another asm with conflicting constraints */
    register int32_t *reg_offset asm ("r15") = &chain->offset;
    asm volatile (
        "movl (%[reg]), %%ebx\n\t"
        "imull $3, %%ebx\n\t"
        "movl %%ebx, %[mem]"
        : [mem] "=m" (global_array[index].i)
        : [reg] "r" (reg_offset)
        : "ebx", "r15", "memory"
    );
    
    /* Compute address using helper - should trigger RELOAD_FOR_OPADDR_ADDR */
    compute_address((volatile void**)&chain->next, index * 2);
    
    if (index++ < 2) goto addr_chain;
    
    /* More complex pattern with output address reloads */
    volatile struct MixedData *output_array[5];
    
    for (int i = 0; i < 5; i++) {
        /* Complex addressing for output */
        output_array[i] = p1 + (i * (index + 1));
        
        /* Inline asm that clobbers address registers */
        asm volatile (
            "movq %[src], %%rax\n\t"
            "movq %%rax, %[dst]"
            : [dst] "=m" (output_array[i]->l)
            : [src] "r" ((uint64_t)(i * 1000))
            : "rax", "r12", "memory"
        );
    }
    
    /* Pattern for RELOAD_FOR_OPERAND_ADDRESS */
    volatile int32_t *addr_calc = &global_array[25].i;
    
    /* Multiple asm statements in sequence with same registers */
    asm volatile ("" : : "r" (addr_calc) : "r12", "memory");
    
    /* Change the address calculation */
    addr_calc = &global_array[30].i + index;
    
    asm volatile (
        "addl $5, %[val]"
        : [val] "+m" (*addr_calc)
        : 
        : "r12", "r13", "memory"
    );
    
    /* Final complex pattern with mixed operations */
    {
        volatile double *dptr = &p2->d;
        volatile int32_t *iptr = &p1->i;
        
        /* Should trigger RELOAD_FOR_OTHER_ADDRESS */
        asm volatile (
            "cvtsi2sd %[int], %%xmm0\n\t"
            "movsd %%xmm0, %[double]"
            : [double] "=m" (*dptr)
            : [int] "m" (*iptr)
            : "xmm0", "r12", "r13", "memory"
        );
    }
}

/* Additional stress patterns */
void more_stress(void) {
    /* Use explicit goto to create disjoint control flow */
    volatile int32_t counter = 0;
    volatile struct MixedData *alternate_ptr = &global_array[70];
    
    block_a:
    {
        register volatile char *cptr asm ("r12") = alternate_ptr->c;
        
        /* Complex array indexing */
        cptr += (counter * 3) % 7;
        
        asm volatile (
            "movb $65, (%[ptr])"
            : 
            : [ptr] "r" (cptr)
            : "memory"
        );
        
        if (counter++ % 2 == 0) goto block_b;
        else goto block_c;
    }
    
    block_b:
    {
        /* Different use of same register */
        register volatile int64_t *lptr asm ("r12") = &alternate_ptr->l;
        *lptr = counter * 1000LL;
        goto block_d;
    }
    
    block_c:
    {
        /* Yet another use */
        register volatile double *dptr asm ("r12") = &alternate_ptr->d;
        *dptr = (double)counter;
        goto block_d;
    }
    
    block_d:
    if (counter < 4) goto block_a;
}

int main(void) {
    /* Initialize some data */
    for (int i = 0; i < 100; i++) {
        global_array[i].i = i;
        global_array[i].d = i * 1.5;
        global_array[i].l = i * 1000LL;
    }
    
    for (int i = 0; i < 50; i++) {
        chain_array[i].ptr = &global_array[i % 100];
        chain_array[i].offset = i;
    }
    
    /* Call stress functions multiple times */
    stress_reloads();
    more_stress();
    
    /* Additional inline stress in main */
    {
        volatile struct MixedData *local_ptr = &global_array[90];
        volatile int32_t idx = 0;
        
        /* Loop with complex addressing */
        for (idx = 0; idx < 5; idx++) {
            /* Force address computation reloads */
            volatile struct MixedData *elem = local_ptr + (idx * (idx + 1));
            
            /* Nested function call with address-taken argument */
            volatile struct MixedData **ptr_to_ptr = &elem;
            modify_pointer(&ptr_to_ptr);
            
            /* Inline asm with clobbers */
            asm volatile (
                "movl %[idx], %%eax\n\t"
                "movl %%eax, (%[mem])"
                : 
                : [idx] "r" (idx), [mem] "r" (&elem->i)
                : "eax", "memory"
            );
        }
    }
    
    return 0;
}
