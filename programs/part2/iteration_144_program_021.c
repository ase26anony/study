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
    volatile MixedType arr[3];
    volatile long long extra;
} Container;

/* Global volatile arrays */
volatile Container big_array[100];
volatile int global_buffer[256];

/* Helper functions that take pointer-to-pointer */
void modify_pptr(int*** ppp) {
    volatile static int dummy = 42;
    **ppp = dummy++;
}

void complex_address_helper(volatile int** pp, int offset) {
    *pp = (volatile int*)((char*)*pp + offset);
}

/* Function with complex addressing patterns */
__attribute__((noinline))
void stress_reload_patterns(int iter) {
    /* Bind specific variables to registers */
    register volatile Container* p1 asm ("r12") = &big_array[0];
    register volatile int* p2 asm ("r13") = &global_buffer[0];
    register int* p3 asm ("r14") = (int*)&big_array[0];
    
    volatile int local_var = iter;
    volatile int* local_ptr = &local_var;
    
    /* Complex control flow with goto */
    if (iter & 1) goto compute_addr1;
    else goto compute_addr2;
    
compute_addr1:
    {
        /* Complex address computation forcing RELOAD_FOR_INPUT_ADDRESS */
        volatile MixedType* addr1 = (volatile MixedType*)
            ((char*)p1 + iter * sizeof(Container) + 
             ((uintptr_t)p2 & 0xF) * 8 + 3);
        
        /* Inline asm with memory operand and clobber */
        asm volatile (
            "movl %1, %%eax\n\t"
            "addl %%eax, %0\n\t"
            : "+m" (addr1->arr[0].a)
            : "m" (local_var)
            : "eax", "r12", "r13", "memory"
        );
        
        /* Nested pointer usage */
        int** pptr = (int**)&addr1->arr[1].d;
        modify_pptr(&pptr);
        
        goto after_asm;
    }
    
compute_addr2:
    {
        /* Different complex addressing for RELOAD_FOR_OUTPUT_ADDRESS */
        register volatile int* p4 asm ("r15") = 
            (volatile int*)((char*)p2 + iter * 16 - 8);
        
        /* Multi-operand asm with conflicting constraints */
        int temp;
        asm volatile (
            "leal (%1, %2, 4), %%ecx\n\t"
            "movl (%%ecx), %%edx\n\t"
            "movl %%edx, %0\n\t"
            : "=r" (temp), "+m" (*p4)
            : "r" (iter), "m" (local_var)
            : "ecx", "edx", "r15", "memory"
        );
        
        /* Force address reload for operand */
        complex_address_helper((volatile int**)&p4, temp);
        
        /* Use computed address */
        *p4 = temp + iter;
    }
    
after_asm:
    /* More complex addressing with mixed types */
    volatile double* dbl_ptr = (volatile double*)
        ((char*)p1 + ((uintptr_t)p3 >> 2) * sizeof(MixedType));
    
    /* Inline asm that clobbers address registers */
    asm volatile (
        "movq %1, %%xmm0\n\t"
        "addsd %%xmm0, %%xmm0\n\t"
        "movq %%xmm0, %0\n\t"
        : "=m" (*dbl_ptr)
        : "m" (big_array[iter].arr[0].b)
        : "xmm0", "r12", "r13", "r14", "memory"
    );
    
    /* Chain of address computations */
    volatile char* char_ptr = (volatile char*)dbl_ptr + 5;
    volatile int* int_ptr = (volatile int*)(char_ptr + iter * 3 - 1);
    
    /* Another asm with operand address reloads */
    int result;
    asm volatile (
        "movl (%1), %%ebx\n\t"
        "imull %2, %%ebx\n\t"
        "movl %%ebx, %0\n\t"
        : "=r" (result)
        : "r" (int_ptr), "r" (iter)
        : "ebx", "memory"
    );
    
    /* Use result in another memory op */
    *int_ptr = result;
    
    /* Jump back with different register usage */
    if (iter > 0) {
        /* Force RELOAD_FOR_OPERAND_ADDRESS */
        volatile int** addr_of_ptr = &int_ptr;
        asm volatile ("" : "+r" (addr_of_ptr) : : "memory");
        
        /* Complex expression with address taken */
        int offset = (**addr_of_ptr) & 0xFF;
        volatile int* final_addr = (volatile int*)
            ((char*)p2 + offset * 4 + ((uintptr_t)p1 & 0xFFF));
        
        /* Final asm with multiple clobbers */
        asm volatile (
            "movl (%1), %%edi\n\t"
            "addl %%edi, (%2)\n\t"
            : 
            : "r" (addr_of_ptr), "r" (final_addr)
            : "edi", "r12", "r13", "r14", "memory"
        );
    }
}

/* Second stress function for different patterns */
__attribute__((noinline))
void more_reload_stress(void) {
    volatile static int counter = 0;
    
    /* Array of register-bound pointers */
    register volatile int* reg_ptrs[4] asm ("r12");
    reg_ptrs[0] = &global_buffer[0];
    reg_ptrs[1] = (volatile int*)&big_array[0];
    reg_ptrs[2] = (volatile int*)&big_array[50];
    reg_ptrs[3] = &counter;
    
    for (int i = 0; i < 4; i++) {
        /* Switch between different addressing modes */
        switch (i) {
            case 0: {
                /* RELOAD_FOR_INPADDR_ADDRESS pattern */
                volatile int** addr_of_regptr = (volatile int**)&reg_ptrs[i];
                int offset = (*addr_of_regptr)[i];
                
                /* Complex asm with memory indirect */
                asm volatile (
                    "movl (%[base], %[idx], 4), %%esi\n\t"
                    "movl %%esi, (%[dest])\n\t"
                    : 
                    : [base] "r" (*addr_of_regptr), 
                      [idx] "r" (i),
                      [dest] "r" (&global_buffer[i * 8])
                    : "esi", "memory"
                );
                break;
            }
            
            case 1: {
                /* RELOAD_FOR_OUTADDR_ADDRESS pattern */
                volatile long long* ll_ptr = 
                    (volatile long long*)((char*)reg_ptrs[i] + i * 16);
                
                /* Double word operation forcing reloads */
                asm volatile (
                    "movq %1, %%rax\n\t"
                    "bswapq %%rax\n\t"
                    "movq %%rax, %0\n\t"
                    : "=m" (*ll_ptr)
                    : "m" (big_array[i].extra)
                    : "rax", "r12", "memory"
                );
                break;
            }
            
            case 2: {
                /* RELOAD_FOR_OTHER_ADDRESS pattern */
                volatile MixedType* mt_ptr = 
                    (volatile MixedType*)(reg_ptrs[i] + i * 3);
                
                /* Multiple memory accesses in one asm */
                double dbl_temp;
                asm volatile (
                    "movsd %1, %%xmm1\n\t"
                    "mulsd %2, %%xmm1\n\t"
                    "movsd %%xmm1, %0\n\t"
                    : "=m" (mt_ptr->arr[0].b)
                    : "m" (mt_ptr->arr[1].b),
                      "m" (mt_ptr->arr[2].b)
                    : "xmm1", "memory"
                );
                
                /* Chain address computation */
                volatile char* char_base = (volatile char*)mt_ptr;
                for (int j = 0; j < 3; j++) {
                    volatile int* int_addr = 
                        (volatile int*)(char_base + j * 7 + 2);
                    *int_addr += j;
                }
                break;
            }
            
            case 3: {
                /* RELOAD_FOR_OPADDR_ADDR pattern */
                register int* bound_var asm ("r15") = (int*)&counter;
                
                /* Asm with explicit address register usage */
                asm volatile (
                    "movl (%[addr]), %%r8d\n\t"
                    "leal 1(%%r8d), %%r9d\n\t"
                    "movl %%r9d, (%[addr])\n\t"
                    : 
                    : [addr] "r" (bound_var)
                    : "r8", "r9", "r15", "memory"
                );
                
                /* Use in complex expression */
                int* another_addr = bound_var + *bound_var;
                asm volatile ("" : "+r" (another_addr) : : "memory");
                break;
            }
        }
    }
}

/* Main function creating compilation stress */
int main(void) {
    /* Initialize some data */
    for (int i = 0; i < 100; i++) {
        big_array[i].extra = i * 3LL;
        for (int j = 0; j < 3; j++) {
            big_array[i].arr[j].a = i + j;
            big_array[i].arr[j].b = (double)(i * j) / 3.0;
        }
    }
    
    /* Call stress functions multiple times with different args */
    for (int i = 0; i < 10; i++) {
        stress_reload_patterns(i);
        more_reload_stress();
        
        /* Additional inline complex addressing */
        volatile int* dynamic_ptr = 
            (volatile int*)((char*)&big_array[0] + i * sizeof(Container));
        
        /* Mixed asm and C address computation */
        asm volatile (
            "movl %1, %%r10d\n\t"
            "shll $2, %%r10d\n\t"
            "addl %%r10d, %0\n\t"
            : "+m" (*dynamic_ptr)
            : "r" (i)
            : "r10", "memory"
        );
        
        /* Pointer-to-pointer chain */
        volatile int** pp1 = &dynamic_ptr;
        volatile int*** pp2 = &pp1;
        ***pp2 += i;
    }
    
    return 0;
}
