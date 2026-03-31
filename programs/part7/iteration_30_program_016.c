/* reload_coverage.c
 * This program is designed to trigger various reload types in GCC's reload pass
 * by creating complex addressing modes, register pressure, and operand constraints.
 */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

/* Packed struct to force misaligned accesses */
struct __attribute__((packed)) misaligned_data {
    char c;
    int i;
    long l;
    char pad[3];
};

/* Volatile globals to prevent optimization */
volatile long global_counter = 0;
volatile int global_index = 0;

/* Function to create varying addressing modes */
void create_complex_addressing(int iteration) {
    /* Local variables with different types and storage */
    int arr_int[32] __attribute__((aligned(4)));
    long arr_long[16] __attribute__((aligned(8)));
    char arr_char[64];
    volatile double arr_double[8];
    
    /* Initialize arrays */
    for (int i = 0; i < 32; i++) arr_int[i] = i + iteration;
    for (int i = 0; i < 16; i++) arr_long[i] = i * 2 + iteration;
    for (int i = 0; i < 64; i++) arr_char[i] = (char)(i ^ iteration);
    for (int i = 0; i < 8; i++) arr_double[i] = i * 1.5 + iteration;
    
    /* Packed struct with misaligned members */
    struct misaligned_data packed;
    packed.c = 'A' + iteration;
    packed.i = 0xDEADBEEF + iteration;
    packed.l = 0xCAFEBABE12345678ULL + iteration;
    
    /* Pointers with complex computations */
    int *ptr_int = &arr_int[16];
    long *ptr_long = &arr_long[8];
    char *ptr_char = &arr_char[32];
    
    /* Register variables to encourage register allocation */
    register int r1 asm("r15") = iteration * 3;
    register int r2 asm("r14") = iteration * 5;
    register long r3 asm("r13") = iteration * 7;
    
    /* Varying constraints based on iteration */
    const char *constraints[][8] = {
        /* RELOAD_FOR_INPUT, RELOAD_FOR_INPUT_ADDRESS */
        {"=r","r","r","m","r","r","r","r"},
        /* RELOAD_FOR_OUTPUT_ADDRESS, RELOAD_FOR_OUTADDR_ADDRESS */
        {"=&r","=m","r","r","r","r","r","r"},
        /* RELOAD_FOR_OPERAND_ADDRESS, RELOAD_FOR_OPADDR_ADDR */
        {"r","m","m","m","r","r","r","r"},
        /* RELOAD_FOR_OTHER_ADDRESS, RELOAD_OTHER */
        {"=r","r","r","r","r","r","r","r"}
    };
    
    int constraint_set = iteration % 4;
    
    /* Complex inline assembly with multiple memory operands */
    /* This should trigger RELOAD_FOR_INPUT_ADDRESS and RELOAD_FOR_INPADDR_ADDRESS */
    asm volatile (
        /* Output operands with early clobber */
        "mov %[out1], %[in1]\n\t"
        "lea (%[base], %[index], 4), %[addr1]\n\t"
        "mov (%[addr1], %[disp]), %[out2]\n\t"
        /* Complex addressing for input */
        "add (%[base2], %[index2], 8, %[scale]), %[out3]\n\t"
        /* Take address of memory operand */
        "lea (%[memop]), %[addr2]\n\t"
        "mov %[addr2], %[out4]"
        
        : [out1] "=r" (arr_int[0]),
          [out2] "=r" (arr_int[1]),
          [out3] "=r" (arr_int[2]),
          [out4] "=r" (arr_int[3]),
          [addr1] "=&r" (r1),
          [addr2] "=&r" (r2)
        
        : [in1] "r" (iteration),
          [base] "r" (arr_long),
          [index] "r" (r3),
          [disp] "r" (8),
          [base2] "r" (arr_char),
          [index2] "r" (iteration * 2),
          [scale] "r" (1),
          [memop] "m" (packed.i)
        
        : "memory", "cc", "r11", "r12"
    );
    
    /* Second asm block with different constraints to trigger more reload types */
    /* This should trigger RELOAD_FOR_OUTPUT_ADDRESS and RELOAD_FOR_OUTADDR_ADDRESS */
    long complex_addr;
    asm volatile (
        /* Compute complex address and store */
        "lea (%[ptr], %[idx], %[stride]), %[addr]\n\t"
        "mov %[val], (%[addr], %[offset])\n\t"
        /* Address of address computation */
        "lea (%[addr]), %[addr2]\n\t"
        "mov %[addr2], %[stored_addr]"
        
        : [addr] "=&r" (complex_addr),
          [addr2] "=&r" (r3),
          [stored_addr] "=m" (arr_long[4]),
          "=m" (arr_char[16])
        
        : [ptr] "r" (ptr_int),
          [idx] "r" (iteration),
          [stride] "i" (sizeof(int) * 4),
          [val] "r" (iteration + 0x1000),
          [offset] "r" (-16)
        
        : "memory"
    );
    
    /* Third asm: Many operands to cause register pressure */
    /* Should trigger RELOAD_FOR_INPUT and RELOAD_OTHER */
    int temp1, temp2, temp3, temp4, temp5, temp6, temp7, temp8;
    asm volatile (
        "mov %[a], %[t1]\n\t"
        "add %[b], %[t1]\n\t"
        "imul %[c], %[t1]\n\t"
        "mov %[t1], %[t2]\n\t"
        "add %[d], %[t2]\n\t"
        "mov %[t2], %[t3]\n\t"
        "sub %[e], %[t3]\n\t"
        "mov %[t3], %[t4]\n\t"
        "and %[f], %[t4]\n\t"
        "mov %[t4], %[t5]\n\t"
        "or %[g], %[t5]\n\t"
        "mov %[t5], %[t6]\n\t"
        "xor %[h], %[t6]\n\t"
        "mov %[t6], %[t7]\n\t"
        "shl $3, %[t7]\n\t"
        "mov %[t7], %[t8]"
        
        : [t1] "=&r" (temp1),
          [t2] "=&r" (temp2),
          [t3] "=&r" (temp3),
          [t4] "=&r" (temp4),
          [t5] "=&r" (temp5),
          [t6] "=&r" (temp6),
          [t7] "=&r" (temp7),
          [t8] "=r" (temp8)
        
        : [a] "r" (arr_int[0]),
          [b] "r" (arr_int[1]),
          [c] "r" (arr_int[2]),
          [d] "r" (arr_int[3]),
          [e] "r" (arr_long[0]),
          [f] "r" (arr_long[1]),
          [g] "r" (arr_long[2]),
          [h] "r" (arr_long[3])
        
        : "cc"
    );
    
    /* Use the results to prevent dead code elimination */
    arr_int[4] = temp1 + temp8;
    global_counter += complex_addr + r1 + r2 + r3;
}

/* Main function with loop creating varying constraints */
int main() {
    uint64_t checksum = 0;
    
    /* Create varying addressing scenarios in loop */
    for (int i = 0; i < 16; i++) {
        global_index = i;
        
        /* Call function with iteration-dependent behavior */
        create_complex_addressing(i);
        
        /* Additional inline asm with operand address reloads */
        /* Should trigger RELOAD_FOR_OPERAND_ADDRESS and RELOAD_FOR_OPADDR_ADDR */
        int var1 = i * 11;
        int var2 = i * 13;
        int var3 = i * 17;
        int *ptr_var = &var1;
        
        asm volatile (
            /* Take address of operand, then use it */
            "lea %[var], %[addr]\n\t"
            "mov (%[addr]), %[tmp]\n\t"
            "add %[inc], %[tmp]\n\t"
            "mov %[tmp], (%[addr])"
            
            : [tmp] "=&r" (var2),
              [addr] "=&r" (ptr_var),
              [var] "+m" (var1)
            
            : [inc] "ri" (i + 1)
            
            : "cc"
        );
        
        /* Complex asm with memory clobber for RELOAD_FOR_OTHER_ADDRESS */
        asm volatile (
            "mov %[in1], %%r10\n\t"
            "add %[in2], %%r10\n\t"
            "mov %%r10, %[out1]\n\t"
            "lea (%[mem1], %%r10, 2), %%r11\n\t"
            "mov %%r11, %[out2]"
            
            : [out1] "=m" (var3),
              [out2] "=r" (ptr_var)
            
            : [in1] "r" (i),
              [in2] "r" (i * 19),
              [mem1] "m" (global_counter)
            
            : "memory", "r10", "r11", "cc"
        );
        
        checksum += var1 + var2 + var3 + (uint64_t)ptr_var;
    }
    
    /* Final checksum computation and output */
    checksum += global_counter;
    printf("Checksum: %llu\n", (unsigned long long)checksum);
    
    return checksum != 0 ? 0 : 1;
}
