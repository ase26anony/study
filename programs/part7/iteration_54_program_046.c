/* reload_coverage.c - Stress GCC's reload pass for coverage testing */
#include <stdio.h>
#include <stdint.h>

/* Volatile indices to prevent constant folding */
volatile int vi1 = 7, vi2 = 3, vi3 = 11, vi4 = 5;

/* 1. Complex Addressing Mode Stress */
void complex_addressing(int size) {
    /* Large multi-dimensional array */
    int arr[100][50];
    static int counter = 0;
    
    /* Initialize with pattern */
    for (int i = 0; i < 100; i++) {
        for (int j = 0; j < 50; j++) {
            arr[i][j] = i * 100 + j + counter;
        }
    }
    
    /* Complex addressing with volatile indices */
    /* Triggers RELOAD_FOR_INPUT_ADDRESS, RELOAD_FOR_OUTPUT_ADDRESS */
    arr[vi1][vi2 + 1] = arr[vi3 % 50][(vi4 * 2) % 50];
    arr[(vi1 + vi2) % 100][vi3] = arr[vi4][(vi1 * vi2) % 50];
    
    /* Pointer arithmetic that can't be folded */
    int *ptr1 = &arr[0][0] + vi1 * 50 + vi2;
    int *ptr2 = &arr[vi3][0] + (vi4 << 1);
    *ptr1 = *ptr2 + arr[vi2][vi1];
    
    counter++;
}

/* 2. Inline Assembly with Multiple Operands */
void inline_asm_stress(void) {
    int a = vi1, b = vi2, c = vi3, d = vi4;
    int result1, result2, result3;
    
    /* Chain of asm blocks creating dependencies */
    /* Triggers RELOAD_FOR_INPUT, RELOAD_FOR_OUTPUT_ADDRESS, RELOAD_OTHER */
    asm volatile (
        "mov %[in1], %[out1]\n\t"
        "add %[in2], %[out1]\n\t"
        : [out1] "=r" (result1)
        : [in1] "r" (a), [in2] "r" (b)
        : "cc"
    );
    
    asm volatile (
        "imul %[in1], %[out1]\n\t"
        "sub %[in2], %[out1]\n\t"
        : [out1] "=r" (result2)
        : [in1] "r" (result1), [in2] "rm" (c)
        : "cc"
    );
    
    asm volatile (
        "lea (%[in1], %[in2], 2), %[out1]\n\t"
        "and %[in3], %[out1]\n\t"
        : [out1] "=r" (result3)
        : [in1] "r" (result2), [in2] "r" (d), [in3] "rm" (0xFF)
        : "cc"
    );
    
    /* Mixed register/memory constraints */
    int mem_var = vi1 * vi2;
    asm volatile (
        "addl %[mem], %[reg]\n\t"
        "mov %[reg], %[mem]\n\t"
        : [mem] "+m" (mem_var), [reg] "+r" (result3)
        :
        : "cc"
    );
}

/* 3. Structures for value passing */
struct SmallStruct {
    int a, b, c, d;
};

struct MediumStruct {
    int arr[8];
    char padding[7]; /* Make size non-power-of-two */
};

/* 4. Nested Function Calls with Aggregates */
struct SmallStruct process_struct(struct SmallStruct s1, struct SmallStruct s2) {
    /* Triggers RELOAD_FOR_INPADDR_ADDRESS, RELOAD_FOR_OUTADDR_ADDRESS */
    struct SmallStruct result;
    result.a = s1.a + s2.d;
    result.b = s1.b * s2.c;
    result.c = s1.c - s2.b;
    result.d = s1.d / (s2.a ? s2.a : 1);
    return result;
}

struct MediumStruct process_medium(struct MediumStruct m, int idx) {
    /* Complex indexing within returned struct */
    m.arr[idx % 8] = vi1 + vi2;
    m.arr[(idx + 1) % 8] = m.arr[idx % 8] * vi3;
    return m;
}

void struct_passing_chain(void) {
    struct SmallStruct s1 = {vi1, vi2, vi3, vi4};
    struct SmallStruct s2 = {vi2, vi3, vi4, vi1};
    struct SmallStruct s3 = {vi3, vi4, vi1, vi2};
    
    /* Chain of struct operations */
    struct SmallStruct tmp = process_struct(s1, s2);
    struct SmallStruct result = process_struct(tmp, s3);
    
    /* Use result to prevent optimization */
    volatile int sink __attribute__((unused));
    sink = result.a + result.b + result.c + result.d;
}

/* 5. Vector Extensions and Builtins */
#ifdef __GNUC__
void vector_operations(void) {
    /* GCC vector extensions */
    typedef int v4si __attribute__((vector_size(16)));
    typedef short v8hi __attribute__((vector_size(16)));
    
    v4si vec1 = {vi1, vi2, vi3, vi4};
    v4si vec2 = {vi4, vi3, vi2, vi1};
    v4si vec3;
    
    /* Vector operations that may need decomposition */
    vec3 = vec1 + vec2;
    vec3 = vec3 * vec1;
    
    /* Shuffle operation */
    v4si shuffled;
    asm volatile (
        "pshufd $0x1B, %[in], %[out]\n\t"
        : [out] "=x" (shuffled)
        : [in] "x" (vec3)
    );
    
    /* Mixed-size vector operations */
    v8hi short_vec1 = {vi1, vi2, vi3, vi4, vi1, vi2, vi3, vi4};
    v8hi short_vec2 = {vi4, vi3, vi2, vi1, vi4, vi3, vi2, vi1};
    v8hi short_result = short_vec1 + short_vec2;
    
    /* Use results */
    volatile v4si vsink __attribute__((unused));
    vsink = vec3 + shuffled;
}
#endif

/* 6. Control Flow with Split Live Ranges */
void control_flow_stress(int iterations) {
    int a = vi1, b = vi2, c = vi3, d = vi4;
    int result = 0;
    
    /* Complex control flow with gotos */
    int i = 0;
    
loop_start:
    if (i >= iterations) goto loop_end;
    
    /* Values defined here, used in distant blocks */
    int x = a * i + b;
    int y = c * (i + 1) - d;
    
    if (i % 3 == 0) {
        goto case0;
    } else if (i % 3 == 1) {
        goto case1;
    } else {
        goto case2;
    }

case0:
    result += x * y;
    i++;
    goto loop_start;

case1:
    /* x and y live across multiple basic blocks */
    result += x - y;
    if (result > 1000) {
        result /= 2;
    }
    i++;
    goto loop_start;

case2:
    result += (x << 2) | (y & 0xF);
    i++;
    goto loop_start;

loop_end:
    /* Use result */
    volatile int sink __attribute__((unused));
    sink = result;
}

/* Main orchestrator */
int main(void) {
    int checksum = 0;
    
    /* Execute all patterns */
    for (int i = 0; i < 10; i++) {
        complex_addressing(i);
        inline_asm_stress();
        struct_passing_chain();
        
        #ifdef __GNUC__
        vector_operations();
        #endif
        
        control_flow_stress(5);
        
        /* Update checksum with volatile values */
        checksum += vi1 + vi2 + vi3 + vi4;
        
        /* Modify volatiles to change patterns */
        vi1 = (vi1 * 13 + 7) % 97;
        vi2 = (vi2 * 17 + 11) % 97;
        vi3 = (vi3 * 19 + 13) % 97;
        vi4 = (vi4 * 23 + 17) % 97;
    }
    
    printf("Checksum: %d\n", checksum);
    return 0;
}
