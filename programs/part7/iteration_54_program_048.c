/* reload_coverage.c - Stress GCC reload pass for coverage of reload1.cc lines 7146-7174 */

#include <stdio.h>
#include <stdint.h>
#include <string.h>

/* Pattern 1: Complex addressing modes with volatile indices */
volatile int idx1 = 3, idx2 = 7, idx3 = 11;
int arr1[100][50];
int arr2[75][60];

/* Pattern 2: Structure passing by value */
struct SmallStruct {
    int a;
    int b;
    int c;
    int d;
};

struct MediumStruct {
    int data[8];
    char padding[12];
};

/* Pattern 3: Vector extensions */
typedef int v4si __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));

/* Pattern 4: Function prototypes */
struct SmallStruct process_struct(struct SmallStruct s, int multiplier);
struct MediumStruct process_medium(struct MediumStruct m, int offset);
int complex_addressing(int i, int j);
void inline_asm_chain(int *a, int *b, int *c);

/* Pattern 1 Implementation: Complex addressing */
int complex_addressing(int i, int j) {
    /* Force multiple address reload types */
    arr1[idx1 + i][j * 2] = arr2[idx2 * 3][idx3 + j];
    arr2[i + idx1][j] = arr1[j][i] + arr1[idx2][idx3];
    
    /* More complex addressing with pointer arithmetic */
    int *ptr1 = &arr1[idx1][idx2];
    int *ptr2 = &arr2[idx3][idx1];
    
    /* This should trigger various address reloads */
    for (int k = 0; k < 10; k++) {
        ptr1[k * idx1] = ptr2[k * idx2] + ptr1[(k + 1) * idx3];
    }
    
    return arr1[idx1][idx2] + arr2[idx3][idx1];
}

/* Pattern 2 Implementation: Structure passing */
struct SmallStruct process_struct(struct SmallStruct s, int multiplier) {
    struct SmallStruct result;
    result.a = s.a * multiplier + s.b;
    result.b = s.b * multiplier + s.c;
    result.c = s.c * multiplier + s.d;
    result.d = s.d * multiplier + s.a;
    
    /* Chain structure operations */
    struct SmallStruct temp = result;
    temp.a += complex_addressing(temp.a % 10, temp.b % 10);
    
    return temp;
}

struct MediumStruct process_medium(struct MediumStruct m, int offset) {
    struct MediumStruct result;
    
    /* Complex structure access pattern */
    for (int i = 0; i < 8; i++) {
        result.data[i] = m.data[(i + offset) % 8] + 
                        m.data[(i + offset + 1) % 8] * 2;
    }
    
    /* Mix with volatile access */
    result.data[0] += idx1;
    result.data[1] += idx2;
    
    return result;
}

/* Pattern 3: Vector operations */
v4si vector_operations(v4si a, v4si b) {
    v4si result;
    
    /* Shuffle operations that may need reloads */
    result = __builtin_shuffle(a, b, (v4si){0, 2, 1, 3});
    result += __builtin_shuffle(b, a, (v4si){3, 1, 2, 0});
    
    /* Vector with scalar mixing */
    for (int i = 0; i < 4; i++) {
        result[i] += complex_addressing(result[i] % 20, i);
    }
    
    return result;
}

/* Pattern 4: Inline assembly with multiple constraints */
void inline_asm_chain(int *a, int *b, int *c) {
    int temp1, temp2, temp3;
    
    /* Chain of asm blocks creating dependencies */
    asm volatile (
        "movl %1, %0\n\t"
        "addl $5, %0"
        : "=r" (temp1)
        : "m" (*a)
        : "cc"
    );
    
    asm volatile (
        "imull %2, %1\n\t"
        "movl %1, %0"
        : "=rm" (temp2), "=r" (temp3)
        : "r" (temp1), "1" (temp1)
        : "cc"
    );
    
    asm volatile (
        "leal (%1, %2, 4), %0"
        : "=r" (*c)
        : "r" (temp2), "r" (temp3)
        : "cc"
    );
    
    /* More complex asm with memory constraints */
    asm volatile (
        "movl %1, %%eax\n\t"
        "addl %%eax, %0\n\t"
        "movl %0, %2"
        : "+m" (*b), "+r" (temp1)
        : "m" (*a)
        : "eax", "cc"
    );
}

/* Pattern 5: Control flow splitting live ranges */
int control_flow_live_ranges(int n) {
    int x, y, z;
    
    /* Force register pressure with gotos */
    if (n < 0) goto label1;
    
    x = complex_addressing(n, n * 2);
    y = x * 3 + idx1;
    
    if (y > 100) goto label2;
    
    z = y + idx2;
    goto label3;
    
label1:
    x = idx3 * 5;
    y = complex_addressing(x % 20, x % 15);
    z = y - idx1;
    goto label4;
    
label2:
    z = complex_addressing(y % 25, y % 30);
    x = z * 2;
    goto label4;
    
label3:
    x = complex_addressing(z % 35, z % 40);
    
label4:
    /* Use all variables to keep them live */
    return x + y + z + complex_addressing(x % 10, y % 10);
}

/* Main function orchestrating all patterns */
int main() {
    int checksum = 0;
    
    /* Initialize arrays */
    for (int i = 0; i < 100; i++) {
        for (int j = 0; j < 50; j++) {
            arr1[i][j] = i * 100 + j;
        }
    }
    
    for (int i = 0; i < 75; i++) {
        for (int j = 0; j < 60; j++) {
            arr2[i][j] = i * 60 + j * 3;
        }
    }
    
    /* Pattern 1: Complex addressing */
    checksum += complex_addressing(idx1, idx2);
    checksum += complex_addressing(idx2, idx3);
    checksum += complex_addressing(idx3, idx1);
    
    /* Pattern 2: Structure passing */
    struct SmallStruct s1 = {1, 2, 3, 4};
    struct SmallStruct s2 = process_struct(s1, idx1);
    struct SmallStruct s3 = process_struct(s2, idx2);
    
    checksum += s3.a + s3.b + s3.c + s3.d;
    
    struct MediumStruct m1;
    for (int i = 0; i < 8; i++) {
        m1.data[i] = i * 10 + idx1;
    }
    
    struct MediumStruct m2 = process_medium(m1, idx2);
    for (int i = 0; i < 8; i++) {
        checksum += m2.data[i];
    }
    
    /* Pattern 3: Vector operations */
    v4si vec1 = {1, 2, 3, 4};
    v4si vec2 = {5, 6, 7, 8};
    v4si vec3 = vector_operations(vec1, vec2);
    
    for (int i = 0; i < 4; i++) {
        checksum += vec3[i];
    }
    
    /* Pattern 4: Inline assembly chain */
    int asm_a = 10, asm_b = 20, asm_c = 0;
    inline_asm_chain(&asm_a, &asm_b, &asm_c);
    checksum += asm_a + asm_b + asm_c;
    
    /* Pattern 5: Control flow with live ranges */
    for (int i = -5; i < 10; i++) {
        checksum += control_flow_live_ranges(i);
    }
    
    /* Final complex addressing with all patterns mixed */
    int final_result = 0;
    for (int i = 0; i < 5; i++) {
        struct SmallStruct temp_s = {i, i*2, i*3, i*4};
        temp_s = process_struct(temp_s, idx1 + i);
        
        int addr_result = complex_addressing(temp_s.a % 50, temp_s.b % 50);
        
        int flow_result = control_flow_live_ranges(addr_result % 20);
        
        final_result += temp_s.c + temp_s.d + addr_result + flow_result;
    }
    
    checksum += final_result;
    
    /* Use volatile to prevent optimization */
    volatile int output = checksum;
    printf("Result: %d\n", output);
    
    return 0;
}
