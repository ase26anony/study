/* reload_coverage.c - Complex program to trigger GCC reload pass coverage */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Force no optimization on specific variables */
#define NOOPT __attribute__((optimize("O0")))

/* Function that will trigger multiple reload types */
__attribute__((noinline, noipa, optimize("O2")))
NOOPT int trigger_reloads(
    volatile int v1, volatile int v2, volatile int v3,
    volatile int v4, volatile int v5, volatile int v6,
    volatile long v7, volatile long v8, volatile double v9)
{
    /* Create high register pressure with many live values */
    int local1 = v1 * 2;
    int local2 = v2 + v3;
    int local3 = v4 ^ v5;
    int local4 = v6 << 2;
    long local5 = v7 + v8;
    double local6 = (double)v1 * v9;
    double local7 = v9 * 2.0;
    
    /* Multi-dimensional arrays with volatile indexing */
    int arr3d[10][10][10];
    double dbl_arr[20][20];
    char char_arr[100][50];
    
    /* Force memory barrier to extend live ranges */
    __asm__ volatile("" : : : "memory");
    
    /* Complex pointer chains for address reloads */
    int *ptr1 = &arr3d[v1][v2][0];
    int *ptr2 = ptr1 + v3;
    int **ptr3 = (int **)&ptr2;
    int ***ptr4 = &ptr3;
    
    /* RELOAD_FOR_INPUT_ADDRESS: Complex address calculation */
    int idx1 = v1 * v2 + v3;
    int idx2 = v4 % 10;
    int idx3 = v5 & 0xF;
    
    /* Multi-level array access with volatile indices */
    arr3d[idx1][idx2][idx3] = v6;
    
    /* More complex: pointer arithmetic with different types */
    char *cptr = (char *)arr3d;
    cptr += v1 * sizeof(int) * 100 + v2 * sizeof(int) * 10 + v3 * sizeof(int);
    
    __asm__ volatile("" : : : "memory");
    
    /* RELOAD_FOR_OUTPUT_ADDRESS: Store with complex address */
    *(int *)(cptr + v4 * 4) = v5;
    
    /* Mixed register class pressure */
    for (int i = 0; i < v1 % 5; i++) {
        for (int j = 0; j < v2 % 5; j++) {
            /* Integer computation */
            dbl_arr[i][j] = (double)(arr3d[i][j][0] * local1);
            
            /* Floating point computation */
            local6 += dbl_arr[i][j] * local7;
            
            /* Type conversion forcing register moves */
            local1 += (int)dbl_arr[i][j];
        }
    }
    
    __asm__ volatile("" : : : "memory");
    
    /* Inline assembly to trigger RELOAD_FOR_OPERAND_ADDRESS */
    int asm_result1, asm_result2;
    volatile int *volatile_ptr = &local2;
    
    /* Assembly with memory input and register output */
    __asm__ volatile (
        "movl (%[input]), %[out1]\n\t"
        "leal 1(%[out1]), %[out2]"
        : [out1] "=&r" (asm_result1), [out2] "=r" (asm_result2)
        : [input] "r" (volatile_ptr)
        : "memory"
    );
    
    /* More complex assembly with multiple constraints */
    long asm_result3;
    int *addr1 = &arr3d[v1][v2][v3];
    int *addr2 = &arr3d[v4][v5][v6 % 10];
    
    __asm__ volatile (
        "movq %[addr1], %%rax\n\t"
        "subq %[addr2], %%rax\n\t"
        "movq %%rax, %[result]"
        : [result] "=r" (asm_result3)
        : [addr1] "rm" (addr1), [addr2] "rm" (addr2)
        : "rax", "memory"
    );
    
    /* RELOAD_FOR_INPADDR_ADDRESS: Address of address */
    int ****complex_ptr = (int ****)malloc(sizeof(int ***));
    *complex_ptr = ptr4;
    
    /* Access through multiple indirections */
    int value = ****complex_ptr;
    
    /* RELOAD_FOR_OUTADDR_ADDRESS: Store address */
    int *out_addr;
    __asm__ volatile (
        "leaq %[array], %[out]"
        : [out] "=r" (out_addr)
        : [array] "m" (arr3d[0][0][0])
        : "memory"
    );
    
    *out_addr = v1 + v2;
    
    /* Use all local variables to prevent optimization */
    int sum = local1 + local2 + local3 + local4 + (int)local5 
              + (int)local6 + (int)local7 + asm_result1 
              + asm_result2 + (int)asm_result3 + value + *out_addr;
    
    free(complex_ptr);
    
    /* Force return value usage */
    __asm__ volatile("" : : "r" (sum) : "memory");
    return sum;
}

/* Helper with different calling convention */
__attribute__((noinline, regparm(3)))
int helper_func(volatile int a, volatile int b, volatile int c, 
                volatile int d, volatile int e) {
    /* Creates additional register pressure */
    int arr[50];
    for (int i = 0; i < 50; i++) {
        arr[i] = a * i + b * (i % 10) + c;
    }
    
    /* Complex address calculation */
    int *ptr = &arr[a % 50];
    ptr += b % 20;
    ptr += c % 10;
    
    return *ptr + d + e;
}

int main(void) {
    /* Initialize with random values */
    srand(42);
    
    volatile int v1 = rand() % 100;
    volatile int v2 = rand() % 100;
    volatile int v3 = rand() % 100;
    volatile int v4 = rand() % 100;
    volatile int v5 = rand() % 100;
    volatile int v6 = rand() % 100;
    volatile long v7 = rand() % 1000;
    volatile long v8 = rand() % 1000;
    volatile double v9 = (double)(rand() % 100) / 10.0;
    
    /* Call helper to increase register pressure */
    int intermediate = helper_func(v1, v2, v3, v4, v5);
    
    /* Main call that should trigger reloads */
    int result = trigger_reloads(v1 + intermediate, v2, v3, v4, v5, v6, 
                                 v7, v8, v9);
    
    /* Use result to prevent optimization */
    printf("Result: %d (v1=%d, v2=%d, v3=%d)\n", result, v1, v2, v3);
    
    /* Additional computation to keep variables live */
    volatile int check = v1 + v2 + v3 + v4 + v5 + v6;
    printf("Check: %d\n", check);
    
    return 0;
}
