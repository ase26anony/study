/* reload_coverage.c - Complex program to trigger GCC reload pass edge cases */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* Prevent inlining and inter-procedural analysis */
__attribute__((noinline, noipa))
static int trigger_reloads(
    volatile int v1, volatile int v2, volatile int v3,
    volatile int v4, volatile int v5, volatile int v6,
    volatile long v7, volatile long v8, volatile double v9)
{
    /* Create high register pressure with many live values */
    int local1 = v1 * 2;
    int local2 = v2 + v3;
    double local3 = (double)v4 * v9;
    long local4 = v7 ^ v8;
    int local5 = v5 - v6;
    double local6 = local3 * 2.5;
    
    /* Multi-dimensional arrays with volatile indexing */
    int arr3d[10][10][10];
    double dbl_arr[20][15];
    char char_arr[100][50];
    
    /* Complex pointer chains */
    int ***ptr_chain1;
    double **ptr_chain2;
    
    /* Force spills across register classes */
    __asm__ volatile("" : : : "memory");  /* Compiler barrier */
    
    /* RELOAD_FOR_INPUT_ADDRESS: Complex array addressing */
    for (volatile int i = 0; i < v1 % 5; i++) {
        for (volatile int j = 0; j < v2 % 5; j++) {
            for (volatile int k = 0; k < v3 % 5; k++) {
                /* Multi-level addressing with volatile indices */
                arr3d[i + v1][j * v2][k % v3] = 
                    (i * v4) + (j * v5) + (k * v6);
                
                /* Mixed-type addressing */
                dbl_arr[(i + j) % 20][(k + v4) % 15] = 
                    (double)arr3d[i][j][k] * v9;
            }
        }
    }
    
    __asm__ volatile("" : : : "memory");
    
    /* RELOAD_FOR_OUTPUT_ADDRESS: Complex pointer arithmetic */
    int *base_ptr = (int*)arr3d;
    volatile int offset1 = v1 * sizeof(int);
    volatile int offset2 = v2 * 10 * sizeof(int);
    volatile int offset3 = v3 * 100 * sizeof(int);
    
    /* Chain of pointer calculations requiring temporary registers */
    int *ptr1 = base_ptr + offset1;
    int *ptr2 = ptr1 + offset2;
    int *ptr3 = ptr2 + offset3;
    
    /* Store through complex address */
    *ptr3 = v4 + v5 + v6;
    
    /* RELOAD_FOR_OPERAND_ADDRESS: Inline assembly with address constraints */
    int asm_result1, asm_result2;
    __asm__ volatile(
        "movl (%[addr1]), %[out1]\n\t"
        "addl %[in1], %[out1]\n\t"
        "movl %[out1], (%[addr2])"
        : [out1] "=&r" (asm_result1), "=m" (*ptr3)
        : [addr1] "r" (&local1), [in1] "r" (v1),
          [addr2] "r" (ptr3)
        : "memory"
    );
    
    __asm__ volatile("" : : : "memory");
    
    /* RELOAD_FOR_INPADDR_ADDRESS: Nested addressing in assembly */
    long asm_result3;
    __asm__ volatile(
        "leaq (%[base], %[idx], 4), %[temp]\n\t"
        "movq (%[temp]), %[out]"
        : [out] "=r" (asm_result3)
        : [base] "r" (base_ptr), [idx] "r" (v7),
          [temp] "r" (0)  /* Force reload for temp register */
        : "memory"
    );
    
    /* RELOAD_FOR_OUTADDR_ADDRESS: Output address calculation */
    volatile long *out_addr;
    __asm__ volatile(
        "leaq (%[base], %[scale], 8), %[addr]"
        : [addr] "=r" (out_addr)
        : [base] "r" (base_ptr), [scale] "r" (v8)
        : "cc"
    );
    
    *out_addr = asm_result3 + v7;
    
    /* Mixed floating-point and integer operations */
    double fp_result = 0.0;
    for (volatile int i = 0; i < v4 % 10; i++) {
        /* Force integer-to-float conversions */
        fp_result += (double)arr3d[i][i % 5][i % 3] * local3;
        
        /* Complex addressing in float array */
        dbl_arr[(i + v5) % 20][(i * v6) % 15] = 
            fp_result / (double)(local1 + 1);
    }
    
    __asm__ volatile("" : : : "memory");
    
    /* RELOAD_FOR_OTHER_ADDRESS: Unusual addressing modes */
    char *char_ptr = (char*)char_arr;
    volatile int char_offset = v1 * 50 + v2;
    
    /* Pointer with different scale */
    int *int_from_char = (int*)(char_ptr + char_offset * sizeof(char));
    
    /* Access with type conversion */
    local2 = *int_from_char + v3;
    
    /* More complex: array of pointers */
    int *ptr_array[20];
    for (int i = 0; i < 20; i++) {
        ptr_array[i] = &arr3d[i % 10][0][0];
    }
    
    /* Chain dereference with volatile index */
    volatile int idx = v6 % 20;
    int chain_result = *(ptr_array[idx] + v5);
    
    /* RELOAD_OTHER: Miscellaneous complex operations */
    /* Force multiple temporary registers */
    long complex_calc = 
        (v7 << (v1 % 8)) | 
        (v8 >> (v2 % 8)) |
        (local4 & (v3 * 0xFFFF));
    
    /* Use all local variables to extend live ranges */
    int final_result = 
        local1 + local2 + local5 + 
        (int)local3 + (int)local6 +
        (int)fp_result + chain_result +
        asm_result1 + (int)asm_result3 +
        (int)complex_calc;
    
    /* Volatile return prevents optimization */
    volatile int volatile_return = final_result;
    return volatile_return;
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
    volatile long v7 = rand() % 1000;
    volatile long v8 = rand() % 1000;
    volatile double v9 = (double)(rand() % 100) / 10.0;
    
    printf("Initial values: %d %d %d %ld %ld %.2f\n", 
           v1, v2, v3, v7, v8, v9);
    
    /* Call the reload-intensive function multiple times */
    int total = 0;
    for (int i = 0; i < 3; i++) {
        total += trigger_reloads(v1 + i, v2 + i, v3 + i,
                                v4, v5, v6,
                                v7 + i, v8 + i, v9);
    }
    
    printf("Result: %d\n", total);
    
    return 0;
}
