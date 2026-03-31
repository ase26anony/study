/* reload_coverage.c - Stress GCC's reload pass for coverage testing */
#include <stdio.h>
#include <stdint.h>

/* Volatile indices to prevent constant folding */
volatile int vi1 = 1, vi2 = 2, vi3 = 3, vi4 = 4;

/* 1. Complex Addressing Mode Stress */
void complex_addressing(int n) {
    /* Large multi-dimensional arrays */
    int arr1[100][100];
    int arr2[100][100];
    
    /* Initialize with volatile indices */
    for (int i = 0; i < 100; i++) {
        for (int j = 0; j < 100; j++) {
            arr1[i][j] = i * 100 + j;
            arr2[i][j] = 0;
        }
    }
    
    /* Complex addressing with volatile indices */
    for (int i = 0; i < n; i++) {
        /* This should trigger RELOAD_FOR_INPUT_ADDRESS and RELOAD_FOR_OUTPUT_ADDRESS */
        arr2[vi1 + i][vi2 * i] = arr1[vi3 * i][vi4 + i * 2];
        
        /* Nested array access with pointer arithmetic */
        int (*ptr1)[100] = &arr1[vi1];
        int (*ptr2)[100] = &arr2[vi2];
        ptr2[i][vi3] = ptr1[vi4][i] + ptr1[i][vi1];
    }
}

/* 2. Structure passing for address reloads */
struct SmallStruct {
    int a, b, c, d;
};

struct SmallStruct process_struct(struct SmallStruct s1, struct SmallStruct s2) {
    /* Mix structure members with volatile */
    struct SmallStruct result;
    result.a = s1.a + s2.a + vi1;
    result.b = s1.b * s2.b - vi2;
    result.c = s1.c ^ s2.c ^ vi3;
    result.d = s1.d | s2.d | vi4;
    return result;
}

struct SmallStruct chain_struct_calls(struct SmallStruct s) {
    struct SmallStruct temp1 = {vi1, vi2, vi3, vi4};
    struct SmallStruct temp2 = process_struct(s, temp1);
    struct SmallStruct temp3 = process_struct(temp2, s);
    return process_struct(temp3, temp2);
}

/* 3. Vector extensions for complex register allocation */
typedef int v4si __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));

void vector_operations(void) {
    v4si vec1 = {vi1, vi2, vi3, vi4};
    v4si vec2 = {vi4, vi3, vi2, vi1};
    v4si vec3, vec4;
    
    /* Complex vector operations */
    vec3 = vec1 + vec2 * vec1;
    vec4 = __builtin_shuffle(vec1, vec2, (v4si){3, 2, 1, 0});
    
    /* Mix with scalar operations */
    int arr[8];
    for (int i = 0; i < 4; i++) {
        arr[i] = vec3[i];
        arr[i + 4] = vec4[i];
    }
}

/* 4. Inline assembly with multiple constraints */
void inline_asm_chains(void) {
    int a = vi1, b = vi2, c = vi3, d = vi4;
    int result1, result2, result3;
    
    /* Chain of asm blocks creating dependencies */
    asm volatile (
        "addl %2, %1\n\t"
        "movl %1, %0"
        : "=r" (result1), "+r" (a)
        : "r" (b)
        : "cc"
    );
    
    /* Memory constraint forcing address reloads */
    asm volatile (
        "imull %1, %0\n\t"
        "addl %%ebx, %0"
        : "+r" (result1)
        : "m" (c), "b" (d)
        : "cc"
    );
    
    /* Multiple output constraints */
    asm volatile (
        "leal (%1, %2, 2), %0\n\t"
        "subl %3, %0"
        : "=&r" (result2), "=r" (result3)
        : "r" (a), "r" (b), "1" (c)
        : "cc"
    );
}

/* 5. Control flow with split live ranges */
int control_flow_split(int x) {
    int a = vi1, b = vi2, c = vi3;
    volatile int trigger = 0;
    
    /* goto to split basic blocks */
    if (x > 0) {
        a = x * vi1;
        goto label1;
    } else {
        b = x * vi2;
        goto label2;
    }
    
label1:
    /* Use values defined in different blocks */
    c = a + b + vi3;
    if (trigger) {
        goto label3;
    }
    
label2:
    /* More complex flow */
    a = b * c - vi4;
    if (x % 2) {
        goto label1;
    }
    
label3:
    /* Final computation using all variables */
    return a + b * 2 + c * 3;
}

/* 6. Mixed operations in main */
int main(void) {
    int checksum = 0;
    
    /* Initialize volatile indices */
    vi1 = 1; vi2 = 2; vi3 = 3; vi4 = 4;
    
    /* 1. Complex addressing */
    complex_addressing(50);
    
    /* 2. Structure passing chain */
    struct SmallStruct s = {10, 20, 30, 40};
    for (int i = 0; i < 100; i++) {
        s = chain_struct_calls(s);
        checksum += s.a + s.b - s.c + s.d;
    }
    
    /* 3. Vector operations */
    vector_operations();
    
    /* 4. Inline assembly chains */
    for (int i = 0; i < 100; i++) {
        inline_asm_chains();
        checksum += i * vi1;
    }
    
    /* 5. Control flow with various inputs */
    for (int i = -50; i < 50; i++) {
        checksum += control_flow_split(i);
    }
    
    /* Large array with volatile access */
    volatile int big_array[1000];
    for (int i = 0; i < 1000; i++) {
        big_array[i] = i * vi1;
        if (i % 3 == 0) {
            big_array[i] += vi2;
        }
        checksum += big_array[i];
    }
    
    /* Final mixed operation preventing optimization */
    checksum = (checksum & 0xFFFF) + vi1 * vi2 - vi3 / vi4;
    
    printf("Checksum: %d\n", checksum);
    return checksum != 0 ? 0 : 1;
}
