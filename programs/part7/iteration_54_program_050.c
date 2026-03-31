/* Test program to trigger various reload types in GCC's reload1.cc */
#include <stdio.h>
#include <stdint.h>

/* Volatile indices to prevent constant folding */
volatile int vi1 = 1, vi2 = 2, vi3 = 3, vi4 = 4;

/* Structure for passing by value */
struct SmallStruct {
    int a, b, c, d;
};

/* Vector type for GCC extensions */
typedef int v4si __attribute__((vector_size(16)));

/* Function to pass/return structures by value */
struct SmallStruct process_struct(struct SmallStruct s1, struct SmallStruct s2) {
    struct SmallStruct result;
    result.a = s1.a + s2.b;
    result.b = s1.b - s2.c;
    result.c = s1.c * s2.d;
    result.d = s1.d / (s2.a ? s2.a : 1);
    return result;
}

/* Another function to create call chain */
struct SmallStruct chain_struct(struct SmallStruct s) {
    struct SmallStruct local = {vi1, vi2, vi3, vi4};
    return process_struct(s, local);
}

/* Complex addressing mode stress */
void complex_addressing(int size) {
    /* Large arrays to increase register pressure */
    int arr1[100][100];
    int arr2[100][100];
    int arr3[100][100];
    
    /* Initialize with volatile indices */
    for (int i = 0; i < 10; i++) {
        for (int j = 0; j < 10; j++) {
            arr1[i][j] = i * 100 + j;
            arr2[i][j] = (i + vi1) * (j + vi2);
        }
    }
    
    /* Complex array accesses with non-constant indices */
    /* This should trigger RELOAD_FOR_INPUT_ADDRESS, RELOAD_FOR_OUTPUT_ADDRESS */
    for (int iter = 0; iter < 5; iter++) {
        for (int i = 1; i < 9; i++) {
            for (int j = 1; j < 9; j++) {
                /* Multi-dimensional with arithmetic in indices */
                arr3[i + vi1][j + vi2] = 
                    arr1[vi1 * i][vi2 * j] + 
                    arr2[i + vi3][j + vi4] +
                    arr1[j][i] * arr2[vi4][vi3];
                
                /* More complex addressing */
                arr1[(i * vi1 + j * vi2) % 10][(i * vi3 - j * vi4) % 10] =
                    arr3[(j * vi2) % 10][(i * vi1) % 10] +
                    arr2[(i + j) % 10][(i - j + 10) % 10];
            }
        }
    }
    
    /* Use the results to prevent optimization */
    volatile int sum = 0;
    for (int i = 0; i < 10; i++) {
        for (int j = 0; j < 10; j++) {
            sum += arr3[i][j];
        }
    }
    (void)sum;
}

/* Inline assembly with multiple operands */
void inline_asm_stress(void) {
    int a = vi1, b = vi2, c = vi3, d = vi4;
    int result1, result2, result3;
    
    /* Chain of asm blocks creating dependencies */
    /* Should trigger RELOAD_FOR_INPUT, RELOAD_FOR_OUTPUT_ADDRESS, RELOAD_OTHER */
    
    /* First asm: compute something */
    asm volatile (
        "addl %2, %1\n\t"
        "movl %1, %0"
        : "=r" (result1), "+r" (a)
        : "r" (b)
        : "cc"
    );
    
    /* Second asm: use previous result */
    asm volatile (
        "imull %2, %1\n\t"
        "leal (%1,%3,2), %0"
        : "=r" (result2), "+r" (c)
        : "r" (result1), "r" (d)
        : "cc"
    );
    
    /* Third asm: memory operand */
    asm volatile (
        "movl %1, %%eax\n\t"
        "addl %%eax, %0"
        : "+m" (result3)
        : "r" (result2)
        : "%eax", "cc"
    );
    
    /* Fourth asm: multiple outputs */
    asm volatile (
        "movl %2, %%eax\n\t"
        "movl %%eax, %0\n\t"
        "addl $1, %%eax\n\t"
        "movl %%eax, %1"
        : "=r" (a), "=r" (b)
        : "r" (result3)
        : "%eax", "cc"
    );
    
    /* Use results */
    volatile int sum = a + b + c + d + result1 + result2 + result3;
    (void)sum;
}

/* Vector operations */
void vector_operations(void) {
    v4si v1 = {vi1, vi2, vi3, vi4};
    v4si v2 = {vi4, vi3, vi2, vi1};
    v4si v3, v4, v5;
    
    /* Vector operations that might need decomposition */
    v3 = v1 + v2;
    v4 = v1 * v2;
    v5 = v3 - v4;
    
    /* Shuffle operation */
    v4si shuffle_mask = {3, 2, 1, 0};
    v4si v6 = __builtin_shuffle(v5, shuffle_mask);
    
    /* Mix with scalar operations */
    int arr[10];
    for (int i = 0; i < 10; i++) {
        arr[i] = i + vi1;
    }
    
    /* Access vector elements individually */
    int sum = 0;
    for (int i = 0; i < 4; i++) {
        sum += v6[i] + arr[i];
    }
    
    volatile int vol_sum = sum;
    (void)vol_sum;
}

/* Complex control flow to split live ranges */
void control_flow_stress(int iterations) {
    int a = vi1, b = vi2, c = vi3, d = vi4;
    int result = 0;
    
    /* Use goto to create complex CFG */
    int i = 0;
    
start_loop:
    if (i >= iterations) goto end;
    
    /* Different paths based on complex condition */
    if ((a * b + c * d) % 3 == 0) {
        a = a + b;
        goto path1;
    } else if ((a + b + c + d) % 5 == 0) {
        b = b - c;
        goto path2;
    } else {
        c = c * d;
        goto path3;
    }

path1:
    result += a * 2;
    i++;
    goto start_loop;

path2:
    result += b * 3;
    i++;
    goto start_loop;

path3:
    result += c / 2;
    i++;
    goto start_loop;

end:
    /* Use all variables to keep them live across blocks */
    volatile int vol_result = result + a + b + c + d;
    (void)vol_result;
}

/* Main function orchestrating all patterns */
int main(void) {
    int checksum = 0;
    
    /* 1. Structure passing chain */
    struct SmallStruct s1 = {vi1, vi2, vi3, vi4};
    struct SmallStruct s2 = {vi4, vi3, vi2, vi1};
    
    for (int i = 0; i < 3; i++) {
        struct SmallStruct result = chain_struct(s1);
        checksum += result.a + result.b + result.c + result.d;
        s1 = result;  /* Use as input for next iteration */
    }
    
    /* 2. Complex addressing modes */
    complex_addressing(10);
    
    /* 3. Inline assembly stress */
    for (int i = 0; i < 2; i++) {
        inline_asm_stress();
    }
    
    /* 4. Vector operations */
    vector_operations();
    
    /* 5. Control flow stress */
    control_flow_stress(10);
    
    /* Final checksum */
    checksum += vi1 + vi2 + vi3 + vi4;
    
    printf("Checksum: %d\n", checksum);
    return 0;
}
