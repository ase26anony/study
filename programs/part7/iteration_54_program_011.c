/* reload_coverage.c - Stress GCC's reload pass for coverage testing */
#include <stdio.h>
#include <stdint.h>

/* Volatile indices to prevent constant folding */
volatile int vi1 = 7, vi2 = 3, vi3 = 11, vi4 = 5;

/* Structure for passing by value */
struct SmallStruct {
    int a;
    int b;
    int c;
    int d;
};

/* Large array to force spilling */
int large_array[1000];

/* Vector type for GCC extensions */
typedef int v4si __attribute__((vector_size(16)));

/* Function passing/returning struct by value */
struct SmallStruct process_struct(struct SmallStruct s1, struct SmallStruct s2) {
    struct SmallStruct result;
    result.a = s1.a + s2.b;
    result.b = s1.b - s2.c;
    result.c = s1.c * s2.d;
    result.d = s1.d / (s2.a ? s2.a : 1);
    return result;
}

/* Another function to create call chain */
struct SmallStruct chain_struct(struct SmallStruct s, int multiplier) {
    struct SmallStruct temp = s;
    temp.a *= multiplier;
    temp.b *= multiplier + 1;
    temp.c *= multiplier + 2;
    temp.d *= multiplier + 3;
    
    /* Inline asm with register constraints */
    asm volatile (
        "addl $1, %0\n\t"
        "subl $2, %1\n\t"
        : "+r"(temp.a), "+r"(temp.b)
        : 
        : "cc"
    );
    
    return temp;
}

/* Function with complex addressing modes */
void complex_addressing(int n) {
    /* Multi-dimensional array with volatile indices */
    int arr[50][50];
    static int counter = 0;
    
    /* Force non-trivial address calculations */
    for (int i = 0; i < n; i++) {
        /* RELOAD_FOR_INPUT_ADDRESS and RELOAD_FOR_OUTPUT_ADDRESS */
        arr[vi1 + i][vi2 * 2] = arr[vi3 % 20][vi4 + i * 3] + i;
        
        /* More complex addressing with pointer arithmetic */
        int *ptr1 = &arr[i][0];
        int *ptr2 = &arr[vi2][i];
        
        /* RELOAD_FOR_OPERAND_ADDRESS */
        *(ptr1 + vi1) = *(ptr2 + vi3) * 2;
        
        /* Chain of address calculations */
        int **pptr = &ptr1;
        **pptr = i * counter++;
    }
}

/* Function with inline assembly chains */
void asm_reload_chain(int iterations) {
    int a = vi1, b = vi2, c = vi3, d = vi4;
    int tmp1, tmp2, tmp3;
    
    for (int i = 0; i < iterations; i++) {
        /* First asm: outputs become inputs for next */
        asm volatile (
            "movl %1, %0\n\t"
            "addl $100, %0\n\t"
            : "=r"(tmp1)
            : "r"(a)
            : 
        );
        
        /* Second asm: uses previous output */
        asm volatile (
            "imull %2, %1\n\t"
            "addl %1, %0\n\t"
            : "+r"(tmp2), "=r"(tmp3)
            : "r"(tmp1), "0"(b)
            : "cc"
        );
        
        /* Third asm: memory operand */
        asm volatile (
            "movl %1, %0\n\t"
            : "=m"(large_array[i % 1000])
            : "r"(tmp2)
            : 
        );
        
        /* Cycle values */
        a = b;
        b = c;
        c = d;
        d = tmp3;
    }
}

/* Function with vector extensions */
void vector_reloads(void) {
    v4si v1 = {1, 2, 3, 4};
    v4si v2 = {vi1, vi2, vi3, vi4};
    v4si v3, v4;
    
    /* Vector operations */
    v3 = v1 + v2;
    v4 = v1 * v2;
    
    /* Shuffle - may require special handling */
    v4si v5 = __builtin_shuffle(v3, v4, (v4si){3, 2, 1, 0});
    
    /* Store to memory with complex addressing */
    int *p = (int*)&v5;
    for (int i = 0; i < 4; i++) {
        large_array[vi1 * i + vi2] = p[i];
    }
}

/* Function with goto to split live ranges */
void control_flow_reloads(int x) {
    int a = vi1 * 10;
    int b = vi2 * 20;
    int c = vi3 * 30;
    
    if (x > 100) {
        goto complex_path;
    }
    
    /* Simple path */
    a = b + c;
    large_array[x] = a;
    return;
    
complex_path:
    /* Variables defined earlier but used here - split live ranges */
    int *ptr = &large_array[a % 1000];
    
    /* Complex addressing with multiple calculations */
    for (int i = 0; i < x; i++) {
        ptr[(b + i) % 1000] = c * i;
        
        /* Inline asm with multiple constraints */
        asm volatile (
            "leal (%1, %2, 4), %0\n\t"
            : "=r"(c)
            : "r"(a), "r"(b)
            : 
        );
        
        /* Update pointers */
        ptr = &large_array[(ptr - large_array + 1) % 1000];
    }
    
    /* Store final result */
    large_array[0] = a + b + c;
}

/* Main function orchestrating all patterns */
int main(void) {
    int checksum = 0;
    
    /* Initialize array */
    for (int i = 0; i < 1000; i++) {
        large_array[i] = i;
    }
    
    /* 1. Complex addressing modes */
    complex_addressing(50);
    
    /* 2. Structure passing chain */
    struct SmallStruct s1 = {vi1, vi2, vi3, vi4};
    struct SmallStruct s2 = {vi2, vi3, vi4, vi1};
    
    for (int i = 0; i < 10; i++) {
        struct SmallStruct s3 = process_struct(s1, s2);
        struct SmallStruct s4 = chain_struct(s3, i + 1);
        
        /* Use results to prevent optimization */
        checksum += s4.a + s4.b - s4.c + s4.d;
        
        /* Update for next iteration */
        s1 = s2;
        s2 = s3;
    }
    
    /* 3. Inline assembly chains */
    asm_reload_chain(20);
    
    /* 4. Vector extensions */
    vector_reloads();
    
    /* 5. Control flow with split live ranges */
    control_flow_reloads(150);
    
    /* Compute final checksum from array */
    for (int i = 0; i < 100; i++) {
        checksum += large_array[i * 7 % 1000];
    }
    
    /* Use volatile to ensure computation happens */
    volatile int final_result = checksum % 1000000;
    
    printf("Result: %d\n", final_result);
    return 0;
}
