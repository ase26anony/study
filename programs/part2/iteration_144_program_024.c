/* reload_stress.c - Designed to stress GCC's reload pass */
#include <stdint.h>

/* Volatile structures with mixed types to prevent optimization */
struct mixed_data {
    volatile int counter;
    volatile double value;
    volatile char tag;
    volatile int64_t big;
    volatile float small;
};

/* Global volatile arrays to force memory accesses */
static volatile struct mixed_data data_array[256];
static volatile int* volatile ptr_array[128];

/* Helper functions that take pointer-to-pointer arguments */
static void modify_pptr(int** pp) {
    volatile int dummy = **pp;
    (void)dummy;
}

static void compute_address(void* addr) {
    volatile char dummy = *(volatile char*)addr;
    (void)dummy;
}

/* Function with complex addressing patterns */
static void stress_address_calculations(int iter) {
    /* Bind specific registers for address computation */
    register volatile struct mixed_data* p1 asm ("r12") = &data_array[0];
    register volatile int* p2 asm ("r13") = (volatile int*)&data_array[0];
    register volatile char* p3 asm ("r14") = (volatile char*)&data_array[0];
    
    /* Complex offset computation */
    int offset1 = iter * 3 + 7;
    int offset2 = iter * 5 + 11;
    int offset3 = iter * 7 + 13;
    
    /* Label for goto jumps */
    compute_addr:
    
    /* RELOAD_FOR_INPUT_ADDRESS: Complex address computation for input */
    {
        /* Force address reload by using register variable in complex expression */
        volatile double* addr1 = (volatile double*)((char*)p1 + offset1 * sizeof(struct mixed_data));
        volatile int* addr2 = (volatile int*)((char*)p2 + offset2 * sizeof(int));
        
        /* Inline assembly with memory operand constraints */
        asm volatile (
            "movq (%[a1]), %%rax\n\t"
            "addl (%[a2]), %%eax\n\t"
            : /* no outputs */
            : [a1] "m" (*addr1), [a2] "m" (*addr2)
            : "rax", "memory"
        );
    }
    
    /* Jump to create control flow complexity */
    if (iter & 1) {
        goto output_section;
    }
    
    /* RELOAD_FOR_INPADDR_ADDRESS: Address of address computation */
    {
        volatile int** pptr = &ptr_array[iter % 128];
        
        /* Nested function call with address-taken argument */
        modify_pptr((int**)pptr);
        
        /* Complex addressing with the result */
        volatile int* derived = *pptr;
        if (derived) {
            asm volatile (
                "incl %0\n\t"
                : "+m" (*derived)
                :
                : "memory"
            );
        }
    }
    
    output_section:
    
    /* RELOAD_FOR_OUTPUT_ADDRESS: Complex address for output */
    {
        /* Force output address reload */
        register volatile int64_t* p4 asm ("r15") = &data_array[iter % 256].big;
        
        /* Complex offset computation */
        int complex_offset = (offset1 ^ offset2) + offset3;
        
        /* Inline assembly with output memory operand */
        asm volatile (
            "movq %[val], (%[out])\n\t"
            : 
            : [out] "r" ((char*)p4 + complex_offset), 
              [val] "r" ((int64_t)iter)
            : "memory"
        );
    }
    
    /* RELOAD_FOR_OUTADDR_ADDRESS: Address of output address */
    {
        volatile int64_t* out_addr = &data_array[(iter + 1) % 256].big;
        volatile int64_t** out_addr_ptr = &out_addr;
        
        /* Use in assembly with clobbered address register */
        asm volatile (
            "movq %[ptr], %%r12\n\t"
            "movq $0x12345678, (%%r12)\n\t"
            : 
            : [ptr] "r" (out_addr_ptr)
            : "r12", "memory"
        );
    }
    
    /* RELOAD_FOR_OPERAND_ADDRESS: Multiple memory operands */
    {
        volatile float* f1 = &data_array[iter % 256].small;
        volatile float* f2 = &data_array[(iter + 64) % 256].small;
        
        /* Assembly with conflicting constraints */
        asm volatile (
            "movss (%[in]), %%xmm0\n\t"
            "addss (%[in2]), %%xmm0\n\t"
            "movss %%xmm0, (%[out])\n\t"
            : 
            : [in] "r" (f1), [in2] "r" (f2), [out] "r" (f1)
            : "xmm0", "memory"
        );
    }
    
    /* RELOAD_FOR_OPADDR_ADDR: Operand address of address */
    {
        volatile char* base = p3;
        volatile char** base_ptr = &base;
        
        /* Complex chain of address computations */
        for (int i = 0; i < 3; i++) {
            volatile char* current = *base_ptr + i * 17;
            compute_address(current);
            
            /* Jump back to create loop with address computation */
            if (i == 1) {
                goto compute_addr;
            }
        }
    }
    
    /* RELOAD_FOR_OTHER_ADDRESS: Other address computations */
    {
        /* Mix different address computations in same basic block */
        volatile int* addr_array[4];
        addr_array[0] = (volatile int*)((char*)p1 + offset1);
        addr_array[1] = (volatile int*)((char*)p2 + offset2);
        addr_array[2] = (volatile int*)((char*)p3 + offset3);
        addr_array[3] = (volatile int*)&iter;
        
        /* Use all addresses in a complex way */
        for (int i = 0; i < 4; i++) {
            if (addr_array[i]) {
                asm volatile (
                    "addl $1, %0\n\t"
                    : "+m" (*addr_array[i])
                    :
                    : "memory"
                );
            }
        }
    }
}

/* Main function that creates the stress pattern */
int main(void) {
    /* Initialize data */
    for (int i = 0; i < 256; i++) {
        data_array[i].counter = i;
        data_array[i].value = i * 0.5;
        data_array[i].tag = 'A' + (i % 26);
        data_array[i].big = i * 1000LL;
        data_array[i].small = i * 0.25f;
    }
    
    for (int i = 0; i < 128; i++) {
        ptr_array[i] = (volatile int*)&data_array[i * 2].counter;
    }
    
    /* Multiple iterations with different parameters */
    for (int iter = 0; iter < 8; iter++) {
        stress_address_calculations(iter);
        
        /* Additional inline complex addressing in main */
        register volatile struct mixed_data* reg_ptr asm ("rbx") = &data_array[iter * 16 % 256];
        
        /* Complex addressing with multiple constraints */
        volatile double* dptr = (volatile double*)((char*)reg_ptr + iter * 8);
        volatile int* iptr = (volatile int*)dptr;
        
        asm volatile (
            "movsd (%[dbl]), %%xmm1\n\t"
            "cvttsd2si %%xmm1, %%eax\n\t"
            "addl %%eax, (%[int])\n\t"
            : 
            : [dbl] "r" (dptr), [int] "r" (iptr)
            : "rax", "xmm1", "memory"
        );
        
        /* Address of local variable with complex use */
        int local_var = iter;
        int* local_ptr = &local_var;
        int** local_pptr = &local_ptr;
        
        modify_pptr(local_pptr);
        
        /* Force spill/reload with volatile and inline asm */
        asm volatile (
            "movl %[val], %%r12d\n\t"
            "leal 1(%%r12d), %%eax\n\t"
            "movl %%eax, %[out]\n\t"
            : [out] "=m" (*local_ptr)
            : [val] "rm" (local_var)
            : "r12", "rax", "memory"
        );
    }
    
    return 0;
}
