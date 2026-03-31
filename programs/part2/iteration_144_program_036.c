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

/* Global volatile arrays */
static volatile BigStruct global_data[10];
static volatile int* volatile ptr_array[20];

/* Helper functions that take pointer-to-pointer arguments */
static void modify_pptr(volatile int*** ppp) {
    **ppp += 1;
}

static void complex_address_helper(volatile MixedType**** mppp, int offset) {
    ***mppp += offset;
}

/* Function with complex addressing patterns */
static void stress_reloads(int iter) {
    /* Bind specific variables to registers */
    register volatile MixedType* p1 asm ("r12");
    register volatile int* p2 asm ("r13");
    register volatile char* p3 asm ("r14");
    register int index asm ("r15");
    
    /* Initialize register-bound pointers */
    p1 = &global_data[0].arr[0];
    p2 = &global_data[1].arr[10].a;
    p3 = &global_data[2].arr[20].c[0];
    index = iter;
    
    /* Complex addressing computation block A */
    {
        volatile int* addr1;
        volatile double* addr2;
        
        /* Compute addresses using register-bound pointers with non-constant offsets */
        addr1 = &p1[(index * 3) % 50].a + (index & 7);
        addr2 = &p1[((index + 1) * 5) % 50].b - (index % 3);
        
        /* Inline assembly with memory operands and clobbers */
        asm volatile (
            "addl $1, %[mem1]\n\t"
            "fldl %[mem2]\n\t"
            "fstpl %[mem2]\n\t"
            : [mem1] "+m" (*addr1), [mem2] "+m" (*addr2)
            : 
            : "r12", "r13", "r14", "r15", "memory", "st", "st(1)"
        );
        
        /* Jump to force control flow complexity */
        goto after_block_a;
        
        /* Unreachable but forces label creation */
        {
            volatile int dummy = 0;
            dummy++;
        }
    }
    
after_block_a:
    
    /* Re-initialize register variables for different use */
    p2 = &global_data[3].arr[30].a;
    p3 = &global_data[4].arr[40].c[2];
    
    /* Nested addressing with pointer-to-pointer */
    {
        volatile int** pptr;
        volatile MixedType*** mpptr;
        int local_var = index * 2;
        
        /* Complex address computation */
        pptr = (volatile int**)&p2 + (local_var & 3);
        mpptr = (volatile MixedType***)&ptr_array[5] + (local_var % 2);
        
        /* Call function with address-taken arguments */
        modify_pptr((volatile int***)pptr);
        
        /* More inline assembly with conflicting constraints */
        asm volatile (
            "movl %%r13d, %%eax\n\t"
            "leal (%%rax, %%r15, 4), %%ebx\n\t"
            "movl %%ebx, %[out]\n\t"
            : [out] "=m" (local_var)
            : 
            : "rax", "rbx", "r12", "r13", "r14", "r15", "cc"
        );
        
        /* Use computed address in memory operation */
        *((volatile int*)((uintptr_t)p2 + (local_var * 4))) += 1;
    }
    
    /* Block for output address reloads */
    {
        volatile int output_data[50];
        register volatile int* out_ptr asm ("r12");
        
        out_ptr = &output_data[20];
        
        /* Inline assembly with output memory operand */
        asm volatile (
            "movl $42, %%eax\n\t"
            "movl %%eax, %[out]\n\t"
            : [out] "=m" (*out_ptr)
            : 
            : "rax", "r12", "memory"
        );
        
        /* Complex output address computation */
        volatile int* out_addr = out_ptr + (index * 3) % 10;
        
        /* Another asm with output address */
        asm volatile (
            "incl %[outaddr]\n\t"
            : [outaddr] "+m" (*out_addr)
            : 
            : "r12", "r13", "memory"
        );
    }
    
    /* Block for input address reloads */
    {
        volatile double input_buffer[100];
        register volatile double* in_ptr asm ("r13");
        
        in_ptr = &input_buffer[30];
        
        /* Inline assembly with input memory operand */
        double result;
        asm volatile (
            "fldl %[in]\n\t"
            "fsqrt\n\t"
            "fstpl %[res]\n\t"
            : [res] "=m" (result)
            : [in] "m" (in_ptr[(index * 7) % 20])
            : "r13", "st", "st(1)", "memory"
        );
        
        /* Jump back to create loop in control flow */
        if (iter > 0) {
            goto after_block_a;  /* Creates complex control flow */
        }
    }
    
    /* Final block with operand address reloads */
    {
        volatile MixedType* base = &global_data[5].arr[0];
        volatile int** indirect = (volatile int**)&base[10].d;
        
        /* Very complex addressing chain */
        volatile int* final_addr = *indirect + 
                                  ((uintptr_t)base >> 4) + 
                                  (index * 11) % 100;
        
        /* Assembly using the computed address */
        asm volatile (
            "movl %[addr], %%esi\n\t"
            "movl (%%esi), %%eax\n\t"
            "addl $100, %%eax\n\t"
            "movl %%eax, (%%esi)\n\t"
            : 
            : [addr] "r" (final_addr)
            : "rax", "rsi", "r12", "r13", "r14", "r15", "memory", "cc"
        );
    }
}

/* Main function that creates the stress pattern */
int main(void) {
    /* Initialize some data */
    for (int i = 0; i < 10; i++) {
        for (int j = 0; j < 100; j++) {
            global_data[i].arr[j].a = i * 1000 + j;
            global_data[i].arr[j].b = (double)(i + j) / 2.0;
            for (int k = 0; k < 7; k++) {
                global_data[i].arr[j].c[k] = (char)((i + j + k) & 0xFF);
            }
            global_data[i].arr[j].d = (volatile int*)&global_data[(i + 1) % 10].arr[j].a;
        }
    }
    
    /* Multiple iterations with different parameters */
    for (int i = 0; i < 5; i++) {
        stress_reloads(i);
        
        /* Additional inline assembly between calls */
        register int temp asm ("r12");
        temp = i * 100;
        
        asm volatile (
            "movl %%r12d, %%eax\n\t"
            "imull $37, %%eax, %%ebx\n\t"
            "movl %%ebx, %%r12d\n\t"
            : 
            : 
            : "rax", "rbx", "r12", "cc"
        );
        
        /* Call again with modified parameter */
        stress_reloads(temp % 10);
    }
    
    /* Final complex addressing pattern */
    {
        volatile MixedType*** triple_ptr;
        volatile int offset = 17;
        
        triple_ptr = (volatile MixedType***)&global_data[6].arr[60].d;
        
        /* This should trigger various address reload types */
        complex_address_helper(triple_ptr, offset);
        
        /* One more asm with multiple clobbers */
        asm volatile (
            "movq %[ptr], %%r12\n\t"
            "movq (%%r12), %%r13\n\t"
            "addq $8, %%r13\n\t"
            "movq %%r13, (%%r12)\n\t"
            : 
            : [ptr] "r" (&ptr_array[0])
            : "r12", "r13", "memory", "cc"
        );
    }
    
    return 0;
}
