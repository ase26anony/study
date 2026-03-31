/* Test program to trigger various reload types in GCC's reload1.cc */
#include <stdio.h>
#include <stdint.h>

/* Volatile indices to prevent constant folding */
volatile int vi1 = 1, vi2 = 2, vi3 = 3, vi4 = 4;

/* Structure for passing by value */
struct SmallStruct {
    int a;
    int b;
    int c;
    int d;
};

/* Vector type to stress register allocation */
typedef int v4si __attribute__((vector_size(16)));

/* Function to pass/return structures by value */
struct SmallStruct process_struct(struct SmallStruct s1, struct SmallStruct s2) {
    struct SmallStruct result;
    result.a = s1.a + s2.b;
    result.b = s1.b + s2.c;
    result.c = s1.c + s2.d;
    result.d = s1.d + s2.a;
    
    /* Complex addressing with volatile indices */
    volatile int local_arr[10][10];
    local_arr[vi1][vi2] = s1.a;
    local_arr[vi3][vi4] = s2.b;
    
    return result;
}

/* Another function to create call chain */
struct SmallStruct chain_struct(struct SmallStruct s) {
    struct SmallStruct temp = {vi1, vi2, vi3, vi4};
    return process_struct(s, temp);
}

/* Function with inline assembly blocks */
void asm_reload_test(int *arr, int size) {
    int i, j, k;
    
    /* Multiple inline asm blocks with dependencies */
    for (i = 0; i < size; i++) {
        int tmp1, tmp2, tmp3;
        
        /* First asm: output used as input in next */
        asm volatile (
            "movl %[input], %[output1]\n\t"
            "addl $100, %[output1]"
            : [output1] "=r" (tmp1)
            : [input] "r" (arr[i])
            : "cc"
        );
        
        /* Second asm: uses previous output, produces new output */
        asm volatile (
            "imull %[in1], %[out1]\n\t"
            "movl %[out1], %[out2]"
            : [out1] "=r" (tmp2), [out2] "=r" (tmp3)
            : [in1] "r" (tmp1), "0" (tmp2)
            : "cc"
        );
        
        /* Third asm: memory constraint */
        asm volatile (
            "addl %%eax, %[mem]\n\t"
            : [mem] "+m" (arr[i])
            : "a" (tmp3)
            : "cc"
        );
    }
}

/* Function with complex array addressing */
void complex_addressing(int size) {
    /* Large local array to increase register pressure */
    int big_array[100][100];
    volatile int *volatile_ptr = (volatile int *)big_array;
    int i, j, k;
    
    /* Initialize with volatile indices */
    for (i = 0; i < 10; i++) {
        for (j = 0; j < 10; j++) {
            big_array[i][j] = i * 100 + j;
        }
    }
    
    /* Complex addressing patterns that need address reloads */
    for (k = 0; k < 5; k++) {
        /* RELOAD_FOR_INPUT_ADDRESS and RELOAD_FOR_OUTPUT_ADDRESS */
        big_array[vi1 + k][vi2 * k] = big_array[vi3 - k][vi4 / (k + 1)];
        
        /* More complex: nested array accesses with computation */
        big_array[big_array[k][0] % 10][big_array[0][k] % 10] = 
            big_array[vi1][vi2] + big_array[vi3][vi4];
        
        /* Pointer arithmetic that can't be folded */
        int *ptr1 = &big_array[k][0];
        int *ptr2 = &big_array[0][k];
        volatile_ptr[vi1 * k] = ptr1[vi2] + ptr2[vi3];
    }
    
    /* Vector operations to stress register file */
    v4si vec1 = {1, 2, 3, 4};
    v4si vec2 = {vi1, vi2, vi3, vi4};
    v4si vec3;
    
    /* Vector operations may need decomposition */
    vec3 = vec1 + vec2;
    vec3 = vec3 * vec1;
    
    /* Use __builtin_shuffle for complex pattern */
    vec3 = __builtin_shuffle(vec1, vec2, (v4si){3, 2, 1, 0});
    
    /* Store vector to memory with complex addressing */
    int *vec_store = (int *)&big_array[50][0];
    for (i = 0; i < 4; i++) {
        vec_store[vi1 * i] = vec3[i];
    }
}

/* Function with goto to split live ranges */
int control_flow_test(int n) {
    int a = vi1, b = vi2, c = vi3, d = vi4;
    int result = 0;
    int i = 0;
    
    /* Complex control flow with gotos */
    if (n > 100) goto block1;
    
    for (i = 0; i < n; i++) {
        if (i % 2 == 0) {
            a = b + c;
            goto block2;
        } else {
            c = d + a;
            goto block3;
        }
        
    block2:
        d = a * b;
        continue;
        
    block3:
        b = c * d;
        /* Fall through */
    }
    
    goto finish;
    
block1:
    /* Different path with different computations */
    for (i = 0; i < n; i++) {
        a = a + i;
        b = b - i;
        c = c * (i + 1);
        d = d / (i + 1);
    }
    
finish:
    /* Use all variables to keep them live */
    result = a + b + c + d;
    
    /* Inline asm that clobbers many registers */
    asm volatile (
        "movl %0, %%eax\n\t"
        "addl %1, %%eax\n\t"
        "movl %%eax, %0\n\t"
        : "+r" (result)
        : "r" (vi1)
        : "%eax", "cc"
    );
    
    return result;
}

/* Main orchestrator */
int main() {
    int checksum = 0;
    int i;
    
    /* Test array for inline assembly */
    int test_array[50];
    for (i = 0; i < 50; i++) {
        test_array[i] = i * 2;
    }
    
    /* 1. Test structure passing (triggers address reloads for temporaries) */
    struct SmallStruct s1 = {100, 200, 300, 400};
    struct SmallStruct s2 = {vi1, vi2, vi3, vi4};
    
    for (i = 0; i < 10; i++) {
        s1 = process_struct(s1, s2);
        s2 = chain_struct(s1);
        checksum += s1.a + s2.b;
    }
    
    /* 2. Test inline assembly reload patterns */
    asm_reload_test(test_array, 50);
    for (i = 0; i < 50; i++) {
        checksum += test_array[i];
    }
    
    /* 3. Test complex addressing modes */
    complex_addressing(10);
    
    /* 4. Test control flow with split live ranges */
    checksum += control_flow_test(20);
    
    /* 5. Additional stress: mixed operations in loop */
    volatile int vol_arr[100];
    for (i = 0; i < 100; i++) {
        vol_arr[i] = i;
    }
    
    int *ptr = (int *)vol_arr;
    for (i = 0; i < 90; i++) {
        /* Complex address calculation */
        ptr[vi1 + i] = ptr[vi2 * i] + ptr[vi3 - i];
        
        /* Another inline asm with memory constraints */
        int temp;
        asm volatile (
            "movl (%[addr]), %[val]\n\t"
            "addl %%ecx, %[val]\n\t"
            "movl %[val], (%[addr])"
            : [val] "=&r" (temp)
            : [addr] "r" (&ptr[i]), "c" (i)
            : "memory"
        );
        
        checksum += ptr[i];
    }
    
    /* Final checksum output */
    printf("Checksum: %d\n", checksum);
    
    return 0;
}
