/* auto-inc-dec-test.c
 * 
 * This program is designed to generate RTL patterns that trigger the
 * auto-increment/decrement optimization in GCC, specifically targeting
 * the uncovered lines in auto-inc-dec.cc (lines 1352-1358).
 * 
 * The code uses post-increment/decrement pointer arithmetic in various
 * contexts to create opportunities for auto-inc-dec addressing modes.
 */

#include <stdio.h>
#include <string.h>
#include <stdint.h>

/* ========== Function 1: Array copy with post-increment ========== */
/* This creates a tight loop where both source and destination use
 * post-increment operations, likely generating mem+reg addressing. */
void copy_array_postinc(int *dest, const int *src, size_t n) {
    if (n == 0) return;
    
    /* Simple copy loop with post-increment */
    int *d = dest;
    const int *s = src;
    size_t i = 0;
    
    /* Loop with post-increment in the condition */
    while (i++ < n) {
        *d++ = *s++;
    }
}

/* ========== Function 2: Volatile memory access with post-increment ========== */
/* Mixes volatile and non-volatile pointers to test different RTL patterns */
int sum_volatile_array(volatile int *arr, size_t n) {
    int sum = 0;
    volatile int *p = arr;
    
    /* Loop with post-increment in the update statement */
    for (size_t i = 0; i < n; i++) {
        sum += *p++;  /* Volatile access with post-increment */
    }
    
    return sum;
}

/* ========== Function 3: String operations with post-increment ========== */
/* Classic string copy and length functions that use post-increment */
size_t strlen_postinc(const char *str) {
    const char *p = str;
    
    /* While loop with post-increment in condition */
    while (*p++ != '\0') {
        /* Empty body - increment happens in condition */
    }
    
    return (size_t)(p - str - 1);
}

void strcpy_postinc(char *dest, const char *src) {
    char *d = dest;
    const char *s = src;
    
    /* Classic K&R string copy with post-increment */
    while ((*d++ = *s++) != '\0') {
        /* Empty body */
    }
}

/* ========== Function 4: Complex control flow with post-increment ========== */
/* Places post-increment operations in multiple basic blocks */
int process_with_branches(int *data, size_t n, int threshold) {
    int count = 0;
    int *p = data;
    
    for (size_t i = 0; i < n; i++) {
        /* Post-increment in both branches of conditional */
        if (*p > threshold) {
            count += *p++;  /* Taken path */
        } else {
            *p++;  /* Not-taken path - just increment */
        }
    }
    
    return count;
}

/* ========== Function 5: Nested loops with post-increment ========== */
/* Creates complex control flow with inner loop using post-increment */
void matrix_copy(int dest[3][3], const int src[3][3]) {
    for (int i = 0; i < 3; i++) {
        int *d = dest[i];
        const int *s = src[i];
        
        /* Inner loop with post-increment */
        for (int j = 0; j < 3; j++) {
            *d++ = *s++;
        }
    }
}

/* ========== Function 6: Structure access with post-increment ========== */
typedef struct {
    int x;
    int y;
    int z;
} Point3D;

int sum_points(Point3D *points, size_t n) {
    int sum = 0;
    Point3D *p = points;
    
    /* Access structure fields with pointer post-increment */
    for (size_t i = 0; i < n; i++) {
        sum += p->x + p->y + p->z;
        p++;  /* Post-increment after structure access */
    }
    
    return sum;
}

/* ========== Function 7: Comma expressions with post-increment ========== */
/* Uses comma expressions to sequence memory access and increment */
int process_with_comma(int *arr, size_t n) {
    int sum = 0;
    int *p = arr;
    
    for (size_t i = 0; i < n; i++) {
        /* Comma expression: access then increment */
        int val = (*p, p++, val);
        sum += val;
    }
    
    return sum;
}

/* ========== Function 8: Switch statement with post-increment ========== */
int switch_with_postinc(int *data, size_t n) {
    int result = 0;
    int *p = data;
    
    for (size_t i = 0; i < n; i++) {
        switch (*p % 4) {
            case 0:
                result += *p++;  /* Fall through */
            case 1:
                result -= *p++;  /* Different increment in different case */
                break;
            case 2:
                p++;  /* Just increment */
                break;
            default:
                result *= *p++;
                break;
        }
    }
    
    return result;
}

/* ========== Main function ========== */
int main() {
    /* Test data arrays - mix volatile and non-volatile */
    int array1[10] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    volatile int array2[10] = {10, 20, 30, 40, 50, 60, 70, 80, 90, 100};
    int array3[10];
    char str1[50] = "Hello, World!";
    char str2[50];
    int matrix1[3][3] = {{1,2,3},{4,5,6},{7,8,9}};
    int matrix2[3][3];
    Point3D points[5] = {{1,2,3},{4,5,6},{7,8,9},{10,11,12},{13,14,15}};
    
    printf("=== Testing auto-increment/decrement patterns ===\n\n");
    
    /* Test 1: Array copy with post-increment */
    copy_array_postinc(array3, array1, 10);
    printf("Test 1 - Array copy: ");
    for (int i = 0; i < 10; i++) {
        printf("%d ", array3[i]);
    }
    printf("\n");
    
    /* Test 2: Volatile array sum */
    int sum_vol = sum_volatile_array(array2, 10);
    printf("Test 2 - Volatile array sum: %d\n", sum_vol);
    
    /* Test 3: String operations */
    size_t len = strlen_postinc(str1);
    printf("Test 3 - String length: %zu\n", len);
    
    strcpy_postinc(str2, str1);
    printf("Test 3 - String copy result: %s\n", str2);
    
    /* Test 4: Branches with post-increment */
    int branch_result = process_with_branches(array1, 10, 5);
    printf("Test 4 - Branches result: %d\n", branch_result);
    
    /* Test 5: Nested loops (matrix copy) */
    matrix_copy(matrix2, matrix1);
    printf("Test 5 - Matrix copy successful\n");
    
    /* Test 6: Structure access */
    int point_sum = sum_points(points, 5);
    printf("Test 6 - Structure points sum: %d\n", point_sum);
    
    /* Test 7: Comma expressions */
    int comma_result = process_with_comma(array1, 10);
    printf("Test 7 - Comma expression result: %d\n", comma_result);
    
    /* Test 8: Switch statement */
    int switch_result = switch_with_postinc(array1, 10);
    printf("Test 8 - Switch statement result: %d\n", switch_result);
    
    /* Additional test: Mixed volatile/non-volatile in same expression */
    {
        volatile int *vp = array2;
        int *np = array1;
        int mixed_sum = 0;
        
        /* Access both volatile and non-volatile with post-increment */
        for (int i = 0; i < 5; i++) {
            mixed_sum += *vp++ + *np++;
        }
        printf("Test 9 - Mixed volatile/non-volatile sum: %d\n", mixed_sum);
    }
    
    printf("\n=== All tests completed ===\n");
    
    return 0;
}
