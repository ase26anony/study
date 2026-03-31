/* reload_stress.c - Stress GCC's reload pass for coverage testing */
#include <stdint.h>

/* Volatile structures to prevent optimization */
volatile struct MixedData {
    int id;
    double value;
    char tag[7];
    int *ptr;
    long long big;
} data_array[100];

volatile struct NestedPtrs {
    int **pp;
    volatile int *vp;
    char padding[8];
} ptr_array[50];

/* Helper functions that take complex pointer arguments */
static void use_pointer_to_pointer(int ***ppp) {
    volatile int dummy = ***ppp;
    (void)dummy;
}

static void modify_through_indirect(volatile int ****q) {
    ****q = 42;
}

/* Function with complex addressing patterns */
static void stress_address_calculations(int iter) {
    /* Bind specific variables to registers */
    register volatile struct MixedData *p1 asm ("r12") = &data_array[0];
    register volatile struct NestedPtrs *p2 asm ("r13") = &ptr_array[0];
    register int *p3 asm ("r14") = (int*)&data_array[0].id;
    
    /* Complex offset calculations */
    int offset1 = iter * 3 + 7;
    int offset2 = iter * 5 - 2;
    long offset3 = (long)iter * sizeof(struct MixedData);
    
    /* Jump label for control flow complexity */
    void *target = &&compute_address;
    
    /* First block: compute addresses with register-bound pointers */
    compute_address:
    {
        /* Force RELOAD_FOR_INPUT_ADDRESS */
        volatile int *addr1 = &p1[offset1].id + offset2;
        
        /* Inline assembly with memory operand and clobber */
        asm volatile (
            "movl %1, %%eax\n\t"
            "addl $1, %%eax\n\t"
            "movl %%eax, %0\n\t"
            : "=m" (*addr1)
            : "m" (*addr1)
            : "eax", "r12", "r13", "r14", "memory"
        );
    }
    
    /* Jump to create control flow complexity */
    if (iter & 1) {
        goto after_asm;
    }
    
    /* Second block: different addressing mode */
    {
        /* Force RELOAD_FOR_OUTPUT_ADDRESS */
        volatile double *addr2 = &p1[offset2].value + iter;
        
        /* Another asm with conflicting constraints */
        register double temp asm ("xmm0");
        asm volatile (
            "movsd %1, %0\n\t"
            "addsd %2, %0\n\t"
            "movsd %0, %1\n\t"
            : "=x" (temp), "+m" (*addr2)
            : "m" (data_array[iter].value)
            : "xmm0", "r12", "r13", "memory"
        );
    }
    
    after_asm:
    
    /* Third block: pointer-to-pointer operations */
    {
        /* Force RELOAD_FOR_INPADDR_ADDRESS */
        int **pp = (int**)&p2[iter].pp;
        use_pointer_to_pointer(&pp);
        
        /* Complex chain of address computations */
        volatile int ****q = (volatile int****)&p2[offset1].pp;
        modify_through_indirect((volatile int****)q);
    }
    
    /* Fourth block: operand address reloads */
    {
        /* Force RELOAD_FOR_OPERAND_ADDRESS */
        register volatile char *cptr asm ("r15") = p1->tag;
        
        asm volatile (
            "movb $65, (%0)\n\t"
            "addb $1, 1(%0)\n\t"
            : 
            : "r" (cptr + iter)
            : "memory", "r15"
        );
    }
    
    /* Fifth block: output address reloads */
    {
        /* Force RELOAD_FOR_OUTPUT_ADDRESS */
        volatile long long *bigptr = &p1[iter].big + offset1;
        
        asm volatile (
            "lock xaddq %1, %0\n\t"
            : "+m" (*bigptr)
            : "r" ((long long)iter)
            : "cc", "r12", "memory"
        );
    }
}

/* Another stress function with different patterns */
static void stress_with_gotos(int count) {
    register volatile int *base asm ("r12") = (int*)data_array;
    register int index asm ("r13") = count * 2;
    
    /* Complex control flow with gotos */
    if (count & 1) goto block_a;
    else goto block_b;
    
    block_a:
    {
        /* Address computation that needs reloading */
        volatile int *addr = base + index * 3 + 7;
        
        asm volatile (
            "movl (%1), %%eax\n\t"
            "imull %%eax, %%eax\n\t"
            "movl %%eax, (%0)\n\t"
            : 
            : "r" (addr), "r" (&index)
            : "eax", "r12", "r13", "memory"
        );
        
        goto block_c;
    }
    
    block_b:
    {
        /* Different addressing mode */
        volatile int *addr2 = &base[index * 5 - 3];
        
        asm volatile (
            "lock decl %0\n\t"
            : "+m" (*addr2)
            : 
            : "cc", "r12", "memory"
        );
        
        goto block_c;
    }
    
    block_c:
    {
        /* Mixed addressing with multiple constraints */
        register int temp asm ("eax");
        volatile int *addr3 = base + count;
        
        asm volatile (
            "movl $100, %%eax\n\t"
            "subl %1, %%eax\n\t"
            "movl %%eax, %0\n\t"
            : "=m" (*addr3)
            : "m" (*addr3), "m" (count)
            : "eax", "r12", "memory"
        );
    }
}

/* Function that creates address computation chains */
static void chain_address_computations(void) {
    volatile struct MixedData *p = &data_array[0];
    volatile struct NestedPtrs *q = &ptr_array[0];
    
    /* Chain 1: multiple levels of indirection */
    {
        int *****chain5 = (int*****)&q[10].pp;
        volatile int value = *****(int*****)chain5;
        (void)value;
    }
    
    /* Chain 2: array indexing with mixed types */
    {
        char *cptr = (char*)p;
        double *dptr = (double*)(cptr + 17);
        int *iptr = (int*)((char*)dptr + 8);
        
        /* Force multiple reload types */
        asm volatile (
            "fldl %1\n\t"
            "fistpl %0\n\t"
            : "=m" (*iptr)
            : "m" (*dptr)
            : "st", "memory"
        );
    }
}

int main(void) {
    /* Initialize some data */
    for (int i = 0; i < 100; i++) {
        data_array[i].id = i;
        data_array[i].value = i * 1.5;
        data_array[i].tag[0] = 'A' + (i % 26);
        data_array[i].big = i * 1000LL;
    }
    
    for (int i = 0; i < 50; i++) {
        ptr_array[i].pp = (int**)&data_array[i].ptr;
        ptr_array[i].vp = &data_array[i].id;
    }
    
    /* Call stress functions multiple times with different parameters */
    for (int i = 0; i < 10; i++) {
        stress_address_calculations(i);
        stress_with_gotos(i);
    }
    
    chain_address_computations();
    
    /* Final complex block mixing everything */
    {
        register volatile struct MixedData *rp1 asm ("r12") = &data_array[20];
        register volatile struct MixedData *rp2 asm ("r13") = &data_array[30];
        register int idx asm ("r14") = 5;
        
        /* Multiple address computations in sequence */
        volatile int *a1 = &rp1[idx * 2].id;
        volatile double *a2 = &rp2[idx * 3].value;
        volatile char *a3 = rp1->tag + idx;
        
        asm volatile (
            "movl (%1), %%eax\n\t"
            "cvtsi2sd %%eax, %%xmm0\n\t"
            "movsd (%2), %%xmm1\n\t"
            "addsd %%xmm0, %%xmm1\n\t"
            "movsd %%xmm1, (%2)\n\t"
            "movb $42, (%3)\n\t"
            : 
            : "r" (a1), "r" (a2), "r" (a3)
            : "eax", "xmm0", "xmm1", "r12", "r13", "r14", "memory"
        );
    }
    
    return 0;
}
