/* reload1_stress.c - Stress GCC reload pass for uncovered switch cases */
#include <stdint.h>
#include <stdlib.h>

/* Volatile mixed-type structures to prevent optimization */
typedef struct {
    volatile int a;
    volatile double b;
    volatile char c[7];
    volatile int* d;
} MixedType;

typedef struct {
    volatile MixedType* arr;
    volatile int** ptr_ptr;
    volatile long offset;
} AddrHolder;

/* Global volatile arrays to force complex addressing */
static volatile MixedType global_array[100];
static volatile int* global_ptr_array[50];
static volatile AddrHolder addr_holders[20];

/* Helper functions that take pointer-to-pointer arguments */
static void modify_pptr(volatile int*** ppp) {
    volatile int** local_pp = *ppp;
    if (local_pp) {
        asm volatile("" : "+m" (*local_pp) : : "r12", "r13", "memory");
    }
}

static void compute_address(volatile void** addr_store, volatile void* base, long offset) {
    *addr_store = (volatile char*)base + offset * sizeof(MixedType);
}

/* Main stress function with complex addressing patterns */
static void stress_reload(void) {
    /* Bind specific pointers to explicit registers */
    register volatile MixedType* p1 asm ("r12") = &global_array[0];
    register volatile int** p2 asm ("r13") = &global_ptr_array[0];
    register volatile AddrHolder* p3 asm ("r14") = &addr_holders[0];
    
    volatile int local_var = 42;
    volatile int* local_ptr = &local_var;
    volatile int** local_pptr = &local_ptr;
    
    /* Complex addressing mode 1: Array indexing with register base */
    volatile MixedType* addr1;
    compute_address((volatile void**)&addr1, p1, 17);
    
    /* Force RELOAD_FOR_INPUT_ADDRESS through inline asm */
    asm volatile (
        "movl (%[input]), %%eax\n\t"
        "addl $1, %%eax\n\t"
        "movl %%eax, (%[input])"
        : [input] "+m" (*(volatile int*)addr1)
        : 
        : "eax", "r12", "memory"
    );
    
    /* Jump to create control flow complexity */
    goto label1;
    
back_from_label1:
    /* Complex addressing mode 2: Pointer-to-pointer chain */
    volatile int*** ppp = (volatile int***)&local_pptr;
    modify_pptr((volatile int***)ppp);
    
    /* Force RELOAD_FOR_OUTPUT_ADDRESS */
    volatile int* output_addr;
    asm volatile (
        "leal (%[base], %[index], 4), %%ebx\n\t"
        "movl %%ebx, %[out]"
        : [out] "=m" (output_addr)
        : [base] "r" (p1), [index] "r" (local_var)
        : "ebx", "r12", "memory"
    );
    
    /* Nested addressing computation */
    volatile MixedType* addr2 = p1 + (local_var & 0xF);
    volatile char* byte_ptr = (volatile char*)addr2 + 3;
    
    /* Force RELOAD_FOR_INPADDR_ADDRESS */
    asm volatile (
        "movb (%[addr]), %%al\n\t"
        "orb $0x1, %%al\n\t"
        "movb %%al, (%[addr])"
        : 
        : [addr] "r" (byte_ptr)
        : "al", "memory"
    );
    
    /* Complex loop with register pressure */
    for (int i = 0; i < 3; i++) {
        /* Bind to different register each iteration */
        register volatile MixedType* loop_ptr asm ("r15") = p1 + i * 5;
        
        /* Force RELOAD_FOR_OPERAND_ADDRESS */
        asm volatile (
            "fldl (%[base])\n\t"
            "fadd %[val]\n\t"
            "fstpl (%[base])"
            : 
            : [base] "r" (&loop_ptr->b), [val] "f" (1.0)
            : "st", "st(1)", "r15", "memory"
        );
        
        /* Address taken of register variable */
        volatile MixedType** ptr_to_reg = &loop_ptr;
        p3->arr = *ptr_to_reg;
        
        /* Force RELOAD_FOR_OPADDR_ADDR */
        asm volatile (
            "movq (%[ptr]), %%rax\n\t"
            "movq %%rax, %[store]"
            : [store] "=m" (p3->offset)
            : [ptr] "r" (ptr_to_reg)
            : "rax", "r15", "memory"
        );
    }
    
    return;
    
label1:
    /* Different addressing mode after goto */
    volatile double* dbl_ptr = &p1->b;
    
    /* Force RELOAD_FOR_OTHER_ADDRESS */
    asm volatile (
        "movsd (%[src]), %%xmm0\n\t"
        "addsd %[add], %%xmm0\n\t"
        "movsd %%xmm0, (%[src])"
        : 
        : [src] "r" (dbl_ptr), [add] "x" (2.0)
        : "xmm0", "r12", "memory"
    );
    
    /* Force RELOAD_FOR_OUTADDR_ADDRESS */
    volatile long* out_addr;
    asm volatile (
        "movq %[in], %%rax\n\t"
        "addq $16, %%rax\n\t"
        "movq %%rax, %[out]"
        : [out] "=m" (out_addr)
        : [in] "r" (p2)
        : "rax", "r13", "memory"
    );
    
    goto back_from_label1;
}

/* Secondary stress function with different patterns */
static void stress_reload2(void) {
    volatile MixedType stack_array[10];
    volatile int* stack_ptrs[5];
    
    /* Multiple register binding */
    register volatile MixedType* r1 asm ("r10") = stack_array;
    register volatile int** r2 asm ("r11") = stack_ptrs;
    
    /* Complex expression with multiple address computations */
    volatile MixedType* addr3 = r1 + (stack_array[0].a % 8);
    volatile MixedType* addr4 = addr3 + (stack_array[1].a % 4);
    
    /* Chain of address computations */
    volatile char* chain_ptr = (volatile char*)addr4;
    chain_ptr += 3;
    chain_ptr -= 1;
    chain_ptr += stack_array[2].a & 0x3;
    
    /* Force multiple reload types in sequence */
    asm volatile (
        "movb (%[chain]), %%cl\n\t"
        "incb %%cl\n\t"
        "movb %%cl, (%[chain])"
        : 
        : [chain] "r" (chain_ptr)
        : "cl", "r10", "memory"
    );
    
    /* Pointer-to-pointer with address taken */
    volatile int val = 99;
    volatile int* pval = &val;
    volatile int** ppval = &pval;
    volatile int*** pppval = &ppval;
    
    modify_pptr((volatile int***)pppval);
    
    /* Mixed addressing modes */
    volatile double* dbl_arr[4];
    for (int i = 0; i < 4; i++) {
        dbl_arr[i] = &r1[i].b;
        
        /* Force address reloads in loop */
        asm volatile (
            "movsd (%[addr]), %%xmm1\n\t"
            "mulsd %[mul], %%xmm1\n\t"
            "movsd %%xmm1, (%[addr])"
            : 
            : [addr] "r" (dbl_arr[i]), [mul] "x" (1.5)
            : "xmm1", "r10", "memory"
        );
    }
}

int main(void) {
    /* Initialize global data */
    for (int i = 0; i < 100; i++) {
        global_array[i].a = i;
        global_array[i].b = i * 1.5;
        for (int j = 0; j < 7; j++) {
            global_array[i].c[j] = (i + j) & 0xFF;
        }
    }
    
    for (int i = 0; i < 20; i++) {
        addr_holders[i].arr = &global_array[i * 5];
        addr_holders[i].offset = i * 100;
    }
    
    /* Call stress functions multiple times */
    stress_reload();
    stress_reload2();
    
    /* Additional stress in main */
    register volatile MixedType* main_ptr asm ("rbx") = &global_array[50];
    
    /* Complex addressing with inline asm */
    volatile long result;
    asm volatile (
        "movq (%[base], %[idx], 8), %%rax\n\t"
        "imulq $3, %%rax\n\t"
        "movq %%rax, %[res]"
        : [res] "=m" (result)
        : [base] "r" (main_ptr), [idx] "r" (25L)
        : "rax", "rbx", "memory"
    );
    
    /* Final mixed operation */
    volatile MixedType* final_ptr = main_ptr + (result & 0xF);
    volatile int* int_ptr = (volatile int*)final_ptr;
    
    asm volatile (
        "lock addl $1, (%[ptr])"
        : 
        : [ptr] "r" (int_ptr)
        : "memory", "rbx"
    );
    
    return 0;
}
