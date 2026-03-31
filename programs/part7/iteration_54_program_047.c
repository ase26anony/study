/* Compile with: gcc -O2 -fno-optimize-sibling-calls -mtune=generic -fno-inline -fomit-frame-pointer -march=x86-64 -mno-sse -o reload_test reload_test.c */

#include <stdio.h>
#include <stdint.h>

/* Pattern 1: Complex addressing modes with volatile indices */
volatile int idx1 = 3, idx2 = 7, idx3 = 11;

/* Pattern 2: Large structures and arrays */
struct LargeStruct {
    int data[8];
    volatile int volatile_data[4];
    char padding[64];
};

/* Pattern 3: Vector extensions */
typedef int v4si __attribute__((vector_size(16)));
typedef short v8hi __attribute__((vector_size(16)));

/* Pattern 4: Structure passing functions */
struct SmallStruct {
    int a, b, c, d;
};

/* Function that returns structure by value - forces complex parameter passing */
struct SmallStruct func1(struct SmallStruct s) {
    s.a += idx1;
    s.b ^= idx2;
    return s;
}

struct SmallStruct func2(struct SmallStruct s) {
    s.c *= idx3;
    s.d = s.d + s.a - s.b;
    return s;
}

/* Pattern 5: Complex control flow with gotos */
void complex_control_flow(int *arr, int n) {
    int i = 0;
    
    if (n <= 0) goto cleanup;
    
loop_start:
    /* Force register pressure */
    int t1 = arr[i] * 3;
    int t2 = t1 + idx1;
    int t3 = t2 ^ idx2;
    int t4 = t3 - idx3;
    int t5 = t4 * 7;
    int t6 = t5 / 2;
    int t7 = t6 | 0xFF;
    int t8 = t7 & 0x3F;
    int t9 = t8 << 2;
    int t10 = t9 >> 1;
    
    arr[i] = t10;
    
    i++;
    if (i < n) goto loop_start;
    
cleanup:
    /* Force spill code at control flow boundaries */
    volatile int sink = i;
    (void)sink;
}

int main() {
    int checksum = 0;
    
    /* Pattern 1: Multi-dimensional array with complex addressing */
    int arr[20][15];
    
    /* Initialize array */
    for (int i = 0; i < 20; i++) {
        for (int j = 0; j < 15; j++) {
            arr[i][j] = i * 100 + j;
        }
    }
    
    /* Complex array accesses with volatile indices */
    for (int i = 0; i < 10; i++) {
        /* These create RELOAD_FOR_INPUT_ADDRESS and RELOAD_FOR_OUTPUT_ADDRESS */
        arr[idx1 + i][idx2] = arr[idx3][idx1 * 2 + i];
        arr[i + 2][idx2 * 3] = arr[idx1][idx3 - i];
        
        /* More complex addressing */
        arr[(i * idx1) % 20][(i + idx2) % 15] = 
            arr[(idx3 - i) % 20][(idx1 * i) % 15] +
            arr[(idx2 + i) % 20][(idx3 / 2) % 15];
    }
    
    /* Pattern 2: Large structure operations */
    struct LargeStruct ls1, ls2;
    
    /* Force partial spilling */
    for (int i = 0; i < 8; i++) {
        ls1.data[i] = i * 100 + idx1;
        ls2.data[i] = i * 200 + idx2;
    }
    
    /* Mix volatile and non-volatile accesses */
    for (int i = 0; i < 4; i++) {
        ls1.volatile_data[i] = i * 50;
        ls2.volatile_data[i] = i * 75;
    }
    
    /* Structure copying with volatile elements */
    for (int i = 0; i < 8; i++) {
        ls1.data[i] = ls2.data[i] + ls1.volatile_data[i % 4];
    }
    
    /* Pattern 3: Inline assembly with multiple constraints */
    int asm_var1 = 1000, asm_var2 = 2000, asm_var3 = 3000;
    int asm_result1, asm_result2, asm_result3;
    
    /* Chain of asm blocks creating dependencies */
    asm volatile (
        "movl %1, %%eax\n\t"
        "addl %2, %%eax\n\t"
        "movl %%eax, %0\n\t"
        : "=r" (asm_result1)
        : "r" (asm_var1), "r" (idx1)
        : "%eax"
    );
    
    asm volatile (
        "imull %1, %0\n\t"
        "addl %2, %0\n\t"
        : "+r" (asm_result1)
        : "r" (asm_var2), "m" (idx2)
        : "cc"
    );
    
    asm volatile (
        "xorl %1, %0\n\t"
        "subl %2, %0\n\t"
        : "+r" (asm_result1)
        : "r" (asm_var3), "r" (idx3)
        : "cc"
    );
    
    /* Pattern 4: Structure passing chain */
    struct SmallStruct ss = {10, 20, 30, 40};
    
    for (int i = 0; i < 5; i++) {
        /* Chain of structure returns - forces address reloads */
        ss = func1(ss);
        ss = func2(ss);
        ss.a += i;
        ss.b ^= i;
    }
    
    /* Pattern 5: Vector operations */
    v4si vec1 = {1, 2, 3, 4};
    v4si vec2 = {5, 6, 7, 8};
    v4si vec3;
    
    /* Vector operations that might need decomposition */
    vec3 = vec1 + vec2;
    vec3 = vec3 * vec1;
    
    /* Use __builtin_shuffle for complex patterns */
    v4si shuffled = __builtin_shuffle(vec1, vec2, 
        (v4si){3, 2, 1, 0});  /* Reverse order */
    
    /* Pattern 6: Complex control flow */
    int control_arr[50];
    for (int i = 0; i < 50; i++) {
        control_arr[i] = i * 3 + 1;
    }
    
    complex_control_flow(control_arr, 50);
    
    /* Final checksum computation to prevent optimization */
    for (int i = 0; i < 20; i++) {
        for (int j = 0; j < 15; j++) {
            checksum ^= arr[i][j];
        }
    }
    
    for (int i = 0; i < 8; i++) {
        checksum += ls1.data[i];
        checksum -= ls2.data[i];
    }
    
    checksum ^= asm_result1;
    checksum += ss.a + ss.b + ss.c + ss.d;
    
    for (int i = 0; i < 4; i++) {
        checksum ^= shuffled[i];
    }
    
    for (int i = 0; i < 50; i++) {
        checksum += control_arr[i];
    }
    
    printf("Checksum: %d\n", checksum);
    return 0;
}
