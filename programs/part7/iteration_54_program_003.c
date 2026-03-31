/* Compile with: gcc -O3 -fno-inline -fomit-frame-pointer -march=x86-64 -mno-sse -o test_reload test_reload.c */

#include <stdio.h>
#include <stdint.h>

/* Pattern 1: Complex addressing modes with volatile indices */
void complex_addressing(int arr[][100], volatile int idx1, volatile int idx2) {
    for (int i = 0; i < 10; i++) {
        /* This forces RELOAD_FOR_INPUT_ADDRESS and RELOAD_FOR_OUTPUT_ADDRESS */
        arr[idx1 + i][idx2 * 2] = arr[idx2][idx1 + i * 3] + arr[i][idx1 * idx2];
        
        /* More complex addressing with pointer arithmetic */
        int (*ptr)[100] = &arr[idx1];
        ptr[i][idx2] = ptr[idx2][i] * 2;
    }
}

/* Pattern 2: Structure passing by value */
struct SmallStruct {
    int a, b, c, d;
};

struct SmallStruct process_struct(struct SmallStruct s1, struct SmallStruct s2) {
    /* This can trigger RELOAD_FOR_INPADDR_ADDRESS and RELOAD_FOR_OUTADDR_ADDRESS */
    struct SmallStruct result;
    result.a = s1.a + s2.b;
    result.b = s1.b - s2.c;
    result.c = s1.c * s2.d;
    result.d = s1.d / (s2.a ? s2.a : 1);
    return result;
}

struct SmallStruct chain_struct_calls(struct SmallStruct s) {
    struct SmallStruct temp = {s.b, s.c, s.d, s.a};
    return process_struct(s, temp);
}

/* Pattern 3: Vector extensions */
typedef int v4si __attribute__((vector_size(16)));

v4si vector_operations(v4si a, v4si b) {
    /* Complex vector operations that may need decomposition */
    v4si c = __builtin_shufflevector(a, b, 0, 2, 4, 6);
    v4si d = __builtin_shufflevector(a, b, 1, 3, 5, 7);
    return c + d * 2 - a / (b != 0 ? b : (v4si){1,1,1,1});
}

/* Pattern 4: Large local arrays to increase register pressure */
void high_register_pressure(volatile int trigger) {
    /* Large arrays that won't fit in registers */
    int big_array1[200];
    int big_array2[200];
    struct { int x[50]; char y[100]; } big_struct;
    
    /* Complex control flow with goto to split live ranges */
    int sum = 0;
    
    if (trigger > 0) {
        goto compute;
    } else {
        goto init;
    }
    
init:
    for (int i = 0; i < 200; i++) {
        big_array1[i] = i * 2;
        big_array2[i] = i * 3;
    }
    trigger = 1;
    
compute:
    /* This mixing of operations stresses various reload types */
    for (int i = 0; i < 100; i++) {
        /* RELOAD_FOR_OPERAND_ADDRESS and RELOAD_FOR_OPADDR_ADDR */
        int *ptr1 = &big_array1[i * 2];
        int *ptr2 = &big_array2[i + trigger];
        
        /* Complex addressing */
        big_struct.x[i % 50] = *ptr1 + *ptr2;
        
        /* More pointer arithmetic */
        int *ptr3 = ptr1 + (trigger % 10);
        int *ptr4 = ptr2 - (trigger % 5);
        
        sum += *ptr3 - *ptr4;
        
        /* Jump to create complex CFG */
        if (i % 20 == 0) {
            goto extra_calc;
        }
        continue;
        
    extra_calc:
        sum += big_struct.x[i % 50] * 2;
    }
    
    /* Use sum to prevent optimization */
    big_array1[0] = sum;
}

/* Pattern 5: Inline assembly with multiple constraints */
void inline_asm_patterns(volatile int *input, volatile int *output) {
    int temp1, temp2, temp3;
    
    /* First asm: output constraint */
    asm volatile (
        "movl %1, %0\n\t"
        "addl $1, %0"
        : "=r" (temp1)
        : "m" (*input)
        : "cc"
    );
    
    /* Second asm: input from previous output, memory output */
    asm volatile (
        "imull %2, %1\n\t"
        "movl %1, %0"
        : "=m" (*output), "=r" (temp2)
        : "r" (temp1), "1" (temp1)
        : "cc"
    );
    
    /* Third asm: multiple constraints for RELOAD_OTHER */
    asm volatile (
        "leal (%1, %2, 2), %0"
        : "=r" (temp3)
        : "r" (temp2), "r" (temp1)
        : "cc"
    );
    
    /* Chain them together */
    asm volatile (
        "addl %1, %0\n\t"
        "subl %2, %0"
        : "+r" (temp3)
        : "r" (temp2), "r" (temp1)
        : "cc"
    );
    
    *output += temp3;
}

int main() {
    volatile int idx1 = 5, idx2 = 7;
    int array[100][100];
    int checksum = 0;
    
    /* Initialize array */
    for (int i = 0; i < 100; i++) {
        for (int j = 0; j < 100; j++) {
            array[i][j] = i * 100 + j;
        }
    }
    
    /* Pattern 1: Complex addressing */
    complex_addressing(array, idx1, idx2);
    
    /* Pattern 2: Structure passing */
    struct SmallStruct s1 = {1, 2, 3, 4};
    struct SmallStruct s2 = {5, 6, 7, 8};
    for (int i = 0; i < 10; i++) {
        s1 = chain_struct_calls(s1);
        s2 = process_struct(s2, s1);
    }
    checksum += s1.a + s2.b;
    
    /* Pattern 3: Vector operations */
    v4si vec1 = {1, 2, 3, 4};
    v4si vec2 = {5, 6, 7, 8};
    for (int i = 0; i < 5; i++) {
        vec1 = vector_operations(vec1, vec2);
        vec2 = vector_operations(vec2, vec1);
    }
    checksum += vec1[0] + vec2[1];
    
    /* Pattern 4: High register pressure with complex control flow */
    high_register_pressure(idx1);
    
    /* Pattern 5: Inline assembly patterns */
    volatile int asm_input = 42;
    volatile int asm_output = 0;
    for (int i = 0; i < 3; i++) {
        inline_asm_patterns(&asm_input, &asm_output);
        asm_input += asm_output;
    }
    checksum += asm_output;
    
    /* Final checksum from array */
    for (int i = 0; i < 10; i++) {
        for (int j = 0; j < 10; j++) {
            checksum += array[i][j];
        }
    }
    
    printf("Checksum: %d\n", checksum);
    return 0;
}
