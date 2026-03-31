/* Test program to trigger various reload types in GCC's reload1.cc */
#include <stdio.h>
#include <stdint.h>

/* Volatile indices to prevent constant folding */
volatile int vi1 = 1, vi2 = 2, vi3 = 3, vi4 = 4;

/* Structure for passing by value */
struct SmallStruct {
    int a, b, c, d;
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
    return result;
}

/* Another function to create call chain */
struct SmallStruct chain_struct(struct SmallStruct s, int iterations) {
    for (int i = 0; i < iterations; i++) {
        s = process_struct(s, s);
    }
    return s;
}

/* Complex addressing with multi-dimensional arrays */
void complex_addressing(int size) {
    /* Large arrays to increase register pressure */
    int arr1[100][100];
    int arr2[100][100];
    
    /* Initialize with volatile indices */
    for (int i = 0; i < size; i++) {
        for (int j = 0; j < size; j++) {
            arr1[i][j] = i * 100 + j;
            arr2[i][j] = 0;
        }
    }
    
    /* Complex addressing patterns that require address reloads */
    for (int i = vi1; i < size - 1; i++) {
        for (int j = vi2; j < size - 2; j++) {
            /* RELOAD_FOR_OUTPUT_ADDRESS and RELOAD_FOR_INPUT_ADDRESS */
            arr2[i + vi1][j + vi2] = arr1[vi3 * i][vi4 * j] 
                                   + arr1[i * 2][j * 3]
                                   + arr1[i + j][i - j];
        }
    }
    
    /* Even more complex with pointer arithmetic */
    int *ptr1 = &arr1[0][0];
    int *ptr2 = &arr2[0][0];
    
    for (int i = 0; i < size * size - 100; i += vi1) {
        /* RELOAD_FOR_OPERAND_ADDRESS and RELOAD_FOR_OPADDR_ADDR */
        ptr2[i + vi2] = ptr1[i * vi3] + ptr1[i + vi4];
    }
}

/* Inline assembly to trigger various reload types */
void inline_asm_operations(int *a, int *b, int *c) {
    int temp1, temp2, temp3;
    
    /* First asm block with output constraints */
    asm volatile (
        "movl %1, %0\n\t"
        "addl %2, %0"
        : "=r" (temp1)
        : "r" (*a), "r" (*b)
        : "cc"
    );
    
    /* Second asm block using previous output as input */
    asm volatile (
        "imull %1, %0\n\t"
        "addl $0x1234, %0"
        : "+r" (temp1)
        : "r" (*c)
        : "cc"
    );
    
    /* Third asm block with memory constraints */
    asm volatile (
        "movl %1, %%eax\n\t"
        "addl %%eax, %0"
        : "=m" (*a)
        : "r" (temp1)
        : "%eax", "cc", "memory"
    );
    
    /* Chain more asm operations */
    asm volatile (
        "leal (%1, %2, 4), %0"
        : "=r" (temp2)
        : "r" (*b), "r" (*c)
    );
    
    asm volatile (
        "movl %1, %0\n\t"
        "rorl $8, %0"
        : "=r" (temp3)
        : "r" (temp2)
        : "cc"
    );
    
    *b = temp3;
}

/* Vector operations to stress register file */
void vector_operations(v4si *v1, v4si *v2, v4si *v3) {
    /* Complex vector operations */
    *v1 = *v1 + *v2 * *v3;
    
    /* Shuffle operation */
    *v2 = __builtin_shuffle(*v1, *v2, (v4si){3, 2, 1, 0});
    
    /* Non-contiguous access pattern */
    int *p = (int*)v1;
    for (int i = 0; i < 4; i += vi1) {
        p[i] = p[(i + 1) % 4] + p[(i + 2) % 4];
    }
}

/* Function with complex control flow */
int complex_control_flow(int n) {
    int x = 0, y = 0, z = 0;
    
    /* Use goto to split live ranges */
    if (n > 0) goto label1;
    
    x = vi1;
    y = vi2;
    z = vi3;
    
    /* Loop with complex exit condition */
    for (int i = 0; i < n; i++) {
        if (i % 2 == 0) {
            x += y * z;
            goto label2;
        }
    label1:
        y += x * z;
        if (i % 3 == 0) {
            z += x * y;
            goto label3;
        }
    label2:
        x += i;
    }
    
label3:
    /* More operations to increase register pressure */
    int arr[50];
    for (int i = 0; i < 50; i++) {
        arr[i] = x + y * i + z * (i * i);
    }
    
    /* Compute checksum */
    int sum = 0;
    for (int i = 0; i < 50; i++) {
        sum += arr[i % n];
    }
    
    return sum;
}

int main() {
    int checksum = 0;
    
    /* 1. Complex addressing mode stress */
    complex_addressing(50);
    
    /* 2. Structure passing chain */
    struct SmallStruct s1 = {vi1, vi2, vi3, vi4};
    struct SmallStruct s2 = {vi4, vi3, vi2, vi1};
    
    for (int i = 0; i < 10; i++) {
        s1 = chain_struct(s1, 3);
        s2 = process_struct(s2, s1);
        checksum += s1.a + s2.b;
    }
    
    /* 3. Inline assembly operations */
    int a = vi1, b = vi2, c = vi3;
    for (int i = 0; i < 100; i++) {
        inline_asm_operations(&a, &b, &c);
        checksum += a + b + c;
    }
    
    /* 4. Vector operations */
    v4si v1 = {1, 2, 3, 4};
    v4si v2 = {5, 6, 7, 8};
    v4si v3 = {9, 10, 11, 12};
    
    vector_operations(&v1, &v2, &v3);
    
    int *vp = (int*)&v1;
    for (int i = 0; i < 4; i++) {
        checksum += vp[i];
    }
    
    /* 5. Complex control flow */
    checksum += complex_control_flow(20);
    
    /* Final checksum output */
    printf("Checksum: %d\n", checksum);
    
    return 0;
}
