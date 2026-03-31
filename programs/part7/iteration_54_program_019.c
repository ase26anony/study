/* Test program to trigger various reload types in GCC's reload1.cc */
#include <stdio.h>
#include <stdint.h>

/* Volatile indices to prevent constant folding */
volatile int vi1 = 1, vi2 = 2, vi3 = 3, vi4 = 4;

/* Structure for passing by value */
struct SmallStruct {
    int a, b, c, d;
};

/* Function to create structure passing reloads */
struct SmallStruct __attribute__((noinline)) 
process_struct(struct SmallStruct s, int multiplier) {
    struct SmallStruct result;
    result.a = s.a * multiplier;
    result.b = s.b * multiplier;
    result.c = s.c * multiplier;
    result.d = s.d * multiplier;
    return result;
}

/* Another function to chain structure passing */
struct SmallStruct __attribute__((noinline))
chain_struct(struct SmallStruct s1, struct SmallStruct s2) {
    struct SmallStruct result;
    result.a = s1.a + s2.a;
    result.b = s1.b + s2.b;
    result.c = s1.c + s2.c;
    result.d = s1.d + s2.d;
    return result;
}

/* Vector type to stress register allocation */
typedef int v4si __attribute__((vector_size(16)));

/* Large array to force spills */
int large_array[1000];

/* Complex addressing mode function */
void __attribute__((noinline))
complex_addressing(int n) {
    /* Multi-dimensional array with volatile indices */
    int arr[100][100];
    
    /* Initialize */
    for (int i = 0; i < 100; i++) {
        for (int j = 0; j < 100; j++) {
            arr[i][j] = i * 100 + j;
        }
    }
    
    /* Complex addressing that should trigger address reloads */
    for (int i = 0; i < n; i++) {
        /* RELOAD_FOR_INPUT_ADDRESS, RELOAD_FOR_OUTPUT_ADDRESS */
        arr[vi1 + i][vi2 * 2] = arr[vi3][vi4 + i * 3];
        
        /* More complex addressing with pointer arithmetic */
        int *ptr1 = &arr[vi1][i];
        int *ptr2 = &arr[i][vi2];
        *ptr1 = *ptr2 + arr[vi3][i + vi4];
    }
}

/* Function with inline assembly to trigger various reload types */
void __attribute__((noinline))
asm_reload_patterns(void) {
    int a = 1000, b = 2000, c = 3000, d = 4000;
    int result1, result2, result3;
    
    /* Chain of asm statements creating dependencies */
    /* RELOAD_FOR_INPUT, RELOAD_FOR_OUTPUT_ADDRESS */
    asm volatile (
        "movl %1, %0\n\t"
        "addl %2, %0"
        : "=r" (result1)
        : "r" (a), "r" (b)
        : "cc"
    );
    
    /* Another asm using memory operand */
    /* RELOAD_FOR_OPERAND_ADDRESS */
    asm volatile (
        "movl (%1), %0\n\t"
        "imull %2, %0"
        : "=r" (result2)
        : "r" (&c), "r" (result1)
        : "memory"
    );
    
    /* Third asm with multiple constraints */
    /* RELOAD_FOR_INPADDR_ADDRESS, RELOAD_FOR_OUTADDR_ADDRESS */
    asm volatile (
        "leal (%1, %2, 4), %0"
        : "=r" (result3)
        : "r" (result2), "r" (d)
        : "cc"
    );
    
    /* Use results to prevent optimization */
    large_array[0] = result1 + result2 + result3;
}

/* Function with vector operations */
void __attribute__((noinline))
vector_operations(void) {
    v4si v1 = {1, 2, 3, 4};
    v4si v2 = {5, 6, 7, 8};
    v4si v3, v4;
    
    /* Vector operations that may need decomposition */
    v3 = v1 + v2;
    v4 = v1 * v2;
    
    /* Shuffle operation */
    v4si mask = {3, 2, 1, 0};
    v4si shuffled;
    
    /* Use builtin shuffle - may require complex reloads */
    shuffled = __builtin_shuffle(v3, v4, mask);
    
    /* Store to memory */
    __builtin_memcpy(&large_array[100], &shuffled, sizeof(shuffled));
}

/* Function with control flow to split live ranges */
int __attribute__((noinline))
split_live_ranges(int n) {
    int x = 0, y = 0, z = 0;
    
    /* Complex control flow with goto */
    if (n > 0) {
        x = vi1;
        goto label1;
    } else {
        x = vi2;
        goto label2;
    }
    
label1:
    y = x * 2;
    if (y > 10) {
        z = y + vi3;
        goto label3;
    } else {
        z = y - vi4;
        goto label4;
    }
    
label2:
    y = x / 2;
    z = y + 100;
    goto label4;
    
label3:
    z = z * 3;
    /* Fall through */
    
label4:
    /* Use all variables in complex expression */
    return x + y + z + large_array[z % 100];
}

int main(void) {
    int checksum = 0;
    
    /* Initialize large array */
    for (int i = 0; i < 1000; i++) {
        large_array[i] = i;
    }
    
    /* 1. Complex addressing modes */
    complex_addressing(50);
    
    /* 2. Structure passing */
    struct SmallStruct s1 = {vi1, vi2, vi3, vi4};
    struct SmallStruct s2 = {vi4, vi3, vi2, vi1};
    struct SmallStruct s3 = process_struct(s1, vi2);
    struct SmallStruct s4 = chain_struct(s2, s3);
    
    checksum += s4.a + s4.b + s4.c + s4.d;
    
    /* 3. Inline assembly patterns */
    asm_reload_patterns();
    checksum += large_array[0];
    
    /* 4. Vector operations */
    vector_operations();
    checksum += large_array[100] + large_array[101];
    
    /* 5. Split live ranges */
    checksum += split_live_ranges(vi1);
    
    /* 6. More complex array access with volatile indices */
    for (int i = 0; i < 100; i++) {
        /* This should trigger RELOAD_FOR_OTHER_ADDRESS */
        large_array[(vi1 + i) % 1000] = 
            large_array[(vi2 * i) % 1000] + 
            large_array[(vi3 + i * 2) % 1000];
    }
    
    /* Final checksum computation */
    for (int i = 0; i < 100; i++) {
        checksum += large_array[i];
    }
    
    printf("Checksum: %d\n", checksum);
    return 0;
}
