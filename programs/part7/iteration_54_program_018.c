/* reload_coverage.c - Comprehensive test for GCC reload pass coverage */
#include <stdio.h>
#include <stdint.h>

/* Volatile indices to prevent constant folding */
volatile int vi1 = 1, vi2 = 2, vi3 = 3, vi4 = 4;

/* Large arrays to increase register pressure */
int large_arr1[1000];
int large_arr2[1000];
int large_arr3[1000];

/* Complex addressing mode stress - triggers RELOAD_FOR_INPUT_ADDRESS, 
   RELOAD_FOR_OUTPUT_ADDRESS, etc. */
void complex_addressing(int n) {
    int arr[100][100];
    static int counter = 0;
    
    /* Force non-constant indices */
    int i = vi1 + counter;
    int j = vi2 * counter;
    int k = vi3 ^ counter;
    
    /* Complex addressing that can't be folded */
    arr[vi1 + i][j + 1] = arr[k][vi2 * j] + arr[i][k * 2];
    arr[j][k] = arr[vi3][i] + arr[vi4][j];
    
    /* Pointer arithmetic with volatile */
    int *ptr1 = &arr[0][0] + vi1 * 100 + vi2;
    int *ptr2 = &arr[vi3][0] + vi4 * 10;
    *ptr1 = *ptr2 + arr[vi1][vi2];
    
    counter++;
}

/* Structure for value passing - triggers address reloads for temporaries */
struct SmallStruct {
    int a, b, c, d;
};

struct SmallStruct struct_func1(struct SmallStruct s) {
    s.a += vi1;
    s.b ^= vi2;
    return s;
}

struct SmallStruct struct_func2(struct SmallStruct s) {
    s.c *= vi3;
    s.d -= vi4;
    return s;
}

/* Chain of structure passing - triggers RELOAD_FOR_INPADDR_ADDRESS,
   RELOAD_FOR_OUTADDR_ADDRESS */
int structure_passing_test(void) {
    struct SmallStruct s1 = {vi1, vi2, vi3, vi4};
    struct SmallStruct s2, s3;
    
    /* Chain of calls creating address reloads for temporaries */
    s2 = struct_func1(s1);
    s3 = struct_func2(s2);
    
    /* Use all fields to prevent optimization */
    return s3.a + s3.b + s3.c + s3.d;
}

/* Inline assembly with multiple constraints - triggers RELOAD_FOR_INPUT,
   RELOAD_FOR_OUTPUT_ADDRESS, RELOAD_OTHER */
void asm_reload_test(int *arr, int n) {
    int tmp1, tmp2, tmp3;
    
    /* First asm: output used as input in next asm */
    asm volatile (
        "movl %[in1], %[out1]\n\t"
        "addl %[in2], %[out1]"
        : [out1] "=r" (tmp1)
        : [in1] "r" (arr[vi1]),
          [in2] "r" (arr[vi2])
        : "cc"
    );
    
    /* Second asm: memory operand with complex addressing */
    asm volatile (
        "movl %[in1], %%eax\n\t"
        "imull %%eax, %[out1]\n\t"
        "movl %[out1], %[mem]"
        : [out1] "=r" (tmp2), [mem] "=m" (arr[vi3 + n])
        : [in1] "r" (tmp1),
          "[mem]" "m" (arr[vi3 + n])
        : "eax", "cc"
    );
    
    /* Third asm: multiple outputs with different constraints */
    asm volatile (
        "leal (%[in1], %[in2], 4), %[out1]\n\t"
        "movl %[out1], %[out2]"
        : [out1] "=r" (tmp3), [out2] "=m" (arr[vi4])
        : [in1] "r" (tmp2),
          [in2] "r" (vi1)
        : "cc"
    );
}

/* Vector extensions - may trigger various reload types */
typedef int v4si __attribute__((vector_size(16)));

void vector_test(void) {
    v4si a = {vi1, vi2, vi3, vi4};
    v4si b = {vi4, vi3, vi2, vi1};
    v4si c, d;
    
    /* Vector operations */
    c = a + b;
    d = a * b;
    
    /* Shuffle operation */
    d = __builtin_shuffle(a, b, (v4si){3, 2, 1, 0});
    
    /* Store to memory with complex addressing */
    int *ptr = &large_arr1[vi1 * 10];
    *(v4si*)ptr = c + d;
}

/* Control flow that splits live ranges - triggers RELOAD_FOR_OTHER_ADDRESS */
int control_flow_test(int n) {
    int x, y, z;
    
    /* Variable defined before goto */
    x = vi1 * n;
    
    if (n & 1) {
        y = vi2 + x;
        goto label1;
    } else {
        y = vi3 - x;
        goto label2;
    }
    
label1:
    z = y * vi4;
    /* Use in distant basic block */
    large_arr2[x % 1000] = z;
    goto end;
    
label2:
    z = y / vi1;
    /* Different distant use */
    large_arr3[x % 1000] = z;
    
end:
    /* Complex use spanning multiple blocks */
    return x + y + z + large_arr2[0] + large_arr3[0];
}

/* Main orchestrator */
int main(void) {
    int checksum = 0;
    
    /* Initialize arrays */
    for (int i = 0; i < 1000; i++) {
        large_arr1[i] = i;
        large_arr2[i] = i * 2;
        large_arr3[i] = i * 3;
    }
    
    /* Execute all patterns multiple times */
    for (int i = 0; i < 100; i++) {
        /* Complex addressing */
        complex_addressing(i);
        
        /* Structure passing */
        checksum += structure_passing_test();
        
        /* Inline assembly */
        asm_reload_test(large_arr1, i);
        
        /* Vector operations */
        if (i % 10 == 0) {
            vector_test();
        }
        
        /* Control flow with split ranges */
        checksum += control_flow_test(i);
        
        /* Update volatile indices to change patterns */
        vi1 = (vi1 * 13 + 7) & 0xFF;
        vi2 = (vi2 * 17 + 11) & 0xFF;
        vi3 = (vi3 * 19 + 13) & 0xFF;
        vi4 = (vi4 * 23 + 17) & 0xFF;
    }
    
    /* Final computation to prevent optimization */
    for (int i = 0; i < 1000; i++) {
        checksum += large_arr1[i] + large_arr2[i] + large_arr3[i];
    }
    
    printf("Checksum: %d\n", checksum);
    return 0;
}
