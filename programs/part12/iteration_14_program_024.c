/* reload_coverage.c - Complex program to trigger GCC reload pass switch cases */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* Prevent inlining and inter-procedural analysis */
__attribute__((noinline, noipa))
static int trigger_reloads(
    volatile int idx1, volatile int idx2, volatile int idx3,
    volatile int stride1, volatile int stride2, volatile int scale,
    volatile long offset1, volatile long offset2)
{
    /* High register pressure with mixed types */
    int int_arr[128];          /* Integer array - pressure on integer regs */
    double fp_arr[64];         /* Double array - pressure on FP regs */
    long long_arr[96];         /* Long array - more integer pressure */
    char *ptr_chain[32];       /* Pointer array for indirection */
    int temp_results[16];      /* Temporary results */
    double fp_results[8];      /* FP temporaries */
    
    /* Complex addressing computations that need temporary registers */
    int *base_ptr = int_arr;
    double *fp_base = fp_arr;
    long *long_base = long_arr;
    
    /* Initialize arrays */
    for (int i = 0; i < 128; i++) int_arr[i] = i * 3;
    for (int i = 0; i < 64; i++) fp_arr[i] = i * 1.5;
    for (int i = 0; i < 96; i++) long_arr[i] = i * 7LL;
    
    /* Memory barrier to prevent reordering */
    __asm__ volatile("" : : : "memory");
    
    /* ===== RELOAD_FOR_INPUT_ADDRESS ===== */
    /* Complex address calculation requiring temporary register */
    int_arr[idx1 * stride1 + idx2 * stride2 + idx3] = 
        int_arr[idx2 * stride1 + idx3 * stride2] * scale;
    
    /* ===== RELOAD_FOR_OUTPUT_ADDRESS ===== */
    /* Store to complex computed address */
    long_base[offset1 + idx1 * 3 + idx2 * 2] = 
        (long)int_arr[idx1 + idx2] * long_base[idx3];
    
    /* ===== RELOAD_FOR_INPADDR_ADDRESS ===== */
    /* Address of address computation */
    int *addr1 = &int_arr[idx1 * stride1 + idx2];
    int *addr2 = &int_arr[idx2 * stride2 + idx3];
    *addr1 = *addr2 + scale;
    
    /* ===== RELOAD_FOR_OUTADDR_ADDRESS ===== */
    /* Complex address for store operation */
    double *fp_addr = &fp_arr[idx1 * 2 + idx2 * 3];
    *fp_addr = (double)(int_arr[idx3] * scale) / 2.0;
    
    /* Memory barrier */
    __asm__ volatile("" : : : "memory");
    
    /* ===== RELOAD_FOR_OPERAND_ADDRESS ===== */
    /* Inline assembly with memory operand constraints */
    int asm_result1, asm_result2;
    __asm__ volatile(
        "movl (%[addr]), %[out1]\n\t"
        "addl $1, %[out1]\n\t"
        "movl %[out1], (%[addr2])"
        : [out1] "=&r" (asm_result1), "=m" (int_arr[idx1])
        : [addr] "r" (&int_arr[idx2]), 
          [addr2] "r" (&int_arr[idx3]),
          "m" (int_arr[idx2])
        : "memory"
    );
    
    /* ===== RELOAD_FOR_OPADDR_ADDR ===== */
    /* Another assembly with different addressing */
    long asm_result3;
    __asm__ volatile(
        "movq (%[base], %[idx], 8), %[out]\n\t"
        "imulq $3, %[out]"
        : [out] "=&r" (asm_result3)
        : [base] "r" (long_base),
          [idx] "r" ((long)idx1 * sizeof(long)),
          "m" (long_base[idx1])
        : "cc"
    );
    
    /* ===== RELOAD_FOR_OTHER_ADDRESS ===== */
    /* Multi-level pointer indirection */
    char **ptr_ptr = (char**)&ptr_chain[0];
    for (int i = 0; i < 16; i++) {
        ptr_chain[i] = (char*)&int_arr[i * 8];
    }
    
    char *final_ptr = *(ptr_ptr + idx1) + idx2;
    int_arr[0] = *(int*)final_ptr;
    
    /* ===== RELOAD_FOR_INPUT ===== */
    /* Multiple uses of same value with complex addressing */
    int temp1 = int_arr[idx1] + int_arr[idx2];
    int temp2 = int_arr[idx3] * scale;
    int temp3 = int_arr[stride1] - int_arr[stride2];
    
    /* Mix integer and floating point to pressure different reg classes */
    for (int i = 0; i < 8; i++) {
        fp_results[i] = (double)int_arr[i * 4] * fp_arr[i * 2];
        fp_results[i] += (double)long_arr[i * 3];
        
        /* Convert back to integer */
        temp_results[i] = (int)fp_results[i] + scale;
    }
    
    /* ===== RELOAD_OTHER ===== */
    /* Complex expression with multiple intermediate values */
    int complex_result = 
        (int_arr[idx1 * stride1] * int_arr[idx2 * stride2]) +
        (int_arr[idx3] / scale) -
        (int_arr[offset1 % 128] << 2) +
        (temp1 * temp2) / (temp3 ? temp3 : 1);
    
    /* More memory barriers to extend live ranges */
    __asm__ volatile("" : : : "memory");
    
    /* Use all computed values to prevent elimination */
    int final_sum = complex_result + asm_result1 + (int)asm_result3;
    for (int i = 0; i < 8; i++) {
        final_sum += temp_results[i] + (int)fp_results[i];
    }
    
    /* Volatile return prevents optimization */
    volatile int volatile_return = final_sum;
    return volatile_return;
}

/* Helper with complex addressing pattern */
__attribute__((noinline, noipa))
static void additional_pressure(volatile int *arr, volatile double *fp_arr, 
                               volatile int idx1, volatile int idx2)
{
    /* Multi-dimensional access simulation */
    int *matrix[16];
    for (int i = 0; i < 16; i++) {
        matrix[i] = (int*)arr + i * 8;
    }
    
    /* Complex addressing with multiple levels */
    int val1 = matrix[idx1 % 16][idx2 % 8];
    int val2 = matrix[idx2 % 16][idx1 % 8];
    
    /* Chain of dependent computations */
    for (int i = 0; i < 4; i++) {
        int *row = matrix[(idx1 + i) % 16];
        for (int j = 0; j < 4; j++) {
            row[(idx2 + j) % 8] = 
                row[(idx2 + j - 1) % 8] * 3 +
                matrix[(idx1 + j) % 16][(idx2 + i) % 8];
        }
    }
    
    /* FP operation with integer conversion */
    double fp_temp = (double)val1 * 1.7 + (double)val2 * 0.3;
    fp_arr[0] = fp_temp;
    
    /* Inline assembly with multiple constraints */
    int asm_out;
    __asm__ volatile(
        "leaq (%[base], %[idx], 4), %%rax\n\t"
        "movl (%%rax), %[out]\n\t"
        "addl $42, %[out]"
        : [out] "=r" (asm_out)
        : [base] "r" (arr),
          [idx] "r" ((long)idx1),
          "m" (arr[idx1])
        : "rax", "cc"
    );
    
    arr[0] = asm_out;
}

int main(void)
{
    srand(time(NULL));
    
    /* Volatile variables to prevent constant propagation */
    volatile int idx1 = rand() % 50;
    volatile int idx2 = rand() % 50;
    volatile int idx3 = rand() % 50;
    volatile int stride1 = rand() % 10 + 1;
    volatile int stride2 = rand() % 10 + 1;
    volatile int scale = rand() % 100 + 1;
    volatile long offset1 = rand() % 80;
    volatile long offset2 = rand() % 80;
    
    printf("Starting complex reload test...\n");
    printf("Parameters: idx1=%d, idx2=%d, idx3=%d, stride1=%d, stride2=%d\n",
           idx1, idx2, idx3, stride1, stride2);
    
    /* Call main function to trigger reloads */
    int result = trigger_reloads(idx1, idx2, idx3, stride1, stride2, 
                                 scale, offset1, offset2);
    
    /* Additional pressure function */
    volatile int extra_arr[128];
    volatile double extra_fp[64];
    for (int i = 0; i < 128; i++) extra_arr[i] = rand();
    for (int i = 0; i < 64; i++) extra_fp[i] = rand() / 1000.0;
    
    additional_pressure(extra_arr, extra_fp, idx1, idx2);
    
    printf("Result: %d\n", result + extra_arr[0]);
    printf("Test completed.\n");
    
    return 0;
}
