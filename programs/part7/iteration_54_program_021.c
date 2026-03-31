/* reload_coverage.c - Stress GCC's reload pass for various reload types */
#include <stdio.h>
#include <stdint.h>

/* For complex addressing modes and volatile indices */
volatile int vi1 = 7, vi2 = 13, vi3 = 42;

/* Large structure to force spilling */
struct LargeStruct {
    int data[32];
    long long more[16];
    char padding[128];
};

/* Vector type for GCC extensions */
typedef int v4si __attribute__((vector_size(16)));
typedef long long v2di __attribute__((vector_size(16)));

/* Structure for by-value passing */
struct SmallAgg {
    int a, b, c, d;
};

/* Function prototypes */
struct SmallAgg process_aggregate(struct SmallAgg s1, struct SmallAgg s2);
void complex_addressing(int arr[][16], int n);
void inline_asm_chain(int *a, int *b, int *c);

/* Main orchestration */
int main(void) {
    int i, j;
    int checksum = 0;
    
    /* 1. Complex addressing mode stress */
    int arr1[32][16];
    int arr2[24][16];
    
    /* Initialize arrays */
    for (i = 0; i < 32; i++) {
        for (j = 0; j < 16; j++) {
            arr1[i][j] = i * 16 + j;
            if (i < 24) arr2[i][j] = (i * 16 + j) * 2;
        }
    }
    
    /* Force complex addressing with volatile indices */
    /* This should trigger RELOAD_FOR_INPUT_ADDRESS, RELOAD_FOR_OUTPUT_ADDRESS */
    for (i = 0; i < 100; i++) {
        /* Non-constant indices with volatile components */
        int idx1 = vi1 + i;
        int idx2 = vi2 * i;
        int idx3 = vi3 - i;
        
        /* Multi-dimensional access with complex addressing */
        if (idx1 < 32 && idx2 < 16 && idx3 < 24) {
            arr1[idx1][idx2] = arr2[idx3][idx1 % 16] + arr1[idx3 % 32][idx2];
            arr2[idx3][idx1 % 16] = arr1[idx2 % 32][idx3 % 16] * vi1;
        }
    }
    
    /* 2. Inline assembly with multiple operands */
    /* Should trigger RELOAD_FOR_INPUT, RELOAD_FOR_OUTPUT_ADDRESS, RELOAD_OTHER */
    int asm_var1 = 1000, asm_var2 = 2000, asm_var3 = 3000;
    for (i = 0; i < 50; i++) {
        /* Chain of asm blocks creating dependencies */
        asm volatile (
            "addl %1, %0\n\t"
            "movl %0, %2\n\t"
            : "+r"(asm_var1), "+r"(asm_var2)
            : "m"(asm_var3)
            : "cc"
        );
        
        asm volatile (
            "imull %1, %0\n\t"
            "leal (%0,%2,2), %0\n\t"
            : "+r"(asm_var2)
            : "r"(asm_var1), "m"(asm_var3)
            : "cc"
        );
        
        asm_var3 = asm_var1 + asm_var2 + vi1;
    }
    
    /* 3. Volatile and non-addressable variables */
    /* Should trigger RELOAD_FOR_OPERAND_ADDRESS, RELOAD_FOR_OPADDR_ADDR */
    {
        volatile struct LargeStruct big1, big2;
        int *volatile ptr1 = (int*)&big1;
        int *volatile ptr2 = (int*)&big2;
        
        /* Force partial register usage */
        for (i = 0; i < 32; i += 2) {
            big1.data[i] = i * vi1;
            big2.data[i] = i * vi2;
        }
        
        /* Complex pointer arithmetic */
        for (i = 0; i < 16; i++) {
            *(ptr1 + vi1 + i) = *(ptr2 + vi2 + i * 2) + vi3;
            ptr1[vi1 + i * 3] = ptr2[vi2 + i * 4] * 2;
        }
    }
    
    /* 4. Nested function calls with aggregates */
    /* Should trigger RELOAD_FOR_INPADDR_ADDRESS, RELOAD_FOR_OUTADDR_ADDRESS */
    {
        struct SmallAgg s1 = {1, 2, 3, 4};
        struct SmallAgg s2 = {5, 6, 7, 8};
        struct SmallAgg result;
        
        for (i = 0; i < 20; i++) {
            s1.a += vi1;
            s2.b += vi2;
            result = process_aggregate(s1, s2);
            checksum += result.a + result.b + result.c + result.d;
        }
    }
    
    /* 5. Compiler builtins and vector extensions */
    /* Should trigger various reload types for vector decomposition */
    {
        v4si vec1 = {1, 2, 3, 4};
        v4si vec2 = {5, 6, 7, 8};
        v4si vec3;
        v2di vec4 = {vi1, vi2};
        v2di vec5 = {vi2, vi3};
        
        /* Vector operations that may need decomposition */
        for (i = 0; i < 30; i++) {
            vec1 = vec1 + vec2;
            vec2 = vec2 * vec1;
            
            /* Shuffle with variable indices */
            int shuffle_mask[4] = {vi1 & 3, vi2 & 3, vi3 & 3, (vi1 + vi2) & 3};
            vec3 = __builtin_shuffle(vec1, vec2, 
                (v4si){shuffle_mask[0], shuffle_mask[1], 
                       shuffle_mask[2], shuffle_mask[3]});
            
            vec4 = vec4 + vec5;
            vec5 = vec5 - vec4;
        }
        
        /* Extract elements for checksum */
        int vec_arr[4];
        __builtin_memcpy(vec_arr, &vec3, sizeof(vec3));
        for (i = 0; i < 4; i++) checksum += vec_arr[i];
    }
    
    /* 6. Control flow that splits live ranges */
    /* Should trigger RELOAD_FOR_OTHER_ADDRESS */
    {
        int x = 100, y = 200, z = 300;
        volatile int control = vi1;
        
        /* Complex control flow with goto */
        if (control > 20) {
            x = x * 2;
            goto label1;
        } else {
            y = y * 3;
            goto label2;
        }
        
    label1:
        for (i = 0; i < 10; i++) {
            z = x + y + vi1;
            if (z > 500) goto label3;
            x = y + vi2;
        }
        goto label4;
        
    label2:
        for (i = 0; i < 15; i++) {
            y = x + z + vi3;
            x = y * 2;
        }
        goto label4;
        
    label3:
        z = x * y * z;
        
    label4:
        checksum += x + y + z;
    }
    
    /* Final checksum computation using array data */
    for (i = 0; i < 32; i++) {
        for (j = 0; j < 16; j++) {
            checksum += arr1[i][j];
            if (i < 24) checksum += arr2[i][j];
        }
    }
    
    checksum += asm_var1 + asm_var2 + asm_var3;
    
    printf("Checksum: %d\n", checksum);
    return 0;
}

/* Function to process aggregates by value */
struct SmallAgg process_aggregate(struct SmallAgg s1, struct SmallAgg s2) {
    struct SmallAgg result;
    volatile int mod = vi3;
    
    /* Complex operations to prevent optimization */
    result.a = s1.a * s2.b + mod;
    result.b = s1.b * s2.c - mod;
    result.c = s1.c * s2.d + s1.d * s2.a;
    result.d = s1.d + s2.b + s2.c + s2.d;
    
    /* Nested call to increase register pressure */
    if (mod > 30) {
        struct SmallAgg tmp = {result.b, result.c, result.d, result.a};
        result = process_aggregate(result, tmp);
    }
    
    return result;
}

/* Function with complex addressing */
void complex_addressing(int arr[][16], int n) {
    int i, j;
    volatile int v1 = vi1, v2 = vi2;
    
    /* Even more complex addressing patterns */
    for (i = 0; i < n; i++) {
        for (j = 0; j < 16; j++) {
            /* Multiple array dimensions with volatile indices */
            arr[(i + v1) % n][(j + v2) % 16] += 
                arr[(i * v2) % n][(j * v1) % 16] * 3;
            
            /* Pointer arithmetic that can't be easily optimized */
            int *ptr = &arr[i][0];
            ptr[v1 + j] = ptr[v2 + (j * 2) % 16] + i;
        }
    }
}

/* Chain of inline assembly operations */
void inline_asm_chain(int *a, int *b, int *c) {
    int tmp1, tmp2, tmp3;
    
    /* Multiple asm blocks with dependencies */
    asm volatile (
        "movl (%1), %0\n\t"
        "addl (%2), %0\n\t"
        : "=r"(tmp1)
        : "r"(a), "r"(b)
        : "memory"
    );
    
    asm volatile (
        "imull %2, %0\n\t"
        "addl %1, %0\n\t"
        : "+r"(tmp1)
        : "r"(tmp1), "m"(*c)
        : "cc"
    );
    
    asm volatile (
        "movl %1, (%0)\n\t"
        "addl $1, (%0)\n\t"
        : 
        : "r"(a), "r"(tmp1)
        : "memory"
    );
}
