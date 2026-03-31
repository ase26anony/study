/* reload_stress.c - Stress GCC's reload pass for coverage testing */
#include <stdint.h>

/* Volatile structures with mixed types to prevent optimization */
volatile struct MixedData {
    int counter;
    double values[3];
    char tags[8];
    int *ptr;
} data_array[100];

volatile struct Nested {
    struct MixedData *inner;
    long long big;
    short small[6];
} nested_array[50];

/* Helper functions that take pointer-to-pointer arguments */
static void modify_pointer(int ***ppp) {
    **ppp += 1;
}

static void complex_address_helper(void **addr1, void **addr2) {
    volatile int dummy = *(int*)*addr1 + *(int*)*addr2;
    (void)dummy;
}

/* Function with complex addressing patterns */
static void stress_reloads(int iter) {
    /* Bind specific variables to registers */
    register struct MixedData *p1 asm ("r12") = &data_array[0];
    register struct Nested *p2 asm ("r13") = &nested_array[0];
    register int *p3 asm ("r14");
    register volatile char *p4 asm ("r15");
    
    int local_var = iter * 7;
    int *local_ptr = &local_var;
    int **ptr_to_ptr = &local_ptr;
    
    /* Complex addressing computation */
    p3 = &p1[iter * 3 + 1].counter;
    p4 = &p1[iter * 2].tags[iter % 8];
    
    /* Jump label for control flow complexity */
    compute_address:
    
    /* Inline assembly with multiple memory operands and clobbers */
    asm volatile (
        "addl $1, %[mem1]\n\t"
        "subl $2, %[mem2]\n\t"
        : [mem1] "+m" (*p3), [mem2] "+m" (*(int*)p4)
        : 
        : "r12", "r13", "r14", "r15", "memory", "cc"
    );
    
    /* Another addressing mode with different base */
    p3 = &p2[iter / 2].small[iter % 6];
    
    /* Function call with address-taken arguments */
    modify_pointer(&ptr_to_ptr);
    
    /* More inline assembly with conflicting constraints */
    void *addr1 = &p1[local_var % 10];
    void *addr2 = &p2[local_var % 5];
    
    asm volatile (
        "movq %[addr1], %%r12\n\t"
        "movq %[addr2], %%r13\n\t"
        "movl (%%r12), %%eax\n\t"
        "addl (%%r13), %%eax\n\t"
        : 
        : [addr1] "r" (addr1), [addr2] "r" (addr2)
        : "r12", "r13", "rax", "memory", "cc"
    );
    
    /* Complex control flow with goto */
    if (iter % 3 == 0) {
        goto skip_block;
    }
    
    /* Different addressing pattern */
    p4 = (volatile char*)&p2[iter].big + sizeof(long long) - 4;
    
    skip_block:
    
    /* Nested addressing computation */
    int offset = (iter * 13) % 100;
    p3 = &data_array[offset].counter;
    
    /* Inline assembly with explicit memory constraint */
    asm volatile (
        "lock xaddl %%eax, %[counter]\n\t"
        : [counter] "+m" (*p3)
        : "a" (1)
        : "memory", "cc"
    );
    
    /* More pointer-to-pointer usage */
    complex_address_helper(&addr1, &addr2);
    
    /* Loop with register pressure */
    for (int i = 0; i < 3; i++) {
        register int *temp_ptr asm ("r12") = &data_array[i * 7].counter;
        
        asm volatile (
            "incl %[val]\n\t"
            : [val] "+m" (*temp_ptr)
            : 
            : "r12", "memory", "cc"
        );
        
        /* Force address reload by using same register differently */
        if (i == 1) {
            goto compute_address;
        }
    }
}

/* Another stress function focusing on output addresses */
static void stress_output_addresses(void) {
    volatile int outputs[20];
    register int *out1 asm ("r12") = &outputs[0];
    register int *out2 asm ("r13") = &outputs[10];
    
    /* Complex output addressing */
    for (int i = 0; i < 5; i++) {
        int idx = (i * 7) % 10;
        
        asm volatile (
            "movl %[in], (%[out1], %[idx], 4)\n\t"
            "movl %[in2], (%[out2], %[idx], 2)\n\t"
            : 
            : [in] "r" (i), [in2] "r" (i * 2),
              [out1] "r" (out1), [out2] "r" (out2),
              [idx] "r" ((long)idx)
            : "memory", "cc"
        );
    }
    
    /* Address computation that may need reload */
    int **addr_of_out1 = &out1;
    
    asm volatile (
        "movq %[ptr], %%r12\n\t"
        "movq (%%r12), %%r13\n\t"
        "addl $1, (%%r13)\n\t"
        : 
        : [ptr] "r" (addr_of_out1)
        : "r12", "r13", "memory", "cc"
    );
}

/* Main function that orchestrates the stress test */
int main(void) {
    /* Initialize some data */
    for (int i = 0; i < 100; i++) {
        data_array[i].counter = i;
        data_array[i].values[0] = i * 1.5;
        data_array[i].tags[0] = 'A' + (i % 26);
    }
    
    for (int i = 0; i < 50; i++) {
        nested_array[i].inner = (struct MixedData*)&data_array[i * 2];
        nested_array[i].big = i * 1000LL;
    }
    
    /* Call stress functions multiple times with different parameters */
    for (int iter = 0; iter < 10; iter++) {
        stress_reloads(iter);
        
        if (iter % 3 == 0) {
            stress_output_addresses();
        }
        
        /* Additional inline complexity in main */
        register volatile int *main_ptr asm ("r12") = &data_array[iter].counter;
        int offset = (iter * 11) % 20;
        
        asm volatile (
            "movl (%[base], %[off], 4), %%eax\n\t"
            "addl $1, %%eax\n\t"
            "movl %%eax, (%[base], %[off], 4)\n\t"
            : 
            : [base] "r" (main_ptr), [off] "r" ((long)offset)
            : "rax", "memory", "cc"
        );
        
        /* Pointer chain */
        int value = iter;
        int *ptr1 = &value;
        int **ptr2 = &ptr1;
        int ***ptr3 = &ptr2;
        
        modify_pointer(ptr3);
    }
    
    return 0;
}
