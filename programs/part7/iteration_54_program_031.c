/* reload_coverage.c - Comprehensive test for GCC reload pass coverage */
#include <stdio.h>
#include <stdint.h>

/* Volatile indices to prevent constant folding */
static volatile int vi1 = 1, vi2 = 2, vi3 = 3, vi4 = 4;

/* Structure for passing by value */
struct SmallStruct {
    int a;
    int b;
    int c;
    int d;
};

/* Vector type for SIMD operations */
typedef int v4si __attribute__((vector_size(16)));

/* Large arrays to increase register pressure */
int large_array1[1000];
int large_array2[1000];
int large_array3[1000];

/* Multi-dimensional array for complex addressing */
int md_array[10][20][30];

/* Function that returns structure by value - forces complex parameter passing */
struct SmallStruct __attribute__((noinline)) 
process_struct(struct SmallStruct s1, struct SmallStruct s2) {
    struct SmallStruct result;
    result.a = s1.a + s2.b + vi1;
    result.b = s1.b * s2.c + vi2;
    result.c = s1.c - s2.d + vi3;
    result.d = s1.d / (s2.a ? s2.a : 1) + vi4;
    return result;
}

/* Function with complex control flow */
int __attribute__((noinline)) 
complex_control_flow(int *arr, int n) {
    int sum = 0;
    int i = 0;
    
    /* Use goto to create complex CFG */
    loop_start:
    if (i >= n) goto loop_end;
    
    /* Volatile access to prevent optimization */
    int idx = vi1 + i;
    
    /* Complex addressing with multiple computations */
    arr[idx * 2] = arr[idx * 3] + arr[(idx + vi2) * 4];
    
    /* Another level of indirection */
    int *ptr = &arr[idx * 5];
    *ptr = *ptr + vi3;
    
    i += vi1;
    goto loop_start;
    
    loop_end:
    
    /* Compute checksum */
    for (int j = 0; j < n; j++) {
        sum += arr[j];
    }
    return sum;
}

/* Function using inline assembly with multiple constraints */
void __attribute__((noinline))
asm_reload_test(int *a, int *b, int *c) {
    int tmp1, tmp2, tmp3;
    
    /* First asm: output constraint */
    asm volatile (
        "movl %1, %0\n\t"
        "addl $1, %0"
        : "=r" (tmp1)
        : "m" (*a)
        : "cc"
    );
    
    /* Second asm: input from previous output */
    asm volatile (
        "imull %1, %0\n\t"
        "addl %2, %0"
        : "+r" (tmp1)
        : "r" (tmp1), "m" (*b)
        : "cc"
    );
    
    /* Third asm: multiple outputs */
    asm volatile (
        "movl %2, %0\n\t"
        "movl %2, %1\n\t"
        "addl $5, %0\n\t"
        "subl $3, %1"
        : "=r" (tmp2), "=r" (tmp3)
        : "m" (*c)
        : "cc"
    );
    
    /* Chain dependencies */
    *a = tmp1 + tmp2 + tmp3;
}

/* Function with vector operations */
void __attribute__((noinline))
vector_reload_test(void) {
    v4si v1 = {1, 2, 3, 4};
    v4si v2 = {5, 6, 7, 8};
    v4si v3, v4;
    
    /* Vector operations that may need decomposition */
    v3 = v1 + v2;
    v4 = v1 * v2;
    
    /* Use builtin shuffle for complex pattern */
    v4si v5 = __builtin_shuffle(v3, v4, (v4si){0, 4, 1, 5});
    
    /* Store to memory with complex addressing */
    int *p = &large_array3[vi1 * 10];
    *(v4si *)p = v5;
}

/* Main function orchestrating all patterns */
int main(void) {
    int checksum = 0;
    
    /* Initialize arrays */
    for (int i = 0; i < 1000; i++) {
        large_array1[i] = i;
        large_array2[i] = i * 2;
        large_array3[i] = i * 3;
    }
    
    /* Initialize multi-dimensional array */
    for (int i = 0; i < 10; i++) {
        for (int j = 0; j < 20; j++) {
            for (int k = 0; k < 30; k++) {
                md_array[i][j][k] = i * 1000 + j * 100 + k;
            }
        }
    }
    
    /* Pattern 1: Complex addressing modes with multi-dimensional arrays */
    for (int i = 0; i < 5; i++) {
        for (int j = 0; j < 5; j++) {
            /* This forces RELOAD_FOR_INPUT_ADDRESS and RELOAD_FOR_OUTPUT_ADDRESS */
            md_array[vi1 + i][j + vi2][vi3] = 
                md_array[j][vi1 * 2][i + vi4] + 
                md_array[i + 1][j * 3][vi2];
                
            /* More complex addressing with pointer arithmetic */
            int *ptr1 = &md_array[i][j][0];
            int *ptr2 = &md_array[j][i][0];
            ptr1[vi1 * 3] = ptr2[vi2 * 4] + ptr1[vi3 * 2];
        }
    }
    
    /* Pattern 2: Structure passing chain */
    struct SmallStruct s1 = {vi1, vi2, vi3, vi4};
    struct SmallStruct s2 = {vi2, vi3, vi4, vi1};
    
    for (int i = 0; i < 10; i++) {
        /* Chain of structure operations - forces address reloads for temporaries */
        struct SmallStruct s3 = process_struct(s1, s2);
        struct SmallStruct s4 = process_struct(s2, s3);
        struct SmallStruct s5 = process_struct(s3, s4);
        
        checksum += s5.a + s5.b + s5.c + s5.d;
        
        /* Update for next iteration */
        s1 = s3;
        s2 = s4;
    }
    
    /* Pattern 3: Inline assembly with multiple constraints */
    for (int i = 0; i < 100; i += 10) {
        asm_reload_test(&large_array1[i], &large_array2[i + vi1], &large_array3[i + vi2]);
    }
    
    /* Pattern 4: Complex control flow with split live ranges */
    checksum += complex_control_flow(large_array1, 100);
    
    /* Pattern 5: Vector operations */
    vector_reload_test();
    
    /* Pattern 6: Mixed operations in loop with high register pressure */
    int temp1, temp2, temp3, temp4;
    for (int i = 0; i < 50; i++) {
        /* Force many values live across loop iterations */
        temp1 = large_array1[i * 2] + vi1;
        temp2 = large_array2[i * 3] * vi2;
        temp3 = large_array3[i * 4] - vi3;
        temp4 = temp1 + temp2 + temp3;
        
        /* Complex addressing with multiple computations */
        int idx = (i + vi1) * (vi2 + 1);
        large_array1[idx % 1000] = temp4;
        
        /* Pointer chain */
        int *p1 = &large_array2[(idx + vi3) % 1000];
        int *p2 = &large_array3[(idx * 2) % 1000];
        *p1 = *p2 + temp4;
        
        checksum += temp4;
    }
    
    /* Final checksum computation */
    for (int i = 0; i < 100; i++) {
        checksum += large_array1[i] + large_array2[i] + large_array3[i];
    }
    
    /* Access multi-dimensional array with volatile indices */
    checksum += md_array[vi1][vi2][vi3];
    checksum += md_array[vi2][vi3][vi4];
    checksum += md_array[vi3][vi4][vi1];
    
    printf("Checksum: %d\n", checksum);
    return 0;
}
