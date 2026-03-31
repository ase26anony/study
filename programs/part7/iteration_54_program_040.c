/* Test program to trigger various reload types in GCC's reload1.cc */
#include <stdio.h>
#include <stdint.h>

/* Volatile indices to prevent constant folding */
volatile int vi1 = 1, vi2 = 2, vi3 = 3, vi4 = 4;

/* Structure for passing by value */
struct SmallStruct {
    int a, b, c, d;
};

/* Vector type for GCC extensions */
typedef int v4si __attribute__((vector_size(16)));

/* Function to create structure passing reloads */
struct SmallStruct process_struct(struct SmallStruct s1, struct SmallStruct s2) {
    struct SmallStruct result;
    result.a = s1.a + s2.b;
    result.b = s1.b - s2.c;
    result.c = s1.c * s2.d;
    result.d = s1.d / (s2.a ? s2.a : 1);
    return result;
}

/* Another function for chaining structure calls */
struct SmallStruct chain_struct(struct SmallStruct s) {
    struct SmallStruct temp = {vi1, vi2, vi3, vi4};
    return process_struct(s, temp);
}

/* Complex addressing with multi-dimensional arrays */
void complex_addressing(int size) {
    /* Large local arrays to increase register pressure */
    int arr1[100][50];
    int arr2[75][60];
    int arr3[80][55];
    
    /* Initialize with volatile indices */
    for (int i = 0; i < 50; i++) {
        for (int j = 0; j < 30; j++) {
            arr1[i][j] = i * j + vi1;
            arr2[i][j] = i + j * vi2;
            arr3[i][j] = (i << 2) + (j >> 1) + vi3;
        }
    }
    
    /* Complex addressing patterns */
    for (int iter = 0; iter < 10; iter++) {
        /* RELOAD_FOR_INPUT_ADDRESS and RELOAD_FOR_OUTPUT_ADDRESS */
        arr1[vi1 + iter][vi2 * 2] = arr2[iter][vi3 + vi4] + arr3[vi4][iter * 3];
        
        /* More complex addressing with pointer arithmetic */
        int *ptr1 = &arr1[vi1][iter];
        int *ptr2 = &arr2[iter][vi2];
        int *ptr3 = &arr3[vi3][vi4];
        
        /* Chain of operations requiring address reloads */
        *ptr1 = *ptr2 + *ptr3;
        *(ptr1 + vi1) = *(ptr2 - vi2) * *(ptr3 + vi3);
        
        /* RELOAD_FOR_OPERAND_ADDRESS patterns */
        arr1[iter][vi1] = arr2[vi2][iter] + arr1[vi3][vi4];
        arr2[vi4][iter] = arr3[iter][vi1] - arr2[vi2][vi3];
    }
}

/* Inline assembly to force specific reload types */
void inline_asm_reloads(void) {
    int a = vi1, b = vi2, c = vi3, d = vi4;
    int result1, result2, result3;
    
    /* Chain of asm blocks creating dependencies */
    
    /* First asm: output used as input in next */
    asm volatile (
        "mov %[in1], %[out1]\n\t"
        "add %[in2], %[out1]"
        : [out1] "=r" (result1)
        : [in1] "r" (a), [in2] "r" (b)
        : "cc"
    );
    
    /* Second asm: uses previous output, creates new output */
    asm volatile (
        "imul %[in1], %[out1]\n\t"
        "sub %[in2], %[out1]"
        : [out1] "=r" (result2)
        : [in1] "r" (result1), [in2] "r" (c)
        : "cc"
    );
    
    /* Third asm: memory operand with register constraints */
    asm volatile (
        "mov %[in1], %%eax\n\t"
        "add %%eax, %[out1]\n\t"
        "mov %[out1], %[mem]"
        : [out1] "=r" (result3), [mem] "=m" (d)
        : [in1] "r" (result2)
        : "eax", "cc"
    );
    
    /* Mixed constraints to force various reloads */
    int temp = vi1 + vi2;
    asm volatile (
        "lea (%[base], %[index], 4), %[out]\n\t"
        "add $1, %[out]"
        : [out] "=r" (temp)
        : [base] "r" (vi3), [index] "r" (vi4)
        : "cc"
    );
}

/* Vector operations for GCC extensions */
void vector_operations(void) {
    v4si vec1 = {vi1, vi2, vi3, vi4};
    v4si vec2 = {vi4, vi3, vi2, vi1};
    v4si vec3, vec4;
    
    /* Basic vector operations */
    vec3 = vec1 + vec2;
    vec4 = vec1 * vec2;
    
    /* Shuffle to create complex patterns */
    vec3 = __builtin_shuffle(vec1, vec2, 
        (v4si){3, 2, 1, 0});  /* Reverse order */
    
    /* Mixed scalar/vector operations */
    int *p = (int*)&vec3;
    for (int i = 0; i < 4; i++) {
        p[i] += vi1 + i;
    }
    
    /* Store/load with complex addressing */
    v4si vec_array[10];
    vec_array[vi1] = vec1 + vec2;
    vec_array[vi2] = vec3 * vec4;
    
    /* Use result to prevent optimization */
    volatile v4si sink = vec_array[vi1];
    (void)sink;
}

/* Complex control flow to split live ranges */
void split_live_ranges(int iterations) {
    int a = vi1, b = vi2, c = vi3, d = vi4;
    int result = 0;
    
    /* goto to create complex CFG */
    int i = 0;
    
start_loop:
    if (i >= iterations) goto end_loop;
    
    /* Different paths based on complex condition */
    if ((a * i + b) % 3 == 0) {
        /* Path 1 */
        a += c;
        b -= d;
        goto path1_continue;
    } else if ((b * i + c) % 5 == 0) {
        /* Path 2 */
        c *= a;
        d /= (b ? b : 1);
        goto path2_continue;
    } else {
        /* Path 3 */
        a ^= b;
        b |= c;
        c &= d;
        goto path3_continue;
    }

path1_continue:
    result += a * 2;
    i++;
    goto start_loop;

path2_continue:
    result += b * 3;
    i++;
    goto start_loop;

path3_continue:
    result += c * 4;
    i++;
    goto start_loop;

end_loop:
    /* Use result to prevent optimization */
    volatile int sink = result;
    (void)sink;
}

/* Main function orchestrating all patterns */
int main(void) {
    int checksum = 0;
    
    /* 1. Structure passing reloads */
    struct SmallStruct s1 = {vi1, vi2, vi3, vi4};
    struct SmallStruct s2 = {vi4, vi3, vi2, vi1};
    
    for (int i = 0; i < 5; i++) {
        s1 = chain_struct(s1);
        s2 = process_struct(s2, s1);
        checksum += s1.a + s2.b - s1.c + s2.d;
    }
    
    /* 2. Complex addressing patterns */
    complex_addressing(50);
    
    /* 3. Inline assembly reloads */
    for (int i = 0; i < 3; i++) {
        inline_asm_reloads();
        checksum += vi1 + i;
    }
    
    /* 4. Vector operations */
    vector_operations();
    
    /* 5. Split live ranges */
    split_live_ranges(20);
    
    /* 6. Additional mixed patterns */
    {
        /* Large local variable to increase pressure */
        int big_array[200];
        for (int i = 0; i < 200; i++) {
            big_array[i] = i + vi1;
        }
        
        /* Complex addressing with the array */
        for (int i = 0; i < 100; i++) {
            int idx1 = (i * vi2) % 200;
            int idx2 = (i * vi3) % 200;
            int idx3 = (i * vi4) % 200;
            
            big_array[idx1] = big_array[idx2] + big_array[idx3];
            checksum += big_array[idx1];
        }
    }
    
    /* Final checksum output */
    printf("Checksum: %d\n", checksum);
    
    return 0;
}
