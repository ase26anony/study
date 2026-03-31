/* reload_coverage.c - Program to exercise GCC's reload pass */
#include <stdio.h>
#include <stdint.h>

/* Volatile indices to prevent constant folding */
static volatile int vi1 = 1, vi2 = 2, vi3 = 3, vi4 = 4;

/* Structure for passing by value */
struct SmallStruct {
    int a, b, c, d;
};

/* Large array to increase register pressure */
static int large_array[1000];

/* Vector type to force special reloads */
typedef int v4si __attribute__((vector_size(16)));

/* Function 1: Pass and return structure by value */
struct SmallStruct func1(struct SmallStruct s) {
    s.a += vi1;
    s.b += vi2;
    return s;
}

/* Function 2: Another structure manipulator */
struct SmallStruct func2(struct SmallStruct s) {
    s.c *= vi3;
    s.d /= (vi4 ? vi4 : 1);
    return s;
}

/* Function 3: Complex addressing with multi-dimensional array */
void complex_addressing(int n) {
    /* Force non-trivial addressing modes */
    int arr[10][10];
    int i, j;
    
    /* Initialize with volatile to prevent optimization */
    for (i = 0; i < 10; i++) {
        for (j = 0; j < 10; j++) {
            arr[i][j] = i * 10 + j + vi1;
        }
    }
    
    /* Complex array accesses with volatile indices */
    /* This should trigger RELOAD_FOR_INPUT_ADDRESS, RELOAD_FOR_OUTPUT_ADDRESS */
    for (i = 0; i < n; i++) {
        int idx1 = (vi1 * i) % 10;
        int idx2 = (vi2 * i + 1) % 10;
        int idx3 = (vi3 * i * 2) % 10;
        int idx4 = (vi4 * i + 3) % 10;
        
        /* Multiple array operations with complex addressing */
        arr[idx1][idx2] = arr[idx3][idx4] + large_array[i];
        arr[idx2][idx3] = arr[idx4][idx1] * vi1;
        
        /* Pointer arithmetic that can't be easily folded */
        int *ptr1 = &arr[idx1][idx2];
        int *ptr2 = &arr[idx3][idx4];
        *ptr1 = *ptr2 + (ptr2 - ptr1);
    }
}

/* Function using inline assembly with multiple constraints */
void inline_asm_chain(int *a, int *b, int *c) {
    int tmp1, tmp2, tmp3;
    
    /* First asm: output to register, input from memory */
    /* Should trigger RELOAD_FOR_OUTPUT_ADDRESS and RELOAD_FOR_INPUT */
    asm volatile (
        "movl %[input], %[output]\n\t"
        : [output] "=r" (tmp1)
        : [input] "m" (*a)
        : "cc"
    );
    
    /* Second asm: use previous output as input, produce new output */
    /* Chain of dependencies requiring various reloads */
    asm volatile (
        "addl $1, %[out]\n\t"
        "imull %[in], %[out]\n\t"
        : [out] "+r" (tmp1)
        : [in] "r" (tmp1)
        : "cc"
    );
    
    /* Third asm: multiple constraints */
    /* Should trigger RELOAD_FOR_OPERAND_ADDRESS */
    asm volatile (
        "movl %%eax, %0\n\t"
        "movl %1, %%eax\n\t"
        : "=m" (tmp2)
        : "r" (tmp1), "a" (tmp3)
        : "cc"
    );
    
    /* Store result through pointer with complex addressing */
    *b = tmp1 + tmp2;
}

/* Function with vector operations */
void vector_ops(void) {
    v4si v1 = {vi1, vi2, vi3, vi4};
    v4si v2 = {vi4, vi3, vi2, vi1};
    v4si v3, v4;
    
    /* Vector operations that might need decomposition */
    v3 = v1 + v2;
    v4 = v1 * v2;
    
    /* Use __builtin_shuffle for complex pattern */
    /* This can trigger RELOAD_FOR_OTHER and RELOAD_FOR_OTHER_ADDRESS */
    v4si v5 = __builtin_shuffle(v3, v4, (v4si){0, 2, 1, 3});
    
    /* Store to memory with potential misalignment */
    int *p = (int*)&v5;
    for (int i = 0; i < 4; i++) {
        large_array[100 + i] = p[i];
    }
}

/* Function with goto to split live ranges */
void control_flow_split(int x) {
    int a = vi1, b = vi2, c = vi3, d = vi4;
    int result = 0;
    
    /* Complex control flow to increase register pressure */
    if (x > 0) {
        a = x * 2;
        goto label1;
    } else {
        b = x * 3;
        goto label2;
    }
    
label1:
    c = a + b;
    /* Force spill by using many temporaries */
    {
        int t1 = a * 2;
        int t2 = b * 3;
        int t3 = c * 4;
        int t4 = d * 5;
        int t5 = t1 + t2;
        int t6 = t3 + t4;
        result = t5 * t6;
        goto label3;
    }
    
label2:
    d = b - a;
    /* Another block with many temporaries */
    {
        int t1 = a / 2;
        int t2 = b / 3;
        int t3 = c / 4;
        int t4 = d / 5;
        int t5 = t1 - t2;
        int t6 = t3 - t4;
        result = t5 * t6;
    }
    
label3:
    /* Use all variables to keep them live across blocks */
    large_array[x % 100] = a + b + c + d + result;
}

/* Main function orchestrating all patterns */
int main(void) {
    int checksum = 0;
    struct SmallStruct s = {10, 20, 30, 40};
    
    /* Initialize large array */
    for (int i = 0; i < 1000; i++) {
        large_array[i] = i + vi1;
    }
    
    /* 1. Structure passing chain */
    s = func1(s);
    s = func2(s);
    checksum += s.a + s.b + s.c + s.d;
    
    /* 2. Complex addressing patterns */
    complex_addressing(50);
    
    /* 3. Inline assembly chains */
    int asm_a = 100, asm_b = 0, asm_c = 0;
    for (int i = 0; i < 10; i++) {
        inline_asm_chain(&asm_a, &asm_b, &asm_c);
        asm_a = asm_b + i;
        checksum += asm_b;
    }
    
    /* 4. Vector operations */
    vector_ops();
    
    /* 5. Control flow with split live ranges */
    for (int i = -5; i < 5; i++) {
        control_flow_split(i);
    }
    
    /* Final checksum from modified arrays */
    for (int i = 0; i < 100; i++) {
        checksum += large_array[i];
    }
    
    /* Also check some specific elements from vector ops */
    for (int i = 100; i < 104; i++) {
        checksum += large_array[i];
    }
    
    printf("Checksum: %d\n", checksum);
    return 0;
}
