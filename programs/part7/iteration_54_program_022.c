/* reload_coverage.c - Stress GCC's reload pass for various reload types */
#include <stdio.h>
#include <stdint.h>

/* Volatile indices to prevent constant folding */
volatile int vi1 = 3, vi2 = 7, vi3 = 11;

/* 1. Complex addressing mode stress */
void complex_addressing(int n) {
    int arr[100][100];
    int i, j;
    
    /* Initialize array */
    for (i = 0; i < 100; i++)
        for (j = 0; j < 100; j++)
            arr[i][j] = i * 100 + j;
    
    /* Force complex address calculations with volatile indices */
    for (i = 0; i < n; i++) {
        /* These should trigger RELOAD_FOR_INPUT_ADDRESS and RELOAD_FOR_OUTPUT_ADDRESS */
        arr[vi1 + i][vi2 * 2] = arr[vi3][i + vi1] + arr[i][vi2];
        arr[i][vi1] = arr[vi2][i * vi3] * arr[vi1 + i][vi2 - i];
    }
}

/* 2. Structure passing for address reloads */
struct SmallStruct {
    int a, b, c, d;
};

struct SmallStruct process_struct(struct SmallStruct s1, struct SmallStruct s2) {
    /* Mix operations to force register pressure */
    struct SmallStruct result;
    result.a = s1.a + s2.b;
    result.b = s1.b - s2.a;
    result.c = s1.c * s2.d;
    result.d = s1.d / (s2.c ? s2.c : 1);
    return result;
}

struct SmallStruct struct_chain(struct SmallStruct s) {
    struct SmallStruct temp1, temp2, temp3;
    
    /* Chain of structure operations - may trigger RELOAD_FOR_INPADDR_ADDRESS */
    temp1 = process_struct(s, s);
    temp2 = process_struct(temp1, s);
    temp3 = process_struct(temp2, temp1);
    
    return process_struct(temp3, temp2);
}

/* 3. Inline assembly with multiple constraints */
void inline_asm_stress(void) {
    int a = 100, b = 200, c = 300, d = 400;
    int result1, result2, result3;
    
    /* Chain of asm blocks creating dependencies */
    asm volatile (
        "mov %[in1], %[out1]\n\t"
        "add %[in2], %[out1]"
        : [out1] "=r" (result1)
        : [in1] "r" (a), [in2] "r" (b)
        : "cc"
    );
    
    /* Memory constraint to force address reloads */
    asm volatile (
        "imul %[in1], %[out1]\n\t"
        "sub %[in2], %[out1]"
        : [out1] "=r" (result2)
        : [in1] "r" (result1), [in2] "m" (c)
        : "cc"
    );
    
    /* Complex asm with multiple outputs/inputs */
    asm volatile (
        "lea (%[in1], %[in2], 2), %[out1]\n\t"
        "mov %[out1], %[out2]"
        : [out1] "=r" (result1), [out2] "=r" (result3)
        : [in1] "r" (result2), [in2] "r" (d)
        : "cc"
    );
}

/* 4. Vector extensions for RELOAD_OTHER */
typedef int v4si __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));

void vector_operations(void) {
    v4si vec1 = {1, 2, 3, 4};
    v4si vec2 = {5, 6, 7, 8};
    v4si vec3, vec4;
    
    /* Vector operations that might need decomposition */
    vec3 = vec1 + vec2;
    vec4 = vec1 * vec2;
    
    /* Shuffle operation */
    vec3 = __builtin_shuffle(vec3, vec4, (v4si){3, 2, 1, 0});
    
    /* Mix with scalar operations */
    int temp[4];
    temp[0] = vec3[0] + vi1;
    temp[1] = vec3[1] * vi2;
    temp[2] = vec3[2] - vi3;
    temp[3] = vec3[3] / (vi1 ? vi1 : 1);
}

/* 5. Control flow with register pressure */
void control_flow_stress(int n) {
    int i, j;
    int arr[100];
    int sum = 0;
    
    /* Initialize with pattern */
    for (i = 0; i < 100; i++)
        arr[i] = i * 3;
    
    /* Complex control flow with gotos */
    i = 0;
loop_start:
    if (i >= n) goto loop_end;
    
    /* Multiple computations to increase register pressure */
    int t1 = arr[i] * vi1;
    int t2 = arr[i + 1] + vi2;
    int t3 = arr[i + 2] - vi3;
    int t4 = arr[i + 3] / (vi1 + 1);
    
    /* Conditional jump creates live range splits */
    if (t1 > t2) {
        arr[i] = t3 + t4;
        goto update;
    } else {
        arr[i] = t1 - t2;
    }
    
    /* Another basic block */
    int t5 = t1 * t2 + t3 - t4;
    arr[i + 1] = t5;
    
update:
    sum += arr[i];
    i++;
    goto loop_start;
    
loop_end:
    /* Use sum to prevent optimization */
    asm volatile ("" : : "r" (sum));
}

/* 6. Mixed operations for various reload types */
void mixed_reload_patterns(void) {
    /* Large local array - non-addressable parts */
    struct {
        int a[50];
        char b[100];
        int c[25];
    } big_struct;
    
    /* Initialize */
    for (int i = 0; i < 50; i++) big_struct.a[i] = i * 2;
    for (int i = 0; i < 100; i++) big_struct.b[i] = i % 26 + 'A';
    for (int i = 0; i < 25; i++) big_struct.c[i] = i * i;
    
    /* Pointer arithmetic that can't be easily optimized */
    int *ptr1 = &big_struct.a[vi1];
    int *ptr2 = &big_struct.c[vi2 % 25];
    
    /* Complex addressing */
    for (int i = 0; i < 10; i++) {
        /* May trigger RELOAD_FOR_OPERAND_ADDRESS */
        ptr1[i * vi3] = ptr2[(i + vi1) % 25] + big_struct.b[i * 10];
        
        /* More complex expression */
        big_struct.a[(i + vi2) % 50] = 
            ptr1[i] * ptr2[i % 10] - big_struct.c[vi3 % 25];
    }
    
    /* Structure passing with addresses */
    struct SmallStruct ss1 = {1, 2, 3, 4};
    struct SmallStruct ss2 = {5, 6, 7, 8};
    
    /* Chain of operations */
    for (int i = 0; i < 5; i++) {
        ss1 = struct_chain(ss1);
        ss2 = process_struct(ss1, ss2);
        
        /* Mix with array access */
        big_struct.c[i] = ss1.a + ss2.b;
    }
}

int main(void) {
    int checksum = 0;
    
    printf("Starting reload stress test...\n");
    
    /* Execute all patterns */
    complex_addressing(20);
    
    struct SmallStruct ss = {10, 20, 30, 40};
    ss = struct_chain(ss);
    checksum += ss.a + ss.b + ss.c + ss.d;
    
    inline_asm_stress();
    
    vector_operations();
    
    control_flow_stress(50);
    
    mixed_reload_patterns();
    
    /* Final checksum */
    checksum += vi1 + vi2 + vi3;
    printf("Checksum: %d\n", checksum);
    
    return 0;
}
