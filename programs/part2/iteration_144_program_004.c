/* reload_stress.c - Stress GCC's reload pass for uncovered switch cases */
#include <stdint.h>

/* Volatile structures with mixed types to prevent optimization */
volatile struct MixedData {
    int counter;
    double values[3];
    char tags[8];
    void* pointers[2];
} data_array[100];

volatile struct NestedPtrs {
    int** pp;
    volatile int* volatile * volatile ppp;
} ptr_struct;

/* Helper functions that take complex pointer arguments */
static void use_pointer_to_pointer(int*** ppp) {
    volatile int dummy = ***ppp;
    (void)dummy;
}

static void modify_through_indirect(volatile int**** q) {
    ****q = 42;
}

/* Function with complex addressing patterns */
static void stress_address_calculations(void) {
    /* Bind specific registers for address calculations */
    register volatile struct MixedData* p1 asm("r12") = &data_array[0];
    register volatile struct MixedData* p2 asm("r13") = &data_array[50];
    register int* idx_ptr asm("r14");
    
    volatile int index = 10;
    idx_ptr = &index;
    
    /* Complex addressing mode 1: RELOAD_FOR_INPUT_ADDRESS likely */
    volatile double* addr1 = &p1[(*idx_ptr * 3) / 2].values[1];
    
    /* Inline assembly with memory operand and clobbered address register */
    asm volatile (
        "movq (%[mem]), %%rax\n\t"
        "addq $1, %%rax\n\t"
        "movq %%rax, (%[mem])"
        : 
        : [mem] "m" (*addr1)
        : "rax", "r12", "memory"
    );
    
    /* Jump to create control flow complexity */
    goto compute_other_address;
    
recompute_first:
    /* Different addressing using same register-bound variable */
    p1 = &data_array[25];
    volatile char* tag_addr = &p1[(*idx_ptr & 0x7)].tags[3];
    
    /* Another asm with conflicting constraints */
    register char* forced_reg asm("r12") = (char*)tag_addr;
    asm volatile (
        "movb $65, (%[ptr])"
        : 
        : [ptr] "r" (forced_reg)
        : "memory"
    );
    
    goto done;
    
compute_other_address:
    /* RELOAD_FOR_OUTPUT_ADDRESS scenario */
    volatile void** output_addr = &p2[index % 20].pointers[1];
    
    /* Assembly with output memory operand */
    uint64_t dummy_val = 0x12345678;
    asm volatile (
        "movq %[val], (%[out])"
        : 
        : [out] "r" (output_addr), [val] "r" (dummy_val)
        : "memory", "r13"
    );
    
    /* Nested address computation for RELOAD_FOR_INPADDR_ADDRESS */
    int*** triple_ptr = (int***)&ptr_struct.ppp;
    use_pointer_to_pointer(triple_ptr);
    
    goto recompute_first;
    
done:
    return;
}

/* Function with operand address reloads */
static void stress_operand_address(void) {
    register volatile int* base asm("r15") = (volatile int*)&data_array[0];
    
    /* Complex expression forcing address reload before use */
    volatile int* addr = &base[((uintptr_t)base >> 4) & 0xF];
    
    /* Inline asm using the computed address as both input and output */
    int temp;
    asm volatile (
        "movl (%[in]), %[tmp]\n\t"
        "addl $1, %[tmp]\n\t"
        "movl %[tmp], (%[in])"
        : [tmp] "=&r" (temp)
        : [in] "r" (addr)
        : "memory", "r15"
    );
    
    /* RELOAD_FOR_OPADDR_ADDR scenario */
    volatile int**** quad_ptr = (volatile int****)&ptr_struct.ppp;
    modify_through_indirect((volatile int****)quad_ptr);
}

/* Function with other address reload types */
static void stress_other_address_types(void) {
    /* Multiple register bindings to increase pressure */
    register double* dptr1 asm("r10");
    register double* dptr2 asm("r11");
    
    dptr1 = (double*)&data_array[10].values[0];
    dptr2 = (double*)&data_array[60].values[2];
    
    /* Complex addressing with multiple components */
    volatile double* complex_addr = &dptr1[(uintptr_t)dptr2 % 8];
    
    /* Assembly with multiple memory operands and clobbers */
    double result;
    asm volatile (
        "movsd (%[src]), %%xmm0\n\t"
        "addsd (%[src2]), %%xmm0\n\t"
        "movsd %%xmm0, (%[dst])"
        : 
        : [src] "r" (complex_addr), 
          [src2] "r" (&dptr2[(uintptr_t)dptr1 % 4]),
          [dst] "r" (&data_array[0].values[0])
        : "xmm0", "r10", "r11", "memory"
    );
    
    /* Create RELOAD_FOR_OTHER_ADDRESS scenario */
    {
        volatile int* volatile* addr_of_addr = (volatile int* volatile*)&ptr_struct.pp;
        volatile int* target = *addr_of_addr;
        
        /* Use in another computation */
        volatile int* new_addr = &target[((uintptr_t)target >> 3) & 0x7];
        
        asm volatile (
            "orl $1, (%[ptr])"
            :
            : [ptr] "r" (new_addr)
            : "memory"
        );
    }
}

/* Main function creating the stress pattern */
int main(void) {
    /* Initialize some data */
    for (int i = 0; i < 100; i++) {
        data_array[i].counter = i;
        data_array[i].values[0] = i * 1.5;
        data_array[i].values[1] = i * 2.5;
        data_array[i].values[2] = i * 3.5;
        data_array[i].tags[0] = 'A' + (i % 26);
        data_array[i].pointers[0] = (void*)&data_array[(i + 1) % 100];
        data_array[i].pointers[1] = (void*)&data_array[(i + 50) % 100];
    }
    
    int dummy = 42;
    ptr_struct.pp = &dummy;
    ptr_struct.ppp = (volatile int* volatile*)&ptr_struct.pp;
    
    /* Repeated calls with small variations to create multiple reload opportunities */
    for (int i = 0; i < 5; i++) {
        stress_address_calculations();
        stress_operand_address();
        stress_other_address_types();
        
        /* Modify index to change addressing patterns */
        data_array[i].counter = i * 2;
    }
    
    /* Final complex addressing chain */
    {
        register volatile char* cbase asm("rbx") = (volatile char*)&data_array[0];
        volatile char* chain_addr = &cbase[
            ((uintptr_t)cbase & 0xFF) + 
            (data_array[0].counter * sizeof(struct MixedData)) % 256
        ];
        
        asm volatile (
            "incb (%[addr])"
            :
            : [addr] "r" (chain_addr)
            : "memory", "rbx"
        );
    }
    
    return 0;
}
