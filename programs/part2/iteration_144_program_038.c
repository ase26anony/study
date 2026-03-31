/* reload_stress.c - Stress GCC's reload pass for uncovered cases */
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

/* Global volatile arrays for complex addressing */
static volatile BigStruct global_data[10];
static volatile int* volatile ptr_array[100];

/* Helper functions that take pointer-to-pointer arguments */
static void modify_pptr(volatile int*** ppp) {
    **ppp += 1;
}

static void complex_indirect(volatile void**** vppp, int offset) {
    volatile char* cp = (volatile char*)**vppp;
    cp[offset] ^= 0x01;
}

/* Function with register-bound variables and complex addressing */
static void stress_address_calculations(int iter) {
    /* Bind specific pointers to registers */
    register volatile MixedType* p1 asm ("r12");
    register volatile int* p2 asm ("r13");
    register volatile char* p3 asm ("r14");
    register int offset asm ("r15");
    
    /* Initialize with complex expressions */
    p1 = &global_data[iter % 10].arr[iter % 20];
    p2 = (volatile int*)&global_data[(iter + 1) % 10].extra[iter % 25];
    p3 = (volatile char*)&ptr_array[iter % 50];
    offset = iter * 7 + 3;
    
    /* Label for goto jumps */
    addr_calc_block:
    
    /* Complex address calculation forcing RELOAD_FOR_INPUT_ADDRESS */
    volatile int* addr1 = &p1->a + (offset >> 2);
    
    /* Inline asm with memory operand and clobbered address register */
    asm volatile (
        "addl $1, %[mem]\n\t"
        : [mem] "+m" (*addr1)
        : 
        : "r12", "memory"
    );
    
    /* Jump to create control flow complexity */
    if (iter & 1) {
        goto after_asm;
    }
    
    /* Another complex calculation for RELOAD_FOR_OUTPUT_ADDRESS */
    volatile double* addr2 = &p1->b + (offset % 5);
    
    /* Inline asm with multiple constraints on same operand */
    register volatile double* r_addr2 asm ("r12") = addr2;
    asm volatile (
        "movq (%[in]), %%rax\n\t"
        "addq $0x10, %%rax\n\t"
        "movq %%rax, %[out]\n\t"
        : [out] "=m" (*r_addr2)
        : [in] "r" (r_addr2), "m" (*r_addr2)
        : "rax", "r12", "memory"
    );
    
    after_asm:
    
    /* Force RELOAD_FOR_INPADDR_ADDRESS with nested pointer taking */
    volatile int** pptr = (volatile int**)&p1->d;
    modify_pptr((volatile int***)&pptr);
    
    /* Complex indexing with mixed types */
    volatile char* byte_ptr = p1->c + (offset % 7);
    byte_ptr[0] = byte_ptr[1] + byte_ptr[2];
    
    /* Another goto to disrupt register allocation */
    if (iter & 2) {
        goto addr_calc_block;
    }
    
    /* Force RELOAD_FOR_OPERAND_ADDRESS with quadruple pointer */
    volatile void**** vppp = (volatile void****)&ptr_array[iter % 30];
    complex_indirect(vppp, offset);
    
    /* Array indexing with register-bound pointer arithmetic */
    for (int i = 0; i < 3; i++) {
        volatile int* elem = p2 + (i * offset) / sizeof(int);
        
        /* Inline asm with "r" and "m" constraints on same value */
        int temp;
        asm volatile (
            "movl %[input], %[temp]\n\t"
            "leal 1(%[temp]), %[temp]\n\t"
            "movl %[temp], %[output]\n\t"
            : [temp] "=&r" (temp), [output] "=m" (*elem)
            : [input] "m" (*elem)
            : "cc"
        );
    }
}

/* Main function creating various reload scenarios */
int main(void) {
    /* Initialize some data */
    for (int i = 0; i < 100; i++) {
        ptr_array[i] = (volatile int*)&global_data[i % 10];
    }
    
    /* Multiple iterations with different parameters */
    for (int iter = 0; iter < 5; iter++) {
        /* Call stress function */
        stress_address_calculations(iter);
        
        /* Additional inline complex addressing in main */
        register volatile BigStruct* bs_ptr asm ("r12");
        bs_ptr = &global_data[iter];
        
        /* Force RELOAD_FOR_OUTADDR_ADDRESS */
        volatile long long* out_addr = &bs_ptr->extra[iter * 3 % 50];
        
        /* Inline asm with explicit clobbers of address registers */
        asm volatile (
            "movq $0x12345678, %%rax\n\t"
            "xorq %%rbx, %%rbx\n\t"
            "addq %%rax, %[dest]\n\t"
            : [dest] "+m" (*out_addr)
            : 
            : "rax", "rbx", "r12", "r13", "memory"
        );
        
        /* Complex chain of address computations */
        volatile MixedType* mt_ptr = &bs_ptr->arr[iter * 7 % 100];
        volatile int** indirect = (volatile int**)&mt_ptr->d;
        
        /* Nested function call with address-taken argument */
        modify_pptr((volatile int***)&indirect);
        
        /* Mixed-type access with alignment challenges */
        volatile char* char_ptr = (volatile char*)mt_ptr;
        for (int j = 0; j < 20; j += 3) {
            char_ptr[j + iter] = char_ptr[j + iter + 1] + iter;
        }
        
        /* Final inline asm with multiple memory operands */
        register int r1 asm ("r12") = iter * 100;
        register int r2 asm ("r13") = iter * 200;
        
        asm volatile (
            "imull %%r12d, %%r13d\n\t"
            "addl %%r13d, %[out1]\n\t"
            "subl %%r12d, %[out2]\n\t"
            : [out1] "+m" (mt_ptr->a), [out2] "+m" (global_data[iter].arr[0].a)
            : "r" (r1), "r" (r2)
            : "r12", "r13", "cc", "memory"
        );
    }
    
    return 0;
}
