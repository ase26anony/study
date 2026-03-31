/* reload_stress.c - Designed to stress GCC's reload pass */
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
static volatile int* volatile ptr_array[100];

/* Helper functions that take pointer-to-pointer arguments */
static void use_double_ptr(int*** ppp) {
    volatile int dummy = ***ppp;
    (void)dummy;
}

static void modify_through_ptr(volatile int** pp) {
    **pp = (**pp) + 1;
}

/* Function with complex addressing patterns */
static void stress_address_calculations(void) {
    /* Bind specific variables to registers */
    register volatile MixedType* p1 asm ("r12");
    register volatile int* p2 asm ("r13");
    register volatile char* p3 asm ("r14");
    register int index asm ("r15");
    
    /* Initialize pointers */
    p1 = &global_data[0].arr[0];
    p2 = &global_data[1].arr[10].a;
    p3 = &global_data[2].arr[20].c[0];
    index = 5;
    
    /* Complex addressing with multiple computations */
    volatile int temp;
    
    /* Block 1: Multiple address computations */
    {
        volatile int* addr1 = &p1[index * 3 + 1].a;
        volatile double* addr2 = &p1[index * 2 - 3].b;
        
        /* Inline assembly with memory operands and clobbers */
        asm volatile (
            "movl %1, %%eax\n\t"
            "addl %%eax, %0\n\t"
            : "+m" (*addr1)
            : "m" (*addr2)
            : "eax", "r12", "r13", "r14", "memory"
        );
        
        /* Jump to create control flow complexity */
        goto label1;
        
        /* Unreachable code that still affects analysis */
        {
            volatile int unused = *addr1 + *addr2;
            (void)unused;
        }
    }
    
label1:
    /* Recompute addresses using same registers for different purposes */
    {
        /* Different addressing mode */
        volatile char* new_addr = p3 + index * sizeof(MixedType) + offsetof(MixedType, c);
        
        /* Another inline asm with conflicting constraints */
        register int offset asm ("ebx") = 7;
        asm volatile (
            "movb %%bl, (%0)\n\t"
            "addb $1, (%1)\n\t"
            : 
            : "r" (new_addr), "r" (new_addr + offset), "r" (offset)
            : "memory", "ebx"
        );
    }
    
    /* Nested function calls with address-taken arguments */
    {
        volatile int local_var = 42;
        volatile int* ptr_to_local = &local_var;
        volatile int** ptr_to_ptr = &ptr_to_local;
        
        /* Pass address of register variable */
        use_double_ptr((int***)&ptr_to_ptr);
        
        /* More complex chain */
        volatile int*** ppp = (volatile int***)&ptr_array[10];
        *ppp = &p2;
        
        /* Inline asm that uses the computed address */
        asm volatile (
            "movl (%1), %%eax\n\t"
            "movl %%eax, (%0)\n\t"
            : "=m" (**ppp)
            : "r" (p2)
            : "eax", "r13", "memory"
        );
    }
    
    /* Loop with scattered accesses */
    for (int i = 0; i < 3; i++) {
        /* Different addressing in each iteration */
        volatile MixedType* elem = &global_data[i].arr[(i * 17 + index) % 20];
        
        /* Complex offset calculation */
        int offset = (i * 13 + 7) % sizeof(MixedType);
        volatile char* byte_ptr = (volatile char*)elem + offset;
        
        /* Inline asm with multiple memory operands */
        asm volatile (
            "movb (%1), %%al\n\t"
            "xorb %%al, (%2)\n\t"
            "addl $1, (%3)\n\t"
            : 
            : "r" (byte_ptr), "r" (byte_ptr + 1), 
              "r" (&elem->a), "m" (elem->a)
            : "eax", "memory", "r12", "r13"
        );
        
        /* Jump to create more control flow */
        if (i == 1) {
            goto middle_loop;
        }
        
        continue;
        
    middle_loop:
        /* Different addressing mode in middle of loop */
        volatile long long* big_ptr = &global_data[i].big[index * 2];
        
        /* Assembly with output address reload */
        asm volatile (
            "movq (%1), %%rax\n\t"
            "incq %%rax\n\t"
            "movq %%rax, %0\n\t"
            : "=m" (*big_ptr)
            : "r" (big_ptr)
            : "rax", "memory"
        );
    }
    
    /* Final complex block with multiple address types */
    {
        /* Output address computation */
        volatile int output_var;
        volatile int* output_addr = &output_var;
        
        /* Input address with different base */
        volatile int input_val = global_data[3].arr[7].a;
        
        /* Inline asm requiring both input and output address reloads */
        asm volatile (
            "movl %1, %%eax\n\t"
            "leal (%%eax, %%eax, 2), %%ebx\n\t"
            "movl %%ebx, %0\n\t"
            : "=m" (*output_addr)
            : "m" (input_val)
            : "eax", "ebx", "memory"
        );
        
        /* Chain of address computations */
        volatile int** addr_of_addr = &output_addr;
        modify_through_ptr((volatile int**)addr_of_addr);
        
        /* One more asm with operand address reload */
        asm volatile (
            "movl (%0), %%ecx\n\t"
            "addl $100, %%ecx\n\t"
            : 
            : "r" (addr_of_addr)
            : "ecx", "memory"
        );
    }
}

/* Additional stress function with different patterns */
static void more_stress(void) {
    register volatile int* r1 asm ("r10");
    register volatile double* r2 asm ("r11");
    
    r1 = &global_data[4].arr[30].a;
    r2 = &global_data[4].arr[30].b;
    
    /* Simultaneous input/output address requirements */
    volatile int temp_array[10];
    
    for (int i = 0; i < 5; i++) {
        /* Complex array indexing with multiple dimensions */
        volatile int* elem_ptr = &temp_array[(i * 7 + 3) % 10];
        
        /* Inline asm with 'm' and 'r' constraints on same value */
        int dummy;
        asm volatile (
            "movl %2, %%edx\n\t"
            "imull %%edx, %1\n\t"
            "movl %1, %0\n\t"
            : "=r" (dummy), "+m" (*elem_ptr)
            : "m" (*r1)
            : "edx", "r10", "memory"
        );
        
        /* Switch between different addressing modes */
        if (i & 1) {
            volatile double* dbl_ptr = r2 + i;
            asm volatile (
                "movsd %1, %%xmm0\n\t"
                "addsd %%xmm0, %%xmm0\n\t"
                "movsd %%xmm0, %0\n\t"
                : "=m" (*dbl_ptr)
                : "m" (*dbl_ptr)
                : "xmm0", "memory"
            );
        }
    }
    
    /* Address computation for function argument */
    volatile int*** triple_ptr = (volatile int***)&ptr_array[50];
    *triple_ptr = &r1;
    
    /* This should trigger operand address reloads */
    use_double_ptr((int***)triple_ptr);
}

int main(void) {
    /* Initialize some data */
    for (int i = 0; i < 100; i++) {
        ptr_array[i] = (volatile int*)&global_data[0].arr[0].a;
    }
    
    /* Call stress functions multiple times */
    stress_address_calculations();
    more_stress();
    
    /* One more round with different parameters */
    {
        register int counter asm ("r8") = 0;
        while (counter < 2) {
            stress_address_calculations();
            counter++;
            
            /* Inline asm that clobbers address registers */
            asm volatile (
                "nop\n\t"
                "nop\n\t"
                : 
                : 
                : "r12", "r13", "r14", "r15", "memory"
            );
        }
    }
    
    return 0;
}
