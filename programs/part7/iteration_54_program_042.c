/* reload_coverage.c - Stress GCC's reload pass for coverage testing */
#include <stdio.h>
#include <stdint.h>

/* Volatile indices to prevent constant folding */
static volatile int vi1 = 1, vi2 = 2, vi3 = 3, vi4 = 4;

/* Structure for passing by value */
struct SmallStruct {
    int a, b, c, d;
};

/* Function that returns structure by value */
struct SmallStruct func_returns_struct(struct SmallStruct s) {
    s.a += vi1;
    s.b += vi2;
    s.c += vi3;
    s.d += vi4;
    return s;
}

/* Function that takes structure by value */
struct SmallStruct func_takes_struct(struct SmallStruct s1, struct SmallStruct s2) {
    struct SmallStruct result;
    result.a = s1.a + s2.b;
    result.b = s1.b + s2.c;
    result.c = s1.c + s2.d;
    result.d = s1.d + s2.a;
    return func_returns_struct(result);
}

/* Vector type for GCC extensions */
typedef int v4si __attribute__((vector_size(16)));
typedef int v8si __attribute__((vector_size(32)));

/* Complex control flow with goto */
void complex_control_flow(int *arr, int n) {
    int i = 0;
    
    if (n <= 0) goto cleanup;
    
loop_start:
    /* Force register pressure with many live values */
    int a = arr[i] + vi1;
    int b = a * vi2;
    int c = b - vi3;
    int d = c / (vi4 ? vi4 : 1);
    int e = d ^ vi1;
    int f = e | vi2;
    int g = f & vi3;
    int h = g << 2;
    
    /* Use all values to keep them live */
    arr[i] = a + b + c + d + e + f + g + h;
    
    i++;
    if (i < n) goto loop_start;
    
cleanup:
    /* Force spill code with address computation */
    arr[0] += arr[n-1] * 2;
}

int main(void) {
    int checksum = 0;
    
    /* Large local arrays to force spilling */
    int big_array1[100];
    int big_array2[100][10];  /* Multi-dimensional */
    
    /* Initialize arrays */
    for (int i = 0; i < 100; i++) {
        big_array1[i] = i * 2;
        for (int j = 0; j < 10; j++) {
            big_array2[i][j] = i + j * 3;
        }
    }
    
    /* ===== PATTERN 1: Complex Addressing Modes ===== */
    /* Force RELOAD_FOR_INPUT_ADDRESS, RELOAD_FOR_OUTPUT_ADDRESS */
    for (int i = 0; i < 50; i++) {
        /* Non-constant indices with volatile variables */
        int idx1 = (vi1 + i) % 100;
        int idx2 = (vi2 * i) % 100;
        int idx3 = (vi3 + i * 2) % 10;
        int idx4 = (vi4 * i * 3) % 10;
        
        /* Complex address calculations that can't be folded */
        big_array2[idx1][idx3] = big_array2[idx2][idx4] 
                               + big_array1[(idx1 + idx2) % 100];
        
        /* Pointer arithmetic */
        int *ptr1 = &big_array1[idx1];
        int *ptr2 = &big_array1[idx2];
        *ptr1 = *ptr2 + big_array2[i % 100][0];
    }
    
    /* ===== PATTERN 2: Inline Assembly Chains ===== */
    /* Force RELOAD_FOR_INPUT, RELOAD_FOR_OUTPUT_ADDRESS, RELOAD_OTHER */
    int asm_var1 = 1234;
    int asm_var2 = 5678;
    int asm_var3 = 0;
    
    /* Chain of asm blocks with dependencies */
    asm volatile (
        "mov %1, %%eax\n\t"
        "add $100, %%eax\n\t"
        "mov %%eax, %0\n\t"
        : "=r" (asm_var1)
        : "r" (asm_var2)
        : "%eax"
    );
    
    asm volatile (
        "imul %1, %0\n\t"
        "add $42, %0\n\t"
        : "+r" (asm_var1)
        : "rm" (asm_var3)
        : "cc"
    );
    
    asm volatile (
        "lea (%1, %2, 2), %0\n\t"
        : "=r" (asm_var2)
        : "r" (asm_var1), "r" (vi1)
        :
    );
    
    checksum += asm_var1 + asm_var2;
    
    /* ===== PATTERN 3: Structure Passing ===== */
    /* Force RELOAD_FOR_INPADDR_ADDRESS, RELOAD_FOR_OUTADDR_ADDRESS */
    struct SmallStruct s1 = {1, 2, 3, 4};
    struct SmallStruct s2 = {5, 6, 7, 8};
    
    for (int i = 0; i < 10; i++) {
        s1 = func_takes_struct(s1, s2);
        s2 = func_returns_struct(s2);
        
        /* Use structure members in address calculations */
        big_array1[s1.a % 100] = s2.b;
        big_array2[s1.c % 100][s2.d % 10] = s1.d;
    }
    
    checksum += s1.a + s1.b + s1.c + s1.d;
    
    /* ===== PATTERN 4: Vector Operations ===== */
    /* Force RELOAD_FOR_OPERAND_ADDRESS, RELOAD_FOR_OPADDR_ADDR */
    v4si vec1 = {1, 2, 3, 4};
    v4si vec2 = {5, 6, 7, 8};
    v8si vec3 = {1, 2, 3, 4, 5, 6, 7, 8};
    
    /* Complex vector operations */
    vec1 = vec1 + vec2 * 2;
    vec2 = __builtin_shuffle(vec1, vec2, (v4si){3, 2, 1, 0});
    
    /* Mix with scalar operations */
    int *vptr = (int*)&vec1;
    for (int i = 0; i < 4; i++) {
        vptr[i] += big_array1[i * 10];
    }
    
    /* Use vector elements in checksum */
    for (int i = 0; i < 4; i++) {
        checksum += vptr[i];
    }
    
    /* ===== PATTERN 5: Control Flow with Live Range Splitting ===== */
    /* Force RELOAD_FOR_OTHER_ADDRESS */
    complex_control_flow(big_array1, 100);
    
    /* ===== PATTERN 6: Mixed Operations ===== */
    /* Additional stress with volatile and addressing */
    volatile int vol_var = 1000;
    int *volatile vol_ptr = &big_array1[50];
    
    for (int i = 0; i < 20; i++) {
        /* Complex expression with many intermediates */
        int val = (big_array2[i][0] * vol_var 
                  + big_array1[i] * vi1 
                  - big_array1[99-i] * vi2) 
                  / (vi3 ? vi3 : 1);
        
        /* Store with complex address */
        vol_ptr[i % 10] = val + *(&big_array1[i] + vi4);
        
        /* Inline asm with memory constraint */
        asm volatile (
            "addl %1, %0\n\t"
            : "+m" (vol_ptr[i % 10])
            : "ri" (val)
            : "cc"
        );
    }
    
    /* Final checksum computation */
    for (int i = 0; i < 100; i++) {
        checksum += big_array1[i];
        for (int j = 0; j < 10; j++) {
            checksum += big_array2[i][j];
        }
    }
    
    checksum += vol_var;
    
    printf("Checksum: %d\n", checksum);
    return 0;
}
