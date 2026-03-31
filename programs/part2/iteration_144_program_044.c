/* reload_stress.c - Stress GCC's reload pass for uncovered switch cases */
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
    volatile long long extra[50];
} BigStruct;

/* Global volatile arrays to force memory accesses */
volatile BigStruct global_data[10];
volatile int global_ints[1000];
volatile double global_doubles[500];

/* Helper functions that take pointer-to-pointer arguments */
void modify_pptr(int*** ppp) {
    volatile int dummy = ***ppp;
    (void)dummy;
}

void compute_address(void** addr, int offset) {
    volatile int dummy = *(int*)((char*)*addr + offset);
    (void)dummy;
}

/* Function with complex addressing patterns */
void stress_reloads(int iter) {
    /* Bind specific pointers to explicit registers */
    register volatile MixedType* p1 asm ("r12");
    register volatile int* p2 asm ("r13");
    register volatile double* p3 asm ("r14");
    register int offset asm ("r15");
    
    /* Initialize with complex expressions */
    p1 = &global_data[iter % 10].arr[iter % 20];
    p2 = &global_ints[iter * 3 % 1000];
    p3 = &global_doubles[iter * 7 % 500];
    offset = iter * 11;
    
    /* Complex control flow with goto */
    if (iter & 1) goto compute_block1;
    else goto compute_block2;
    
compute_block1:
    {
        /* Complex address computation for input */
        volatile int* addr1 = (volatile int*)((char*)p1 + offset * sizeof(int));
        
        /* Inline assembly with memory operand and clobber */
        asm volatile (
            "movl %1, %%eax\n\t"
            "addl $1, %%eax\n\t"
            "movl %%eax, %0\n\t"
            : "=m" (*addr1)
            : "m" (*addr1)
            : "eax", "r12", "r13", "r14", "r15", "memory"
        );
        
        /* Nested function call with address-taken argument */
        int** temp_ptr = (int**)&addr1;
        modify_pptr(&temp_ptr);
        
        goto after_asm;
    }
    
compute_block2:
    {
        /* Different addressing mode for output */
        volatile double* addr2 = p3 + (offset / 8) % 50;
        
        /* Inline assembly with multiple constraints */
        double temp;
        asm volatile (
            "movsd %1, %%xmm0\n\t"
            "addsd %%xmm0, %%xmm0\n\t"
            "movsd %%xmm0, %0\n\t"
            : "=m" (*addr2), "=r" (temp)
            : "m" (*addr2), "1" (1.0)
            : "xmm0", "r12", "r13", "r14", "r15", "memory"
        );
        
        /* Complex chain of address computations */
        void* complex_addr = (void*)((char*)p2 + (offset << 2));
        compute_address(&complex_addr, offset % 64);
        
        goto after_asm;
    }
    
after_asm:
    /* More complex addressing with pointer arithmetic */
    for (int i = 0; i < 3; i++) {
        /* Force RELOAD_FOR_INPUT_ADDRESS and RELOAD_FOR_OUTPUT_ADDRESS */
        volatile char* byte_ptr = (volatile char*)p1 + offset + i;
        
        /* Mixed-type access within structure */
        volatile int* int_in_struct = &((volatile MixedType*)byte_ptr)->a;
        volatile double* dbl_in_struct = &((volatile MixedType*)byte_ptr)->b;
        
        /* Inline assembly that uses both addresses */
        asm volatile (
            "movl %1, %%ebx\n\t"
            "cvtsi2sd %%ebx, %%xmm1\n\t"
            "movsd %2, %%xmm2\n\t"
            "addsd %%xmm1, %%xmm2\n\t"
            "movsd %%xmm2, %0\n\t"
            : "=m" (*dbl_in_struct)
            : "m" (*int_in_struct), "m" (*dbl_in_struct)
            : "ebx", "xmm1", "xmm2", "r12", "r13", "r14", "r15", "memory"
        );
        
        /* Address computation for operand address */
        int*** pptr = (int***)&int_in_struct;
        modify_pptr(pptr);
    }
}

/* Another function focusing on output address reloads */
void stress_output_address(int seed) {
    register volatile long long* out_ptr asm ("r12");
    register int index asm ("r13");
    
    out_ptr = (volatile long long*)&global_data[seed % 5].extra[0];
    index = seed * 13;
    
    /* Complex output address computation */
    volatile long long* output_addr = out_ptr + (index % 25);
    
    /* Force RELOAD_FOR_OUTPUT_ADDRESS */
    asm volatile (
        "movq %1, %%rax\n\t"
        "imulq $37, %%rax, %%rax\n\t"
        "movq %%rax, %0\n\t"
        : "=m" (*output_addr)
        : "m" (*output_addr)
        : "rax", "rbx", "r12", "r13", "memory"
    );
    
    /* Chain of address computations for outaddr address */
    volatile long long** addr_of_addr = &output_addr;
    compute_address((void**)addr_of_addr, 8);
    
    /* Jump to force register reallocation */
    if (seed & 2) goto skip_part;
    
    /* More output addressing with different base */
    volatile int* out_int = &global_ints[index % 100];
    asm volatile (
        "movl %1, %%ecx\n\t"
        "roll $3, %%ecx\n\t"
        "movl %%ecx, %0\n\t"
        : "=m" (*out_int)
        : "m" (*out_int)
        : "ecx", "r12", "r13", "memory"
    );
    
skip_part:
    /* Use the same registers for different addressing mode */
    out_ptr = (volatile long long*)&global_doubles[0];
    index = seed * 17;
    
    /* This should trigger more reloads */
    volatile double* dbl_out = (volatile double*)out_ptr + (index % 100);
    asm volatile (
        "movsd %1, %%xmm3\n\t"
        "sqrtsd %%xmm3, %%xmm3\n\t"
        "movsd %%xmm3, %0\n\t"
        : "=m" (*dbl_out)
        : "m" (*dbl_out)
        : "xmm3", "r12", "r13", "memory"
    );
}

/* Function for operand address reloads */
void stress_operand_address(void) {
    register volatile char* base asm ("r12");
    register int offset1 asm ("r13");
    register int offset2 asm ("r14");
    
    base = (volatile char*)global_data;
    offset1 = 1234;
    offset2 = 5678;
    
    /* Complex operand address computation */
    volatile int* op_addr = (volatile int*)(base + offset1 * 3 + offset2);
    
    /* Force RELOAD_FOR_OPERAND_ADDRESS */
    asm volatile (
        "leal (%1, %2, 4), %%eax\n\t"
        "movl %%eax, %0\n\t"
        : "=m" (*op_addr)
        : "r" (offset1), "r" (offset2), "m" (*op_addr)
        : "eax", "r12", "r13", "r14", "memory"
    );
    
    /* Multiple memory operands with conflicting constraints */
    volatile int* addr2 = (volatile int*)(base + offset2 * 2);
    int temp;
    asm volatile (
        "movl %2, %%ebx\n\t"
        "addl %3, %%ebx\n\t"
        "movl %%ebx, %0\n\t"
        "movl %%ebx, %1\n\t"
        : "=m" (*op_addr), "=r" (temp)
        : "m" (*op_addr), "m" (*addr2)
        : "ebx", "r12", "r13", "r14", "memory"
    );
}

int main(void) {
    /* Initialize some data to avoid undefined behavior */
    for (int i = 0; i < 1000; i++) {
        global_ints[i] = i;
    }
    for (int i = 0; i < 500; i++) {
        global_doubles[i] = i * 0.5;
    }
    
    /* Call stress functions multiple times with different parameters */
    for (int i = 0; i < 10; i++) {
        stress_reloads(i);
        stress_output_address(i);
    }
    
    stress_operand_address();
    
    /* More complex pattern in main */
    {
        register volatile int* main_ptr asm ("r12");
        main_ptr = &global_ints[500];
        
        /* Mix of addressing modes in a loop */
        for (int j = 0; j < 5; j++) {
            volatile int* addr = main_ptr + j * 73;
            
            /* Inline asm that clobbers address registers */
            asm volatile (
                "movl %1, %%eax\n\t"
                "negl %%eax\n\t"
                "movl %%eax, %0\n\t"
                : "=m" (*addr)
                : "m" (*addr)
                : "eax", "r12", "memory"
            );
            
            /* Function call with computed address */
            int** pptr = (int**)&addr;
            modify_pptr(&pptr);
            
            /* goto to break linear flow */
            if (j & 1) goto loop_continue;
            
            /* Different addressing using same register */
            main_ptr = &global_ints[j * 100];
            
loop_continue:
            /* Empty by design - just for control flow */
            ;
        }
    }
    
    return 0;
}
