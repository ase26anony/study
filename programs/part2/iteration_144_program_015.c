/* reload_stress.c - Stress GCC's reload pass with complex addressing modes */
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
static volatile int* volatile global_ptrs[20];

/* Helper functions that take pointer-to-pointer */
static void modify_pptr(int** pp) {
    if (pp && *pp) {
        **pp += 1;
    }
}

static void compute_address(void* addr1, void* addr2) {
    /* Force address computation */
    volatile int dummy = *(volatile int*)addr1 + *(volatile int*)addr2;
    (void)dummy;
}

/* Function with complex addressing patterns */
static void stress_reloads(int iter) {
    /* Bind specific variables to registers */
    register volatile MixedType* p1 asm ("r12");
    register volatile MixedType* p2 asm ("r13");
    register int offset asm ("r14");
    
    /* Initialize with complex addressing */
    p1 = &global_data[iter % 10].arr[0];
    p2 = &global_data[(iter + 1) % 10].arr[50];
    offset = iter * 3 + 7;
    
    /* Complex addressing computation */
    volatile int* addr1 = &p1[offset].a;
    volatile double* addr2 = &p2[-offset / 2].b;
    
    /* Inline assembly with memory operands and clobbers */
    asm volatile (
        "movl %1, %%eax\n\t"
        "addl %%eax, %0\n\t"
        : "+m" (*addr1)
        : "m" (*addr2)
        : "eax", "r12", "r13", "r14", "memory"
    );
    
    /* Jump to create control flow complexity */
    if (iter & 1) {
        goto compute_more;
    } else {
        goto use_assembly;
    }
    
compute_more:
    /* Different addressing mode */
    register volatile char* p3 asm ("r12");  /* Reuse r12 */
    p3 = &p1[offset * 2].c[3];
    
    /* Nested pointer indirection */
    int** pptr = (int**)&p1[iter].d;
    modify_pptr(pptr);
    
use_assembly:
    /* Another inline asm with conflicting constraints */
    volatile int temp;
    asm volatile (
        "leal (%1, %2, 4), %%ecx\n\t"
        "movl (%%ecx), %%edx\n\t"
        "movl %%edx, %0\n\t"
        : "=r" (temp)
        : "r" (p1), "r" (offset)
        : "ecx", "edx", "r12", "memory"
    );
    
    /* Complex array indexing with mixed types */
    for (int i = 0; i < 3; i++) {
        volatile MixedType* elem = &global_data[(iter + i) % 10].arr[i * 15];
        
        /* Address of address computation */
        int* addr_of_addr = &elem->a;
        compute_address((void*)addr_of_addr, (void*)&temp);
        
        /* Inline asm with "m" constraint on computed address */
        asm volatile (
            "movl %1, %%ebx\n\t"
            "incl (%%ebx)\n\t"
            : 
            : "m" (*addr_of_addr)
            : "ebx", "memory"
        );
    }
}

/* Function specifically for output address reloads */
static void stress_output_address(void) {
    register volatile long long* out_ptr asm ("r15");
    out_ptr = &global_data[0].extra[0];
    
    /* Multiple output operands in asm */
    volatile long long result1, result2;
    
    asm volatile (
        "movq (%1), %%rax\n\t"
        "movq 8(%1), %%rbx\n\t"
        "addq %%rbx, %%rax\n\t"
        "movq %%rax, %0\n\t"
        "movq %%rbx, %2\n\t"
        : "=m" (*out_ptr), "=m" (result1), "=m" (result2)
        : "1" (out_ptr)
        : "rax", "rbx", "r15", "memory"
    );
    
    /* Chain of address computations */
    volatile int* chain[5];
    for (int i = 0; i < 5; i++) {
        chain[i] = &global_data[i].arr[i * 10].a;
    }
    
    /* Use goto to break linear flow */
    int j = 0;
loop_start:
    if (j >= 5) goto loop_end;
    
    /* Inline asm that clobbers address registers */
    asm volatile (
        "movl %1, %%r12d\n\t"
        "addl $1, (%%r12)\n\t"
        : 
        : "m" (chain[j])
        : "r12", "memory"
    );
    
    j++;
    goto loop_start;
    
loop_end:
    /* Final complex addressing */
    register int* final_addr asm ("r12");
    final_addr = chain[3] + 2;
    
    asm volatile (
        "movl (%0), %%eax\n\t"
        "notl %%eax\n\t"
        "movl %%eax, (%0)\n\t"
        : 
        : "r" (final_addr)
        : "eax", "r12", "memory"
    );
}

/* Main function with initialization and calls */
int main(void) {
    /* Initialize some data */
    for (int i = 0; i < 10; i++) {
        for (int j = 0; j < 100; j++) {
            global_data[i].arr[j].a = i * 100 + j;
            global_data[i].arr[j].b = (double)(i + j) / 2.0;
            global_data[i].arr[j].d = (int*)&global_data[i].arr[j].a;
        }
    }
    
    /* Multiple iterations with different parameters */
    for (int i = 0; i < 5; i++) {
        stress_reloads(i);
        
        /* Intermix with pointer-to-pointer operations */
        volatile int local_var = i * 10;
        int* local_ptr = &local_var;
        int** pptr = &local_ptr;
        
        /* Force address reload for pptr */
        asm volatile (
            "movq %0, %%r12\n\t"
            "movq (%%r12), %%r13\n\t"
            "addl $1, (%%r13)\n\t"
            : 
            : "m" (pptr)
            : "r12", "r13", "memory"
        );
        
        /* Complex array indexing in loop */
        for (int j = 0; j < 3; j++) {
            volatile MixedType* elem = 
                &global_data[(i + j) % 10].arr[(i * j + 7) % 100];
            
            /* Multiple addressing modes in one expression */
            volatile int* addr_array[2];
            addr_array[0] = &elem->a;
            addr_array[1] = (int*)&elem->b;
            
            /* Inline asm using both addresses */
            asm volatile (
                "movq %0, %%r12\n\t"
                "movl (%%r12), %%eax\n\t"
                "movq %1, %%r13\n\t"
                "movl (%%r13), %%ebx\n\t"
                "addl %%ebx, %%eax\n\t"
                "movl %%eax, (%%r12)\n\t"
                : 
                : "m" (addr_array[0]), "m" (addr_array[1])
                : "rax", "rbx", "r12", "r13", "memory"
            );
        }
    }
    
    /* Stress output addresses */
    stress_output_address();
    
    /* Final complex pattern */
    register volatile int* final_p asm ("r12");
    final_p = &global_data[9].arr[99].a;
    
    volatile int final_result;
    asm volatile (
        "movl (%1), %%eax\n\t"
        "imull $37, %%eax, %%eax\n\t"
        "movl %%eax, %0\n\t"
        : "=m" (final_result)
        : "r" (final_p)
        : "rax", "r12", "memory"
    );
    
    return final_result;
}
