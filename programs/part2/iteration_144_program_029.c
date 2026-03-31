/* reload_stress.c - Designed to stress GCC's reload pass */
#include <stdint.h>

/* Volatile structures with mixed types to prevent optimization */
struct MixedData {
    volatile int counter;
    volatile double value;
    volatile char tag;
    volatile int64_t big;
};

/* Global volatile arrays to force memory accesses */
static volatile struct MixedData data_array[256];
static volatile int* volatile ptr_array[128];

/* Helper functions that take pointer-to-pointer arguments */
static void modify_pointer(int** pp) {
    if (pp && *pp) {
        **pp += 1;
    }
}

static void complex_address_helper(volatile void* addr1, volatile void* addr2) {
    /* Empty - just for address computation */
    (void)addr1;
    (void)addr2;
}

/* Function with complex addressing patterns */
static void stress_reloads(int iterations) {
    /* Bind specific pointers to explicit registers */
    register volatile struct MixedData* p1 asm ("r12") = &data_array[0];
    register volatile int* p2 asm ("r13") = (volatile int*)&data_array[0].counter;
    register volatile char* p3 asm ("r14") = (volatile char*)&data_array[0].tag;
    
    volatile int local_var = 42;
    volatile double local_dbl = 3.14159;
    int* volatile local_ptr = &local_var;
    
    /* Complex control flow with goto */
    int i = 0;
    
start_loop:
    if (i >= iterations) goto end_function;
    
    /* Complex address computation forcing RELOAD_FOR_INPUT_ADDRESS */
    {
        /* Compute offset using multiple operations */
        int offset = (i * 3 + 7) & 0xFF;
        
        /* Use register variable in complex address calculation */
        volatile struct MixedData* elem = p1 + offset;
        
        /* Access with mixed types - may require address reloads */
        elem->counter = i;
        elem->value = local_dbl * offset;
        elem->tag = 'A' + (i % 26);
        
        /* Inline assembly with memory operand and clobber */
        asm volatile (
            "addl $1, %[mem]\n\t"
            : [mem] "+m" (elem->counter)
            : 
            : "r12", "r13", "r14", "memory"
        );
    }
    
    /* Jump to different addressing pattern */
    goto middle_block;
    
after_middle:
    /* RELOAD_FOR_OUTPUT_ADDRESS pattern */
    {
        /* Complex output address computation */
        int idx = (i * 5 + 11) % 128;
        volatile int** output_addr = &ptr_array[idx];
        
        /* Inline assembly with output memory constraint */
        asm volatile (
            "movl %%eax, %[out]\n\t"
            : [out] "=m" (*output_addr)
            : "a" (p2)
            : "r12", "r13", "memory"
        );
    }
    
    /* RELOAD_FOR_INPADDR_ADDRESS pattern */
    {
        /* Take address of a pointer */
        int** pp = &local_ptr;
        
        /* Call function that takes pointer-to-pointer */
        modify_pointer(pp);
        
        /* Use result in complex address */
        volatile int* temp = *pp;
        if (temp) {
            /* Force address reload for input */
            asm volatile (
                "movl (%[addr]), %%eax\n\t"
                : 
                : [addr] "r" (temp)
                : "eax", "memory"
            );
        }
    }
    
    /* RELOAD_FOR_OPERAND_ADDRESS pattern */
    {
        /* Complex operand address computation */
        register volatile int64_t* big_ptr asm ("r15") = &data_array[i].big;
        
        /* Multiple constraints on same operand */
        volatile int64_t val;
        asm volatile (
            "movq %[src], %[dst]\n\t"
            "addq $1, %[dst]\n\t"
            : [dst] "=r,m" (val)
            : [src] "r,m" (*big_ptr)
            : "r15", "memory"
        );
        
        data_array[i].big = val;
    }
    
    i++;
    goto start_loop;
    
middle_block:
    /* RELOAD_FOR_OUTADDR_ADDRESS pattern */
    {
        /* Complex output address address computation */
        volatile int** outaddr_ptr = (volatile int**)&ptr_array[i % 64];
        
        /* Use in inline assembly */
        asm volatile (
            "leal 100(%%ebx), %%ecx\n\t"
            "movl %%ecx, (%[outaddr])\n\t"
            : 
            : [outaddr] "r" (outaddr_ptr), "b" (i)
            : "ecx", "memory"
        );
    }
    
    /* RELOAD_FOR_OPADDR_ADDR pattern */
    {
        /* Operand address of an address */
        volatile void* addrs[2];
        addrs[0] = (volatile void*)p3;
        addrs[1] = (volatile void*)&data_array[0].tag;
        
        /* Complex address chain */
        complex_address_helper(addrs[0], addrs[1]);
        
        /* Inline assembly with multiple memory clobbers */
        asm volatile (
            "movb $65, (%[addr1])\n\t"
            "movb $66, (%[addr2])\n\t"
            : 
            : [addr1] "r" (p3), [addr2] "r" (&data_array[i].tag)
            : "r12", "r14", "memory"
        );
    }
    
    /* RELOAD_FOR_OTHER_ADDRESS pattern */
    {
        /* Other address computation */
        volatile int* other_ptr = (volatile int*)&data_array[127 - i].counter;
        
        /* Mix with register variable */
        asm volatile (
            "cmpl %[reg], %[mem]\n\t"
            "setg %%al\n\t"
            : 
            : [mem] "m" (*other_ptr), [reg] "r" (i)
            : "eax", "memory"
        );
    }
    
    goto after_middle;
    
end_function:
    /* Final memory barrier */
    asm volatile ("" : : : "memory");
}

/* Additional stress patterns */
static void nested_address_chains(int depth) {
    if (depth <= 0) return;
    
    volatile int chain[8];
    volatile int* ptrs[4];
    
    /* Create chain of pointers */
    for (int j = 0; j < 4; j++) {
        ptrs[j] = &chain[j * 2];
    }
    
    /* Complex nested addressing */
    register volatile int** base asm ("r12") = &ptrs[0];
    
    for (int j = 0; j < depth; j++) {
        volatile int*** pptr = (volatile int***)&base;
        
        /* Force multiple address reloads */
        asm volatile (
            "movl (%[pptr]), %%ebx\n\t"
            "movl (%%ebx), %%ecx\n\t"
            "addl $1, (%%ecx)\n\t"
            : 
            : [pptr] "r" (pptr)
            : "ebx", "ecx", "r12", "memory"
        );
        
        /* Rotate pointers */
        volatile int* temp = *base;
        *base = &chain[(j + 1) % 8];
        chain[j % 8] = (int)(temp);
    }
}

/* Main function that orchestrates the stress test */
int main(void) {
    /* Initialize data */
    for (int i = 0; i < 256; i++) {
        data_array[i].counter = i;
        data_array[i].value = i * 0.1;
        data_array[i].tag = 'X';
        data_array[i].big = i * 1000LL;
    }
    
    for (int i = 0; i < 128; i++) {
        ptr_array[i] = (volatile int*)&data_array[i % 256].counter;
    }
    
    /* Execute stress patterns multiple times */
    for (int run = 0; run < 3; run++) {
        stress_reloads(8);  /* Small iteration count but complex patterns */
        nested_address_chains(4);
        
        /* Additional inline complex addressing */
        {
            register volatile double* dbl_ptr asm ("r15") = &data_array[0].value;
            
            for (int i = 0; i < 4; i++) {
                /* Mix addressing modes */
                volatile double* addr1 = dbl_ptr + i;
                volatile double* addr2 = &data_array[63 - i].value;
                
                /* Inline assembly with conflicting constraints */
                volatile double result;
                asm volatile (
                    "movsd (%[a1]), %%xmm0\n\t"
                    "addsd (%[a2]), %%xmm0\n\t"
                    "movsd %%xmm0, %[res]\n\t"
                    : [res] "=m,r" (result)
                    : [a1] "r,m" (addr1), [a2] "r,m" (addr2)
                    : "xmm0", "r15", "memory"
                );
                
                *addr1 = result;
            }
        }
    }
    
    return 0;
}
