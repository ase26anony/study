/* test-auto-inc-dec.c */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Global volatile to prevent dead code elimination */
volatile int global_sum = 0;

/* Opaque external functions to create aliasing concerns */
extern void use_int_ptr(int *p) __attribute__((noinline));
extern void use_char_ptr(char *p) __attribute__((noinline));
extern void use_void_ptr(void *p) __attribute__((noinline));

/* Prevent inlining and create register pressure */
__attribute__((noinline))
int sum_array_postinc(int *arr, int n) {
    int *p = arr;
    int sum = 0;
    int i = 0;
    
    /* Create register pressure with many live variables */
    int temp1 = 0, temp2 = 0, temp3 = 0, temp4 = 0;
    int temp5 = 0, temp6 = 0, temp7 = 0, temp8 = 0;
    
    /* Use asm to make pointer appear used */
    asm volatile("" : : "r"(p));
    
    while (i < n) {
        /* Post-increment access - target pattern for find_inc() */
        sum += *p++;
        
        /* Use all temp variables to increase register pressure */
        temp1 += sum * 2;
        temp2 += sum * 3;
        temp3 += sum * 4;
        temp4 += sum * 5;
        temp5 = temp1 + temp2;
        temp6 = temp3 + temp4;
        temp7 = temp5 * temp6;
        temp8 = temp7 ^ sum;
        
        i++;
        
        /* Call opaque function to prevent optimization */
        if (i % 16 == 0) {
            use_int_ptr(&temp1);
        }
    }
    
    /* Combine all temps to ensure they're used */
    return sum + temp1 + temp2 + temp3 + temp4 + temp5 + temp6 + temp7 + temp8;
}

__attribute__((noinline))
void string_copy_postinc(char *dst, const char *src, int n) {
    char *d = dst;
    const char *s = src;
    int i = 0;
    
    /* Register pressure variables */
    int checksum = 0;
    char c1, c2, c3, c4;
    
    /* Classic post-increment copy pattern */
    while (i < n && *s != '\0') {
        /* Target pattern: *dst++ = *src++ */
        *d++ = *s++;
        
        /* Additional operations to prevent over-optimization */
        c1 = *(s-1);
        c2 = *(d-1);
        c3 = c1 ^ c2;
        c4 = c3 & 0xFF;
        checksum += c4;
        
        i++;
        
        /* Use asm to prevent elimination */
        asm volatile("" : : "r"(d), "r"(s));
    }
    
    *d = '\0';
    
    /* Use checksum to prevent dead code elimination */
    global_sum += checksum;
    use_char_ptr(dst);
}

struct Point {
    int x;
    int y;
    int z;
};

__attribute__((noinline))
int traverse_struct_array(struct Point *points, int n) {
    struct Point *p = points;
    int total = 0;
    int i = 0;
    
    /* Many live variables for register pressure */
    int sum_x = 0, sum_y = 0, sum_z = 0;
    int prod_xy = 1, prod_xz = 1, prod_yz = 1;
    
    /* Loop with post-increment pointer */
    for (i = 0; i < n; i++) {
        /* Access struct members via post-increment pointer */
        sum_x += p->x;
        sum_y += p->y;
        sum_z += p->z;
        
        /* Post-increment after access */
        p++;
        
        /* Complex calculations to use all variables */
        prod_xy *= (sum_x & 0xFF) + (sum_y & 0xFF);
        prod_xz *= (sum_x & 0xFF) + (sum_z & 0xFF);
        prod_yz *= (sum_y & 0xFF) + (sum_z & 0xFF);
        
        /* Prevent optimization with opaque call */
        if (i % 8 == 0) {
            use_void_ptr(p);
        }
    }
    
    /* Combine results */
    return total + sum_x + sum_y + sum_z + prod_xy + prod_xz + prod_yz;
}

__attribute__((noinline))
int nested_loop_postinc(int *matrix, int rows, int cols) {
    int sum = 0;
    int i, j;
    
    /* High register pressure */
    int row_sums[16];
    int col_sums[16];
    int diag1 = 0, diag2 = 0;
    
    for (i = 0; i < rows; i++) {
        int *row_ptr = matrix + i * cols;
        row_sums[i % 16] = 0;
        
        for (j = 0; j < cols; j++) {
            /* Array indexing with potential post-increment decomposition */
            int val = *(row_ptr + j);  /* May become (plus (reg) (const_int 0)) */
            row_sums[i % 16] += val;
            
            /* Additional operation to use value */
            col_sums[j % 16] += val;
            
            /* Diagonal calculations */
            if (i == j) diag1 += val;
            if (i + j == cols - 1) diag2 += val;
        }
        
        /* Post-increment equivalent pattern */
        row_ptr += cols;  /* Move to next row */
        
        /* Use asm to prevent elimination */
        asm volatile("" : : "r"(row_ptr));
    }
    
    /* Combine all sums */
    for (i = 0; i < 16; i++) {
        sum += row_sums[i] + col_sums[i];
    }
    sum += diag1 + diag2;
    
    return sum;
}

__attribute__((noinline))
int mixed_operations(int *arr, int n) {
    int *p = arr;
    int *q = arr + n/2;
    int sum = 0;
    
    /* Multiple pointers with different update patterns */
    for (int i = 0; i < n/2; i++) {
        /* Various forms that may generate base+offset RTL */
        sum += *p++;          /* Post-increment */
        sum -= *(q + i);      /* Indexed access */
        
        /* Combined form that may decompose */
        int *r = p;
        sum += *(r += 0);     /* (plus (reg) (const_int 0)) pattern */
        
        /* Use asm to prevent optimization */
        asm volatile("" : : "r"(p), "r"(q), "r"(r));
        
        /* Call opaque function periodically */
        if (i % 32 == 0) {
            use_int_ptr(p);
        }
    }
    
    return sum;
}

/* Dummy implementations of opaque functions */
void use_int_ptr(int *p) {
    global_sum += *p;
}

void use_char_ptr(char *p) {
    global_sum += *p;
}

void use_void_ptr(void *p) {
    global_sum += (int)(long)p;
}

int main(int argc, char **argv) {
    /* Initialize data */
    const int ARRAY_SIZE = 1024;
    const int MATRIX_SIZE = 64;
    
    /* Different array types */
    int *int_array = malloc(ARRAY_SIZE * sizeof(int));
    char *char_array = malloc(ARRAY_SIZE * sizeof(char));
    struct Point *struct_array = malloc(ARRAY_SIZE * sizeof(struct Point));
    int *matrix = malloc(MATRIX_SIZE * MATRIX_SIZE * sizeof(int));
    
    /* Initialize with non-zero values */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        int_array[i] = i + 1;
        char_array[i] = 'A' + (i % 26);
        struct_array[i].x = i;
        struct_array[i].y = i * 2;
        struct_array[i].z = i * 3;
    }
    
    for (int i = 0; i < MATRIX_SIZE * MATRIX_SIZE; i++) {
        matrix[i] = i % 100;
    }
    
    int result = 0;
    
    /* Use command-line arguments to select paths */
    if (argc > 1) {
        int test_num = atoi(argv[1]) % 5;
        
        switch (test_num) {
            case 0:
                result = sum_array_postinc(int_array, ARRAY_SIZE);
                break;
            case 1:
                string_copy_postinc(char_array, char_array + ARRAY_SIZE/2, ARRAY_SIZE/2);
                result = global_sum;
                break;
            case 2:
                result = traverse_struct_array(struct_array, ARRAY_SIZE);
                break;
            case 3:
                result = nested_loop_postinc(matrix, MATRIX_SIZE, MATRIX_SIZE);
                break;
            case 4:
                result = mixed_operations(int_array, ARRAY_SIZE);
                break;
            default:
                result = -1;
        }
    } else {
        /* Run all tests */
        result += sum_array_postinc(int_array, ARRAY_SIZE);
        string_copy_postinc(char_array, char_array + ARRAY_SIZE/2, ARRAY_SIZE/2);
        result += traverse_struct_array(struct_array, ARRAY_SIZE);
        result += nested_loop_postinc(matrix, MATRIX_SIZE, MATRIX_SIZE);
        result += mixed_operations(int_array, ARRAY_SIZE);
    }
    
    /* Print result to prevent elimination */
    printf("Result: %d\n", result);
    printf("Global sum: %d\n", global_sum);
    
    /* Cleanup */
    free(int_array);
    free(char_array);
    free(struct_array);
    free(matrix);
    
    return 0;
}
