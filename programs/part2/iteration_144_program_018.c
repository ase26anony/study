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
    volatile MixedType arr1[100];
    volatile int arr2[200];
    volatile double arr3[50];
} BigStruct;

/* Global volatile data */
static volatile BigStruct global_data;
static volatile int* volatile global_ptr_array[20];

/* Function taking pointer-to-pointer */
void use_double_ptr(int*** ppp) {
    volatile int dummy = ***ppp;
    (void)dummy;
}

/* Another function with complex addressing */
void complex_addressing(volatile MixedType* base, int idx1, int idx2) {
    /* Do nothing meaningful, just use the address */
    volatile char c = base[idx1].c[idx2];
    (void)c;
}

/* Main stress function */
void stress_reload(void) {
    /* Bind specific variables to registers */
    register volatile MixedType* p1 asm ("r12");
    register volatile int* p2 asm ("r13");
    register volatile double* p3 asm ("r14");
    register int index asm ("r15");
    
    /* Initialize pointers */
    p1 = &global_data.arr1[0];
    p2 = &global_data.arr2[0];
    p3 = &global_data.arr3[0];
    
    /* Complex addressing with multiple constraints */
    for (index = 0; index < 5; ++index) {
        /* Block A: Complex pointer arithmetic */
        volatile MixedType* addr1 = p1 + index * 3;
        volatile int* addr2 = p2 + index * 7;
        volatile double* addr3 = p3 + index * 2;
        
        /* Inline assembly with memory operands and clobbers */
        asm volatile (
            "movl (%[a1]), %%eax\n\t"
            "addl (%[a2]), %%eax\n\t"
            "movl %%eax, (%[a1])\n\t"
            : 
            : [a1] "m" (*addr1), [a2] "m" (*addr2)
            : "eax", "r12", "r13", "memory"
        );
        
        /* Jump to force control flow complexity */
        if (index & 1) {
            goto compute_address;
        } else {
            goto use_address;
        }
        
compute_address:
        /* Recompute addresses using same registers for different purposes */
        {
            volatile char* char_ptr = (volatile char*)p1;
            char_ptr += index * sizeof(MixedType) + 8;
            
            /* Another asm with different constraints */
            asm volatile (
                "movb (%[ptr]), %%al\n\t"
                "addb $1, %%al\n\t"
                "movb %%al, (%[ptr])\n\t"
                :
                : [ptr] "r" (char_ptr)
                : "eax", "r12", "memory"
            );
        }
        
        goto next_iteration;
        
use_address:
        /* Use address in nested function call */
        {
            volatile int local_var = 42;
            volatile int* ptr_to_local = &local_var;
            volatile int** ptr_to_ptr = &ptr_to_local;
            
            /* This should trigger address reloads */
            use_double_ptr((int***)&ptr_to_ptr);
            
            /* Complex array indexing */
            global_ptr_array[index] = (int*)p2 + index * 11;
        }
        
next_iteration:
        /* More complex addressing with mixed types */
        {
            /* Force RELOAD_FOR_INPUT_ADDRESS and similar */
            volatile int offset = index * 17;
            volatile MixedType* complex_addr = p1 + (offset % 10);
            
            /* Access with non-constant offset */
            complex_addressing(complex_addr, index % 3, index % 5);
            
            /* Inline asm with multiple memory constraints */
            asm volatile (
                "movsd (%[dbl]), %%xmm0\n\t"
                "addsd (%[dbl2]), %%xmm0\n\t"
                "movsd %%xmm0, (%[dbl])\n\t"
                :
                : [dbl] "m" (complex_addr->b), 
                  [dbl2] "m" (p3[index % 5])
                : "xmm0", "r12", "r14", "memory"
            );
        }
        
        /* Address computation for output */
        {
            volatile int* out_addr = p2 + index * 13 + 7;
            
            /* Asm with output memory operand */
            asm volatile (
                "movl $0x12345678, %[out]\n\t"
                : [out] "=m" (*out_addr)
                :
                : "r13", "memory"
            );
        }
    }
    
    /* Final complex pattern with goto spaghetti */
    {
        register volatile int* alt_p asm ("r12");
        alt_p = p2 + 50;
        
    block1:
        {
            volatile int val = *alt_p;
            alt_p += 3;
            if (val != 0) goto block3;
        }
        
    block2:
        {
            /* Force address reload for operand */
            asm volatile (
                "incl %0\n\t"
                : "+m" (*alt_p)
                :
                : "r12", "memory"
            );
            goto block4;
        }
        
    block3:
        {
            alt_p -= 2;
            /* Different addressing mode */
            asm volatile (
                "decl %0\n\t"
                : "+m" (*(alt_p + 1))
                :
                : "r12", "memory"
            );
        }
        
    block4:
        /* Use address in pointer-to-pointer call */
        {
            volatile int** pp = (volatile int**)&alt_p;
            use_double_ptr((int***)pp);
        }
    }
}

/* Additional stress patterns */
void more_stress(void) {
    /* Array of pointers with complex addressing */
    volatile int* ptr_array[10];
    register int i asm ("r15");
    
    for (i = 0; i < 10; i++) {
        /* Non-contiguous memory access pattern */
        ptr_array[i] = (volatile int*)&global_data.arr2[i * 19 % 200];
        
        /* Inline asm that clobbers address registers */
        asm volatile (
            "movl (%[base]), %%ebx\n\t"
            "imull %[idx], %%ebx\n\t"
            "movl %%ebx, (%[base])\n\t"
            :
            : [base] "r" (ptr_array[i]), [idx] "r" (i)
            : "ebx", "r15", "memory"
        );
        
        /* Chain of address computations */
        if (i > 0) {
            volatile int** chain_ptr = (volatile int**)&ptr_array[i-1];
            volatile int* final_addr = *chain_ptr + i * 3;
            
            /* Mixed constraints in asm */
            asm volatile (
                "addl $1, %[mem]\n\t"
                : [mem] "+m" (*final_addr)
                :
                : "r12", "r13", "memory"
            );
        }
    }
    
    /* Structure with pointer members */
    {
        typedef struct {
            volatile int* p1;
            volatile double* p2;
            volatile char* p3;
        } PtrStruct;
        
        volatile PtrStruct ps;
        ps.p1 = (volatile int*)&global_data.arr1[10].a;
        ps.p2 = &global_data.arr3[20];
        ps.p3 = (volatile char*)&global_data.arr1[5].c[3];
        
        /* Simultaneous use of multiple pointer types */
        asm volatile (
            "movl (%[p1]), %%eax\n\t"
            "cvtsi2sd %%eax, %%xmm0\n\t"
            "addsd (%[p2]), %%xmm0\n\t"
            "movsd %%xmm0, (%[p2])\n\t"
            "movb (%[p3]), %%al\n\t"
            "addb $1, %%al\n\t"
            "movb %%al, (%[p3])\n\t"
            :
            : [p1] "m" (ps.p1), [p2] "m" (ps.p2), [p3] "m" (ps.p3)
            : "eax", "xmm0", "memory"
        );
    }
}

int main(void) {
    /* Initialize some data */
    for (int i = 0; i < 20; i++) {
        global_ptr_array[i] = (int*)&global_data.arr2[i * 10];
    }
    
    /* Call stress functions multiple times */
    stress_reload();
    more_stress();
    
    /* One more pattern in main */
    {
        register volatile MixedType* reg_ptr asm ("r12");
        reg_ptr = &global_data.arr1[30];
        
        /* Complex expression as array index */
        volatile int idx = 5;
        volatile MixedType* addr = reg_ptr + (idx * 2 + 1);
        
        /* Asm with both input and output memory constraints */
        asm volatile (
            "movl 0(%[addr]), %%eax\n\t"
            "addl $42, %%eax\n\t"
            "movl %%eax, 0(%[addr])\n\t"
            "movl 16(%[addr]), %%ebx\n\t"
            "subl $1, %%ebx\n\t"
            "movl %%ebx, 16(%[addr])\n\t"
            :
            : [addr] "r" (addr)
            : "eax", "ebx", "r12", "memory"
        );
        
        /* Nested address-of operations */
        {
            volatile int local = 100;
            volatile int* p1 = &local;
            volatile int** p2 = &p1;
            volatile int*** p3 = &p2;
            
            use_double_ptr(p3);
        }
    }
    
    return 0;
}
