/* Test program to trigger various reload types in GCC's reload1.cc */
#include <stdio.h>
#include <stdint.h>

/* Volatile indices to prevent constant folding */
volatile int vi1 = 1, vi2 = 2, vi3 = 3, vi4 = 4;

/* Structure for passing by value */
struct SmallStruct {
    int a, b, c, d;
};

/* Vector type to stress register allocation */
typedef int v4si __attribute__((vector_size(16)));

/* Function to pass/return structures by value */
struct SmallStruct process_struct(struct SmallStruct s1, struct SmallStruct s2) {
    struct SmallStruct result;
    result.a = s1.a + s2.b;
    result.b = s1.b + s2.c;
    result.c = s1.c + s2.d;
    result.d = s1.d + s2.a;
    return result;
}

/* Another function to create call chain */
struct SmallStruct process_struct_chain(struct SmallStruct s) {
    struct SmallStruct temp = {vi1, vi2, vi3, vi4};
    return process_struct(s, temp);
}

/* Function with complex addressing */
void complex_addressing(int size) {
    /* Large arrays to increase register pressure */
    int arr1[100][100];
    int arr2[100][100];
    
    /* Initialize with volatile to prevent optimization */
    for (int i = 0; i < 100; i++) {
        for (int j = 0; j < 100; j++) {
            arr1[i][j] = i * 100 + j;
            arr2[i][j] = 0;
        }
    }
    
    /* Complex addressing with volatile indices */
    /* This should trigger RELOAD_FOR_INPUT_ADDRESS, RELOAD_FOR_OUTPUT_ADDRESS */
    for (int i = 0; i < 10; i++) {
        for (int j = 0; j < 10; j++) {
            /* Non-constant indices with arithmetic */
            int idx1 = (vi1 * i + j) % 100;
            int idx2 = (vi2 * j + i) % 100;
            int idx3 = (vi3 * i + vi4 * j) % 100;
            int idx4 = (vi4 * j + vi1 * i) % 100;
            
            /* Complex array access pattern */
            arr2[idx1][idx2] = arr1[idx3][idx4] + 
                              arr1[idx4][idx3] * vi1 -
                              arr1[idx2][idx1] / (vi2 + 1);
        }
    }
    
    /* Use the result to prevent dead code elimination */
    volatile int sum = 0;
    for (int i = 0; i < 10; i++) {
        sum += arr2[i][0];
    }
}

/* Function with inline assembly to trigger various reload types */
void inline_asm_reloads(void) {
    int a = vi1, b = vi2, c = vi3, d = vi4;
    int result1, result2, result3;
    
    /* Chain of inline asm statements with dependencies */
    /* Should trigger RELOAD_FOR_INPUT, RELOAD_FOR_OUTPUT_ADDRESS, RELOAD_OTHER */
    
    /* First asm: compute something */
    asm volatile (
        "addl %2, %1\n\t"
        "movl %1, %0"
        : "=r" (result1), "+r" (a)
        : "r" (b)
        : "cc"
    );
    
    /* Second asm: use previous result with memory operand */
    asm volatile (
        "imull %2, %1\n\t"
        "leal (%1,%3,4), %0"
        : "=r" (result2), "+r" (result1)
        : "r" (c), "r" (d)
        : "cc"
    );
    
    /* Third asm: complex constraints */
    asm volatile (
        "movl %1, %%eax\n\t"
        "addl %2, %%eax\n\t"
        "movl %%eax, %0"
        : "=rm" (result3)
        : "rm" (result2), "rm" (vi1)
        : "%eax", "cc"
    );
    
    /* Use results */
    volatile int unused = result1 + result2 + result3;
}

/* Function with vector operations */
void vector_operations(void) {
    v4si v1 = {vi1, vi2, vi3, vi4};
    v4si v2 = {vi4, vi3, vi2, vi1};
    v4si v3, v4;
    
    /* Vector operations that might need decomposition */
    v3 = v1 + v2;
    v4 = v1 * v2;
    
    /* Shuffle operation */
    v4si shuffle_mask = {3, 2, 1, 0};
    v4si v5 = __builtin_shuffle(v3, v4, shuffle_mask);
    
    /* Use vector to prevent elimination */
    volatile int sum = v5[0] + v5[1] + v5[2] + v5[3];
}

/* Function with goto to split live ranges */
void control_flow_reloads(int n) {
    int a = vi1, b = vi2, c = vi3;
    int result = 0;
    
    /* Complex control flow */
    if (n > 0) {
        a = n * 2;
        goto label1;
    } else {
        b = n * 3;
        goto label2;
    }
    
label1:
    {
        /* Different scope to force spills */
        int temp1 = a + b;
        int temp2 = c * 2;
        result = temp1 + temp2;
        goto end;
    }
    
label2:
    {
        int temp3 = b - a;
        int temp4 = c / 2;
        result = temp3 * temp4;
        /* Fall through */
    }
    
end:
    /* Use all variables across basic blocks */
    volatile int unused = a + b + c + result;
}

/* Main function orchestrating all patterns */
int main(void) {
    int checksum = 0;
    
    /* 1. Complex addressing modes */
    complex_addressing(100);
    checksum += vi1;
    
    /* 2. Structure passing by value */
    struct SmallStruct s1 = {vi1, vi2, vi3, vi4};
    struct SmallStruct s2 = {vi4, vi3, vi2, vi1};
    
    for (int i = 0; i < 10; i++) {
        struct SmallStruct result = process_struct_chain(s1);
        checksum += result.a + result.b + result.c + result.d;
        
        /* Chain calls */
        s1 = process_struct(result, s2);
        s2 = process_struct(s1, result);
    }
    
    /* 3. Inline assembly reloads */
    inline_asm_reloads();
    checksum += vi2;
    
    /* 4. Vector operations */
    vector_operations();
    checksum += vi3;
    
    /* 5. Control flow with split live ranges */
    for (int i = -5; i < 5; i++) {
        control_flow_reloads(i);
        checksum += i;
    }
    
    /* Final checksum output */
    printf("Checksum: %d\n", checksum);
    
    return 0;
}
