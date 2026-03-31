/* reload1_stress.c - Stress GCC reload pass for uncovered switch cases */
#include <stdint.h>

/* Volatile structures with mixed types to prevent optimization */
typedef struct {
    volatile int a;
    volatile double b;
    volatile char c[7];
    volatile int64_t d;
} MixedType;

typedef struct {
    volatile MixedType arr[100];
    volatile int* volatile ptr_array[50];
} Container;

/* Global volatile arrays to force memory accesses */
volatile Container containers[10];
volatile int global_buffer[1000];

/* Helper functions that take pointer-to-pointer arguments */
void modify_pptr(int** pp) {
    if (pp && *pp) {
        **pp += 1;
    }
}

void complex_address_helper(volatile int**** pppp) {
    /* Force address computation */
    if (pppp && *pppp && **pppp && ***pppp) {
        ****pppp += 42;
    }
}

/* Function with complex addressing patterns */
void stress_reloads(int seed) {
    /* Bind specific variables to registers */
    register volatile MixedType* p1 asm ("r12");
    register volatile int* p2 asm ("r13");
    register volatile char* p3 asm ("r14");
    register int offset asm ("r15");
    
    /* Initialize with complex address computations */
    p1 = &containers[seed % 10].arr[seed % 20];
    p2 = &global_buffer[seed * 3 % 1000];
    p3 = (volatile char*)&containers[(seed + 1) % 10];
    offset = seed * 7 % 100;
    
    /* Complex addressing mode 1: RELOAD_FOR_INPUT_ADDRESS */
    {
        volatile int* addr1;
        /* Force address computation with register constraints */
        asm volatile (
            "lea (%[base], %[idx], 4), %[out]\n\t"
            : [out] "=r" (addr1)
            : [base] "r" (p2), [idx] "r" (offset)
            : "cc"
        );
        
        /* Use the computed address */
        *addr1 = seed;
        
        /* Inline assembly with memory operand and clobbered address register */
        asm volatile (
            "movl $123, (%[mem])\n\t"
            "addl $456, %%r12d\n\t"  /* Clobber r12 */
            : 
            : [mem] "m" (*addr1)
            : "r12", "cc", "memory"
        );
    }
    
    /* Jump to create control flow complexity */
    goto compute_more_addresses;
    
compute_more_addresses:
    /* RELOAD_FOR_OUTPUT_ADDRESS pattern */
    {
        volatile int** addr2;
        int local_var = seed * 2;
        
        /* Complex address computation for output */
        asm volatile (
            "lea (%[base], %[off], 2), %[out]\n\t"
            : [out] "=r" (addr2)
            : [base] "r" (&global_buffer[100]), [off] "r" (offset)
            : "cc"
        );
        
        /* Pass address of local variable */
        modify_pptr((int**)&local_var);
        
        /* Another asm with conflicting constraints */
        asm volatile (
            "movq %[in], %%r12\n\t"
            "movl (%%r12), %%eax\n\t"
            "addl $1, %%eax\n\t"
            "movl %%eax, (%%r12)\n\t"
            : 
            : [in] "r" (addr2)
            : "rax", "r12", "cc", "memory"
        );
    }
    
    /* RELOAD_FOR_INPADDR_ADDRESS pattern */
    {
        volatile int**** complex_ptr;
        volatile int* simple_ptr = &global_buffer[200];
        
        /* Create a chain of address computations */
        complex_ptr = (volatile int****)&simple_ptr;
        
        /* Force address reloads through nested calls */
        complex_address_helper((int****)complex_ptr);
        
        /* Inline asm with multiple memory operands */
        asm volatile (
            "movq (%[ptr1]), %%rax\n\t"
            "movq (%[ptr2]), %%rbx\n\t"
            "addq %%rbx, %%rax\n\t"
            "movq %%rax, (%[ptr3])\n\t"
            : 
            : [ptr1] "r" (&p1), [ptr2] "r" (&p2), [ptr3] "r" (&p3)
            : "rax", "rbx", "cc", "memory"
        );
    }
    
    /* Loop with scattered accesses - RELOAD_FOR_OPERAND_ADDRESS */
    for (int i = 0; i < 5; i++) {
        volatile MixedType* elem;
        int idx = (seed + i * 13) % 100;
        
        /* Complex array indexing */
        asm volatile (
            "imul $112, %[idx], %%eax\n\t"  /* sizeof(MixedType) = 112 */
            "add %[base], %%rax\n\t"
            : "=r" (elem)
            : [base] "r" (containers), [idx] "r" (idx)
            : "rax", "cc"
        );
        
        /* Access mixed types within structure */
        elem->a = i;
        elem->b = i * 3.14;
        elem->c[i % 7] = i + 'A';
        
        /* Asm that clobbers address registers */
        asm volatile (
            "movq %[addr], %%r12\n\t"
            "movq 8(%%r12), %%r13\n\t"  /* Access the double */
            : 
            : [addr] "r" (elem)
            : "r12", "r13", "cc", "memory"
        );
    }
    
    /* RELOAD_FOR_OUTADDR_ADDRESS pattern */
    {
        volatile int* out_addr;
        volatile int value = seed * 11;
        
        /* Compute output address with complex expression */
        asm volatile (
            "lea (%[a], %[b], 8), %[out]\n\t"
            "add $16, %[out]\n\t"
            : [out] "=r" (out_addr)
            : [a] "r" (p2), [b] "r" (offset)
            : "cc"
        );
        
        /* Use in another asm with "m" constraint */
        asm volatile (
            "movl %[val], (%[addr])\n\t"
            : 
            : [addr] "m" (*out_addr), [val] "r" (value)
            : "cc", "memory"
        );
    }
    
    /* RELOAD_FOR_OTHER_ADDRESS pattern */
    {
        volatile int* other_addr;
        int temp1 = seed * 3;
        int temp2 = seed * 5;
        
        /* Multiple address computations in same block */
        asm volatile (
            "mov %[t1], %%eax\n\t"
            "add %[t2], %%eax\n\t"
            "lea global_buffer(%%eax), %[out]\n\t"
            : [out] "=r" (other_addr)
            : [t1] "r" (temp1), [t2] "r" (temp2)
            : "rax", "cc"
        );
        
        /* Jump back to create circular control flow */
        if (seed % 2) {
            goto compute_more_addresses;
        }
    }
}

/* Main function that creates various reload scenarios */
int main() {
    /* Initialize data */
    for (int i = 0; i < 1000; i++) {
        global_buffer[i] = i * 2;
    }
    
    /* Call stress function with different seeds to create
       different address computation patterns */
    for (int i = 0; i < 10; i++) {
        stress_reloads(i);
        
        /* Additional inline complex addressing in main */
        register volatile int* reg_ptr asm ("r12");
        reg_ptr = &global_buffer[i * 50];
        
        /* Force RELOAD_FOR_OPADDR_ADDR */
        {
            volatile int** addr_of_addr;
            asm volatile (
                "lea (%[ptr]), %[out]\n\t"
                : [out] "=r" (addr_of_addr)
                : [ptr] "r" (reg_ptr)
                : "cc"
            );
            
            /* Use in nested asm */
            asm volatile (
                "movq (%[addr]), %%r12\n\t"
                "movl $999, (%%r12)\n\t"
                : 
                : [addr] "r" (addr_of_addr)
                : "r12", "cc", "memory"
            );
        }
        
        /* Mixed addressing modes in loop */
        for (int j = 0; j < 3; j++) {
            volatile MixedType* mt_ptr;
            int idx = (i * 17 + j * 23) % 100;
            
            /* Complex addressing with multiple registers */
            asm volatile (
                "mov %[idx], %%eax\n\t"
                "imul $112, %%eax\n\t"  /* sizeof(MixedType) */
                "add %[base], %%rax\n\t"
                "add $8, %%rax\n\t"     /* offset to double member */
                : "=r" (mt_ptr)
                : [base] "r" (containers), [idx] "r" (idx)
                : "rax", "cc"
            );
            
            /* Access through computed pointer */
            *(volatile double*)mt_ptr = 3.14159 * j;
        }
    }
    
    return 0;
}
