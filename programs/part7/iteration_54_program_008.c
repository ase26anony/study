/* reload_stress_test.c
 * Stress test for GCC's reload pass to cover various reload types
 * Compile with: gcc -O3 -fno-inline -fomit-frame-pointer -mtune=generic reload_stress_test.c -o reload_test
 */

#include <stdio.h>
#include <stdint.h>

/* Technique 1: Complex addressing modes with volatile indices */
volatile int volatile_idx = 3;
volatile int volatile_idx2 = 7;

/* Technique 5: Vector extensions */
typedef int v4si __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));

/* Technique 4: Structure passing */
struct SmallStruct {
    int a, b, c, d;
};

/* Technique 3: Large local arrays */
#define ARRAY_SIZE 100

/* Helper function for structure passing */
struct SmallStruct process_struct(struct SmallStruct s1, struct SmallStruct s2) {
    struct SmallStruct result;
    result.a = s1.a + s2.b;
    result.b = s1.b - s2.a;
    result.c = s1.c * s2.d;
    result.d = s1.d / (s2.c ? s2.c : 1);
    return result;
}

/* Function with complex control flow */
int complex_control_flow(int n) {
    int sum = 0;
    int i = 0;
    
    /* Technique 6: Control flow splitting live ranges */
start_loop:
    if (i >= n) goto end_loop;
    
    /* Mix computations to increase register pressure */
    int temp1 = i * i;
    int temp2 = temp1 + i;
    int temp3 = temp2 * 3;
    
    if (i % 2 == 0) {
        goto even_case;
    } else {
        goto odd_case;
    }
    
even_case:
    sum += temp3;
    i++;
    goto start_loop;
    
odd_case:
    sum -= temp3;
    i += 2;
    goto start_loop;
    
end_loop:
    return sum;
}

int main(void) {
    int checksum = 0;
    
    /* Technique 1: Multi-dimensional array with complex addressing */
    int arr[ARRAY_SIZE][ARRAY_SIZE];
    
    /* Initialize array */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        for (int j = 0; j < ARRAY_SIZE; j++) {
            arr[i][j] = i * 100 + j;
        }
    }
    
    /* Complex array access with volatile indices */
    for (int i = 1; i < ARRAY_SIZE - 1; i++) {
        /* This should trigger address reloads */
        arr[volatile_idx][i + 1] = arr[i][volatile_idx2 * 2];
        arr[i][volatile_idx] = arr[volatile_idx2][i * 3 - 1];
        
        /* Pointer arithmetic that can't be folded */
        int *ptr1 = &arr[i][0];
        int *ptr2 = &arr[0][volatile_idx];
        *(ptr1 + volatile_idx2) = *(ptr2 + i) + *(ptr1 + i * 2);
    }
    
    /* Technique 2: Inline assembly with multiple operands */
    int asm_var1 = 42, asm_var2 = 100, asm_var3 = 0;
    
    /* Chain of asm blocks creating dependencies */
    asm volatile (
        "movl %1, %%eax\n\t"
        "addl %2, %%eax\n\t"
        "movl %%eax, %0\n\t"
        : "=r" (asm_var1)
        : "r" (asm_var2), "r" (asm_var1)
        : "%eax", "memory"
    );
    
    asm volatile (
        "imull %1, %0\n\t"
        "addl $1, %0\n\t"
        : "+r" (asm_var1)
        : "r" (asm_var3)
        : "cc"
    );
    
    asm volatile (
        "leal (%1, %2, 4), %%eax\n\t"
        "movl %%eax, %0\n\t"
        : "=m" (arr[5][5])
        : "r" (asm_var1), "r" (asm_var2)
        : "%eax"
    );
    
    checksum += asm_var1;
    
    /* Technique 3: Large structure operations */
    struct LargeStruct {
        int data[50];
        volatile int volatile_data[10];
        char padding[32];
    };
    
    struct LargeStruct big1, big2;
    
    /* Force partial spilling */
    for (int i = 0; i < 50; i++) {
        big1.data[i] = i * 2;
        big2.data[i] = i * 3;
    }
    
    /* Mix volatile and non-volatile accesses */
    for (int i = 0; i < 10; i++) {
        big1.volatile_data[i] = i;
        checksum += big1.data[i] + big2.data[i];
    }
    
    /* Technique 4: Nested structure passing */
    struct SmallStruct s1 = {1, 2, 3, 4};
    struct SmallStruct s2 = {5, 6, 7, 8};
    struct SmallStruct s3 = {9, 10, 11, 12};
    
    /* Chain of structure operations */
    struct SmallStruct result1 = process_struct(s1, s2);
    struct SmallStruct result2 = process_struct(result1, s3);
    struct SmallStruct final_result = process_struct(result2, s1);
    
    checksum += final_result.a + final_result.b + final_result.c + final_result.d;
    
    /* Technique 5: Vector operations */
    v4si vec1 = {1, 2, 3, 4};
    v4si vec2 = {5, 6, 7, 8};
    v4si vec3 = {9, 10, 11, 12};
    
    /* Complex vector operations */
    v4si vec_result = vec1 + vec2 * vec3;
    vec_result = __builtin_shuffle(vec_result, vec1, (v4si){3, 2, 1, 0});
    
    /* Mix vector and scalar operations */
    int scalar_from_vec = vec_result[0] + vec_result[1] + vec_result[2] + vec_result[3];
    checksum += scalar_from_vec;
    
    /* Technique 6: Complex control flow with register pressure */
    checksum += complex_control_flow(50);
    
    /* Final array computation with mixed addressing modes */
    int * volatile volatile_ptr = &arr[0][0];
    for (int i = 0; i < 1000; i++) {
        /* Complex addressing that should trigger various reloads */
        int idx = (i * 7 + 3) % ARRAY_SIZE;
        int idx2 = (i * 11 + 5) % ARRAY_SIZE;
        
        arr[idx][idx2] += *(volatile_ptr + idx) + arr[idx2][idx];
        
        /* More pointer arithmetic */
        int *ptr = &arr[idx][0];
        for (int j = 0; j < 5; j++) {
            ptr[j] += ptr[j + 1] + volatile_idx;
        }
    }
    
    /* Final checksum computation */
    for (int i = 0; i < ARRAY_SIZE; i += 7) {
        for (int j = 0; j < ARRAY_SIZE; j += 11) {
            checksum += arr[i][j];
            checksum -= arr[j][i];
        }
    }
    
    printf("Checksum: %d\n", checksum);
    return 0;
}
