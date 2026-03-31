/* reload_stress.c - Designed to stress GCC's reload pass */
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

/* Global volatile arrays */
static volatile BigStruct global_data[10];
static volatile int* volatile ptr_array[100];

/* Helper functions that take complex pointer arguments */
static void use_pointer_to_pointer(int*** ppp) {
    volatile int dummy = ***ppp;
    (void)dummy;
}

static void modify_through_indirection(volatile int**** q) {
    ****q = 1234;
}

/* Main stress function */
static void stress_reloads(int iter) {
    /* Bind specific variables to registers */
    register volatile MixedType* p1 asm ("r12");
    register volatile int* p2 asm ("r13");
    register volatile char* p3 asm ("r14");
    register int offset asm ("r15");
    
    /* Initialize with complex address computations */
    p1 = &global_data[iter % 10].arr[iter % 20];
    p2 = (volatile int*)&global_data[(iter + 1) % 10].extra[iter % 30];
    p3 = (volatile char*)&ptr_array[iter % 50];
    offset = iter * 7 + 3;
    
    /* Complex addressing mode 1 */
    volatile int* addr1 = &p1->a + offset;
    volatile double* addr2 = &p1->b + (offset / 2);
    
    /* Inline assembly with memory operands and clobbers */
    asm volatile (
        "movl %[mem1], %%eax\n\t"
        "addl %%eax, %[mem2]\n\t"
        : [mem2] "+m" (*addr1)
        : [mem1] "m" (*addr2), "m" (*p2)
        : "eax", "r12", "r13", "r14", "memory"
    );
    
    /* Jump to create control flow complexity */
    goto label1;
    
    /* Unreachable code that still affects analysis */
    p3 = (volatile char*)&global_data[5].arr[10].c[2];
    
label1:
    /* Different addressing mode */
    volatile int** pptr = (volatile int**)&p1->d;
    
    /* Nested function call with address-taken argument */
    use_pointer_to_pointer((int***)&pptr);
    
    /* More complex addressing with multiple constraints */
    register int idx asm ("ebx");
    idx = iter * 3 + 7;
    
    /* Another inline asm with conflicting constraints */
    asm volatile (
        "leal (%[base], %[index], 4), %%ecx\n\t"
        "movl (%%ecx), %%edx\n\t"
        "movl %%edx, %[out]\n\t"
        : [out] "=m" (global_data[0].arr[0].a)
        : [base] "r" (p2), [index] "r" (idx)
        : "ecx", "edx", "ebx", "memory"
    );
    
    /* Compute address with multiple operations */
    volatile int* complex_addr = &((volatile int*)p3)[idx * 2 + offset];
    
    /* Third asm block with operand address reloads */
    asm volatile (
        "movl %[addr], %%esi\n\t"
        "movl (%%esi), %%edi\n\t"
        "addl $1, %%edi\n\t"
        "movl %%edi, (%%esi)\n\t"
        : 
        : [addr] "r" (complex_addr)
        : "esi", "edi", "memory"
    );
    
    /* Create address chain for outaddr reloads */
    volatile int**** quad_ptr = (volatile int****)&ptr_array[20];
    modify_through_indirection((volatile int****)&quad_ptr);
    
    /* Loop with different addressing each iteration */
    for (int i = 0; i < 3; i++) {
        volatile MixedType* temp = &global_data[i].arr[(i + iter) % 15];
        
        /* Mixed-type access pattern */
        temp->a = temp->c[i] + i;
        temp->b = temp->a * 2.5;
        
        /* Inline asm inside loop with clobbered address regs */
        asm volatile (
            "movq %[ptr], %%r8\n\t"
            "movl 4(%%r8), %%r9d\n\t"
            : 
            : [ptr] "r" (temp)
            : "r8", "r9", "memory"
        );
    }
    
    /* Final jump to create another basic block */
    if (iter & 1) {
        goto final_label;
    }
    
    /* More address computations */
    p2 = (volatile int*)&global_data[2].extra[offset % 25];
    
final_label:
    /* Output address computation */
    volatile int* output_addr = &global_data[3].arr[4].a + (offset >> 1);
    
    /* Asm with output address constraint */
    asm volatile (
        "movl $42, %[out]\n\t"
        : [out] "=m" (*output_addr)
        : 
        : "memory"
    );
}

/* Secondary stress function with different patterns */
static void more_reload_stress(void) {
    register volatile long long* r1 asm ("r10");
    register volatile int* r2 asm ("r11");
    
    r1 = (volatile long long*)&global_data[6].extra[10];
    r2 = (volatile int*)&global_data[7].arr[5].a;
    
    /* Complex pointer arithmetic */
    for (int j = 0; j < 4; j++) {
        volatile int** addr_ptr = (volatile int**)((char*)r2 + j * 12);
        
        /* Inpaddr address computation */
        asm volatile (
            "movq %[base], %%rax\n\t"
            "movq (%[ptr]), %%rbx\n\t"
            "addq %%rax, %%rbx\n\t"
            "movq %%rbx, %[store]\n\t"
            : [store] "=m" (ptr_array[j])
            : [base] "r" (r1), [ptr] "r" (addr_ptr)
            : "rax", "rbx", "r10", "r11", "memory"
        );
        
        /* Jump within loop */
        if (j & 1) {
            goto loop_label;
        }
        
        r1 += 2;
        
loop_label:
        /* Operand address reload scenario */
        asm volatile (
            "movl %[val], (%%r10, %%r11, 2)\n\t"
            : 
            : [val] "ri" (j * 100), "r" (r1), "r" (r2)
            : "memory"
        );
    }
    
    /* Other address reload type */
    volatile int* other_addr = (volatile int*)&r1[5];
    asm volatile (
        "movl $999, (%[addr])\n\t"
        : 
        : [addr] "r" (other_addr)
        : "memory"
    );
}

int main(void) {
    /* Initialize some data */
    for (int i = 0; i < 100; i++) {
        ptr_array[i] = (volatile int*)&global_data[i % 10];
    }
    
    /* Call stress functions multiple times with different parameters */
    for (int iter = 0; iter < 5; iter++) {
        stress_reloads(iter);
        more_reload_stress();
        
        /* Additional inline complexity in main */
        register volatile MixedType* mp asm ("r12");
        mp = &global_data[iter].arr[iter * 3 % 20];
        
        /* Mixed addressing modes in same basic block */
        volatile int* addr_a = &mp->a;
        volatile double* addr_b = &mp->b;
        volatile char* addr_c = &mp->c[iter % 7];
        
        asm volatile (
            "movl (%[a]), %%eax\n\t"
            "cvtsi2sd %%eax, %%xmm0\n\t"
            "movsd %%xmm0, (%[b])\n\t"
            "movb (%[c]), %%cl\n\t"
            "addb %%cl, (%[a])\n\t"
            : 
            : [a] "r" (addr_a), [b] "r" (addr_b), [c] "r" (addr_c)
            : "eax", "ecx", "xmm0", "memory"
        );
        
        /* Function call with address of register variable */
        volatile int** mpp = (volatile int**)&mp->d;
        use_pointer_to_pointer((int***)&mpp);
    }
    
    return 0;
}
