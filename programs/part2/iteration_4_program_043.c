/* Target: MIPS architecture with delay slots */
#ifndef __mips__
#error "This code is designed for MIPS architecture with delay slots. Compile with -march=mips"
#endif

#include <stdio.h>
#include <stdlib.h>

/* Force delay slot usage */
#define LIKELY(x) __builtin_expect(!!(x), 1)
#define UNLIKELY(x) __builtin_expect(!!(x), 0)

/* Avoid optimization removing our labels */
#define KEEP_LABEL(label) asm volatile ("#" #label ":" ::: "memory")

/* Resource separation - use different register sets */
static int __attribute__((used)) resource_set_a[4] = {0};
static int __attribute__((used)) resource_set_b[4] = {0};
static int __attribute__((used)) resource_set_c[4] = {0};

/* Non-trapping arithmetic operations */
static inline int safe_add(int a, int b) {
    return a + b;  /* Never traps for integers */
}

static inline int safe_xor(int a, int b) {
    return a ^ b;  /* Bitwise ops never trap */
}

/* Main computational kernel with label-oriented jumps */
void __attribute__((noinline, optimize("O2"))) 
compute_kernel(int* arr1, int* arr2, int size) {
    int i;
    int acc_a = 0, acc_b = 0, acc_c = 0;
    int temp1, temp2, temp3;
    
    /* Initialize resource sets with distinct values */
    for (int k = 0; k < 4; k++) {
        resource_set_a[k] = k * 2;
        resource_set_b[k] = k * 3;
        resource_set_c[k] = k * 5;
    }
    
    /* Complex loop with multiple jump-to-label patterns */
    for (i = 0; i < size; i++) {
        /* Pattern 1: Conditional jump to label with simple follower */
        if (UNLIKELY(arr1[i] < 0)) {
            goto label_neg;
        }
        
        /* Pattern 2: Another conditional jump pattern */
        if (LIKELY(arr1[i] % 2 == 0)) {
            goto label_even;
        }
        
        /* Default path */
        acc_a = safe_add(acc_a, arr1[i]);
        continue;
        
    /* Label with simple, non-trapping, splittable operation */
    label_neg:
        KEEP_LABEL(label_neg);
        /* This is the candidate for delay slot filling */
        /* Uses resource_set_b, distinct from resource_set_a used above */
        temp1 = resource_set_b[0] + resource_set_b[1];
        acc_b = safe_xor(acc_b, temp1);
        continue;
        
    label_even:
        KEEP_LABEL(label_even);
        /* Another candidate - uses resource_set_c */
        temp2 = resource_set_c[2] - resource_set_c[1];
        acc_c = safe_add(acc_c, temp2);
        
        /* Nested conditional with another jump */
        if (arr2[i] > 100) {
            goto label_large;
        }
        continue;
        
    label_large:
        KEEP_LABEL(label_large);
        /* Yet another candidate */
        temp3 = resource_set_a[3] & resource_set_a[2];
        acc_a = safe_xor(acc_a, temp3);
    }
    
    /* Force use of results */
    asm volatile ("" : : "r"(acc_a), "r"(acc_b), "r"(acc_c));
}

/* Additional patterns in different functions to increase coverage */
void __attribute__((noinline)) pattern_switch(int x) {
    /* Switch with computed goto-like behavior */
    static void* jump_table[] = { &&case0, &&case1, &&case2, &&default_case };
    
    if (x >= 0 && x <= 2) {
        goto *jump_table[x];
    } else {
        goto default_case;
    }
    
case0:
    KEEP_LABEL(case0);
    /* Delay slot candidate */
    resource_set_a[0] = safe_add(resource_set_a[0], 1);
    return;
    
case1:
    KEEP_LABEL(case1);
    resource_set_b[1] = safe_xor(resource_set_b[1], 0xFF);
    return;
    
case2:
    KEEP_LABEL(case2);
    resource_set_c[2] = resource_set_c[2] * 2;  /* Multiplication is safe */
    return;
    
default_case:
    KEEP_LABEL(default_case);
    resource_set_a[3] = resource_set_b[3] | resource_set_c[3];
    return;
}

/* Loop with multiple exit points using goto */
int __attribute__((noinline)) complex_loop(int limit) {
    int i = 0;
    int sum = 0;
    
start_loop:
    KEEP_LABEL(start_loop);
    if (i >= limit) goto loop_end;
    
    /* Multiple conditional jumps to different labels */
    if ((i % 3) == 0) goto handle_mult3;
    if ((i % 5) == 0) goto handle_mult5;
    
    /* Default path */
    sum = safe_add(sum, i);
    i++;
    goto start_loop;
    
handle_mult3:
    KEEP_LABEL(handle_mult3);
    /* Candidate for delay slot - uses distinct resources */
    resource_set_a[i % 4] = safe_add(resource_set_a[i % 4], i);
    sum = safe_xor(sum, i);
    i++;
    goto start_loop;
    
handle_mult5:
    KEEP_LABEL(handle_mult5);
    resource_set_b[i % 4] = resource_set_b[i % 4] - i;
    sum = safe_add(sum, i * 2);
    i++;
    goto start_loop;
    
loop_end:
    return sum;
}

/* Main function with mixed operations */
int main() {
    const int SIZE = 1000;
    int* array1 = malloc(SIZE * sizeof(int));
    int* array2 = malloc(SIZE * sizeof(int));
    
    if (!array1 || !array2) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with pattern that creates branch diversity */
    for (int i = 0; i < SIZE; i++) {
        array1[i] = (i * 17) % 100;
        array2[i] = (i * 23) % 200;
    }
    
    /* Memory barrier to constrain scheduling */
    __sync_synchronize();
    
    /* Execute kernels with different patterns */
    compute_kernel(array1, array2, SIZE);
    
    /* Mix integer and simple operations in different contexts */
    for (int i = 0; i < 100; i++) {
        pattern_switch(i % 4);
    }
    
    int result = complex_loop(500);
    
    /* Use results to prevent dead code elimination */
    printf("Result: %d\n", result);
    printf("Resource checks: %d %d %d\n", 
           resource_set_a[0], resource_set_b[1], resource_set_c[2]);
    
    free(array1);
    free(array2);
    
    return 0;
}
