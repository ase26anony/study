/* reload_stress.c - Stress GCC's reload pass for uncovered switch cases */
#include <stdint.h>

/* Volatile structures with mixed types to prevent optimization */
typedef struct {
    volatile int a;
    volatile double b;
    volatile char c[7];
    volatile int *d;
} MixedType;

typedef struct {
    volatile long x;
    volatile MixedType *y;
    volatile short z[5];
} Container;

/* Global volatile arrays to force memory accesses */
volatile MixedType global_array[100];
volatile Container container_array[50];

/* Helper functions that take pointer-to-pointer arguments */
void modify_pointer(int **pp) {
    if (pp && *pp) {
        **pp += 1;
    }
}

void complex_address_helper(volatile Container ***cpp) {
    if (cpp && *cpp && **cpp) {
        (***cpp).x += 2;
    }
}

/* Function with complex addressing patterns */
void stress_reloads(int iterations) {
    /* Bind specific pointers to explicit registers */
    register volatile MixedType *p1 asm ("r12") = &global_array[0];
    register volatile Container *p2 asm ("r13") = &container_array[0];
    register int *addr_reg asm ("r14") = (int*)&global_array[0].a;
    
    volatile int local_vars[20];
    volatile double local_doubles[10];
    
    int i, j;
    
    /* Label for goto jumps */
    compute_addresses:
    
    for (i = 0; i < iterations; i++) {
        /* Complex pointer arithmetic with multiple constraints */
        volatile MixedType *temp1 = p1 + (i * 3) % 97;
        volatile Container *temp2 = p2 + (i * 7) % 47;
        
        /* RELOAD_FOR_INPUT_ADDRESS: Complex array indexing */
        int idx1 = (i * 11) % 20;
        int idx2 = (i * 13) % 10;
        
        /* Multiple memory accesses with volatile */
        int val1 = temp1->a + temp2->x;
        double val2 = temp1->b + local_doubles[idx2];
        
        /* Inline assembly with multiple memory operands and clobbers */
        asm volatile (
            "movl %[mem1], %%eax\n\t"
            "addl %%eax, %[mem2]\n\t"
            : [mem2] "+m" (local_vars[idx1])
            : [mem1] "m" (temp1->a)
            : "eax", "r12", "r13", "r14", "memory"
        );
        
        /* Jump to create complex control flow */
        if (i % 3 == 0) {
            goto inline_asm_block;
        }
        
        continue_computation:
        
        /* RELOAD_FOR_OUTPUT_ADDRESS: More complex addressing */
        volatile int **pp = (volatile int**)&temp1->d;
        *pp = (int*)&local_vars[(i * 17) % 20];
        
        /* Nested function call with address-taken argument */
        modify_pointer((int**)&temp1->d);
        
        /* Another inline asm with conflicting constraints */
        register int offset asm ("r15") = i * sizeof(MixedType);
        asm volatile (
            "leal (%[base], %[off]), %%ebx\n\t"
            "movl (%%ebx), %%ecx\n\t"
            : 
            : [base] "r" (p1), [off] "r" (offset)
            : "ebx", "ecx", "r15", "memory"
        );
        
        if (i % 5 == 2) {
            goto address_helper_call;
        }
        
        continue_loop:
        
        /* RELOAD_FOR_OPERAND_ADDRESS: Complex expression in address */
        volatile Container ***cpp = (volatile Container***)&temp2->y;
        *cpp = &p2 + (i % 3);
    }
    
    goto program_end;
    
    inline_asm_block:
    {
        /* Another asm block that clobbers address registers */
        asm volatile (
            "movq %[ptr], %%r12\n\t"
            "movl 16(%%r12), %%edx\n\t"
            : 
            : [ptr] "r" (p1)
            : "r12", "edx", "memory"
        );
        goto continue_computation;
    }
    
    address_helper_call:
    {
        /* RELOAD_FOR_INPADDR_ADDRESS / RELOAD_FOR_OUTADDR_ADDRESS */
        volatile Container ***cpp = (volatile Container***)&p2->y;
        complex_address_helper((Container***)cpp);
        
        /* More complex addressing with inline asm */
        asm volatile (
            "movq %[base], %%rsi\n\t"
            "imulq $24, %[idx], %%rdi\n\t"
            "addq %%rdi, %%rsi\n\t"
            "movl (%%rsi), %%r8d\n\t"
            : 
            : [base] "r" (global_array), [idx] "r" (i)
            : "rsi", "rdi", "r8", "memory"
        );
        goto continue_loop;
    }
    
    program_end:
    return;
}

/* Second stress function with different patterns */
void more_reload_stress(void) {
    register volatile char *cptr asm ("r10") = (char*)global_array;
    register volatile int *iptr asm ("r11") = &global_array[0].a;
    
    volatile int results[30];
    volatile double darray[15];
    
    /* RELOAD_FOR_OPADDR_ADDR and RELOAD_FOR_OTHER_ADDRESS patterns */
    for (int k = 0; k < 10; k++) {
        /* Complex address computation */
        volatile char *addr1 = cptr + k * sizeof(MixedType) + 3;
        volatile int *addr2 = iptr + k * 5;
        
        /* Inline asm with "m" and "r" constraints on same operand */
        int temp;
        asm volatile (
            "movl %[input], %[temp]\n\t"
            "addl $42, %[temp]\n\t"
            "movl %[temp], %[output]\n\t"
            : [temp] "=&r" (temp), [output] "=m" (*addr2)
            : [input] "m" (*addr1)
            : "r10", "r11", "memory"
        );
        
        /* Pointer chain */
        volatile int **pp1 = (volatile int**)&results[k * 2];
        volatile int **pp2 = (volatile int**)&results[k * 2 + 1];
        
        *pp1 = (int*)addr1;
        *pp2 = (int*)addr2;
        
        /* Function call with complex address */
        modify_pointer((int**)pp1);
        
        /* goto to disrupt register allocation */
        if (k % 3 == 0) {
            goto asm_clobber_block;
        }
        
        after_asm:
        
        /* Mixed-type access forcing alignment handling */
        double *dptr = (double*)(cptr + k * 8 + 4);
        darray[k] = *dptr + 1.0;
    }
    
    goto function_end;
    
    asm_clobber_block:
    {
        /* Asm that clobbers multiple address registers */
        asm volatile (
            "movq %%r10, %%rax\n\t"
            "movq %%r11, %%rbx\n\t"
            "addq $8, %%rax\n\t"
            "subq $4, %%rbx\n\t"
            : 
            : 
            : "rax", "rbx", "r10", "r11", "memory"
        );
        goto after_asm;
    }
    
    function_end:
    
    /* Final complex expression */
    volatile Container ***triple_ptr = (volatile Container***)&container_array[25].y;
    *triple_ptr = &container_array[0].y;
}

/* Main function that sets up and calls stress functions */
int main(void) {
    /* Initialize some data */
    for (int i = 0; i < 100; i++) {
        global_array[i].a = i;
        global_array[i].b = i * 1.5;
    }
    
    for (int i = 0; i < 50; i++) {
        container_array[i].x = i * 2;
        container_array[i].y = &global_array[i % 100];
    }
    
    /* Call stress functions with small iteration counts */
    stress_reloads(5);
    more_reload_stress();
    
    /* Additional patterns in main */
    {
        register volatile MixedType *reg_ptr asm ("r12") = &global_array[10];
        volatile int local;
        
        /* RELOAD_FOR_INPUT/OUTPUT_ADDRESS mixing */
        asm volatile (
            "movl %[in], %%eax\n\t"
            "leal 100(%%eax), %%ebx\n\t"
            "movl %%ebx, %[out]\n\t"
            : [out] "=m" (local)
            : [in] "m" (reg_ptr->a)
            : "eax", "ebx", "r12", "memory"
        );
        
        /* Complex chain */
        volatile int **pp = (volatile int**)&reg_ptr->d;
        volatile int ***ppp = &pp;
        ***ppp = &local;
    }
    
    return 0;
}
