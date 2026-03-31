/* reload_coverage.c - Comprehensive test for GCC reload pass coverage */
#include <stdio.h>
#include <stdint.h>

/* Packed struct to force misaligned accesses */
struct __attribute__((packed)) misaligned_data {
    char c;
    int i;
    long l;
};

/* Volatile globals to prevent optimization */
volatile int global_counter = 0;
volatile long global_sum = 0;

int main(void) {
    /* Declare variables with different types and storage */
    int arr[256];
    long *ptr_arr[16];
    struct misaligned_data packed;
    volatile int vol_var = 0;
    register int reg_var asm("r12") = 42;  /* Try to bind to specific reg */
    
    /* Initialize arrays */
    for (int i = 0; i < 256; i++) {
        arr[i] = i * 3;
    }
    for (int i = 0; i < 16; i++) {
        ptr_arr[i] = (long*)&arr[i * 16];
    }
    
    packed.c = 'A';
    packed.i = 0xDEADBEEF;
    packed.l = 0xCAFEBABECAFEBABEULL;
    
    /* Loop with varying constraints to trigger different reload types */
    for (int iteration = 0; iteration < 10; iteration++) {
        int idx = iteration * 7;
        long temp1, temp2, temp3;
        void *complex_addr;
        
        /* RELOAD_FOR_INPUT and RELOAD_FOR_OUTPUT with register pressure */
        asm volatile (
            /* Multiple outputs with early-clobber to force RELOAD_FOR_OUTPUT */
            "mov %[out1], %[in1]\n\t"
            "add %[out1], %[in2]\n\t"
            "mov %[out2], %[in3]\n\t"
            "imul %[out2], %[in4]\n\t"
            : [out1] "=&r" (temp1), [out2] "=&r" (temp2)
            : [in1] "r" (arr[idx]), 
              [in2] "r" (iteration),
              [in3] "r" (arr[idx + 1]),
              [in4] "r" (arr[idx + 2])
            : "cc"
        );
        
        /* Complex addressing mode - RELOAD_FOR_INPUT_ADDRESS */
        asm volatile (
            "mov %0, [%1 + %2*8 + %3]\n\t"
            : "=r" (temp3)
            : "r" (arr), 
              "r" (idx), 
              "i" (16),  /* Displacement */
              "m" (*(struct misaligned_data*)&packed)  /* Memory operand */
            : "memory"
        );
        
        /* Nested address computation - RELOAD_FOR_OPERAND_ADDRESS */
        complex_addr = &&arr[idx] + idx * sizeof(int);
        asm volatile (
            "lea %0, [%1 + %2*4]\n\t"
            "mov %3, [%0]\n\t"
            : "=&r" (complex_addr), "+m" (arr[idx])
            : "r" (idx), "r" (temp1)
            : "cc"
        );
        
        /* RELOAD_FOR_INPADDR_ADDRESS and RELOAD_FOR_OUTADDR_ADDRESS */
        long *addr_ptr = &arr[idx];
        long **addr_of_addr = &addr_ptr;
        
        asm volatile (
            /* Input address reload */
            "mov %1, [%2]\n\t"
            /* Output address reload */
            "mov [%0], %1\n\t"
            : "+m" (*addr_of_addr)
            : "r" (temp2), "r" (addr_of_addr)
            : "memory"
        );
        
        /* RELOAD_FOR_OTHER_ADDRESS with memory clobber */
        asm volatile (
            "movq %0, %%mm0\n\t"
            "paddd %%mm0, %%mm0\n\t"
            "movq %%mm0, %1\n\t"
            : "=m" (packed.l), "=m" (vol_var)
            : 
            : "mm0", "memory"
        );
        
        /* Mixed data types with alignment issues */
        char *char_ptr = (char*)&packed + 1;  /* Misaligned access */
        int *int_ptr = (int*)char_ptr;
        
        asm volatile (
            "mov %0, %1\n\t"
            "add %0, %2\n\t"
            "mov %3, %0\n\t"
            : "+r" (reg_var), "+m" (*int_ptr)
            : "r" (packed.c), "m" (arr[idx + 3])
            : "cc"
        );
        
        /* Multiple memory operands with complex constraints */
        asm volatile (
            "mov %0, [%4 + %5*4]\n\t"
            "add %0, [%6 + %7]\n\t"
            "mov [%1 + %2*8], %0\n\t"
            "mov %3, [%1 + %2*8 + 4]\n\t"
            : "=&r" (temp1), "+m" (*(long(*)[16])ptr_arr)
            : "r" (iteration % 8),
              "r" (temp2),
              "r" (arr),
              "r" (idx),
              "r" (ptr_arr[iteration % 8]),
              "r" (iteration * sizeof(long))
            : "memory"
        );
        
        /* Update checksum */
        global_sum += temp1 + temp2 + temp3 + reg_var + vol_var;
        global_counter++;
    }
    
    /* Force all operations to have observable effects */
    printf("Checksum: %ld (iterations: %d)\n", 
           (long)global_sum, (int)global_counter);
    
    /* Additional test: RELOAD_OTHER with asm goto */
    int result = 0;
    asm goto (
        "cmp $0, %0\n\t"
        "je %l[label]\n\t"
        "add $1, %0\n\t"
        : "+r" (result)
        : 
        : "cc"
        : label
    );
    
    return result;
    
label:
    printf("Jump taken\n");
    return 0;
}
