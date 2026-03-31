/* reload_stress.c - Stress GCC's reload pass for coverage testing */
#include <stdint.h>

/* Volatile structures with mixed types to prevent optimization */
typedef struct {
    volatile int a;
    volatile double b;
    volatile char c[7];
    volatile int* d;
} MixedType;

typedef struct {
    volatile MixedType arr[100];
    volatile long long big[50];
} BigStruct;

/* Global volatile arrays to force memory accesses */
static volatile BigStruct global_data[10];
static volatile int* volatile ptr_array[100];

/* Helper functions that take pointer-to-pointer arguments */
static void modify_pptr(volatile int*** ppp) {
    **ppp += 1;
}

static void compute_address(volatile void** addr, int offset) {
    *addr = (volatile void*)((uintptr_t)*addr + offset);
}

/* Main stress function with complex addressing patterns */
static void stress_reload(void) {
    /* Bind specific registers for address computation */
    register volatile MixedType* p1 asm ("r12") = &global_data[0].arr[0];
    register volatile long long* p2 asm ("r13") = &global_data[1].big[0];
    register int index asm ("r14") = 0;
    
    volatile int local_vars[50];
    volatile double local_doubles[20];
    
    /* Complex addressing mode 1: Array indexing with register base */
    for (int i = 0; i < 5; i++) {
        /* Force RELOAD_FOR_INPUT_ADDRESS */
        p1[index + i * 3].a = local_vars[i];
        
        /* Inline assembly with memory operand and clobber */
        asm volatile (
            "addl $1, %[mem]\n\t"
            : [mem] "+m" (p1[index + i * 2].a)
            : 
            : "r12", "r13", "r14", "memory"
        );
        
        /* Jump to create complex control flow */
        if (i & 1) {
            goto compute_addr;
        } else {
            goto use_asm;
        }
        
compute_addr:
        /* Force RELOAD_FOR_INPADDR_ADDRESS */
        compute_address((volatile void**)&p1, i * sizeof(MixedType));
        
use_asm:
        /* Inline assembly with multiple constraints */
        volatile int temp = i * 2;
        asm volatile (
            "movl %[in], %%eax\n\t"
            "movl %%eax, %[out]\n\t"
            : [out] "=m" (p2[i])
            : [in] "r" (temp), "m" (p1[i].a)
            : "eax", "r12", "r13", "memory"
        );
    }
    
    /* Reset registers */
    p1 = &global_data[2].arr[10];
    p2 = &global_data[3].big[5];
    
    /* Force RELOAD_FOR_OUTPUT_ADDRESS */
    for (int j = 0; j < 4; j++) {
        /* Complex pointer arithmetic */
        volatile MixedType* p3 = p1 + j * 7;
        volatile long long* p4 = p2 - j * 3;
        
        /* Inline assembly with output memory operand */
        asm volatile (
            "movq $0x123456789ABCDEF0, %[out]\n\t"
            : [out] "=m" (*p4)
            : 
            : "r12", "r13", "memory"
        );
        
        /* Pass address of address to function */
        volatile int** pptr = (volatile int**)&p3->d;
        modify_pptr((volatile int***)&pptr);
        
        /* Force RELOAD_FOR_OUTADDR_ADDRESS */
        asm volatile (
            "leaq (%[base],%[idx],8), %%r15\n\t"
            "movq %%r15, %[dest]\n\t"
            : [dest] "=m" (local_doubles[j])
            : [base] "r" (p2), [idx] "r" (j)
            : "r15", "r12", "r13", "memory"
        );
    }
    
    /* More complex patterns for other reload types */
    {
        register volatile char* p5 asm ("r12") = (volatile char*)&global_data[4];
        register int offset asm ("r13") = 37;
        
        /* Force RELOAD_FOR_OPERAND_ADDRESS */
        for (int k = 0; k < 3; k++) {
            /* Non-contiguous memory access */
            volatile char val = p5[offset + k * 19];
            
            asm volatile (
                "movb %[val], %%al\n\t"
                "addb $1, %%al\n\t"
                "movb %%al, %[out]\n\t"
                : [out] "=m" (ptr_array[k] = (volatile int*)&val)
                : [val] "m" (val)
                : "al", "r12", "r13", "memory"
            );
            
            /* Jump to different address computation block */
            if (k == 1) goto addr_compute;
        }
        
        goto done;
        
addr_compute:
        /* Force RELOAD_FOR_OPADDR_ADDR */
        asm volatile (
            "addq $64, %[ptr]\n\t"
            : [ptr] "+r" (p5)
            : 
            : "memory"
        );
        
        /* Use the modified pointer */
        p5[offset] = 0xFF;
        
done:
        /* Force RELOAD_FOR_OTHER_ADDRESS */
        volatile void* dynamic_addr = (volatile void*)(p5 + offset * 2);
        
        asm volatile (
            "movq %[addr], %%rbx\n\t"
            "movl $42, (%%rbx)\n\t"
            : 
            : [addr] "r" (dynamic_addr)
            : "rbx", "memory"
        );
    }
    
    /* Final complex pattern mixing everything */
    {
        volatile int*** triple_ptr = (volatile int***)&ptr_array[50];
        volatile int complex_index = 25;
        
        /* Chain of address computations */
        for (int m = 0; m < 2; m++) {
            /* Multiple address calculations in one expression */
            volatile int* addr1 = (volatile int*)&global_data[m].arr[complex_index + m].a;
            volatile int* addr2 = (volatile int*)&global_data[m + 1].arr[complex_index - m].a;
            
            /* Inline assembly with both input and output addresses */
            asm volatile (
                "movl (%[in]), %%ecx\n\t"
                "addl $100, %%ecx\n\t"
                "movl %%ecx, (%[out])\n\t"
                : 
                : [in] "r" (addr1), [out] "r" (addr2)
                : "ecx", "memory"
            );
            
            /* Modify pointer through multiple indirections */
            *triple_ptr = (volatile int**)&addr1;
            **triple_ptr = addr2;
        }
    }
}

int main(void) {
    /* Initialize some data */
    for (int i = 0; i < 10; i++) {
        for (int j = 0; j < 100; j++) {
            global_data[i].arr[j].a = i * 100 + j;
            global_data[i].arr[j].b = (double)(i + j) * 0.5;
        }
        for (int j = 0; j < 50; j++) {
            global_data[i].big[j] = (long long)(i * j) << 32;
        }
    }
    
    /* Call stress function multiple times with different parameters */
    stress_reload();
    
    /* Additional stress patterns in main */
    {
        register volatile int* rp asm ("r12") = (volatile int*)&global_data[5];
        volatile int local = 0;
        
        for (int i = 0; i < 3; i++) {
            /* Complex addressing with inline assembly */
            asm volatile (
                "imull $3, %%r12d, %%eax\n\t"
                "cltq\n\t"
                "movl (%[base],%%rax,4), %%ebx\n\t"
                "movl %%ebx, %[out]\n\t"
                : [out] "=m" (local)
                : [base] "r" (rp), "r" (i)
                : "rax", "rbx", "r12", "memory"
            );
            
            /* Use goto to break linear flow */
            if (local > 1000) goto unexpected;
            
            /* More address computation */
            rp = (volatile int*)((char*)rp + sizeof(MixedType));
        }
        
        goto finish;
        
unexpected:
        /* Alternative path with different addressing */
        rp = (volatile int*)&global_data[6].big[10];
        
finish:
        /* Final memory access */
        *rp = 0xDEADBEEF;
    }
    
    return 0;
}
