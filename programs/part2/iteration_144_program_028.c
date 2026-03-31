/* reload_stress.c - Designed to stress GCC's reload pass */
#include <stdint.h>

/* Volatile structures with mixed types to prevent optimization */
struct MixedData {
    volatile int counter;
    volatile double value;
    volatile char tag;
    volatile int64_t big;
    volatile float small;
};

/* Large volatile arrays */
static volatile struct MixedData data_array[1000];
static volatile int* volatile ptr_array[200];

/* Helper functions that take pointer-to-pointer arguments */
static void modify_pptr(int*** ppp) {
    volatile int dummy = ***ppp;
    (void)dummy;
}

static void compute_address(void** addr, int offset) {
    volatile int dummy = *(int*)((char*)*addr + offset);
    (void)dummy;
}

/* Function with complex addressing patterns */
static void stress_address_calculations(int iter) {
    /* Bind specific pointers to registers */
    register volatile struct MixedData* p1 asm ("r12") = &data_array[100];
    register volatile int* p2 asm ("r13") = (volatile int*)&data_array[200];
    register volatile char* p3 asm ("r14") = (volatile char*)&data_array[300];
    
    /* Complex addressing mode 1: array indexing with register base */
    volatile double val1 = p1[iter * 3].value;
    volatile int cnt1 = p1[iter * 7].counter;
    
    /* Force address computation with multiple constraints */
    {
        volatile int offset = iter * 16 + 8;
        
        /* Inline assembly with memory operand and clobbered address register */
        asm volatile (
            "addl $1, %[mem]\n\t"
            : [mem] "+m" (p1[offset/4].counter)
            : 
            : "r12", "memory"
        );
        
        /* Jump to create control flow complexity */
        if (iter & 1) goto compute_again;
        
        /* Different addressing mode using same register */
        asm volatile (
            "movq %[ptr], %%r12\n\t"
            "addq $32, %%r12\n\t"
            : 
            : [ptr] "r" (p2)
            : "r12"
        );
        
compute_again:
        /* Recompute address using clobbered register */
        p1 = &data_array[400 + iter];
    }
    
    /* Nested pointer operations */
    {
        volatile int** local_pptr = (volatile int**)&ptr_array[50];
        *local_pptr = (volatile int*)&p2;
        
        /* Function call with address-taken argument */
        modify_pptr((int***)&local_pptr);
        
        /* Complex address chain */
        volatile int* chain = (volatile int*)((char*)p3 + iter * 24);
        volatile void* vchain = (void*)chain;
        compute_address(&vchain, iter * 8);
    }
    
    /* Output address computation */
    {
        register volatile int64_t* p4 asm ("r15") = &data_array[500].big;
        
        /* Inline assembly with output address constraint */
        int64_t result;
        asm volatile (
            "movq (%[addr]), %[res]\n\t"
            "addq $8, %[addr]\n\t"
            : [res] "=r" (result), [addr] "+r" (p4)
            : 
            : "memory"
        );
        
        /* Use result to force dependency */
        p1[iter].counter = (int)result;
    }
}

/* Second stress function with different patterns */
static void stress_more_reloads(void) {
    volatile int buffer[256];
    register volatile int* rptr asm ("r12") = &buffer[128];
    
    /* Multiple memory operands in inline assembly */
    for (int i = 0; i < 4; i++) {
        int idx1 = i * 3;
        int idx2 = i * 5 + 1;
        
        asm volatile (
            "movl (%[src]), %%eax\n\t"
            "addl %%eax, (%[dst])\n\t"
            : 
            : [src] "r" (&rptr[idx1]), [dst] "r" (&rptr[idx2])
            : "eax", "memory", "r12"
        );
        
        /* Jump to disrupt register allocation */
        if (i == 2) goto special_case;
        
        /* Operand address reload scenario */
        volatile int* temp = &rptr[i * 8];
        asm volatile (
            "incl %0\n\t"
            : "+m" (*temp)
            : 
            : "r12"
        );
        
        continue;
        
special_case:
        /* Different addressing mode after goto */
        rptr = &buffer[64];
        volatile int offset = i * 12;
        asm volatile (
            "movl %[off], %%ebx\n\t"
            "addl %%ebx, (%[base])\n\t"
            : 
            : [base] "r" (rptr), [off] "r" (offset)
            : "ebx", "memory", "r12"
        );
    }
    
    /* Inpaddr/Outaddr address scenarios */
    {
        volatile int* addr_array[10];
        for (int i = 0; i < 10; i++) {
            addr_array[i] = &buffer[i * 16];
        }
        
        /* Function call with complex address expression */
        compute_address((void**)&addr_array[3], 8);
        
        /* Inline assembly with multiple constraints on same operand */
        volatile int* special = addr_array[5];
        int dummy;
        asm volatile (
            "movl (%[in]), %%ecx\n\t"
            "movl %%ecx, %[out]\n\t"
            : [out] "=r" (dummy)
            : [in] "m" (*special)
            : "ecx", "memory"
        );
    }
}

/* Main function that creates the stress patterns */
int main(void) {
    /* Initialize some data */
    for (int i = 0; i < 1000; i++) {
        data_array[i].counter = i;
        data_array[i].value = i * 0.5;
        data_array[i].tag = 'A' + (i % 26);
        data_array[i].big = i * 1000LL;
        data_array[i].small = i * 0.1f;
    }
    
    for (int i = 0; i < 200; i++) {
        ptr_array[i] = (volatile int*)&data_array[i * 5];
    }
    
    /* Execute stress patterns multiple times */
    for (int iter = 0; iter < 8; iter++) {
        stress_address_calculations(iter);
        stress_more_reloads();
        
        /* Additional inline complexity in main */
        register volatile struct MixedData* main_ptr asm ("r13") = &data_array[600];
        
        /* Mixed addressing modes */
        volatile int idx = iter * 17 % 100;
        volatile double* dptr = (volatile double*)((char*)main_ptr + idx * sizeof(struct MixedData) + 8);
        
        asm volatile (
            "movsd (%[src]), %%xmm0\n\t"
            "addsd %%xmm0, %%xmm0\n\t"
            "movsd %%xmm0, (%[dst])\n\t"
            : 
            : [src] "r" (dptr), [dst] "r" (&main_ptr[idx].value)
            : "xmm0", "memory", "r13"
        );
        
        /* Address computation with goto */
        if (iter & 2) {
            goto recalc;
        }
        
        main_ptr = &data_array[700];
        continue;
        
recalc:
        /* Different address computation after goto */
        volatile int* iptr = (volatile int*)((char*)main_ptr + iter * 4);
        asm volatile (
            "lock addl $1, %0\n\t"
            : "+m" (*iptr)
            : 
            : "r13"
        );
    }
    
    return 0;
}
