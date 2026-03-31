/* reload_stress.c - Stress GCC reload pass for coverage testing */
#include <stdint.h>

/* Volatile structures with mixed types to prevent optimization */
struct mixed {
    volatile int a;
    volatile double b;
    volatile char c[7];
    volatile long d;
};

struct nested {
    volatile struct mixed arr[3];
    volatile int* ptrs[4];
    volatile long long big;
};

/* Global volatile arrays to force memory accesses */
static volatile struct mixed global_mixed[100];
static volatile struct nested global_nested[50];

/* Helper functions that take pointer-to-pointer arguments */
static void modify_pptr(volatile int*** ppp) {
    volatile int dummy = ***ppp;
    (void)dummy;
}

static void compute_address(volatile void** addr_holder, int offset) {
    *addr_holder = (volatile void*)((char*)*addr_holder + offset);
}

/* Main stress function with complex addressing patterns */
static void stress_reloads(int iter) {
    /* Bind specific variables to registers to create constraints */
    register volatile struct mixed* reg_ptr1 asm ("r12") = &global_mixed[0];
    register volatile struct nested* reg_ptr2 asm ("r13") = &global_nested[0];
    register int index asm ("r14") = iter * 7;
    
    volatile int local_array[100];
    volatile double local_doubles[50];
    
    /* Complex addressing computation block 1 */
    {
        volatile char* addr1;
        volatile long* addr2;
        
        /* Force RELOAD_FOR_INPUT_ADDRESS type */
        addr1 = (volatile char*)&reg_ptr1[index % 50].c[(index * 3) % 7];
        addr2 = (volatile long*)&reg_ptr2[(index * 2) % 25].arr[(index + 1) % 3].d;
        
        /* Inline assembly with memory operands and clobbers */
        asm volatile (
            "movl (%[src]), %%eax\n\t"
            "addl $1, %%eax\n\t"
            "movl %%eax, (%[dst])\n\t"
            : /* no outputs */
            : [src] "m" (*addr1), [dst] "m" (*addr2)
            : "eax", "memory", "r12", "r13"
        );
    }
    
    /* Jump to create control flow complexity */
    goto compute_block_2;
    
recompute_block:
    /* Force reloads after clobbered registers */
    reg_ptr1 = &global_mixed[(index + 11) % 100];
    reg_ptr2 = &global_nested[(index + 17) % 50];
    
    /* Nested pointer indirection for RELOAD_FOR_INPADDR_ADDRESS */
    {
        volatile int** pptr = (volatile int**)&local_array[10];
        volatile int* volatile* vpptr = (volatile int* volatile*)&reg_ptr1->a;
        
        *pptr = (volatile int*)&reg_ptr2->arr[0].a;
        modify_pptr((volatile int***)&pptr);
    }
    
    return;
    
compute_block_2:
    /* Different addressing mode for same registers */
    {
        volatile double* dptr1 = (volatile double*)&reg_ptr1[(index * 5) % 100].b;
        volatile double* dptr2 = (volatile double*)&reg_ptr2[(index * 3) % 50].arr[2].b;
        
        /* Another asm with conflicting constraints */
        asm volatile (
            "movsd (%[in]), %%xmm0\n\t"
            "addsd %%xmm0, %%xmm0\n\t"
            "movsd %%xmm0, (%[out])\n\t"
            : /* no outputs */
            : [in] "m" (*dptr1), [out] "m" (*dptr2)
            : "xmm0", "memory", "r12", "r13", "r14"
        );
    }
    
    /* Complex offset computation forcing address reloads */
    {
        volatile void* addr_holder = (volatile void*)&global_mixed[0];
        for (int i = 0; i < 3; i++) {
            compute_address(&addr_holder, (i * index + 7) * sizeof(struct mixed));
        }
        
        /* Use computed address */
        volatile struct mixed* computed = (volatile struct mixed*)addr_holder;
        computed->a = index;
    }
    
    /* Force RELOAD_FOR_OUTPUT_ADDRESS */
    {
        volatile long long* big_ptr = &reg_ptr2[index % 50].big;
        volatile int* out_ptr = &local_array[(index * 13) % 100];
        
        /* Assembly with output memory operand */
        asm volatile (
            "movq (%[in]), %%rax\n\t"
            "shrq $32, %%rax\n\t"
            "movl %%eax, (%[out])\n\t"
            : /* no outputs */
            : [in] "m" (*big_ptr), [out] "m" (*out_ptr)
            : "rax", "memory", "r12", "r13"
        );
    }
    
    /* Jump back to create loop in control flow */
    if (iter % 2 == 0) {
        goto recompute_block;
    }
    
    /* Final complex addressing pattern */
    {
        register volatile char* reg_char asm ("r15") = 
            (volatile char*)&global_mixed[99].c[0];
        
        /* Multiple address computations in one expression */
        volatile int val = *(volatile int*)(reg_char + 
            (index * sizeof(struct mixed)) % 256 - 
            ((index & 0xF) << 2));
        
        /* Use in asm with "r" constraint on address */
        asm volatile (
            "imull $137, %[val], %[val]\n\t"
            : [val] "+r" (val)
            :
            : "cc"
        );
        
        /* Store back through complex address */
        *(volatile int*)((char*)&reg_ptr2->arr[0].a + (index % 16)) = val;
    }
}

/* Secondary stress function with different patterns */
static void more_reloads(void) {
    volatile int buffer[256];
    volatile double dbl_buffer[128];
    
    /* Create pointer chains for address reloads */
    {
        volatile int* ptr1 = &buffer[0];
        volatile int** ptr2 = (volatile int**)&buffer[64];
        volatile int*** ptr3 = (volatile int***)&buffer[128];
        
        *ptr2 = ptr1 + 32;
        *ptr3 = ptr2;
        
        /* Force operand address reloads */
        asm volatile (
            "movq (%[ptr]), %%r12\n\t"
            "movl (%%r12), %%eax\n\t"
            "movl %%eax, (%[dest])\n\t"
            : /* no outputs */
            : [ptr] "m" (ptr3), [dest] "m" (buffer[192])
            : "rax", "r12", "memory"
        );
    }
    
    /* Mixed-type array access with complex indexing */
    {
        for (int i = 0; i < 8; i++) {
            volatile char* byte_ptr = (volatile char*)&dbl_buffer[i * 3];
            volatile int* int_ptr = (volatile int*)(byte_ptr + (i * 7) % 32);
            
            *int_ptr = i * 111;
            
            /* Inline asm that clobbers address registers */
            asm volatile (
                "movl %[idx], %%r12d\n\t"
                "leal (%%r12, %%r12, 2), %%eax\n\t"
                "movl %%eax, (%[addr])\n\t"
                : /* no outputs */
                : [idx] "r" (i), [addr] "m" (*int_ptr)
                : "rax", "r12", "memory"
            );
        }
    }
}

int main(void) {
    /* Initialize some data */
    for (int i = 0; i < 100; i++) {
        global_mixed[i].a = i * 2;
        global_mixed[i].b = i * 3.14;
    }
    
    for (int i = 0; i < 50; i++) {
        global_nested[i].big = i * 1000LL;
    }
    
    /* Call stress functions multiple times with different parameters */
    for (int i = 0; i < 10; i++) {
        stress_reloads(i);
        more_reloads();
        
        /* Additional inline complex addressing */
        {
            register volatile struct mixed* alt_ptr asm ("r12") = 
                &global_mixed[(i * 17) % 100];
            
            /* Force RELOAD_FOR_OPERAND_ADDRESS */
            volatile long* addr = (volatile long*)&alt_ptr->d;
            volatile long val = *addr;
            
            asm volatile (
                "rolq $13, %[val]\n\t"
                : [val] "+r" (val)
                :
                : "cc"
            );
            
            /* Store through different addressing mode */
            *(volatile long*)((char*)alt_ptr + offsetof(struct mixed, d)) = val;
        }
    }
    
    return 0;
}
