/* reload_coverage.c - Stress GCC's reload pass for coverage testing */
#include <stdio.h>
#include <stdint.h>

/* Volatile indices to prevent constant folding */
volatile int vi1 = 7, vi2 = 3, vi3 = 11, vi4 = 5;

/* 1. Complex Addressing Mode Stress */
void complex_addressing(int n) {
    /* Large multi-dimensional array */
    int arr[100][50];
    static int counter = 0;
    
    /* Initialize with pattern */
    for (int i = 0; i < 100; i++)
        for (int j = 0; j < 50; j++)
            arr[i][j] = i * 100 + j + counter;
    
    /* Complex addressing with volatile indices */
    for (int i = 0; i < n; i++) {
        /* This forces address reloads */
        arr[vi1 + i][vi2 * i] = arr[vi3 - i][vi4 * (i + 1)];
        arr[i][vi1] = arr[vi2][i] + arr[vi3 % 20][vi4];
    }
    
    counter++;
}

/* 2. Structure passing for address reloads */
struct SmallStruct {
    int a, b, c, d;
};

struct SmallStruct process_struct(struct SmallStruct s1, struct SmallStruct s2) {
    /* Force spilling of structure components */
    struct SmallStruct result;
    result.a = s1.a + s2.d + vi1;
    result.b = s1.b * s2.c - vi2;
    result.c = s1.c / (s2.b ? s2.b : 1) + vi3;
    result.d = s1.d - s2.a * vi4;
    return result;
}

struct SmallStruct struct_chain(struct SmallStruct s) {
    struct SmallStruct temp1, temp2;
    
    temp1.a = s.b + vi1;
    temp1.b = s.c - vi2;
    temp1.c = s.d * vi3;
    temp1.d = s.a / (vi4 ? vi4 : 1);
    
    temp2 = process_struct(s, temp1);
    
    /* Force address calculations for structure members */
    temp2.a += ((int*)&temp1)[vi1 % 4];
    temp2.b += ((int*)&temp2)[vi2 % 4];
    
    return temp2;
}

/* 3. Inline assembly with multiple constraints */
void inline_asm_stress(int *ptr1, int *ptr2) {
    int a, b, c, d;
    
    /* Chain of asm blocks creating dependencies */
    asm volatile (
        "movl %[input], %[out1]\n\t"
        "addl $1, %[out1]"
        : [out1] "=r" (a)
        : [input] "m" (*ptr1)
        : "cc"
    );
    
    asm volatile (
        "movl %[in1], %%eax\n\t"
        "imull %[in2], %%eax\n\t"
        "movl %%eax, %[out1]"
        : [out1] "=r" (b)
        : [in1] "r" (a), [in2] "m" (*ptr2)
        : "eax", "cc"
    );
    
    asm volatile (
        "leal (%[in1], %[in2], 4), %[out1]"
        : [out1] "=r" (c)
        : [in1] "r" (b), [in2] "r" (vi1)
        : 
    );
    
    asm volatile (
        "movl %[in1], %[out1]\n\t"
        "shrl $2, %[out1]"
        : [out1] "=r" (d)
        : [in1] "m" (c)
        : "cc"
    );
    
    *ptr1 = d;
}

/* 4. Vector extensions for complex patterns */
typedef int v4si __attribute__((vector_size(16)));
typedef long long v2di __attribute__((vector_size(16)));

void vector_operations(v4si *result) {
    v4si a = {vi1, vi2, vi3, vi4};
    v4si b = {vi4, vi3, vi2, vi1};
    v4si c, d;
    
    /* Complex vector operations */
    c = a + b * 2;
    d = __builtin_shuffle(a, b, (v4si){1, 3, 0, 2});
    
    /* Force register pressure */
    v4si e = c * d;
    v4si f = e + a;
    v4si g = f - b;
    
    /* Conditional operations */
    v4si mask = a > b;
    *result = (g & mask) | (e & ~mask);
}

/* 5. Control flow with split live ranges */
int control_flow_split(int n) {
    int x = vi1, y = vi2, z = vi3;
    int result = 0;
    
    /* goto creates complex control flow */
    if (n > 0) goto loop_start;
    
    /* Dead code to create basic block boundaries */
    x = y * z;
    y = x + vi4;
    
loop_start:
    for (int i = 0; i < n; i++) {
        /* Values defined here, used in distant blocks */
        int temp = x + i * y;
        
        if (i % 2 == 0) {
            goto even_block;
        } else {
            goto odd_block;
        }
        
    even_block:
        result += temp * vi1;
        continue;
        
    odd_block:
        result -= temp * vi2;
        continue;
    }
    
    /* Use all variables at the end to keep them live */
    result += x * y * z;
    return result;
}

/* 6. Mixed operations in main */
int main() {
    int checksum = 0;
    int data1[100], data2[100];
    v4si vec_result;
    
    /* Initialize data arrays */
    for (int i = 0; i < 100; i++) {
        data1[i] = i * 3 + vi1;
        data2[i] = i * 7 - vi2;
    }
    
    /* Stress complex addressing */
    complex_addressing(50);
    
    /* Structure passing chain */
    struct SmallStruct s1 = {vi1, vi2, vi3, vi4};
    struct SmallStruct s2 = {vi4, vi3, vi2, vi1};
    
    for (int i = 0; i < 10; i++) {
        s1 = struct_chain(s1);
        s2 = process_struct(s2, s1);
        checksum += s1.a + s2.b;
    }
    
    /* Inline assembly stress in loop */
    for (int i = 0; i < 20; i++) {
        inline_asm_stress(&data1[i % 50], &data2[i % 50]);
        checksum += data1[i % 50];
    }
    
    /* Vector operations */
    vector_operations(&vec_result);
    checksum += vec_result[0] + vec_result[2];
    
    /* Control flow with split ranges */
    checksum += control_flow_split(25);
    
    /* Final array processing */
    for (int i = 0; i < 50; i++) {
        /* More complex addressing */
        data1[vi1 + i % 10] = data2[vi2 * (i % 5)] + checksum;
        checksum ^= data1[i];
    }
    
    printf("Checksum: %d\n", checksum);
    return 0;
}
