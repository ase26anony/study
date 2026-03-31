/* reload_stress.c - Stress GCC's reload pass for coverage testing */
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

/* Global volatile arrays for complex addressing */
volatile BigStruct global_data[10];
volatile int global_ints[1000];
volatile double global_doubles[500];

/* Helper functions that take pointer-to-pointer arguments */
static void modify_pptr(volatile int*** ppp) {
    volatile int dummy = ***ppp;
    (void)dummy;
}

static void compute_address(volatile void** addr_ptr, int offset) {
    *addr_ptr = (volatile void*)((char*)*addr_ptr + offset);
}

/* Main stress function with complex control flow */
static void stress_reloads(int iterations) {
    /* Bind specific pointers to explicit registers */
    register volatile MixedType* p1 asm ("r12");
    register volatile int* p2 asm ("r13");
    register volatile double* p3 asm ("r14");
    register volatile char* p4 asm ("r15");
    
    volatile int local_array[100];
    volatile double local_doubles[50];
    int i, j;
    
    /* Initialize register-bound pointers */
    p1 = &global_data[0].arr[0];
    p2 = &global_ints[0];
    p3 = &global_doubles[0];
    p4 = (volatile char*)&local_array[0];
    
    /* Label for goto jumps */
    compute_addr_again:
    
    for (i = 0; i < iterations; i++) {
        /* Complex address computation with multiple constraints */
        volatile MixedType* temp1;
        volatile int* temp2;
        volatile double* temp3;
        
        /* Force RELOAD_FOR_INPUT_ADDRESS through complex indexing */
        temp1 = p1 + (i * 3) % 97;
        temp2 = p2 + ((i * 7) & 0xFF) * sizeof(int);
        temp3 = p3 + ((i * 11) % 200) * sizeof(double);
        
        /* Inline assembly with multiple memory operands and clobbers */
        asm volatile (
            "movl %[val1], %%eax\n\t"
            "addl %%eax, %[val2]\n\t"
            : [val2] "+m" (temp1->a)  /* Memory constraint */
            : [val1] "r" (temp2[0])   /* Register constraint - creates conflict */
            : "eax", "r12", "r13", "memory"  /* Clobber address registers */
        );
        
        /* Jump to create complex control flow */
        if (i % 3 == 0) {
            goto use_different_addressing;
        }
        
        /* More complex addressing with pointer arithmetic */
        volatile int** pptr = (volatile int**)&temp1->d;
        *pptr = temp2 + (i * 13) % 50;
        
        /* Force RELOAD_FOR_INPADDR_ADDRESS */
        modify_pptr((volatile int***)&pptr);
        
        /* Inline assembly with explicit address register usage */
        register volatile void* addr_reg asm ("r12");
        addr_reg = (volatile void*)temp3;
        
        asm volatile (
            "movsd (%[addr]), %%xmm0\n\t"
            "addsd %%xmm0, %%xmm0\n\t"
            "movsd %%xmm0, (%[addr])\n\t"
            : 
            : [addr] "r" (addr_reg)
            : "xmm0", "r12", "memory"
        );
        
        continue;
        
        use_different_addressing:
        /* Different addressing mode using the same registers */
        p4 = (volatile char*)temp1 + sizeof(int) + ((i * 17) % 64);
        
        /* Force RELOAD_FOR_OUTPUT_ADDRESS */
        asm volatile (
            "movb $0x42, (%[ptr])\n\t"
            : 
            : [ptr] "r" (p4)
            : "r15", "memory"
        );
        
        /* Nested address computation */
        volatile void* complex_addr = (volatile void*)(
            (uintptr_t)p1 + 
            (uintptr_t)p2 * (i % 5) + 
            (uintptr_t)p3 * ((i + 1) % 3)
        );
        
        compute_address(&complex_addr, i * 16);
        
        /* Force RELOAD_FOR_OPERAND_ADDRESS */
        asm volatile (
            "lock addl $1, (%[addr])\n\t"
            : 
            : [addr] "m" (*(volatile int*)complex_addr)
            : "memory"
        );
    }
    
    /* Jump back to create loop with register pressure */
    if (iterations > 1) {
        iterations--;
        goto compute_addr_again;
    }
    
    /* Final complex addressing chain */
    volatile int* chain[5];
    chain[0] = (volatile int*)p1;
    chain[1] = p2 + 16;
    chain[2] = (volatile int*)p3;
    chain[3] = &local_array[32];
    chain[4] = (volatile int*)&local_doubles[0];
    
    for (j = 0; j < 5; j++) {
        /* Force RELOAD_FOR_OUTADDR_ADDRESS */
        volatile int** chain_ptr = &chain[j];
        
        asm volatile (
            "movl (%[ptr]), %%ebx\n\t"
            "incl %%ebx\n\t"
            "movl %%ebx, (%[ptr])\n\t"
            : 
            : [ptr] "r" (chain_ptr)
            : "ebx", "memory"
        );
    }
}

/* Secondary stress function with different patterns */
static void stress_more_reloads(void) {
    register volatile long long* rbx_ptr asm ("rbx");
    register volatile int* rsi_ptr asm ("rsi");
    register volatile char* rdi_ptr asm ("rdi");
    
    volatile int data[256];
    volatile long long big_data[128];
    
    /* Initialize with non-sequential pattern */
    for (int i = 0; i < 256; i++) {
        data[i] = i * 3 + (i % 7);
    }
    
    rbx_ptr = (volatile long long*)&data[0];
    rsi_ptr = &data[64];
    rdi_ptr = (volatile char*)&big_data[0];
    
    /* Complex addressing with mixed types */
    for (int i = 0; i < 32; i++) {
        /* Force RELOAD_FOR_OPADDR_ADDR */
        volatile long long** indirect = (volatile long long**)&rdi_ptr;
        
        asm volatile (
            "movq (%[base], %[index], 8), %%rax\n\t"
            "addq $1, %%rax\n\t"
            "movq %%rax, (%[base], %[index], 8)\n\t"
            : 
            : [base] "r" (rbx_ptr),
              [index] "r" ((long)i)
            : "rax", "rbx", "rdi", "memory"
        );
        
        /* Switch addressing mode */
        if (i % 4 == 0) {
            volatile int* alt_ptr = rsi_ptr + (i * 9) % 128;
            
            asm volatile (
                "imull $37, (%[ptr]), %%ecx\n\t"
                "movl %%ecx, (%[ptr])\n\t"
                : 
                : [ptr] "m" (*alt_ptr)  /* Memory constraint */
                : "ecx", "rsi", "memory"
            );
        }
        
        /* Force RELOAD_FOR_OTHER_ADDRESS */
        volatile void* other_addr = (volatile void*)(
            (uintptr_t)rbx_ptr + 
            (uintptr_t)rsi_ptr * (i % 3) +
            i * sizeof(long long)
        );
        
        asm volatile (
            "lock xaddl %%eax, (%[addr])\n\t"
            : 
            : [addr] "r" (other_addr), "a" (1)
            : "memory"
        );
    }
}

int main(void) {
    /* Initialize global data */
    for (int i = 0; i < 10; i++) {
        for (int j = 0; j < 100; j++) {
            global_data[i].arr[j].a = i + j;
            global_data[i].arr[j].b = (double)(i * j) / 3.0;
            global_data[i].arr[j].d = &global_ints[(i * j) % 1000];
        }
    }
    
    for (int i = 0; i < 1000; i++) {
        global_ints[i] = i * 2;
    }
    
    for (int i = 0; i < 500; i++) {
        global_doubles[i] = (double)i / 2.0;
    }
    
    /* Call stress functions multiple times with different parameters */
    stress_reloads(5);
    stress_more_reloads();
    stress_reloads(3);
    
    /* Additional complex addressing in main */
    {
        register volatile int* r10_ptr asm ("r10");
        register volatile double* r11_ptr asm ("r11");
        
        r10_ptr = &global_ints[500];
        r11_ptr = &global_doubles[250];
        
        /* Mixed addressing in a loop */
        for (int k = 0; k < 10; k++) {
            volatile int* addr1 = r10_ptr + (k * 23) % 100;
            volatile double* addr2 = r11_ptr + (k * 19) % 100;
            
            /* Force multiple reload types in one block */
            asm volatile (
                "cvtsi2sdl (%[int_ptr]), %%xmm1\n\t"
                "addsd (%[dbl_ptr]), %%xmm1\n\t"
                "movsd %%xmm1, (%[dbl_ptr])\n\t"
                : 
                : [int_ptr] "r" (addr1),
                  [dbl_ptr] "r" (addr2)
                : "xmm1", "r10", "r11", "memory"
            );
            
            if (k % 2) {
                goto skip_point;
            }
            
            /* Address computation that requires reload */
            volatile int** double_indirect = (volatile int**)&addr1;
            modify_pptr((volatile int***)&double_indirect);
            
            skip_point:
            /* Use computed address */
            asm volatile ("nop" : : "r" (addr1), "r" (addr2));
        }
    }
    
    return 0;
}
