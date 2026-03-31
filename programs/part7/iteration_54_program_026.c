/* Test program to exercise GCC reload pass for various reload types */
#include <stdio.h>
#include <stdint.h>

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

/* Function to create structure passing reloads */
struct SmallStruct process_struct(struct SmallStruct s1, struct SmallStruct s2) {
    struct SmallStruct result;
    result.a = s1.a + s2.b;
    result.b = s1.b - s2.c;
    result.c = s1.c * s2.d;
    result.d = s1.d / (s2.a ? s2.a : 1);
    return result;
}

/* Another function for chaining structure passes */
struct SmallStruct chain_struct(struct SmallStruct s) {
    struct SmallStruct temp;
    temp.a = s.b + vi1;
    temp.b = s.c - vi2;
    temp.c = s.d * vi3;
    temp.d = s.a / (vi4 ? vi4 : 1);
    
    /* Force spill with large array */
    int large_array[100];
    for (int i = 0; i < 100; i++) {
        large_array[i] = i * s.a;
    }
    
    /* Use array to prevent optimization */
    temp.a += large_array[vi1];
    return temp;
}

/* Function with complex addressing modes */
void complex_addressing(int arr[][20][10], int n) {
    /* RELOAD_FOR_INPUT_ADDRESS, RELOAD_FOR_OUTPUT_ADDRESS */
    arr[vi1][n+1][vi2*2] = arr[vi3][n-1][vi4/2];
    
    /* More complex addressing with pointer arithmetic */
    int *ptr1 = &arr[vi1][n][0];
    int *ptr2 = &arr[vi2][n+2][0];
    
    /* Force address reloads with volatile */
    for (int i = 0; i < 5; i++) {
        ptr1[i + vi1] = ptr2[i * vi2];
    }
}

/* Function with inline assembly for various reload types */
void inline_asm_reloads(int *a, int *b, int *c) {
    int tmp1, tmp2, tmp3;
    
    /* RELOAD_FOR_INPUT with register constraints */
    asm volatile (
        "movl %1, %0\n\t"
        "addl %2, %0"
        : "=r" (tmp1)
        : "r" (*a), "r" (vi1)
        : "cc"
    );
    
    /* RELOAD_FOR_OUTPUT_ADDRESS with memory constraint */
    asm volatile (
        "movl %1, %%eax\n\t"
        "addl %%eax, %0"
        : "=m" (*b)
        : "r" (tmp1)
        : "%eax", "cc"
    );
    
    /* Chain asm blocks for dependency */
    asm volatile (
        "imull %1, %0"
        : "+r" (tmp1)
        : "r" (vi2)
        : "cc"
    );
    
    /* RELOAD_OTHER type with multiple clobbers */
    asm volatile (
        "leal (%1, %2, 4), %0\n\t"
        "addl %%ecx, %0"
        : "=r" (tmp2)
        : "r" (tmp1), "r" (vi3)
        : "%ecx", "cc"
    );
    
    *c = tmp2;
}

/* Function with control flow to split live ranges */
int control_flow_reloads(int x) {
    int a, b, c, d;
    
    /* Initialize in different blocks */
    if (x > 0) {
        a = x + vi1;
        b = x * vi2;
        goto label1;
    } else {
        a = x - vi3;
        b = x / (vi4 ? vi4 : 1);
        goto label2;
    }
    
label1:
    c = a * b + vi1;
    /* Force spill with many variables live */
    d = c + a + b + vi2 + vi3 + vi4;
    goto label3;
    
label2:
    c = a - b + vi3;
    d = c * a * b * vi1 * vi2;
    
label3:
    /* Complex expression with many operands */
    return a + b + c + d + 
           (a * b) + (c * d) + 
           (a > b ? a : b) + 
           (c < d ? c : d) +
           vi1 + vi2 + vi3 + vi4;
}

/* Function using vector extensions */
void vector_operations(v4si *v1, v4si *v2, v4si *result) {
    /* Vector operations that may need decomposition */
    v4si temp1 = *v1 + *v2;
    v4si temp2 = *v1 - *v2;
    
    /* Shuffle for complex pattern */
    v4si shuffle_mask = {3, 2, 1, 0};
    v4si shuffled = __builtin_shuffle(temp1, temp2, shuffle_mask);
    
    /* Mix with scalar operations */
    int *scalar_ptr = (int*)&shuffled;
    for (int i = 0; i < 4; i++) {
        scalar_ptr[i] += vi1 + i;
    }
    
    *result = shuffled;
}

int main() {
    int checksum = 0;
    
    /* Initialize multi-dimensional array */
    int arr[15][20][10];
    for (int i = 0; i < 15; i++) {
        for (int j = 0; j < 20; j++) {
            for (int k = 0; k < 10; k++) {
                arr[i][j][k] = i * 100 + j * 10 + k;
            }
        }
    }
    
    /* Test complex addressing modes */
    for (int i = 0; i < 5; i++) {
        complex_addressing(arr, i);
    }
    
    /* Test structure passing */
    struct SmallStruct s1 = {vi1, vi2, vi3, vi4};
    struct SmallStruct s2 = {vi2, vi3, vi4, vi1};
    
    for (int i = 0; i < 10; i++) {
        s1 = process_struct(s1, s2);
        s2 = chain_struct(s1);
        
        /* Use results to prevent optimization */
        checksum += s1.a + s2.b;
    }
    
    /* Test inline assembly reloads */
    int asm_a = 100, asm_b = 200, asm_c = 300;
    for (int i = 0; i < 8; i++) {
        inline_asm_reloads(&asm_a, &asm_b, &asm_c);
        checksum += asm_a + asm_b + asm_c;
        
        /* Modify to create different patterns */
        asm_a += vi1;
        asm_b -= vi2;
        asm_c *= vi3;
    }
    
    /* Test control flow reloads */
    for (int i = -5; i < 5; i++) {
        checksum += control_flow_reloads(i);
    }
    
    /* Test vector operations */
    v4si vec1 = {1, 2, 3, 4};
    v4si vec2 = {5, 6, 7, 8};
    v4si vec_result;
    
    for (int i = 0; i < 4; i++) {
        vector_operations(&vec1, &vec2, &vec_result);
        
        /* Use vector results */
        int *vptr = (int*)&vec_result;
        for (int j = 0; j < 4; j++) {
            checksum += vptr[j];
        }
        
        /* Modify vectors */
        vec1[0] += vi1;
        vec2[3] -= vi2;
    }
    
    /* Final array checksum */
    for (int i = 0; i < 15; i += 2) {
        for (int j = 0; j < 20; j += 3) {
            for (int k = 0; k < 10; k += 4) {
                checksum += arr[i][j][k];
            }
        }
    }
    
    printf("Checksum: %d\n", checksum);
    return 0;
}
