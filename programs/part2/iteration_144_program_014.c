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
static volatile int* volatile ptr_array[100];

/* Helper functions that take complex pointers */
static void use_pointer_to_pointer(volatile int*** ppp) {
    volatile int** pp = *ppp;
    if (pp) {
        volatile int* p = *pp;
        if (p) {
            *p += 1;
        }
    }
}

static void compute_address(volatile void** addr, int offset) {
    /* Complex address computation */
    *addr = (volatile void*)((uintptr_t)*addr + offset * sizeof(MixedType));
}

/* Main stress function */
static void stress_reload(void) {
    /* Bind specific pointers to registers */
    register volatile MixedType* p1 asm ("r12") = &global_data[0].arr[0];
    register volatile MixedType* p2 asm ("r13") = &global_data[1].arr[0];
    register volatile int* p3 asm ("r14") = (volatile int*)&global_data[0];
    register volatile char* p4 asm ("r15") = (volatile char*)&global_data[0];
    
    volatile int local_var = 42;
    volatile int* local_ptr = &local_var;
    volatile int** local_pptr = &local_ptr;
    
    /* Complex addressing with register variables */
    int offset1 = 0, offset2 = 0;
    
    /* Loop with complex indexing */
    for (int i = 0; i < 3; i++) {
        offset1 = (i * 7 + 3) % 20;
        offset2 = (i * 13 + 5) % 15;
        
        /* Block 1: Complex address computation */
        volatile MixedType* addr1 = p1 + offset1;
        volatile MixedType* addr2 = p2 + offset2;
        
        /* Inline asm with memory operands and clobbers */
        asm volatile (
            "movl %[val1], (%[mem1])\n\t"
            "movl %[val2], (%[mem2])\n\t"
            : 
            : [mem1] "r" (&addr1->a), [val1] "r" (i),
              [mem2] "r" (&addr2->a), [val2] "r" (i + 1)
            : "memory", "r12", "r13"
        );
        
        /* Jump to different block */
        if (i & 1) goto compute_block;
        
        /* Block 2: More complex addressing */
        volatile double* dbl_ptr = &addr1->b;
        volatile char* char_ptr = &addr1->c[offset2 % 7];
        
        /* Another asm with conflicting constraints */
        int temp;
        asm volatile (
            "movq (%[src]), %%rax\n\t"
            "movq %%rax, %[dst]\n\t"
            : [dst] "=m" (*dbl_ptr)
            : [src] "r" (char_ptr)
            : "rax", "memory", "r12", "r13", "r14"
        );
        
        continue;
        
    compute_block:
        /* Different address computation using same registers */
        volatile int* int_ptr = (volatile int*)p3;
        int_ptr += offset1 * 3;
        
        /* Nested function call with address-taken argument */
        volatile int*** ppp = (volatile int***)&local_pptr;
        use_pointer_to_pointer(ppp);
        
        /* Complex pointer arithmetic */
        volatile char* new_ptr = p4 + (offset1 * sizeof(MixedType) + offset2 * 4);
        
        /* Asm with multiple memory operands */
        asm volatile (
            "movb $0xAA, (%[ptr])\n\t"
            "addl $1, %[counter]\n\t"
            : [counter] "+m" (local_var)
            : [ptr] "r" (new_ptr)
            : "memory", "r15"
        );
    }
    
    /* More complex control flow with goto */
    int mode = 0;
    
address_chain:
    {
        volatile void* chain_ptr = &global_data[2];
        
        /* Chain of address computations */
        for (int j = 0; j < 2; j++) {
            compute_address(&chain_ptr, j * 8 + 3);
            
            /* Inline asm that clobbers address registers */
            asm volatile (
                "test %[val], %[val]\n\t"
                : 
                : [val] "r" ((uintptr_t)chain_ptr)
                : "cc", "r12", "r13", "r14", "r15"
            );
        }
    }
    
    if (mode == 0) {
        mode = 1;
        goto output_address_block;
    }
    
    return;
    
output_address_block:
    {
        /* Output address computations */
        register volatile long long* out_ptr asm ("r12") = &global_data[3].extra[0];
        
        /* Complex offset computation */
        int complex_offset = (local_var * 17) % 40;
        
        /* Multiple asm statements with output constraints */
        volatile long long result;
        asm volatile (
            "movq (%[base], %[offset], 8), %[res]\n\t"
            : [res] "=r" (result)
            : [base] "r" (out_ptr), [offset] "r" (complex_offset)
            : "memory"
        );
        
        /* Another asm that uses the result */
        asm volatile (
            "addq %[val], %%r12\n\t"
            : 
            : [val] "r" (result)
            : "r12"
        );
        
        /* Store to computed address */
        volatile long long* store_addr = out_ptr + complex_offset;
        *store_addr = result + 1;
    }
    
    goto address_chain;
}

/* Additional stress patterns */
static void stress_operand_address(void) {
    volatile int data[100];
    volatile int* ptrs[10];
    
    /* Initialize pointer array */
    for (int i = 0; i < 10; i++) {
        ptrs[i] = &data[i * 7];
    }
    
    /* Complex addressing chain */
    register volatile int** base_ptrs asm ("r12") = &ptrs[0];
    
    for (int i = 0; i < 5; i++) {
        volatile int** current = base_ptrs + i;
        volatile int* target = *current;
        
        /* Inline asm with operand address constraints */
        int index = i * 3;
        asm volatile (
            "movl %[idx], (%[target], %[idx], 4)\n\t"
            : 
            : [target] "r" (target), [idx] "r" (index)
            : "memory", "r12", "r13"
        );
        
        /* Jump to create control flow complexity */
        if (i & 1) {
            asm volatile ("nop" ::: "r12");
            goto skip_point;
        }
        
        /* More address computation */
        volatile int* next_target = target + index;
        *next_target = i;
        
    skip_point:
        /* Empty but creates label for goto */
        ;
    }
}

/* Main function */
int main(void) {
    /* Initialize some data */
    for (int i = 0; i < 10; i++) {
        global_data[i].arr[0].a = i;
        global_data[i].arr[0].b = i * 1.5;
    }
    
    /* Call stress functions multiple times */
    stress_reload();
    stress_operand_address();
    
    /* More complex patterns in main */
    {
        volatile MixedType* dynamic_ptr = &global_data[4].arr[10];
        volatile int** pptr = (volatile int**)&dynamic_ptr->d;
        
        /* Chain of address operations */
        for (int k = 0; k < 2; k++) {
            /* Compute address with multiple steps */
            uintptr_t raw_addr = (uintptr_t)dynamic_ptr;
            raw_addr += k * sizeof(MixedType) * 3;
            raw_addr += ((k * 17) & 0xFF);
            
            volatile MixedType* computed = (volatile MixedType*)raw_addr;
            
            /* Asm with complex addressing */
            asm volatile (
                "leaq (%[base], %[idx], 8), %%r13\n\t"
                "movq %%r13, %[out]\n\t"
                : [out] "=m" (computed->d)
                : [base] "r" (computed), [idx] "r" (k)
                : "r13", "memory"
            );
            
            /* Function call with complex argument */
            use_pointer_to_pointer((volatile int***)&pptr);
        }
    }
    
    return 0;
}
