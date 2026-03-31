/* reload_stress.c - Stress GCC's reload pass for uncovered switch cases */
#include <stdint.h>

/* Volatile structures to prevent optimization */
typedef struct {
    volatile int a;
    volatile double b;
    volatile char c[7];
    volatile int* d;
} MixedType;

typedef struct {
    volatile MixedType arr[50];
    volatile long long extra[10];
} BigStruct;

/* Global volatile arrays */
static volatile BigStruct global_data[5];
static volatile int* volatile ptr_array[100];

/* Helper functions that take pointer-to-pointer */
static void modify_pptr(int*** ppp) {
    volatile int dummy = (int)(intptr_t)*ppp;
    (void)dummy;
}

static void use_address(void* addr1, void* addr2) {
    volatile int dummy = (int)((intptr_t)addr1 ^ (intptr_t)addr2);
    (void)dummy;
}

/* Function with complex addressing patterns */
static void stress_reloads(int iter) {
    /* Bind specific variables to registers */
    register volatile MixedType* p1 asm ("r12");
    register volatile int* p2 asm ("r13");
    register volatile char* p3 asm ("r14");
    register int offset asm ("r15");
    
    /* Initialize with complex address computations */
    p1 = &global_data[iter % 3].arr[10];
    p2 = (volatile int*)&global_data[(iter + 1) % 3].arr[20];
    p3 = (volatile char*)&global_data[(iter + 2) % 3].arr[30];
    offset = iter * 7 + 3;
    
    /* Block 1: Complex addressing with multiple constraints */
    {
        volatile int* addr1;
        volatile double* addr2;
        
        /* Complex address computation forcing RELOAD_FOR_INPUT_ADDRESS */
        addr1 = &p1[offset / 5].a + (offset % 3);
        addr2 = (volatile double*)((char*)&p1[offset / 7].b + (offset % 11));
        
        /* Inline assembly with memory operands and clobbers */
        asm volatile (
            "movl %1, %%eax\n\t"
            "addl %%eax, %0\n\t"
            : "+m" (*addr1)
            : "r" (offset)
            : "eax", "r12", "r13", "r14", "memory"
        );
        
        /* Jump to create control flow complexity */
        if (iter & 1) goto compute_more;
        
        /* Another asm with different constraints */
        asm volatile (
            "movq %1, %%xmm0\n\t"
            "addsd %%xmm0, %0\n\t"
            : "+m" (*addr2)
            : "m" (*addr2)
            : "xmm0", "r12", "r13", "memory"
        );
    }
    
compute_more:
    /* Block 2: More complex addressing for RELOAD_FOR_OUTPUT_ADDRESS */
    {
        volatile int** pptr;
        volatile long long* llptr;
        
        /* Address computation that may need RELOAD_FOR_OUTPUT_ADDRESS */
        pptr = (volatile int**)&p2[offset * 2];
        llptr = (volatile long long*)&global_data[iter % 2].extra[offset % 5];
        
        /* Nested function call with address-taken argument */
        modify_pptr((int***)&pptr);
        
        /* Inline assembly with output memory operand */
        asm volatile (
            "movq %1, %%rax\n\t"
            "movq %%rax, %0\n\t"
            : "=m" (*llptr)
            : "r" ((long long)offset * 17)
            : "rax", "r12", "r13", "r14", "r15", "memory"
        );
        
        /* Use goto to jump back and forth */
        if (iter & 2) goto final_block;
    }
    
    /* Block 3: Addressing for RELOAD_FOR_INPADDR_ADDRESS */
    {
        volatile char* base = p3 + offset;
        volatile int* derived;
        
        /* Complex chain of address computations */
        derived = (volatile int*)(base + (offset % 19) * 3);
        
        /* Inline assembly with input address reload */
        asm volatile (
            "movl (%1), %%ebx\n\t"
            "imull %%ebx, %0\n\t"
            : "+r" (offset)
            : "r" (derived)
            : "ebx", "r12", "r13", "r14", "memory"
        );
        
        /* Function call using computed address */
        use_address((void*)derived, (void*)&offset);
    }
    
final_block:
    /* Block 4: Mixed addressing modes for various reload types */
    {
        volatile MixedType* mptr;
        volatile int* iptr_array[4];
        
        /* Multiple address computations in same block */
        mptr = &global_data[0].arr[offset % 20];
        iptr_array[0] = &mptr->a;
        iptr_array[1] = (volatile int*)&mptr->b;
        iptr_array[2] = (volatile int*)&mptr->c[offset % 7];
        iptr_array[3] = mptr->d;
        
        /* Loop with complex addressing */
        for (int i = 0; i < 4; i++) {
            if (iptr_array[i]) {
                /* Inline assembly that may trigger RELOAD_FOR_OPERAND_ADDRESS */
                asm volatile (
                    "movl %1, %%ecx\n\t"
                    "orl %%ecx, %0\n\t"
                    : "+m" (*iptr_array[i])
                    : "r" (i * 0x1001)
                    : "ecx", "r12", "r13", "r14", "r15", "memory"
                );
            }
            
            /* Small jump to disrupt register allocation */
            if (i == 2) goto skip_point;
            continue;
            
        skip_point:
            /* Additional computation at skip point */
            volatile int temp = *iptr_array[i] + offset;
            (void)temp;
        }
        
        /* Final asm for RELOAD_FOR_OTHER_ADDRESS */
        asm volatile (
            "leal (%1, %2, 4), %%edx\n\t"
            "movl %%edx, %0\n\t"
            : "=m" (global_data[0].arr[0].a)
            : "r" (offset), "r" (iter)
            : "edx", "r12", "r13", "r14", "r15", "memory"
        );
    }
}

/* Another stress function with different patterns */
static void more_stress(int base) {
    register volatile int* r1 asm ("r12");
    register volatile char* r2 asm ("r13");
    
    r1 = (volatile int*)&global_data[base % 4];
    r2 = (volatile char*)&ptr_array[base % 50];
    
    /* Complex addressing with pointer arithmetic */
    volatile int** addr_of_addr = (volatile int**)(r2 + base * 3);
    
    /* This may trigger RELOAD_FOR_OUTADDR_ADDRESS */
    asm volatile (
        "movq %1, %%rsi\n\t"
        "movq (%%rsi), %%rdi\n\t"
        "addl $1, (%%rdi)\n\t"
        : 
        : "r" (addr_of_addr)
        : "rsi", "rdi", "r12", "r13", "memory"
    );
    
    /* Chain of address computations */
    volatile int* final_ptr = *addr_of_addr + base;
    
    /* Mixed constraints in asm */
    asm volatile (
        "movl %1, %%r8d\n\t"
        "xorl %%r8d, %0\n\t"
        : "+m" (*final_ptr)
        : "r" (base * 0xABCD)
        : "r8", "r12", "r13", "memory"
    );
}

int main(void) {
    /* Initialize some data */
    for (int i = 0; i < 5; i++) {
        for (int j = 0; j < 50; j++) {
            global_data[i].arr[j].a = i * 100 + j;
            global_data[i].arr[j].b = i * 100.0 + j;
            global_data[i].arr[j].d = (volatile int*)&global_data[(i + 1) % 5];
        }
    }
    
    /* Call stress functions multiple times with different parameters */
    for (int i = 0; i < 10; i++) {
        stress_reloads(i);
        more_stress(i * 7);
        
        /* Additional inline complexity in main */
        register volatile MixedType* mp asm ("r12");
        mp = &global_data[i % 3].arr[i % 20];
        
        volatile int* volatile* pp;
        pp = (volatile int* volatile*)&mp->d;
        
        /* Address computation that may need various reloads */
        asm volatile (
            "movq %1, %%rax\n\t"
            "movq (%%rax), %%rbx\n\t"
            "addl $42, (%%rbx)\n\t"
            : 
            : "r" (pp)
            : "rax", "rbx", "r12", "memory"
        );
    }
    
    return 0;
}
