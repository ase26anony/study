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
    char buffer[128];
    volatile int volatile_var = 42;
    struct misaligned_data packed;
    register int reg_var asm ("r12") = 0;  /* Try to tie up a register */
    
    /* Initialize arrays */
    for (int i = 0; i < 256; i++) arr[i] = i * 3;
    for (int i = 0; i < 16; i++) ptr_arr[i] = (long*)&arr[i * 8];
    for (int i = 0; i < 128; i++) buffer[i] = i;
    
    packed.c = 'A';
    packed.i = 0xDEADBEEF;
    packed.l = 0x123456789ABCDEF0LL;
    
    /* Loop with varying constraints to trigger different reload types */
    for (int iter = 0; iter < 10; iter++) {
        int idx = iter * 7;
        long temp_sum = 0;
        
        /* VARYING CONSTRAINT PATTERNS TO TRIGGER DIFFERENT RELOAD TYPES */
        
        /* Pattern 1: Complex addressing modes - RELOAD_FOR_INPUT_ADDRESS */
        asm volatile (
            "mov %[val1], %[dest1]\n\t"
            : [dest1] "=r" (temp_sum)
            : [val1] "m" (arr[idx * 2 + idx / 3 + 5])
            : "memory"
        );
        
        /* Pattern 2: Multiple operands with register pressure - RELOAD_FOR_INPUT/OUTPUT */
        asm volatile (
            "add %[in1], %[in2], %[out1]\n\t"
            "sub %[in3], %[out1], %[out2]\n\t"
            : [out1] "=&r" (arr[iter]), [out2] "=&r" (arr[iter + 1])
            : [in1] "r" (temp_sum), [in2] "r" (iter * 100), 
              [in3] "m" (packed.i), "0" (arr[iter]), "1" (arr[iter + 1])
            : "cc"
        );
        
        /* Pattern 3: Nested address computation - RELOAD_FOR_OPERAND_ADDRESS */
        long* addr_ptr;
        asm volatile (
            "lea (%[base], %[index], 4), %[addr]\n\t"
            : [addr] "=r" (addr_ptr)
            : [base] "r" (arr), [index] "r" (idx)
            : 
        );
        
        /* Pattern 4: Output address reload - RELOAD_FOR_OUTPUT_ADDRESS */
        long output_val;
        asm volatile (
            "mov %[src], %[dst]\n\t"
            : [dst] "=m" (*(long*)(buffer + iter * 8))
            : [src] "r" ((long)iter * 0x1000)
            : "memory"
        );
        
        /* Pattern 5: Input address address - RELOAD_FOR_INPADDR_ADDRESS */
        asm volatile (
            "mov (%[addr]), %[val]\n\t"
            : [val] "=r" (temp_sum)
            : [addr] "r" (&ptr_arr[iter % 8]), "m" (ptr_arr[iter % 8])
            : "memory"
        );
        
        /* Pattern 6: Mixed constraints with early clobber - RELOAD_OTHER */
        int tmp1, tmp2, tmp3;
        asm volatile (
            "imul %[a], %[b]\n\t"
            "add %[c], %[b]\n\t"
            "mov %[b], %[d]\n\t"
            : [b] "=&r" (tmp1), [d] "=r" (tmp2)
            : [a] "r" (iter), [c] "m" (volatile_var), "0" (tmp1), "1" (tmp2)
            : "cc"
        );
        
        /* Pattern 7: Address of address computation - RELOAD_FOR_OPADDR_ADDR */
        long** double_ptr;
        asm volatile (
            "mov %[ptr], %[dptr]\n\t"
            : [dptr] "=r" (double_ptr)
            : [ptr] "r" (&addr_ptr)
            : 
        );
        
        /* Pattern 8: Other address reloads - RELOAD_FOR_OTHER_ADDRESS */
        asm volatile (
            "movq (%[base], %[idx], 8), %%rax\n\t"
            "addq $1, %%rax\n\t"
            "movq %%rax, (%[base], %[idx], 8)\n\t"
            : 
            : [base] "r" (arr), [idx] "r" (iter * 2)
            : "rax", "memory", "cc"
        );
        
        /* Pattern 9: Output address address - RELOAD_FOR_OUTADDR_ADDRESS */
        long* outaddr_ptr;
        asm volatile (
            "mov %[val], (%[ptr])\n\t"
            : "=m" (*(long**)&outaddr_ptr)
            : [val] "r" (&arr[iter]), [ptr] "r" (&outaddr_ptr)
            : "memory"
        );
        
        /* Accumulate results to prevent dead code elimination */
        global_sum += temp_sum + arr[iter] + *addr_ptr + iter;
        volatile_var += iter;
        reg_var += tmp1 + tmp2;
        
        /* Force memory barrier */
        asm volatile ("" ::: "memory");
    }
    
    /* Compute final checksum */
    long final_checksum = global_sum;
    for (int i = 0; i < 256; i++) final_checksum += arr[i];
    for (int i = 0; i < 128; i++) final_checksum += buffer[i];
    final_checksum += packed.i + packed.l + volatile_var + reg_var;
    
    printf("Checksum: %ld\n", final_checksum);
    return (final_checksum != 0) ? 0 : 1;
}
