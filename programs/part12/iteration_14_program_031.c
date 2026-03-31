/* reload_coverage.c - Complex program to trigger GCC reload pass switch cases */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Prevent inlining and inter-procedural analysis */
__attribute__((noinline, noipa))
static int trigger_reloads(
    volatile int idx1, volatile int idx2, volatile int idx3,
    volatile int stride1, volatile int stride2, volatile int scale,
    volatile long offset1, volatile long offset2)
{
    /* High register pressure with mixed types */
    int int_arr[128];          /* Integer array - pressure on integer regs */
    double fp_arr[64];         /* FP array - pressure on FP regs */
    long long_arr[96];         /* Long array - more integer pressure */
    char *ptr_chain[32];       /* Pointer array for indirection */
    
    /* Many scalar variables to increase register pressure */
    int temp1, temp2, temp3, temp4, temp5, temp6, temp7, temp8;
    double ftemp1, ftemp2, ftemp3, ftemp4;
    long ltemp1, ltemp2, ltemp3;
    void *addr1, *addr2, *addr3;
    
    /* Initialize arrays */
    for (int i = 0; i < 128; i++) int_arr[i] = i * 3;
    for (int i = 0; i < 64; i++) fp_arr[i] = i * 1.5;
    for (int i = 0; i < 96; i++) long_arr[i] = i * 5L;
    
    /* Memory barrier to prevent optimization */
    __asm__ volatile("" : : : "memory");
    
    /* ====== RELOAD_FOR_INPUT_ADDRESS ====== */
    /* Complex address calculation requiring temporary register */
    temp1 = int_arr[idx1 * stride1 + idx2 * stride2 + idx3];
    
    /* Multi-level pointer indirection */
    char *base_ptr = (char *)int_arr;
    char *mid_ptr = base_ptr + offset1 * sizeof(int);
    int *final_ptr = (int *)(mid_ptr + offset2);
    temp2 = *final_ptr;
    
    /* ====== RELOAD_FOR_OUTPUT_ADDRESS ====== */
    /* Complex store address calculation */
    int_arr[(idx1 * scale + idx2) % 128] = temp1 + temp2;
    
    /* ====== Mixed register class pressure ====== */
    /* Integer to FP conversion and computation */
    ftemp1 = (double)temp1;
    ftemp2 = (double)temp2;
    ftemp3 = fp_arr[idx1 % 64] * ftemp1 + fp_arr[idx2 % 64] * ftemp2;
    
    /* FP to integer conversion */
    temp3 = (int)(ftemp3 * scale);
    
    /* ====== Inline assembly with constraints ====== */
    /* RELOAD_FOR_OPERAND_ADDRESS and RELOAD_FOR_OUTADDR_ADDRESS */
    int asm_result1, asm_result2;
    void *asm_addr;
    
    /* Assembly taking memory address as input */
    __asm__ volatile(
        "mov %[addr], %%rsi\n\t"          /* Load address into register */
        "mov (%%rsi), %[out1]\n\t"        /* Load from that address */
        "lea 8(%%rsi), %[out2]\n\t"       /* Compute new address */
        : [out1] "=r" (asm_result1),      /* Output in register */
          [out2] "=r" (asm_result2)
        : [addr] "m" (&int_arr[idx1])     /* Memory address input */
        : "rsi", "memory"
    );
    
    /* Assembly with earlyclobber and multiple outputs */
    int out_reg1, out_reg2;
    __asm__ volatile(
        "mov %[in1], %%rax\n\t"
        "imul %[in2], %%rax\n\t"
        "mov %%rax, %[out1]\n\t"
        "add $100, %%rax\n\t"
        "mov %%rax, %[out2]"
        : [out1] "=&r" (out_reg1),        /* Earlyclobber */
          [out2] "=r" (out_reg2)
        : [in1] "r" (temp1),
          [in2] "r" (temp2)
        : "rax", "cc"
    );
    
    /* ====== Complex multi-dimensional access ====== */
    /* Simulate 2D array access with volatile indices */
    int pseudo_2d[16][16];
    for (int i = 0; i < 16; i++)
        for (int j = 0; j < 16; j++)
            pseudo_2d[i][j] = i * 16 + j;
    
    /* Complex address with multiple volatile components */
    temp4 = pseudo_2d[idx1 % 16][(idx2 + idx3) % 16];
    
    /* Pointer chain dereference */
    ptr_chain[0] = (char *)pseudo_2d;
    for (int i = 1; i < 8; i++) {
        ptr_chain[i] = ptr_chain[i-1] + stride1 * i;
    }
    
    /* Triple indirection */
    char ***triple_ptr = (char ***)&ptr_chain;
    char **second_ptr = *triple_ptr;
    char *first_ptr = second_ptr[idx1 % 8];
    temp5 = *(int *)first_ptr;
    
    /* ====== More memory barriers ====== */
    __asm__ volatile("" : : : "memory");
    
    /* ====== Mixed operations ====== */
    /* Integer arithmetic */
    temp6 = temp1 * temp2 + temp3 * temp4 - temp5;
    
    /* Floating point with integer conversion */
    ftemp4 = ftemp3 * (double)temp6 / (double)scale;
    
    /* Long integer operations */
    ltemp1 = (long)temp1 * (long)temp2;
    ltemp2 = (long)temp3 * (long)temp4;
    ltemp3 = ltemp1 + ltemp2 + offset1 - offset2;
    
    /* Store with complex address */
    long_arr[ltemp3 % 96] = ltemp1 - ltemp2;
    
    /* ====== Final computation ====== */
    int result = (temp1 + temp2 + temp3 + temp4 + temp5 + temp6 +
                  (int)ftemp1 + (int)ftemp2 + (int)ftemp3 + (int)ftemp4 +
                  (int)ltemp1 + (int)ltemp2 + (int)ltemp3 +
                  asm_result1 + asm_result2 + out_reg1 + out_reg2);
    
    /* Force all values to be used */
    __asm__ volatile("" 
        : 
        : "r" (temp1), "r" (temp2), "r" (temp3), "r" (temp4),
          "r" (temp5), "r" (temp6), "r" (asm_result1), "r" (asm_result2),
          "r" (out_reg1), "r" (out_reg2),
          "x" (ftemp1), "x" (ftemp2), "x" (ftemp3), "x" (ftemp4)
        : "memory");
    
    return result % 1000;  /* Keep result small */
}

int main(void) {
    /* Initialize volatile variables with random values */
    volatile int idx1 = rand() % 100;
    volatile int idx2 = rand() % 100;
    volatile int idx3 = rand() % 100;
    volatile int stride1 = (rand() % 10) + 1;
    volatile int stride2 = (rand() % 10) + 1;
    volatile int scale = (rand() % 8) + 2;
    volatile long offset1 = rand() % 50;
    volatile long offset2 = rand() % 50;
    
    /* Call the reload-intensive function multiple times */
    int total = 0;
    for (int i = 0; i < 10; i++) {
        total += trigger_reloads(idx1 + i, idx2 + i, idx3 + i,
                                stride1, stride2, scale,
                                offset1 + i, offset2 + i);
    }
    
    printf("Result: %d\n", total);
    
    /* Additional test with different patterns */
    volatile int arr_indices[4] = {rand() % 20, rand() % 20, 
                                   rand() % 20, rand() % 20};
    
    /* Test with array of volatile indices */
    int sum = 0;
    for (int i = 0; i < 4; i++) {
        sum += trigger_reloads(
            arr_indices[0], arr_indices[1], arr_indices[2],
            arr_indices[3], stride1, stride2,
            offset1, offset2
        );
    }
    
    printf("Sum: %d\n", sum);
    
    return 0;
}
