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

/* Helper functions that take pointer-to-pointer arguments */
static void modify_pptr(volatile int*** ppp) {
    volatile int** temp = *ppp;
    if (temp) {
        **temp += 1;
    }
}

static void compute_address(volatile void** addr, int offset) {
    *addr = (volatile void*)((uintptr_t)*addr + offset);
}

/* Main stress function with complex addressing */
static void stress_reload(void) {
    /* Bind specific pointers to registers */
    register volatile MixedType* p1 asm ("r12") = &global_data[0].arr[0];
    register volatile int* p2 asm ("r13") = (volatile int*)&global_data[1];
    register volatile char* p3 asm ("r14") = (volatile char*)&global_data[2];
    
    volatile int local_var = 42;
    volatile int* local_ptr = &local_var;
    volatile int** local_pptr = &local_ptr;
    
    int i, j;
    
    /* Complex addressing mode 1: Array indexing with register variables */
    for (i = 0; i < 3; i++) {
        /* Force RELOAD_FOR_INPUT_ADDRESS */
        volatile MixedType* addr1 = p1 + i * 7 + (i & 1) * 3;
        
        /* Inline asm with memory operand and clobbered address register */
        asm volatile (
            "addl $1, %[mem]\n\t"
            : [mem] "+m" (addr1->a)
            : 
            : "r12", "r13", "r14", "memory"
        );
        
        /* Jump to create complex control flow */
        if (i == 1) {
            goto recompute_addr;
        }
        
        /* Use computed address in another operation */
        addr1->b = (double)addr1->a;
        
        continue;
        
recompute_addr:
        /* Same register used for different base - forces reloads */
        p1 = (volatile MixedType*)((uintptr_t)p2 + i * sizeof(int));
        
        /* Force RELOAD_FOR_INPADDR_ADDRESS */
        volatile int** temp_pptr = &local_ptr;
        modify_pptr((volatile int***)&temp_pptr);
    }
    
    /* Complex addressing mode 2: Pointer chains */
    for (j = 0; j < 2; j++) {
        /* Bind to another register */
        register volatile long long* p4 asm ("r15") = 
            (volatile long long*)&global_data[j].extra[0];
        
        /* Force RELOAD_FOR_OUTPUT_ADDRESS */
        volatile long long* out_addr = p4 + j * 11;
        
        /* Inline asm with multiple memory operands */
        asm volatile (
            "movq %[in], %%rax\n\t"
            "addq $8, %%rax\n\t"
            "movq %%rax, %[out]\n\t"
            : [out] "=m" (*out_addr)
            : [in] "m" (global_data[0].arr[0].a)
            : "rax", "r15", "memory"
        );
        
        /* Force RELOAD_FOR_OPERAND_ADDRESS */
        volatile void* complex_addr = (volatile void*)(
            (uintptr_t)p3 + 
            (uintptr_t)out_addr * 2 + 
            j * sizeof(MixedType)
        );
        
        /* Call function with address-taken argument */
        compute_address(&complex_addr, 16);
        
        /* Use goto to break linear flow */
        if (j == 0) {
            goto skip_block;
        }
        
        /* More complex addressing */
        global_ptrs[j] = (volatile int*)complex_addr;
        
skip_block:
        /* Different use of same register */
        p3 = (volatile char*)&global_data[3].arr[10].c[0];
    }
    
    /* Force RELOAD_FOR_OUTADDR_ADDRESS and RELOAD_FOR_OPADDR_ADDR */
    {
        register volatile int** p5 asm ("r12") = &global_ptrs[5];
        register volatile int* p6 asm ("r13") = *p5;
        
        /* Complex expression requiring multiple reloads */
        volatile int* final_addr = p6 + 
            (local_var & 0xF) * 3 + 
            ((uintptr_t)p5 >> 4) % 16;
        
        /* Inline asm with conflicting constraints */
        asm volatile (
            "movl %[val], (%[addr])\n\t"
            : 
            : [addr] "r" (final_addr), [val] "i" (99)
            : "memory", "r12", "r13"
        );
        
        /* Force RELOAD_FOR_OTHER_ADDRESS */
        volatile int*** super_ptr = &local_pptr;
        modify_pptr((volatile int***)super_ptr);
    }
    
    /* Mixed data type accesses with alignment challenges */
    {
        volatile char* byte_ptr = (volatile char*)&global_data[4];
        
        /* Non-contiguous, misaligned accesses */
        for (i = 0; i < 5; i++) {
            int offset = i * 7 + 3;  /* Purposefully odd offsets */
            
            /* Force various reload types through complex addressing */
            volatile int* int_ptr = (volatile int*)(byte_ptr + offset);
            
            /* Inline asm that clobbers address registers */
            asm volatile (
                "lock addl $1, %[target]\n\t"
                : [target] "+m" (*int_ptr)
                : 
                : "r12", "r13", "r14", "r15", "memory"
            );
            
            /* Pointer arithmetic with mixed types */
            byte_ptr = (volatile char*)((uintptr_t)byte_ptr + 
                       sizeof(double) - (offset % sizeof(double)));
        }
    }
}

/* Secondary stress function */
static void more_stress(void) {
    volatile int array[100];
    volatile double darray[50];
    
    /* Register binding with explicit registers */
    register volatile int* r1 asm ("r12") = &array[0];
    register volatile double* r2 asm ("r13") = &darray[0];
    
    /* Complex addressing chains */
    for (int i = 0; i < 10; i++) {
        /* Force RELOAD_FOR_INPUT_ADDRESS */
        volatile int* addr1 = r1 + i * (i + 1) * 2;
        
        /* Force RELOAD_FOR_OUTPUT_ADDRESS */
        volatile double* addr2 = r2 + i * 3;
        
        /* Inline asm using both addresses */
        asm volatile (
            "cvtsi2sd %[in], %%xmm0\n\t"
            "movsd %%xmm0, %[out]\n\t"
            : [out] "=m" (*addr2)
            : [in] "m" (*addr1)
            : "xmm0", "r12", "r13", "memory"
        );
        
        /* Jump to force register reloading */
        if (i & 1) {
            goto switch_regs;
        }
        
        continue;
        
switch_regs:
        /* Swap register usage */
        register volatile int* temp asm ("r12") = (volatile int*)addr2;
        r1 = temp;
        r2 = (volatile double*)addr1;
    }
}

int main(void) {
    /* Initialize some data */
    for (int i = 0; i < 10; i++) {
        global_data[i].arr[0].a = i * 100;
        global_data[i].arr[0].b = i * 100.0;
        global_ptrs[i] = (volatile int*)&global_data[i];
    }
    
    /* Call stress functions multiple times */
    stress_reload();
    more_stress();
    
    /* Additional stress in main */
    {
        register volatile BigStruct* bs asm ("r12") = &global_data[5];
        
        /* Complex nested addressing */
        volatile MixedType* mt = &bs->arr[bs->arr[0].a % 10];
        
        /* Force multiple reload types */
        asm volatile (
            "imull $37, %[val], %[val]\n\t"
            : [val] "+r" (mt->a)
            : 
            : "r12", "memory"
        );
        
        /* Address of address computation */
        volatile MixedType** mt_ptr = &mt;
        compute_address((volatile void**)mt_ptr, 8);
    }
    
    return 0;
}
