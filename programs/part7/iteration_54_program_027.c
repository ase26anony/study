/* reload_coverage.c - Stress GCC's reload pass for coverage testing */
#include <stdio.h>
#include <stdint.h>

/* Volatile indices to prevent constant folding */
static volatile int vi1 = 1, vi2 = 2, vi3 = 3, vi4 = 4;

/* Structure for by-value passing */
struct SmallStruct {
    int a, b, c, d;
};

/* Large arrays to increase register pressure */
int large_array1[1000];
int large_array2[1000];

/* Vector type for GCC extensions */
typedef int v4si __attribute__((vector_size(16)));

/* Function passing/returning structures by value */
struct SmallStruct func1(struct SmallStruct s) {
    s.a += vi1;
    s.b *= vi2;
    return s;
}

struct SmallStruct func2(struct SmallStruct s) {
    s.c -= vi3;
    s.d /= (vi4 ? vi4 : 1);
    return s;
}

/* Complex addressing with multi-dimensional arrays */
void complex_addressing(int n) {
    int arr[10][20];
    int arr2[15][25];
    
    /* Force address reloads with volatile indices */
    for (int i = 0; i < 10; i++) {
        for (int j = 0; j < 20; j++) {
            /* RELOAD_FOR_INPUT_ADDRESS, RELOAD_FOR_OUTPUT_ADDRESS */
            arr[vi1 + i][j * vi2] = arr2[vi3 * j][i + vi4];
            
            /* More complex addressing */
            arr[(i * vi1) % 10][(j + vi2) % 20] += 
                arr2[(i + vi3) % 15][(j * vi4) % 25];
        }
    }
    
    /* Pointer arithmetic that can't be folded */
    int *ptr1 = &arr[0][0];
    int *ptr2 = &arr2[0][0];
    
    for (int i = 0; i < 100; i++) {
        /* RELOAD_FOR_OPERAND_ADDRESS */
        *(ptr1 + vi1 * i) = *(ptr2 + vi2 * (i + 1));
    }
}

/* Inline assembly with multiple constraints */
void inline_asm_stress(void) {
    int a = vi1, b = vi2, c = vi3, d = vi4;
    int result1, result2;
    
    /* Chain of asm blocks creating dependencies */
    asm volatile (
        "mov %[in1], %[out1]\n\t"
        "add %[in2], %[out1]"
        : [out1] "=r" (result1)
        : [in1] "r" (a), [in2] "r" (b)
        : "cc"
    );
    
    /* Second asm using output of first as input */
    asm volatile (
        "imul %[in1], %[in2]\n\t"
        "mov %%eax, %[out1]"
        : [out1] "=m" (result2)
        : [in1] "r" (result1), [in2] "r" (c)
        : "eax", "cc"
    );
    
    /* Third asm with memory constraint */
    asm volatile (
        "addl $1, %0"
        : "+m" (result2)
        :
        : "cc"
    );
    
    /* Use results to prevent optimization */
    large_array1[0] = result1 + result2;
}

/* Vector operations forcing decomposition */
void vector_operations(void) {
    v4si v1 = {vi1, vi2, vi3, vi4};
    v4si v2 = {vi4, vi3, vi2, vi1};
    v4si v3, v4;
    
    /* Complex vector operations */
    v3 = v1 + v2 * 3;
    
    /* Shuffle operation */
    v4 = __builtin_shuffle(v1, v2, 
        (v4si){3, 2, 1, 0});
    
    /* Store to memory with complex addressing */
    int *p = (int*)&v3;
    for (int i = 0; i < 4; i++) {
        large_array2[vi1 * i] = p[i];
    }
}

/* Complex control flow splitting live ranges */
void split_live_ranges(int n) {
    int x = vi1, y = vi2, z = vi3;
    int result = 0;
    
    /* goto creates complex CFG */
    if (n > 0) goto label1;
    
    x = y * z;
    y = x + vi4;
    
    /* Loop with volatile condition */
    for (int i = 0; i < vi1; i++) {
        if (i % 2 == 0) {
            z = x * y;
            goto label2;
        }
    label1:
        x = y + z;
    label2:
        y = z - x;
        
        /* Array access with volatile index */
        result += large_array1[vi2 * i % 1000];
    }
    
    /* Use all variables */
    large_array2[0] = x + y + z + result;
}

/* Main orchestrator */
int main(void) {
    int checksum = 0;
    
    /* Initialize arrays */
    for (int i = 0; i < 1000; i++) {
        large_array1[i] = i;
        large_array2[i] = 1000 - i;
    }
    
    /* 1. Structure passing */
    struct SmallStruct s = {vi1, vi2, vi3, vi4};
    for (int i = 0; i < 10; i++) {
        s = func1(s);
        s = func2(s);
    }
    checksum += s.a + s.b + s.c + s.d;
    
    /* 2. Complex addressing */
    complex_addressing(vi1);
    
    /* 3. Inline assembly stress */
    for (int i = 0; i < 5; i++) {
        inline_asm_stress();
    }
    
    /* 4. Vector operations */
    vector_operations();
    
    /* 5. Split live ranges */
    split_live_ranges(vi2);
    
    /* Compute final checksum from arrays */
    for (int i = 0; i < 100; i++) {
        checksum += large_array1[i] + large_array2[i];
    }
    
    printf("Checksum: %d\n", checksum);
    return 0;
}
