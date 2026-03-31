/* reload_coverage.c - Comprehensive test to trigger all reload types in GCC's reload pass */
#include <stdio.h>
#include <stdint.h>

/* Packed struct to force misaligned accesses */
struct __attribute__((packed)) misaligned_data {
    char c;
    int i;
    long l;
    char tail;
};

/* Volatile globals to prevent optimization */
volatile long global_base = 1000;
volatile int global_index = 2;
volatile long global_disp = 100;

int main() {
    /* Declare diverse variables with different storage characteristics */
    register int reg_var asm("r12") = 42;  /* Try to bind to specific reg */
    volatile int vol_var = 123;
    int auto_var = 456;
    long long_var = 789;
    char char_array[256];
    int int_array[128];
    long long_array[64];
    
    /* Packed/misaligned data */
    struct misaligned_data packed;
    packed.c = 'A';
    packed.i = 0xDEADBEEF;
    packed.l = 0xCAFEBABE;
    
    /* Pointers with different properties */
    int *restrict restr_ptr = int_array;
    volatile int *vol_ptr = &vol_var;
    char *char_ptr = char_array;
    
    /* Initialize arrays */
    for (int i = 0; i < 256; i++) char_array[i] = i;
    for (int i = 0; i < 128; i++) int_array[i] = i * 3;
    for (int i = 0; i < 64; i++) long_array[i] = i * 5;
    
    /* Result accumulator with side effects */
    unsigned long checksum = 0;
    
    /* Loop to vary constraints and trigger different reload types */
    for (int iteration = 0; iteration < 10; iteration++) {
        /* Vary base, index, and displacement each iteration */
        long base = global_base + iteration * 100;
        int index = global_index + iteration;
        long displacement = global_disp + iteration * 20;
        
        /* Force RELOAD_FOR_INPUT_ADDRESS and RELOAD_FOR_INPADDR_ADDRESS:
           Complex addressing mode that likely needs temporary register */
        asm volatile (
            "mov %[dst], %[src]\n\t"
            : [dst] "=m" (int_array[index * 2 + 1])
            : [src] "r" (int_array[base + index * 4 + displacement]),
              "m" (int_array[base + index * 4 + displacement])
            : "memory"
        );
        
        /* Force RELOAD_FOR_OUTPUT_ADDRESS and RELOAD_FOR_OUTADDR_ADDRESS:
           Output with complex addressing */
        asm volatile (
            "lea (%[base], %[index], 4), %[tmp]\n\t"
            "mov %[val], (%[tmp], %[disp], 1)\n\t"
            : [tmp] "=&r" (reg_var), 
              "=m" (*(int*)((char*)long_array + displacement))
            : [base] "r" (long_array),
              [index] "r" (index),
              [disp] "r" (displacement),
              [val] "r" (iteration)
            : "memory"
        );
        
        /* Force RELOAD_FOR_INPUT and RELOAD_FOR_OUTPUT:
           Many operands to cause register pressure */
        asm volatile (
            "add %[a], %[b]\n\t"
            "sub %[c], %[d]\n\t"
            "imul %[e], %[f]\n\t"
            : [a] "+&r" (auto_var),
              [c] "+&r" (long_var),
              [out1] "=m" (char_array[index]),
              [out2] "=m" (int_array[index])
            : [b] "r" (iteration),
              [d] "r" (base),
              [e] "m" (packed.i),
              [f] "r" (displacement),
              [m1] "m" (packed.l),
              [m2] "m" (global_base)
            : "cc", "memory"
        );
        
        /* Force RELOAD_FOR_OPERAND_ADDRESS and RELOAD_FOR_OPADDR_ADDR:
           Take address of already complex operand */
        int *addr1, *addr2;
        asm volatile (
            "mov %[ptr1], %[base]\n\t"
            "add %[ptr1], %[idx1]\n\t"
            "mov %[ptr2], %[ptr1]\n\t"
            "add %[ptr2], %[disp]\n\t"
            : [ptr1] "=&r" (addr1),
              [ptr2] "=&r" (addr2)
            : [base] "r" (int_array),
              [idx1] "r" (index * sizeof(int)),
              [disp] "r" (displacement)
            : "memory"
        );
        
        /* Use the computed addresses */
        asm volatile (
            "mov (%[addr]), %[tmp]\n\t"
            "add %[tmp], %[sum]\n\t"
            : [sum] "+r" (checksum),
              [tmp] "=&r" (reg_var)
            : [addr] "r" (addr1),
              "m" (*addr1)
            : "cc"
        );
        
        /* Force RELOAD_OTHER and RELOAD_FOR_OTHER_ADDRESS:
           Volatile asm with memory clobber and many constraints */
        asm volatile (
            "mov %[in1], %%rax\n\t"
            "mov %[in2], %%rbx\n\t"
            "add %%rbx, %%rax\n\t"
            "mov %%rax, %[out1]\n\t"
            "mov %[in3], %%rcx\n\t"
            "lea (%[in4], %[in5], 8), %%rdx\n\t"
            "mov %%rdx, %[out2]\n\t"
            : [out1] "=m" (packed.l),
              [out2] "=m" (*(volatile long*)vol_ptr)
            : [in1] "m" (global_base),
              [in2] "r" (displacement),
              [in3] "m" (packed.i),
              [in4] "r" (char_ptr),
              [in5] "r" (index),
              "m" (*(char*)(char_ptr + index))
            : "rax", "rbx", "rcx", "rdx", "memory"
        );
        
        /* Mix data types to stress reload further */
        if (iteration % 2 == 0) {
            /* Use char* with int* operations */
            asm volatile (
                "movsbl (%[charptr], %[idx], 1), %[tmp]\n\t"
                "add %[tmp], %[intval]\n\t"
                : [intval] "+r" (auto_var),
                  [tmp] "=&r" (reg_var)
                : [charptr] "r" (char_array),
                  [idx] "r" (index * 2),
                  "m" (char_array[index * 2])
                : "cc"
            );
        }
        
        /* Update checksum with all modified values */
        checksum += auto_var + long_var + reg_var + packed.l + *addr1;
    }
    
    /* Final computation to ensure all asm has side effects */
    checksum += vol_var + packed.i + global_base;
    
    /* Use the results */
    for (int i = 0; i < 128; i++) {
        checksum += int_array[i];
    }
    
    printf("Checksum: %lu\n", checksum);
    
    /* Return value based on checksum to prevent dead code elimination */
    return (checksum & 0xFF) == 0 ? 0 : 1;
}
