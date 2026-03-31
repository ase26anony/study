/* Test program to trigger selective scheduler debug dumps in GCC */
#include <stdio.h>
#include <stdlib.h>

/* Volatile variables to prevent optimization */
volatile int g_volatile_counter = 0;
volatile int g_volatile_array[256];

/* Function 1: Inner loop with conditional branch and memory write */
void test_inner_loop(int n, int *arr) {
    int i, j;
    for (i = 0; i < n; i++) {
        int sum = 0;
        /* Inner loop with data dependency */
        for (j = 0; j < 100; j++) {
            sum += arr[j % n];
            /* Create anti-dependency */
            asm volatile ("" : : "r"(sum) : "memory");
        }
        
        /* Conditional store with side effect */
        if (sum > 1000) {
            arr[i % n] = sum / 2;
            g_volatile_counter++;
        } else {
            arr[i % n] = sum * 2;
            /* Force memory barrier */
            asm volatile ("" : : : "memory");
        }
        
        /* Complex condition with multiple branches */
        if (i % 3 == 0) {
            g_volatile_array[i % 256] = sum;
        } else if (i % 3 == 1) {
            g_volatile_array[(i + 1) % 256] = sum + 1;
        } else {
            g_volatile_array[(i + 2) % 256] = sum + 2;
        }
    }
}

/* Function 2: Nested loops with different iteration counts */
int test_nested_loops(int rows, int cols, int (*matrix)[64]) {
    int total = 0;
    int i, j, k;
    
    /* Outer loop with variable bound */
    for (i = 0; i < rows; i++) {
        /* Middle loop */
        for (j = 0; j < cols; j++) {
            int prod = 1;
            /* Innermost loop with computation */
            for (k = 0; k < 8; k++) {
                prod *= (matrix[i][j] + k);
                /* Prevent reordering */
                asm volatile ("" : "+r"(prod) : : "memory");
            }
            
            total += prod;
            
            /* Conditional with early exit */
            if (total > 1000000) {
                goto early_exit;
            }
        }
        
        /* Function call within loop to create basic block boundaries */
        if (i % 10 == 0) {
            g_volatile_counter = i;
        }
    }
    
early_exit:
    return total;
}

/* Function 3: Switch statement with computed goto for complex control flow */
int test_switch_complex(int x) {
    static void *labels[] = { &&case0, &&case1, &&case2, &&case3, &&default_case };
    int result = 0;
    
    /* Indirect jump through array */
    goto *labels[x % 5];
    
case0:
    result = x * 2;
    /* Loop with multiple exits */
    for (int i = 0; i < 10; i++) {
        if (i == x) break;
        result += i;
        if (result > 50) goto end;
    }
    goto end;
    
case1:
    result = x * 3;
    /* Nested switch */
    switch (x % 3) {
        case 0: result += 1; break;
        case 1: result += 2; break;
        case 2: result += 3; break;
    }
    goto end;
    
case2:
    result = x * 4;
    /* Do-while loop */
    do {
        result--;
        asm volatile ("" : : "r"(result) : "memory");
    } while (result > 0);
    goto end;
    
case3:
    result = x * 5;
    /* Multiple conditions */
    if (x > 10 && x < 20) {
        result += 100;
    } else if (x >= 20 || x < 5) {
        result += 200;
    } else {
        result += 300;
    }
    goto end;
    
default_case:
    result = x;
    /* Loop with continue */
    for (int i = 0; i < 8; i++) {
        if (i % 2 == 0) continue;
        result += i * i;
    }
    /* Fall through */
    
end:
    return result;
}

/* Function 4: Mixed operations with pointer chasing */
int test_pointer_chasing(int *base, int steps) {
    int *ptr = base;
    int sum = 0;
    
    for (int i = 0; i < steps; i++) {
        /* Pointer arithmetic with dependency chain */
        sum += *ptr;
        ptr = base + (sum % 16);
        
        /* Memory barrier */
        asm volatile ("" : : : "memory");
        
        /* Conditional based on sum */
        if (sum & 1) {
            *ptr = sum >> 1;
        } else {
            *(ptr + 1) = sum >> 2;
        }
        
        /* Volatile access */
        g_volatile_counter = sum;
    }
    
    return sum;
}

/* Function 5: Recursive-like pattern using loops */
void test_data_dependent_loop(int n, int *out) {
    int i = 0;
    int acc = 1;
    
    while (i < n) {
        /* Data-dependent loop condition */
        int limit = (acc % 7) + 3;
        
        for (int j = 0; j < limit; j++) {
            acc = (acc * 13 + 17) % 100;
            out[i % 64] = acc;
            
            /* Inline asm to create specific RTL */
            asm volatile ("# Dependency chain" : "+r"(acc) : : "memory");
        }
        
        i += (acc % 3) + 1;
        
        /* Branch with side effect */
        if (acc > 50) {
            g_volatile_array[i % 256] = acc;
        }
    }
}

/* Main driver to ensure all functions are compiled */
int main(int argc, char **argv) {
    int arr1[100];
    int matrix[32][64];
    int arr2[256];
    
    /* Initialize arrays */
    for (int i = 0; i < 100; i++) arr1[i] = i;
    for (int i = 0; i < 32; i++)
        for (int j = 0; j < 64; j++)
            matrix[i][j] = i * j;
    for (int i = 0; i < 256; i++) arr2[i] = i * 2;
    
    /* Call all test functions */
    test_inner_loop(50, arr1);
    
    int result1 = test_nested_loops(16, 32, matrix);
    
    int result2 = test_switch_complex(argc);
    
    int result3 = test_pointer_chasing(arr2, 100);
    
    test_data_dependent_loop(200, arr2);
    
    /* Use results to prevent dead code elimination */
    printf("Results: %d %d %d %d\n", 
           result1, result2, result3, g_volatile_counter);
    
    return 0;
}
