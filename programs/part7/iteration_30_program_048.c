/* reload_coverage.c - Comprehensive test to trigger all reload types in GCC */
#include <stdio.h>
#include <stdint.h>

/* Packed struct to force misaligned accesses */
struct __attribute__((packed)) misaligned_data {
    char c;
    int i;
    long l;
    char pad[3];
};

/* Volatile globals to prevent optimization */
volatile int global_counter = 0;
volatile long global_sum = 0;

int main() {
    /* Declare variables with different types and storage */
    int arr_int[256];
    long arr_long[128];
    char arr_char[512];
    volatile int vol_var = 42;
    struct misaligned_data packed_struct;
    
    /* Initialize arrays */
    for (int i = 0; i < 256; i++) arr_int[i] = i * 3;
    for (int i = 0; i < 128; i++) arr_long[i] = i * 5;
    for (int i = 0; i < 512; i++) arr_char[i] = (char)(i % 256);
    
    packed_struct.c = 'A';
    packed_struct.i = 0xDEADBEEF;
    packed_struct.l = 0xCAFEBABE12345678ULL;
    
    /* Take addresses to force spill/reload */
    int *ptr_int = &arr_int[100];
    long *ptr_long = &arr_long[50];
    char *ptr_char = &arr_char[200];
    struct misaligned_data *ptr_packed = &packed_struct;
    
    /* Loop with varying constraints to trigger different reload types */
    for (int iteration = 0; iteration < 10; iteration++) {
        int temp1, temp2, temp3;
        long ltemp1, ltemp2;
        char ctemp;
        
        /* VARYING CONSTRAINT 1: Complex addressing with multiple index registers
           Triggers: RELOAD_FOR_INPUT_ADDRESS, RELOAD_FOR_INPADDR_ADDRESS */
        asm volatile (
            "mov %[idx1], %[val1]\n\t"
            "mov %[idx2], %[val2]\n\t"
            "add %[base1] + %[idx1]*4 + %[disp1], %[res1]\n\t"
            : [res1] "=r" (temp1)
            : [base1] "r" (arr_int), 
              [idx1] "r" (iteration * 16),
              [val1] "i" (iteration),
              [idx2] "r" (iteration * 8),
              [val2] "i" (iteration * 2),
              [disp1] "i" (64),
              "m" (*(volatile int (*)[256])arr_int)
            : "memory", "cc"
        );
        
        /* VARYING CONSTRAINT 2: Early clobber with overlapping operands
           Triggers: RELOAD_FOR_INPUT, RELOAD_FOR_OUTPUT, RELOAD_OTHER */
        asm volatile (
            "mov %[in1], %[out1]\n\t"
            "add %[in2], %[out1]\n\t"
            "mov %[out1], %[addr]\n\t"
            : [out1] "=&r" (temp2), [addr] "=m" (arr_int[iteration])
            : [in1] "r" (vol_var), 
              [in2] "rm" (arr_long[iteration % 128]),
              "0" (temp2)  /* Ties output to input for extra pressure */
            : "cc"
        );
        
        /* VARYING CONSTRAINT 3: Nested address computation
           Triggers: RELOAD_FOR_OPERAND_ADDRESS, RELOAD_FOR_OPADDR_ADDR */
        asm volatile (
            "lea (%[base], %[index], 8), %[temp]\n\t"
            "mov (%[temp], %[offset]), %[result]\n\t"
            : [result] "=r" (temp3), [temp] "=&r" (ltemp1)
            : [base] "r" (arr_char),
              [index] "r" (iteration * 32),
              [offset] "r" (iteration * 2),
              "m" (*(volatile char (*)[512])arr_char)
            : "cc"
        );
        
        /* VARYING CONSTRAINT 4: Output address reloads
           Triggers: RELOAD_FOR_OUTPUT_ADDRESS, RELOAD_FOR_OUTADDR_ADDRESS */
        asm volatile (
            "mov %[val], %%rax\n\t"
            "mov %%rax, %[out]\n\t"
            : [out] "=m" (*(long*)((char*)ptr_packed + iteration))
            : [val] "r" (packed_struct.l + iteration),
              [dummy] "m" (packed_struct)
            : "rax", "memory", "cc"
        );
        
        /* VARYING CONSTRAINT 5: Multiple memory operands with displacement
           Triggers: RELOAD_FOR_OTHER_ADDRESS */
        asm volatile (
            "mov %[src1], %[dst1]\n\t"
            "mov %[src2], %[dst2]\n\t"
            "add %[src3], %[dst3]\n\t"
            : [dst1] "=rm" (arr_int[iteration + 10]),
              [dst2] "=rm" (arr_long[iteration % 64]),
              [dst3] "=rm" (arr_char[iteration + 100])
            : [src1] "irm" (iteration * 100),
              [src2] "irm" (global_counter),
              [src3] "irm" (temp1),
              "m" (*(volatile int (*)[256])arr_int),
              "m" (*(volatile long (*)[128])arr_long),
              "m" (*(volatile char (*)[512])arr_char)
            : "cc"
        );
        
        /* Mix register classes - more operands than available registers */
        asm volatile (
            "mov %[a], %%r10\n\t"
            "add %[b], %%r10\n\t"
            "mov %%r10, %[c]\n\t"
            "imul %[d], %%r10\n\t"
            "add %[e], %%r10\n\t"
            "mov %%r10, %[f]\n\t"
            "sub %[g], %%r10\n\t"
            "mov %%r10, %[h]\n\t"
            : [c] "=rm" (arr_int[iteration + 20]),
              [f] "=rm" (arr_int[iteration + 30]),
              [h] "=rm" (arr_int[iteration + 40])
            : [a] "r" (iteration),
              [b] "r" (global_counter),
              [d] "r" (temp2),
              [e] "r" (temp3),
              [g] "r" (vol_var),
              "m" (*(volatile int (*)[256])arr_int)
            : "r10", "cc", "memory"
        );
        
        /* Update volatile to prevent dead code elimination */
        global_counter += iteration;
        vol_var = temp1 + temp2 + temp3;
    }
    
    /* Compute checksum to ensure all operations have side effects */
    long checksum = 0;
    for (int i = 0; i < 256; i++) checksum += arr_int[i];
    for (int i = 0; i < 128; i++) checksum += arr_long[i];
    for (int i = 0; i < 512; i++) checksum += arr_char[i];
    
    checksum += packed_struct.i + packed_struct.l;
    checksum += global_counter + vol_var;
    
    printf("Checksum: %ld\n", checksum);
    printf("Reload coverage test completed.\n");
    
    return (checksum != 0) ? 0 : 1;
}
