/* reload1_stress.c - Stress GCC reload pass for uncovered switch cases */
#include <stdint.h>

/* Volatile structures to prevent optimization */
typedef struct {
    volatile int a;
    volatile double b;
    volatile char c[7];
    volatile int* d;
} MixedType;

typedef struct {
    volatile MixedType arr[100];
    volatile long long extra[50];
} BigStruct;

/* Global volatile arrays to force complex addressing */
static volatile BigStruct global_data[10];
static volatile int global_ints[1000];
static volatile double global_doubles[500];

/* Helper functions that take pointer-to-pointer arguments */
static void modify_pptr(volatile int*** ppp) {
    volatile int dummy = ***ppp;
    (void)dummy;
}

static void compute_address(volatile void** addr_ptr, int offset) {
    *addr_ptr = (volatile void*)((uintptr_t)*addr_ptr + offset);
}

/* Main stress function */
static void stress_reload(void) {
    /* Bind specific pointers to registers */
    register volatile MixedType* p1 asm ("r12") = &global_data[0].arr[0];
    register volatile int* p2 asm ("r13") = &global_ints[0];
    register volatile double* p3 asm ("r14") = &global_doubles[0];
    
    volatile int local_var = 42;
    volatile double local_dbl = 3.14159;
    volatile int* local_ptr = &local_var;
    
    /* Complex addressing computations */
    int idx1 = 0, idx2 = 0, idx3 = 0;
    
    /* Label for goto jumps */
    compute_addr:
    
    /* RELOAD_FOR_INPUT_ADDRESS and RELOAD_FOR_INPADDR_ADDRESS */
    for (idx1 = 0; idx1 < 3; idx1++) {
        /* Complex addressing with register variables */
        volatile MixedType* addr1 = p1 + idx1 * 17 + (idx2 % 5);
        volatile int* addr2 = p2 + (idx1 * 29) | (idx3 * 3);
        volatile double* addr3 = p3 + (idx1 ^ idx2) * 11;
        
        /* Inline assembly with memory operands and clobbers */
        asm volatile (
            "movl (%[mem1]), %%eax\n\t"
            "addl %%eax, (%[mem2])\n\t"
            "movsd (%[mem3]), %%xmm0\n\t"
            : 
            : [mem1] "m" (*((volatile int*)addr1)),
              [mem2] "m" (*addr2),
              [mem3] "m" (*addr3)
            : "eax", "xmm0", "memory", "r12", "r13", "r14"
        );
        
        /* Nested function call with address-taken argument */
        volatile int** pptr = &local_ptr;
        modify_pptr((volatile int***)&pptr);
        
        /* Another complex address computation */
        volatile void* complex_addr = (volatile void*)(
            (uintptr_t)addr1 + (uintptr_t)addr2 * 2 - (uintptr_t)addr3
        );
        
        /* RELOAD_FOR_OPERAND_ADDRESS and RELOAD_FOR_OPADDR_ADDR */
        asm volatile (
            "leal (%[base], %[index], 4), %%ecx\n\t"
            "movl (%%ecx), %%edx\n\t"
            : 
            : [base] "r" (addr1), [index] "r" (idx1)
            : "ecx", "edx", "memory"
        );
    }
    
    /* Jump to create complex control flow */
    if (idx2++ < 2) {
        goto compute_addr;
    }
    
    /* RELOAD_FOR_OUTPUT_ADDRESS and RELOAD_FOR_OUTADDR_ADDRESS */
    register volatile int* out_ptr asm ("r15") = &global_ints[100];
    
    for (idx3 = 0; idx3 < 4; idx3++) {
        /* Output addressing with complex computation */
        volatile int* output_addr = out_ptr + idx3 * 7 + (idx1 % 3);
        volatile double* output_dbl = (volatile double*)(
            (uintptr_t)output_addr + sizeof(int) * 2
        );
        
        /* Inline assembly with output memory operands */
        asm volatile (
            "movl %%ebx, (%[out1])\n\t"
            "movsd %%xmm1, (%[out2])\n\t"
            : 
            : [out1] "m" (*output_addr),
              [out2] "m" (*output_dbl)
            : "memory", "r15"
        );
        
        /* Address computation that requires reload */
        volatile void** addr_holder = (volatile void**)&output_addr;
        compute_address(addr_holder, idx3 * 16);
        
        /* Mixed constraints on same operand */
        volatile int temp;
        asm volatile (
            "movl %[in], %%eax\n\t"
            "movl %%eax, %[out]\n\t"
            : [out] "=m,r" (temp)
            : [in] "r,m" (local_var)
            : "eax"
        );
    }
    
    /* More complex control flow with labels */
    after_loop:
    
    /* Use same registers for different purposes */
    p1 = (volatile MixedType*)&local_dbl;
    p2 = (volatile int*)p3;
    
    /* Final inline assembly with multiple clobbers */
    asm volatile (
        "movq %[ptr1], %%r12\n\t"
        "movq %[ptr2], %%r13\n\t"
        "movq %[ptr3], %%r14\n\t"
        : 
        : [ptr1] "rm" (p1),
          [ptr2] "rm" (p2),
          [ptr3] "rm" (p3)
        : "r12", "r13", "r14", "memory"
    );
}

/* Second stress function with different patterns */
static void stress_reload_2(void) {
    volatile int array[100];
    register volatile int* r1 asm ("r10") = &array[0];
    register volatile int* r2 asm ("r11") = &array[50];
    
    /* Chain of address computations */
    for (int i = 0; i < 5; i++) {
        volatile int* addr1 = r1 + i * (i + 1) * 2;
        volatile int* addr2 = r2 - i * 3;
        
        /* RELOAD_FOR_OTHER_ADDRESS */
        asm volatile (
            "movl (%[a1]), %%r8d\n\t"
            "imull %%r8d, (%[a2])\n\t"
            : 
            : [a1] "m" (*addr1), [a2] "m" (*addr2)
            : "r8", "r10", "r11", "memory"
        );
        
        /* Jump to disrupt register allocation */
        if (i == 2) {
            goto special_case;
        }
        
        continue;
        
        special_case:
        /* Different addressing mode */
        asm volatile (
            "leal (%[base], %[idx], 8), %%r9d\n\t"
            "movl %%r9d, (%[dest])\n\t"
            : 
            : [base] "r" (addr1), [idx] "r" (i), [dest] "m" (*addr2)
            : "r9", "memory"
        );
    }
    
    /* Pointer-to-pointer chain */
    volatile int x = 100;
    volatile int* px = &x;
    volatile int** ppx = &px;
    volatile int*** pppx = &ppx;
    
    modify_pptr((volatile int***)pppx);
}

int main(void) {
    /* Initialize some data */
    for (int i = 0; i < 10; i++) {
        global_data[i].arr[0].a = i;
        global_data[i].arr[0].b = i * 1.5;
    }
    
    for (int i = 0; i < 1000; i++) {
        global_ints[i] = i * 2;
    }
    
    for (int i = 0; i < 500; i++) {
        global_doubles[i] = i * 0.5;
    }
    
    /* Call stress functions multiple times */
    stress_reload();
    stress_reload_2();
    stress_reload();
    
    return 0;
}
