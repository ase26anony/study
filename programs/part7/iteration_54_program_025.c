/* reload_coverage.c - Comprehensive test for GCC reload pass coverage */
#include <stdio.h>
#include <stdint.h>

/* Volatile indices to prevent constant folding */
volatile int vi1 = 7, vi2 = 13, vi3 = 42;

/* Structure for passing by value */
struct SmallStruct {
    int a, b, c, d;
};

/* Large local arrays to increase register pressure */
#define ARRAY_SIZE 256
int global_arr[ARRAY_SIZE][ARRAY_SIZE];

/* Vector type for GCC vector extensions */
typedef int v4si __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));

/* Function prototypes */
struct SmallStruct process_struct(struct SmallStruct s1, struct SmallStruct s2);
int complex_addressing(int i, int j, int k);
void inline_asm_chain(int *a, int *b, int *c);
void vector_operations(v4si *v1, v4si *v2);

/* Pattern 1: Complex addressing modes with multi-dimensional arrays */
int complex_addressing(int i, int j, int k) {
    volatile int volatile_index = vi1;
    int result = 0;
    
    /* Force address reloads with complex indexing */
    global_arr[volatile_index][i+1] = global_arr[j][volatile_index*2];
    
    /* Chain of array accesses with non-constant indices */
    result += global_arr[vi2 + i][j * 3];
    result -= global_arr[k][volatile_index % 8];
    
    /* Pointer arithmetic that can't be folded */
    int *ptr1 = &global_arr[0][0] + volatile_index * ARRAY_SIZE + i;
    int *ptr2 = &global_arr[0][0] + j * ARRAY_SIZE + k;
    
    *ptr1 = *ptr2 + result;
    
    return result;
}

/* Pattern 2: Inline assembly with multiple operands and constraints */
void inline_asm_chain(int *a, int *b, int *c) {
    int tmp1, tmp2, tmp3;
    
    /* First asm: output to register, input from memory */
    asm volatile (
        "movl %[input], %[output]\n\t"
        "addl $1, %[output]"
        : [output] "=r" (tmp1)
        : [input] "m" (*a)
        : "cc"
    );
    
    /* Second asm: output to memory, input from previous output */
    asm volatile (
        "movl %[in1], %%eax\n\t"
        "imull %[in2], %%eax\n\t"
        "movl %%eax, %[out]"
        : [out] "=m" (*b)
        : [in1] "r" (tmp1), [in2] "r" (vi2)
        : "eax", "cc"
    );
    
    /* Third asm: multiple outputs, clobber many registers */
    asm volatile (
        "movl %[in], %%ebx\n\t"
        "leal (%%ebx, %%ebx, 2), %%ecx\n\t"
        "movl %%ecx, %[out1]\n\t"
        "movl %%ebx, %[out2]"
        : [out1] "=r" (tmp2), [out2] "=r" (tmp3)
        : [in] "r" (*c)
        : "ebx", "ecx", "cc"
    );
    
    /* Use results to prevent elimination */
    *c = tmp2 + tmp3;
}

/* Pattern 3: Structure passing by value */
struct SmallStruct process_struct(struct SmallStruct s1, struct SmallStruct s2) {
    struct SmallStruct result;
    
    /* Complex operations on structure members */
    result.a = s1.a + s2.b * vi1;
    result.b = s1.b - s2.a / (vi2 ? vi2 : 1);
    result.c = s1.c * s2.d + vi3;
    result.d = s1.d % (s2.c ? s2.c : 1);
    
    /* Address taken to force spills */
    int *ptr = &result.a;
    *ptr += complex_addressing(result.b, result.c, result.d);
    
    return result;
}

/* Pattern 4: Vector operations with GCC extensions */
void vector_operations(v4si *v1, v4si *v2) {
    v4si a = *v1, b = *v2;
    
    /* Vector operations that may need decomposition */
    v4si c = a + b * (v4si){1, 2, 3, 4};
    
    /* Shuffle operation */
    v4si d = __builtin_shuffle(a, b, (v4si){3, 2, 1, 0});
    
    /* Conditional blend-like operation */
    v4si mask = a > b;
    v4si result = (a & mask) | (b & ~mask);
    
    /* Store with complex addressing */
    volatile v4si *volatile_ptr = (volatile v4si *)v1;
    *volatile_ptr = result + c - d;
}

/* Pattern 5: Control flow that splits live ranges */
int control_flow_split(int x) {
    int a, b, c, d;
    
    /* goto creates complex control flow */
    if (x < 0) goto label1;
    
    a = x * 2;
    b = a + vi1;
    
    if (x > 100) {
        c = b * 3;
        goto label2;
    }
    
label1:
    c = x / 2;
    b = vi2;
    
label2:
    d = complex_addressing(a, b, c);
    
    /* Loop with varying register pressure */
    for (int i = 0; i < vi3 % 16; i++) {
        /* Force spills by using many temporaries */
        int t1 = a + i;
        int t2 = b * i;
        int t3 = c - i;
        int t4 = d % (i + 1);
        
        a = t1 ^ t2;
        b = t3 | t4;
        c = t1 + t2 + t3 + t4;
    }
    
    return a + b + c + d;
}

/* Main function orchestrating all patterns */
int main() {
    int checksum = 0;
    
    /* Initialize array */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        for (int j = 0; j < ARRAY_SIZE; j++) {
            global_arr[i][j] = i * 1000 + j;
        }
    }
    
    /* Pattern 1: Complex addressing */
    checksum += complex_addressing(vi1, vi2, vi3);
    
    /* Pattern 2: Inline assembly chain */
    int asm_a = 100, asm_b = 200, asm_c = 300;
    inline_asm_chain(&asm_a, &asm_b, &asm_c);
    checksum += asm_a + asm_b + asm_c;
    
    /* Pattern 3: Structure passing */
    struct SmallStruct s1 = {10, 20, 30, 40};
    struct SmallStruct s2 = {50, 60, 70, 80};
    
    for (int i = 0; i < 10; i++) {
        s1 = process_struct(s1, s2);
        checksum += s1.a + s1.b + s1.c + s1.d;
    }
    
    /* Pattern 4: Vector operations */
    v4si vec1 = {1, 2, 3, 4};
    v4si vec2 = {5, 6, 7, 8};
    vector_operations(&vec1, &vec2);
    
    /* Extract vector elements for checksum */
    int vec_elems[4];
    __builtin_memcpy(vec_elems, &vec1, sizeof(vec1));
    for (int i = 0; i < 4; i++) checksum += vec_elems[i];
    
    /* Pattern 5: Control flow with split live ranges */
    checksum += control_flow_split(vi1 * 2);
    
    /* Final checksum output */
    printf("Checksum: %d\n", checksum);
    
    return 0;
}
