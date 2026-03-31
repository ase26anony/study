/* reload_coverage.c - Comprehensive test for GCC reload pass coverage */
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
    int arr[256];
    long *ptr_arr[16];
    struct misaligned_data packed_data[8];
    volatile int vol_var = 0;
    register int reg_var asm("r12") = 42; /* Try to pin to specific reg */
    
    /* Initialize arrays */
    for (int i = 0; i < 256; i++) arr[i] = i * 3;
    for (int i = 0; i < 8; i++) {
        packed_data[i].c = i;
        packed_data[i].i = i * 100;
        packed_data[i].l = i * 1000L;
    }
    
    /* Loop with varying constraints to trigger different reload types */
    for (int iter = 0; iter < 10; iter++) {
        int idx = iter * 7 % 256;
        long base = (long)(&arr[0]);
        long offset = idx * sizeof(int);
        long scale = (iter % 3) + 1;
        
        /* Force RELOAD_FOR_INPUT_ADDRESS and RELOAD_FOR_INPADDR_ADDRESS */
        /* Complex addressing mode that may need reload */
        asm volatile (
            "mov %[val], [%[base] + %[idx]*4 + %[disp]]\n\t"
            "add %[sum], %[sum], %[val]"
            : [sum] "+r" (global_sum), [val] "=r" (reg_var)
            : [base] "r" (base), [idx] "r" (idx), [disp] "i" (16),
              "m" (*(const int (*)[256])arr) /* memory input */
            : "memory", "cc"
        );
        
        /* Force RELOAD_FOR_OUTPUT_ADDRESS and RELOAD_FOR_OUTADDR_ADDRESS */
        /* Output with complex address computation */
        long *output_ptr = &arr[idx];
        asm volatile (
            "lea %[addr], [%[base] + %[idx]*%[scale] + 8]\n\t"
            "mov [%[addr]], %[val]"
            : [addr] "=r" (output_ptr), [val] "+r" (reg_var)
            : [base] "r" (base), [idx] "r" (idx), [scale] "r" (scale),
              "m" (*(int (*)[256])arr) /* memory output */
            : "memory"
        );
        
        /* Force RELOAD_FOR_OPERAND_ADDRESS and RELOAD_FOR_OPADDR_ADDR */
        /* Taking address of already complex operand */
        int (*complex_addr)[256] = &arr;
        asm volatile (
            "mov %[tmp1], %[arrptr]\n\t"
            "add %[tmp1], %[tmp1], %[offset]\n\t"
            "mov %[tmp2], [%[tmp1]]\n\t"
            "add %[sum], %[sum], %[tmp2]"
            : [sum] "+r" (global_sum), [tmp1] "=&r" (reg_var), 
              [tmp2] "=&r" (vol_var)
            : [arrptr] "r" (complex_addr), [offset] "r" (offset),
              "m" (*(const int (*)[256])arr)
            : "cc"
        );
        
        /* Force RELOAD_FOR_INPUT with register pressure */
        /* Many input operands to exhaust registers */
        int a = iter, b = iter + 1, c = iter + 2, d = iter + 3;
        int e = iter + 4, f = iter + 5, g = iter + 6, h = iter + 7;
        
        asm volatile (
            "add %[a], %[a], %[b]\n\t"
            "add %[c], %[c], %[d]\n\t"
            "add %[e], %[e], %[f]\n\t"
            "add %[g], %[g], %[h]\n\t"
            "imul %[a], %[a], %[c]\n\t"
            "imul %[e], %[e], %[g]\n\t"
            "add %[result], %[a], %[e]"
            : [result] "=r" (reg_var),
              [a] "+r" (a), [c] "+r" (c), [e] "+r" (e), [g] "+r" (g)
            : [b] "r" (b), [d] "r" (d), [f] "r" (f), [h] "r" (h)
            : "cc"
        );
        
        /* Force RELOAD_FOR_OTHER and RELOAD_FOR_OTHER_ADDRESS */
        /* Mixed constraints with early clobber and memory */
        struct misaligned_data *packed_ptr = &packed_data[iter % 8];
        asm volatile (
            "mov %[tmp], [%[ptr]]\n\t"        /* Load misaligned int */
            "add %[tmp], %[tmp], 1\n\t"
            "mov [%[ptr]], %[tmp]\n\t"
            "movzx %[chtmp], byte ptr [%[ptr] + 4]\n\t"  /* Load char after int */
            "add %[sum], %[sum], %[chtmp]"
            : [sum] "+r" (global_sum), [tmp] "=&r" (vol_var),
              [chtmp] "=&r" (a)  /* early clobber */
            : [ptr] "r" (packed_ptr), "m" (*packed_ptr)
            : "memory", "cc"
        );
        
        /* Force RELOAD_FOR_OUTPUT with early clobber */
        int out1, out2, out3;
        asm volatile (
            "mov %[o1], %[in1]\n\t"
            "lea %[o2], [%[in1] + %[in2]]\n\t"
            "imul %[o3], %[in1], %[in2]"
            : [o1] "=&r" (out1), [o2] "=&r" (out2), [o3] "=&r" (out3)
            : [in1] "r" (iter), [in2] "r" (idx)
            : "cc"
        );
        
        global_counter += reg_var + vol_var + out1 + out2 + out3;
    }
    
    /* Compute checksum to ensure all operations have effect */
    unsigned long checksum = global_counter + global_sum;
    for (int i = 0; i < 256; i++) checksum += arr[i];
    for (int i = 0; i < 8; i++) {
        checksum += packed_data[i].c + packed_data[i].i + packed_data[i].l;
    }
    
    printf("Checksum: %lu\n", checksum);
    return (int)(checksum % 1000);
}
