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

/* Function to create structure passing reloads */
struct SmallStruct process_struct(struct SmallStruct s1, struct SmallStruct s2) {
    struct SmallStruct result;
    result.a = s1.a + s2.b;
    result.b = s1.b + s2.c;
    result.c = s1.c + s2.d;
    result.d = s1.d + s2.a;
    return result;
}

/* Another function for nested structure passing */
struct SmallStruct chain_struct(struct SmallStruct s, int iterations) {
    for (int i = 0; i < iterations; i++) {
        s = process_struct(s, s);
    }
    return s;
}

/* Function with complex addressing modes */
void complex_addressing(int size) {
    /* Large arrays to increase register pressure */
    int arr1[100][100];
    int arr2[100][100];
    
    /* Initialize with volatile indices */
    for (int i = 0; i < 10; i++) {
        for (int j = 0; j < 10; j++) {
            arr1[i][j] = i * 100 + j;
            arr2[i][j] = j * 100 + i;
        }
    }
    
    /* Complex addressing with volatile indices - triggers address reloads */
    for (int i = 0; i < 5; i++) {
        /* RELOAD_FOR_INPUT_ADDRESS, RELOAD_FOR_OUTPUT_ADDRESS */
        arr1[vi1 + i][vi2 * 2] = arr2[vi3][vi4 + i * 3];
        
        /* More complex addressing */
        arr1[i + vi2][arr2[vi1][i] % 10] = arr2[arr1[i][vi3] % 10][vi4];
    }
}

/* Function with inline assembly chains */
void asm_reload_chains(void) {
    int a = 1000, b = 2000, c = 3000, d = 4000;
    int result1, result2, result3;
    
    /* Chain of inline assembly with register constraints */
    /* RELOAD_FOR_INPUT, RELOAD_FOR_OUTPUT_ADDRESS */
    asm volatile (
        "addl %2, %1\n\t"
        "movl %1, %0"
        : "=r" (result1), "+r" (a)
        : "r" (b)
        : "cc"
    );
    
    /* Another asm with memory constraints */
    /* RELOAD_FOR_OPERAND_ADDRESS, RELOAD_FOR_OPADDR_ADDR */
    asm volatile (
        "imull %1, %0\n\t"
        "addl $42, %0"
        : "+r" (result1)
        : "m" (c)
        : "cc"
    );
    
    /* Third asm using previous results */
    /* RELOAD_FOR_OTHER, RELOAD_FOR_OTHER_ADDRESS */
    asm volatile (
        "leal (%1, %2, 2), %0"
        : "=r" (result2)
        : "r" (result1), "r" (d)
        : 
    );
}

/* Function with vector extensions */
void vector_reloads(void) {
    v4si v1 = {1, 2, 3, 4};
    v4si v2 = {5, 6, 7, 8};
    v4si v3, v4;
    
    /* Vector operations that may need decomposition */
    v3 = v1 + v2;
    v4 = v1 * v2;
    
    /* Shuffle operation - complex pattern */
    v4 = __builtin_shuffle(v3, v4, (v4si){3, 2, 1, 0});
    
    /* Mix with scalar operations */
    int* p = (int*)&v4;
    for (int i = 0; i < 4; i++) {
        p[i] += vi1;
    }
}

/* Function with control flow splitting live ranges */
int control_flow_reloads(int x) {
    int a, b, c, d, e, f;
    
    /* Multiple variables to increase register pressure */
    a = x + vi1;
    b = x * vi2;
    
    /* goto to split live ranges */
    if (x > 100) {
        goto compute;
    }
    
    c = a + b;
    d = a * b;
    
    /* Another branch */
    if (x < 50) {
        e = c - d;
        f = e * 2;
    } else {
        e = d - c;
        f = e / 2;
    }
    
compute:
    /* Use variables defined before goto - forces spills/reloads */
    /* RELOAD_FOR_INPADDR_ADDRESS, RELOAD_FOR_OUTADDR_ADDRESS */
    int* ptr1 = &a;
    int* ptr2 = &b;
    
    asm volatile (
        "movl (%1), %%eax\n\t"
        "addl (%2), %%eax\n\t"
        "movl %%eax, %0"
        : "=m" (*ptr1)
        : "r" (ptr1), "r" (ptr2)
        : "%eax", "memory"
    );
    
    return a + b + (x > 100 ? 0 : c + d + e + f);
}

int main(void) {
    int checksum = 0;
    
    /* 1. Complex addressing mode stress */
    complex_addressing(100);
    
    /* 2. Structure passing reloads */
    struct SmallStruct s1 = {vi1, vi2, vi3, vi4};
    struct SmallStruct s2 = {vi4, vi3, vi2, vi1};
    
    for (int i = 0; i < 10; i++) {
        s1 = process_struct(s1, s2);
        s2 = chain_struct(s2, 2);
        checksum += s1.a + s2.b;
    }
    
    /* 3. Inline assembly chains */
    for (int i = 0; i < 5; i++) {
        asm_reload_chains();
        checksum += i * 10;
    }
    
    /* 4. Vector extension reloads */
    vector_reloads();
    
    /* 5. Control flow with split live ranges */
    for (int i = 0; i < 20; i++) {
        checksum += control_flow_reloads(i);
    }
    
    /* Additional stress: mixed operations in loop */
    int large_array[256];
    for (int i = 0; i < 256; i++) {
        large_array[i] = i;
    }
    
    /* Pointer arithmetic with volatile */
    int* ptr = large_array;
    for (int i = 0; i < 100; i++) {
        ptr[vi1 + i] = ptr[vi2 * i] + ptr[vi3 + i * 2];
        checksum += ptr[i];
    }
    
    /* Final checksum output */
    printf("Checksum: %d\n", checksum);
    
    return 0;
}
