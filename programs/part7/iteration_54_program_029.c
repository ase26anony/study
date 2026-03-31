/* Compile with: gcc -O3 -fno-inline -fomit-frame-pointer -march=x86-64 -mno-sse -S -o test.s test.c */

#include <stdio.h>
#include <stdint.h>

/* Pattern 1: Complex addressing modes with volatile indices */
volatile int idx1 = 3, idx2 = 7, idx3 = 11;

/* Pattern 2: Vector extensions */
typedef int v4si __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));

/* Pattern 3: Structure for value passing */
struct DataPair {
    int a;
    int b;
    int c;
    int d;
};

/* Pattern 4: Function to force structure passing reloads */
struct DataPair process_pair(struct DataPair p1, struct DataPair p2) {
    struct DataPair result;
    result.a = p1.a + p2.b;
    result.b = p1.b - p2.a;
    result.c = p1.c * p2.c;
    result.d = p1.d ^ p2.d;
    return result;
}

/* Pattern 5: Another function with different structure layout */
struct MixedData {
    char c;
    int i;
    short s;
    long l;
};

struct MixedData process_mixed(struct MixedData m1, struct MixedData m2) {
    struct MixedData result;
    result.c = m1.c + m2.c;
    result.i = m1.i - m2.i;
    result.s = m1.s * m2.s;
    result.l = m1.l ^ m2.l;
    return result;
}

/* Pattern 6: Function with inline assembly chains */
void asm_chain_operations(int *a, int *b, int *c) {
    int temp1, temp2, temp3;
    
    /* First asm: output used as input in next */
    asm volatile (
        "movl %1, %0\n\t"
        "addl $5, %0"
        : "=r" (temp1)
        : "m" (*a)
        : "cc"
    );
    
    /* Second asm: complex constraints */
    asm volatile (
        "imull %2, %1\n\t"
        "addl %1, %0"
        : "=r" (temp2), "+r" (temp1)
        : "rm" (*b)
        : "cc"
    );
    
    /* Third asm: memory output */
    asm volatile (
        "movl %1, %0\n\t"
        "xorl %%eax, %0"
        : "=m" (*c)
        : "r" (temp2)
        : "eax", "cc"
    );
}

int main() {
    int checksum = 0;
    
    /* Pattern 1: Multi-dimensional array with volatile indices */
    int arr[100][50];
    
    /* Initialize array */
    for (int i = 0; i < 100; i++) {
        for (int j = 0; j < 50; j++) {
            arr[i][j] = i * 100 + j;
        }
    }
    
    /* Complex addressing with volatile indices - forces address reloads */
    for (int i = 0; i < 10; i++) {
        /* RELOAD_FOR_INPUT_ADDRESS, RELOAD_FOR_OUTPUT_ADDRESS */
        arr[idx1 + i][idx2 * 2] = arr[idx3][idx1 * i + idx2];
        
        /* More complex addressing */
        arr[(idx1 * i) % 50][(idx2 + i * 3) % 30] = 
            arr[(idx3 - i) % 40][(idx1 + idx2 * i) % 20];
    }
    
    /* Pattern 2: Vector operations */
    v4si vec1 = {1, 2, 3, 4};
    v4si vec2 = {5, 6, 7, 8};
    v4si vec3;
    
    /* Vector operations that might need decomposition */
    vec3 = vec1 + vec2;
    vec3 = vec3 * vec1;
    
    /* Use __builtin_shuffle for complex patterns */
    v4si shuffled = __builtin_shuffle(vec1, vec2, 
        (v4si){3, 1, 2, 0});
    
    /* Pattern 3 & 4: Structure passing chains */
    struct DataPair dp1 = {1, 2, 3, 4};
    struct DataPair dp2 = {5, 6, 7, 8};
    struct DataPair dp3, dp4;
    
    /* Chain of structure returns - forces operand address reloads */
    dp3 = process_pair(dp1, dp2);
    dp4 = process_pair(dp3, dp1);
    
    /* Pattern 5: Mixed structure */
    struct MixedData md1 = {'a', 100, 20, 1000};
    struct MixedData md2 = {'b', 200, 30, 2000};
    struct MixedData md3;
    
    md3 = process_mixed(md1, md2);
    
    /* Pattern 6: Inline assembly with complex constraints */
    int x = 10, y = 20, z = 0;
    asm_chain_operations(&x, &y, &z);
    
    /* Pattern 7: Control flow splitting live ranges */
    int value1 = 0, value2 = 0, value3 = 0;
    
    /* Complex control flow with gotos */
    for (int i = 0; i < 100; i++) {
        if (i % 3 == 0) {
            value1 = arr[i][0];
            goto label1;
        } else if (i % 3 == 1) {
            value2 = arr[i][1];
            goto label2;
        } else {
            value3 = arr[i][2];
            goto label3;
        }
        
    label1:
        checksum += value1;
        continue;
        
    label2:
        checksum += value2 * 2;
        continue;
        
    label3:
        checksum += value3 * 3;
        continue;
    }
    
    /* More complex addressing in loop */
    int *ptr_arr[100];
    for (int i = 0; i < 100; i++) {
        ptr_arr[i] = &arr[i][0];
    }
    
    /* Pointer arithmetic that can't be folded */
    for (int i = 1; i < 99; i++) {
        int *p1 = ptr_arr[i - 1] + idx1;
        int *p2 = ptr_arr[i] + idx2;
        int *p3 = ptr_arr[i + 1] + idx3;
        
        *p2 = *p1 + *p3;
    }
    
    /* Final checksum computation using all patterns */
    checksum += dp3.a + dp3.b + dp3.c + dp3.d;
    checksum += md3.i + md3.s;
    checksum += vec3[0] + vec3[1] + vec3[2] + vec3[3];
    checksum += shuffled[0] + shuffled[1];
    checksum += z;
    
    /* Use array elements with complex addressing */
    checksum += arr[idx1][idx2];
    checksum += arr[idx2 * 2][idx3 / 2];
    checksum += arr[(idx1 + idx2) % 50][(idx3 * 3) % 30];
    
    printf("Checksum: %d\n", checksum);
    
    return 0;
}
