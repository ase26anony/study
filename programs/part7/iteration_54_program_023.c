/* Test program to exercise GCC reload pass switch cases */
#include <stdio.h>
#include <stdint.h>

/* Volatile indices to prevent constant folding */
volatile int vi1 = 3, vi2 = 7, vi3 = 11;

/* Large structure to force spills */
struct LargeStruct {
    int data[256];
    char padding[64];
    volatile int flags[8];
};

/* Vector type for GCC extensions */
typedef int v4si __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));

/* Function passing/returning structures by value */
struct SmallStruct {
    int a, b, c, d;
};

/* Pattern 1: Complex addressing modes with multi-dimensional arrays */
void complex_addressing(int n) {
    int arr[100][100];
    volatile int idx1 = vi1, idx2 = vi2;
    
    /* Force RELOAD_FOR_INPUT_ADDRESS and RELOAD_FOR_OUTPUT_ADDRESS */
    for (int i = 0; i < n; i++) {
        /* Complex address calculations that can't be folded */
        arr[idx1 + i][idx2 * 2] = arr[idx2][idx1 * i + 3];
        arr[i][idx1] = arr[idx2 * i][idx1 + idx2];
    }
    
    /* Use results to prevent dead code elimination */
    vi1 = arr[0][0] + arr[1][1];
}

/* Pattern 2: Inline assembly with multiple constraints */
void inline_asm_pattern(void) {
    int a = 1234, b = 5678, c = 0;
    volatile int mem_var = 9999;
    
    /* Chain of asm blocks creating dependencies */
    asm volatile (
        "mov %1, %0\n\t"
        "add $1, %0"
        : "=r"(a)
        : "r"(b)
        : "cc"
    );
    
    /* Memory constraint forcing address reloads */
    asm volatile (
        "addl $100, (%1)\n\t"
        "mov (%1), %0"
        : "=r"(c)
        : "m"(mem_var)
        : "memory"
    );
    
    /* Multiple output constraints */
    asm volatile (
        "imul %2, %0\n\t"
        "add %0, %1"
        : "=r"(a), "=r"(b)
        : "r"(c), "0"(a), "1"(b)
        : "cc"
    );
}

/* Pattern 3: Structure passing by value */
struct SmallStruct process_struct(struct SmallStruct s1, struct SmallStruct s2) {
    /* Operations forcing register pressure */
    s1.a = s1.b + s2.c;
    s1.b = s1.c * s2.d;
    s1.c = s1.d - s2.a;
    s1.d = s1.a ^ s2.b;
    return s1;
}

struct SmallStruct struct_chain(struct SmallStruct s, int n) {
    struct SmallStruct temp = s;
    for (int i = 0; i < n; i++) {
        temp = process_struct(temp, s);
        /* Force address reloads for temporary locations */
        temp.a += ((volatile int*)&s)[i % 4];
    }
    return temp;
}

/* Pattern 4: Vector operations with GCC extensions */
void vector_operations(void) {
    v4si vec1 = {1, 2, 3, 4};
    v4si vec2 = {5, 6, 7, 8};
    volatile v4si vec3;
    
    /* Complex vector operations */
    vec1 = vec1 + vec2 * 3;
    vec2 = __builtin_shuffle(vec1, vec2, (v4si){3, 2, 1, 0});
    
    /* Force memory operations */
    vec3 = vec1;
    vec1 = vec1 + vec3;
    
    /* Use result */
    vi2 = vec1[0] + vec2[1];
}

/* Pattern 5: Control flow splitting live ranges */
int control_flow_live_ranges(int x) {
    int a, b, c, d;
    
    /* Define values in different blocks */
    if (x > 0) {
        a = vi1 * 2;
        b = vi2 + 3;
        goto label1;
    } else {
        a = vi3 - 1;
        b = vi1 * vi2;
        goto label2;
    }
    
label1:
    c = a * b;
    /* Complex addressing in this path */
    {
        volatile int arr[100];
        arr[a % 100] = c;
        d = arr[b % 100];
    }
    goto label3;
    
label2:
    c = a + b;
    /* Different complex addressing */
    {
        volatile int arr[100];
        arr[c % 100] = a;
        d = arr[b % 100] + c;
    }
    
label3:
    /* Use all values across split ranges */
    return a + b + c + d;
}

/* Pattern 6: Mixed addressing modes with pointers */
void pointer_arithmetic(void) {
    struct LargeStruct ls;
    volatile int* volatile ptrs[10];
    int* restrict rptr = ls.data;
    
    /* Initialize */
    for (int i = 0; i < 256; i++) {
        ls.data[i] = i;
    }
    
    /* Complex pointer chains */
    ptrs[0] = &ls.data[vi1];
    ptrs[1] = &ls.data[vi2];
    ptrs[2] = &ls.flags[0];
    
    /* Force various address reload types */
    for (int i = 0; i < 100; i++) {
        *ptrs[i % 3] = *(ptrs[(i + 1) % 3]) + i;
        rptr[i] = rptr[i * 2 % 256] + *ptrs[i % 3];
    }
    
    /* Use result */
    vi3 = ls.data[0] + ls.data[255];
}

/* Main function orchestrating all patterns */
int main(void) {
    int checksum = 0;
    
    /* Execute all patterns multiple times */
    for (int iter = 0; iter < 3; iter++) {
        /* Pattern 1: Complex addressing */
        complex_addressing(50);
        checksum += vi1;
        
        /* Pattern 2: Inline assembly */
        inline_asm_pattern();
        checksum += vi2;
        
        /* Pattern 3: Structure passing */
        struct SmallStruct s1 = {1, 2, 3, 4};
        struct SmallStruct s2 = {5, 6, 7, 8};
        struct SmallStruct result = struct_chain(s1, 10);
        checksum += result.a + result.b + result.c + result.d;
        
        /* Pattern 4: Vector operations */
        vector_operations();
        checksum += vi2;
        
        /* Pattern 5: Control flow */
        checksum += control_flow_live_ranges(iter);
        
        /* Pattern 6: Pointer arithmetic */
        pointer_arithmetic();
        checksum += vi3;
    }
    
    printf("Checksum: %d\n", checksum);
    return 0;
}
