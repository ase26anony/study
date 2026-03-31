/* reload_coverage.c - Program to trigger multiple reload types in GCC's reload pass */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* Force no optimization across this function */
__attribute__((noinline, noipa))
static int trigger_reloads(
    volatile int v1, volatile int v2, volatile int v3,
    volatile int v4, volatile int v5, volatile int v6,
    volatile int v7, volatile int v8, volatile int v9,
    volatile int v10)
{
    /* Create high register pressure with many live values */
    int local1 = v1 * 2;
    int local2 = v2 + v3;
    int local3 = v4 - v5;
    int local4 = v6 * v7;
    int local5 = v8 / (v9 + 1);
    int local6 = v10 ^ v1;
    int local7 = v2 | v3;
    int local8 = v4 & v5;
    int local9 = v6 << 2;
    int local10 = v7 >> 1;
    
    /* Arrays to force spills */
    int int_array[100];
    double double_array[50];
    long long_array[75];
    
    /* Complex pointer chains for address reloads */
    char *char_ptr1;
    char *char_ptr2;
    int *int_ptr1;
    int *int_ptr2;
    long *long_ptr1;
    double *double_ptr1;
    
    /* Initialize arrays with volatile values to prevent optimization */
    for (int i = 0; i < 100; i++) {
        int_array[i] = v1 + i * v2;
    }
    
    for (int i = 0; i < 50; i++) {
        double_array[i] = (double)(v3 + i) * 1.5;
    }
    
    for (int i = 0; i < 75; i++) {
        long_array[i] = (long)v4 * i + v5;
    }
    
    /* Memory barrier to prevent reordering */
    __asm__ volatile("" : : : "memory");
    
    /* RELOAD_FOR_INPUT_ADDRESS: Complex addressing mode 1 */
    /* Multi-dimensional array access simulation with volatile indices */
    int idx1 = v1 % 50;
    int idx2 = v2 % 25;
    int idx3 = v3 % 10;
    
    /* Complex address calculation requiring temporary register */
    int complex_addr1 = int_array[idx1 * v4 + idx2 * v5 + idx3];
    
    /* RELOAD_FOR_OUTPUT_ADDRESS: Complex addressing mode 2 */
    /* Pointer arithmetic with multiple volatile offsets */
    char_ptr1 = (char *)&int_array[0];
    char_ptr1 += v6 * sizeof(int) + v7;
    
    int_ptr1 = (int *)char_ptr1;
    *int_ptr1 = v8 + v9;  /* Store with complex address */
    
    /* Memory barrier */
    __asm__ volatile("" : : : "memory");
    
    /* RELOAD_FOR_INPADDR_ADDRESS: Nested pointer indirection */
    int **ptr_to_int_ptr = &int_ptr1;
    int_ptr2 = *ptr_to_int_ptr;
    int_ptr2 += v10 % 20;
    
    /* RELOAD_FOR_OUTADDR_ADDRESS: Output address computation */
    long_ptr1 = &long_array[v1 % 30];
    long_ptr1 += v2 % 15;
    
    /* Inline assembly to force RELOAD_FOR_OPERAND_ADDRESS */
    int asm_result1, asm_result2;
    int *addr_for_asm = &int_array[v3 % 40];
    
    /* Assembly with memory input and register output */
    __asm__ volatile (
        "movl (%1), %0\n\t"
        "addl $100, %0"
        : "=r" (asm_result1)
        : "r" (addr_for_asm)
        : "memory"
    );
    
    /* Another assembly with different constraints */
    double *double_addr = &double_array[v4 % 25];
    double asm_double_result;
    
    __asm__ volatile (
        "movsd (%1), %%xmm0\n\t"
        "addsd %%xmm1, %%xmm0\n\t"
        "movsd %%xmm0, %0"
        : "=m" (asm_double_result)
        : "r" (double_addr)
        : "xmm0", "xmm1", "memory"
    );
    
    /* Mixed register class pressure */
    /* Integer to float conversion and back */
    double temp_double = (double)local1 * 3.14159;
    int temp_int = (int)(temp_double * 2.0);
    
    /* More complex addressing with different types */
    /* RELOAD_FOR_OTHER_ADDRESS */
    char *base_ptr = (char *)&long_array[0];
    base_ptr += v5 * sizeof(long) + v6 * 2;
    
    long *derived_long_ptr = (long *)base_ptr;
    derived_long_ptr += v7 % 10;
    
    /* Nested pointer chain */
    char **char_ptr_ptr = &char_ptr2;
    *char_ptr_ptr = base_ptr + v8;
    
    /* Use all local variables to extend live ranges */
    int sum = local1 + local2 + local3 + local4 + local5 +
              local6 + local7 + local8 + local9 + local10 +
              complex_addr1 + *int_ptr1 + temp_int;
    
    /* Memory barrier before return */
    __asm__ volatile("" : : : "memory");
    
    return sum + v1 + v2 + v3;  /* Use volatile args to prevent elimination */
}

int main(void) {
    srand(time(NULL));
    
    /* Initialize volatile variables with random values */
    volatile int v1 = rand() % 100;
    volatile int v2 = rand() % 100;
    volatile int v3 = rand() % 100;
    volatile int v4 = rand() % 100;
    volatile int v5 = rand() % 100;
    volatile int v6 = rand() % 100;
    volatile int v7 = rand() % 100;
    volatile int v8 = rand() % 100;
    volatile int v9 = rand() % 100;
    volatile int v10 = rand() % 100;
    
    /* Call the function that should trigger reloads */
    int result = trigger_reloads(v1, v2, v3, v4, v5, v6, v7, v8, v9, v10);
    
    printf("Result: %d\n", result);
    printf("Volatile values: %d %d %d %d %d\n", v1, v2, v3, v4, v5);
    
    return 0;
}
