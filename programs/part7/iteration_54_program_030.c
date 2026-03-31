/* reload_coverage.c - Comprehensive test to trigger various reload types in GCC */
#include <stdio.h>
#include <stdint.h>
#include <string.h>

/* Volatile indices to prevent constant folding */
volatile int vi1 = 7, vi2 = 3, vi3 = 11, vi4 = 5;

/* Structure for passing by value */
struct SmallStruct {
    int a;
    int b;
    int c;
    int d;
};

/* Vector type to stress register allocation */
typedef int v4si __attribute__((vector_size(16)));

/* Large arrays to increase register pressure */
int large_array1[1000];
int large_array2[1000][50];
int large_array3[200][50][10];

/* Function that returns structure by value - forces complex parameter passing */
struct SmallStruct process_struct(struct SmallStruct s1, struct SmallStruct s2) {
    struct SmallStruct result;
    result.a = s1.a + s2.b;
    result.b = s1.b - s2.a;
    result.c = s1.c * s2.c;
    result.d = s1.d ^ s2.d;
    return result;
}

/* Function with complex addressing modes */
void complex_addressing(int n) {
    /* Multi-dimensional array access with volatile indices */
    for (int i = 0; i < 10; i++) {
        /* This should trigger RELOAD_FOR_INPUT_ADDRESS and RELOAD_FOR_OUTPUT_ADDRESS */
        large_array2[vi1 + i][vi2 * 2] = large_array2[vi3 % 20][(vi4 + i) * 3];
        
        /* More complex addressing with pointer arithmetic */
        int *ptr1 = &large_array2[i][0];
        int *ptr2 = &large_array2[vi2][i];
        ptr1[vi1] = ptr2[vi3] + large_array1[vi4 + i * 2];
    }
}

/* Function using inline assembly with multiple constraints */
void inline_asm_stress(void) {
    int a = vi1, b = vi2, c = vi3, d = vi4;
    int result1, result2, result3;
    
    /* Chain of inline assembly blocks creating dependencies */
    asm volatile (
        "movl %1, %0\n\t"
        "addl %2, %0"
        : "=r" (result1)
        : "r" (a), "r" (b)
        : "cc"
    );
    
    /* Memory constraint to force address reloads */
    asm volatile (
        "imull %1, %0\n\t"
        "addl %%ecx, %0"
        : "+r" (result1)
        : "m" (large_array1[vi1]), "c" (c)
        : "cc"
    );
    
    /* Multiple outputs with different constraints */
    asm volatile (
        "leal (%1,%2,4), %0\n\t"
        "movl %3, %%eax"
        : "=r" (result2), "=a" (result3)
        : "r" (result1), "m" (large_array2[0][vi2]), "1" (d)
        : "cc"
    );
}

/* Function with vector operations */
void vector_operations(void) {
    v4si v1 = {1, 2, 3, 4};
    v4si v2 = {vi1, vi2, vi3, vi4};
    v4si v3, v4;
    
    /* Vector operations that may need decomposition */
    v3 = v1 + v2;
    v4 = v1 * v2;
    
    /* Shuffle operation */
    v4si v5 = __builtin_shuffle(v3, v4, (v4si){3, 2, 1, 0});
    
    /* Store to memory with complex addressing */
    int *p = (int*)&large_array3[vi1][vi2][0];
    *(v4si*)p = v5;
}

/* Function with control flow that splits live ranges */
int control_flow_split(int x) {
    int a = vi1 * x;
    int b = vi2 + x;
    int c = vi3 - x;
    
    /* goto to create complex control flow */
    if (a > 100) {
        goto label1;
    }
    
    for (int i = 0; i < 50; i++) {
        if (i % 7 == 0) {
            a += large_array1[i];
        } else {
            b += large_array2[i][vi4];
        }
        
        if (i == 25) {
            c = a * b;
            goto label2;
        }
    }
    
    return a + b + c;

label1:
    /* Different basic block using same variables */
    a = b * c;
    b = a + vi1;
    c = b - vi2;
    
    /* Complex addressing in this block */
    large_array2[a % 50][b % 10] = c;
    
    return a - b + c;

label2:
    /* Another basic block with register pressure */
    int d = a + b + c;
    int e = d * vi3;
    int f = e / (vi4 + 1);
    
    /* More complex addressing */
    large_array3[a % 20][b % 5][c % 2] = d + e + f;
    
    return d;
}

/* Main function orchestrating all patterns */
int main(void) {
    int checksum = 0;
    
    /* Initialize arrays */
    for (int i = 0; i < 1000; i++) {
        large_array1[i] = i * 3;
    }
    
    for (int i = 0; i < 1000; i++) {
        for (int j = 0; j < 50; j++) {
            large_array2[i][j] = i + j * 2;
        }
    }
    
    /* Test 1: Complex addressing modes */
    complex_addressing(20);
    
    /* Test 2: Structure passing */
    struct SmallStruct s1 = {vi1, vi2, vi3, vi4};
    struct SmallStruct s2 = {vi2, vi3, vi4, vi1};
    struct SmallStruct s3 = process_struct(s1, s2);
    checksum += s3.a + s3.b + s3.c + s3.d;
    
    /* Test 3: Inline assembly stress */
    inline_asm_stress();
    
    /* Test 4: Vector operations */
    vector_operations();
    
    /* Test 5: Control flow with split live ranges */
    checksum += control_flow_split(vi1);
    
    /* Test 6: Mixed operations in loop to increase pressure */
    for (int i = 0; i < 100; i++) {
        /* Volatile access to prevent optimization */
        volatile int *volatile_ptr = &large_array1[vi1 + i];
        
        /* Complex expression with multiple memory accesses */
        int val = *volatile_ptr 
                + large_array2[i % 100][(vi2 + i) % 50] 
                - large_array3[i % 20][i % 5][i % 2];
        
        /* Inline asm with memory operand */
        asm volatile (
            "addl %1, %0\n\t"
            "subl %%ecx, %0"
            : "+r" (val)
            : "m" (large_array2[vi3][i % 50]), "c" (vi4)
            : "cc"
        );
        
        checksum += val;
    }
    
    /* Final checksum computation */
    for (int i = 0; i < 100; i++) {
        checksum += large_array1[i] + large_array2[i][0];
    }
    
    printf("Checksum: %d\n", checksum);
    return 0;
}
