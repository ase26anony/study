/* reload_coverage.c - Stress GCC's reload pass for various reload types */
#include <stdio.h>
#include <stdint.h>

/* Volatile indices to prevent constant folding */
volatile int vi1 = 7, vi2 = 13, vi3 = 42;

/* Structure for passing by value */
struct SmallStruct {
    int a;
    int b;
    int c;
    int d;
};

/* Vector type to stress register allocation */
typedef int v4si __attribute__((vector_size(16)));

/* Global arrays for complex addressing */
int global_arr[100][50];
int global_arr2[100][50];

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
struct SmallStruct chain_struct(struct SmallStruct s, int iterations) {
    for (int i = 0; i < iterations; i++) {
        struct SmallStruct tmp = {i, i+1, i+2, i+3};
        s = process_struct(s, tmp);
    }
    return s;
}

/* Function with complex addressing modes */
void complex_addressing(int n) {
    /* Large local arrays to increase register pressure */
    int local_arr[100][50];
    int local_arr2[100][50];
    
    /* Initialize with volatile indices */
    for (int i = 0; i < 100; i++) {
        for (int j = 0; j < 50; j++) {
            local_arr[i][j] = i * j;
            local_arr2[i][j] = i + j;
        }
    }
    
    /* Complex array accesses with non-constant indices */
    /* This stresses RELOAD_FOR_INPUT_ADDRESS, RELOAD_FOR_OUTPUT_ADDRESS */
    for (int iter = 0; iter < n; iter++) {
        /* Multi-dimensional with arithmetic in indices */
        local_arr[vi1 + iter][vi2 * 2] = 
            local_arr2[vi3 % 50][(vi1 + vi2) & 31] +
            local_arr[iter][(vi2 + iter) % 50];
        
        /* Pointer arithmetic that can't be folded */
        int *ptr1 = &local_arr[vi1][vi2];
        int *ptr2 = &local_arr2[vi3 % 20][vi1 % 30];
        ptr1[iter * 2] = ptr2[iter * 3] + ptr1[iter];
        
        /* Chain of array accesses */
        global_arr[iter][vi1] = 
            local_arr[vi2][iter] + 
            global_arr2[iter * 2 % 100][vi3 % 50];
    }
}

/* Function with inline assembly to trigger various reloads */
void asm_reloads(void) {
    int a = vi1, b = vi2, c = vi3;
    int d, e, f;
    
    /* Chain of asm blocks creating dependencies */
    /* RELOAD_FOR_INPUT, RELOAD_FOR_OUTPUT_ADDRESS */
    asm volatile (
        "movl %1, %0\n\t"
        "addl %2, %0"
        : "=r" (d)
        : "r" (a), "r" (b)
        : "cc"
    );
    
    /* Memory operand reloads */
    asm volatile (
        "imull %1, %0\n\t"
        "addl %%ecx, %0"
        : "+r" (d)
        : "m" (c)
        : "ecx", "cc"
    );
    
    /* Multiple output operands */
    asm volatile (
        "leal (%1, %2), %0\n\t"
        "movl %2, %3"
        : "=r" (e), "=r" (f)
        : "r" (d), "r" (a)
        : "cc"
    );
    
    /* Use results to prevent optimization */
    vi1 = d + e + f;
}

/* Function with control flow to split live ranges */
int control_flow_reloads(int x) {
    int a = x * 2;
    int b, c, d;
    
    /* goto to create complex CFG */
    if (a > 100) {
        goto label1;
    }
    
    b = a + vi1;
    if (b < 50) {
        c = b * vi2;
        goto label2;
    }
    
label1:
    c = a - vi3;
    /* Force spill/reload across basic blocks */
    d = c + global_arr[vi1][vi2];
    
label2:
    /* Use all variables in distant block */
    for (int i = 0; i < 10; i++) {
        /* Complex addressing in loop */
        d += global_arr2[i][a % 50] * c;
        if (d > 1000) {
            b += d / (i + 1);
            goto label1;  /* Back edge */
        }
    }
    
    return a + b + c + d;
}

/* Function using vector extensions */
void vector_reloads(void) {
    v4si v1 = {vi1, vi2, vi3, vi1 + vi2};
    v4si v2 = {vi3, vi1, vi2, vi3 - vi1};
    v4si v3, v4;
    
    /* Vector operations */
    v3 = v1 + v2;
    v4 = v1 * v2;
    
    /* Shuffle to create complex pattern */
    v4 = __builtin_shuffle(v3, v4, (v4si){3, 2, 1, 0});
    
    /* Mix with scalar operations */
    int *p = (int*)&v4;
    for (int i = 0; i < 4; i++) {
        p[i] += global_arr[i][vi1 % 50];
    }
    
    /* Store back to global */
    *(v4si*)&global_arr[0][0] = v4;
}

int main(void) {
    int checksum = 0;
    
    /* Initialize global arrays */
    for (int i = 0; i < 100; i++) {
        for (int j = 0; j < 50; j++) {
            global_arr[i][j] = i - j;
            global_arr2[i][j] = i * j;
        }
    }
    
    /* 1. Structure passing chain */
    struct SmallStruct s1 = {vi1, vi2, vi3, vi1 + vi2};
    struct SmallStruct s2 = {vi2, vi3, vi1, vi2 + vi3};
    
    for (int i = 0; i < 5; i++) {
        s1 = chain_struct(s1, 3);
        checksum += s1.a + s1.b + s1.c + s1.d;
    }
    
    /* 2. Complex addressing modes */
    complex_addressing(20);
    
    /* 3. Inline assembly reloads */
    asm_reloads();
    
    /* 4. Control flow with split live ranges */
    checksum += control_flow_reloads(vi1);
    
    /* 5. Vector operations */
    vector_reloads();
    
    /* Final checksum from modified arrays */
    for (int i = 0; i < 10; i++) {
        for (int j = 0; j < 10; j++) {
            checksum += global_arr[i][j];
            checksum += global_arr2[i][j];
        }
    }
    
    printf("Checksum: %d\n", checksum);
    return 0;
}
