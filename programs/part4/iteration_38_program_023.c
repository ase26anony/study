/* test_modulo_sched.c
 * Designed to trigger GCC's modulo scheduler debug output
 * Compile with: gcc -O3 -fmodulo-sched -fdump-rtl-sms -c test_modulo_sched.c
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define SIZE 1024
#define ITERATIONS 1000000

/* Prevent compiler from optimizing away loops */
static volatile int g_volatile_sink = 0;

/* Function 1: Loop with multiple carried dependencies */
void loop_carried_deps(int n, int *restrict a, int *restrict b, int *restrict c, int *restrict d) {
    volatile int sum = 0;  /* Force dependence across iterations */
    int acc1 = 0, acc2 = 0;
    
    /* Multiple distance-1 dependencies */
    for (int i = 1; i < n; i++) {
        /* Distance-1 dependence: a[i] depends on a[i-1] */
        a[i] = a[i-1] * b[i] + c[i];
        
        /* Another distance-1 dependence with accumulation */
        acc1 = acc1 + d[i] * 3;
        
        /* Cross-iteration dependency with different distance */
        b[i] = b[i-1] + a[i] * 2;
        
        /* Second accumulator with different operation */
        acc2 = acc2 * 2 + c[i];
        
        /* Memory barrier to preserve dependencies */
        asm volatile("" : : "r"(acc1), "r"(acc2) : "memory");
    }
    
    g_volatile_sink = sum + acc1 + acc2;
}

/* Function 2: Nested loop with inner carried dependency */
void nested_loop_carried(int n, int m, int *restrict x, int *restrict y) {
    volatile int outer_acc = 0;
    
    for (int j = 0; j < m; j++) {
        int inner_acc = y[j];
        
        /* Inner loop with carried dependency */
        for (int i = 1; i < n; i++) {
            /* Distance-1 dependence in inner loop */
            x[i] = x[i-1] + y[i] * j;
            inner_acc = inner_acc + x[i] * 7;
            
            /* Additional operations to create ILP */
            y[i] = y[i] ^ (x[i] & 0xFF);
            x[i-1] = x[i] - inner_acc;
        }
        
        outer_acc = outer_acc + inner_acc;
        asm volatile("" : : "r"(inner_acc) : "memory");
    }
    
    g_volatile_sink = outer_acc;
}

/* Function 3: Multiple interleaved accumulators */
void multi_accumulator(int n, int *restrict arr1, int *restrict arr2, 
                       int *restrict arr3, int *restrict arr4) {
    volatile int sum1 = 0, sum2 = 0, sum3 = 0;
    
    for (int i = 1; i < n; i++) {
        /* Three separate carried dependencies */
        sum1 = sum1 + arr1[i] * arr2[i-1];  /* Distance-1 */
        sum2 = sum2 * 3 + arr3[i];          /* Distance-1 with multiplication */
        sum3 = arr4[i] - sum3;              /* Distance-1 with subtraction */
        
        /* Cross-dependencies between accumulators */
        arr1[i] = sum1 + sum2;
        arr2[i] = sum2 - sum3;
        
        /* Complex expression with multiple uses */
        arr3[i] = (arr1[i-1] * sum1 + arr2[i] * sum2) / (sum3 + 1);
        
        /* Force dependency preservation */
        asm volatile("" : : "r"(sum1), "r"(sum2), "r"(sum3) : "memory");
    }
    
    g_volatile_sink = sum1 + sum2 + sum3;
}

/* Function 4: Loop with unknown trip count (prevents unrolling) */
void variable_trip_loop(int n, int *restrict a, int *restrict b) {
    if (n <= 1) return;
    
    volatile int carry = a[0];
    
    /* Loop with multiple distance-1 dependencies */
    for (int i = 1; i < n; i++) {
        /* Primary carried dependency */
        int temp = carry * b[i];
        a[i] = temp + a[i-1];  /* Another distance-1 */
        
        /* Update carry for next iteration */
        carry = (carry + a[i]) & 0xFFFF;
        
        /* Additional operations to create scheduling opportunities */
        b[i-1] = b[i] ^ carry;
        a[i] = a[i] | (temp & 0xFF);
        
        /* Memory clobber to prevent optimization */
        asm volatile("" : : "r"(carry), "r"(temp) : "memory");
    }
    
    g_volatile_sink = carry;
}

/* Function 5: Complex loop with if-conversion opportunities */
void conditional_loop(int n, int *restrict x, int *restrict y, int threshold) {
    volatile int count = 0;
    int last = x[0];
    
    for (int i = 1; i < n; i++) {
        /* Distance-1 dependence */
        int diff = x[i] - last;
        last = x[i];
        
        /* Conditional that creates control dependencies */
        if (diff > threshold) {
            y[i] = y[i-1] + diff * 2;  /* Another distance-1 */
            count = count * 3 + 1;
        } else {
            y[i] = y[i-1] - diff;      /* Distance-1 */
            count = count / 2;
        }
        
        /* Additional arithmetic */
        x[i] = (x[i] * 13 + y[i] * 7) & 0x3FF;
        
        /* Prevent optimization */
        asm volatile("" : : "r"(count), "r"(last) : "memory");
    }
    
    g_volatile_sink = count;
}

/* Main driver that calls all test functions */
int main(int argc, char *argv[]) {
    /* Allocate and initialize arrays */
    int *a = malloc(SIZE * sizeof(int));
    int *b = malloc(SIZE * sizeof(int));
    int *c = malloc(SIZE * sizeof(int));
    int *d = malloc(SIZE * sizeof(int));
    int *x = malloc(SIZE * sizeof(int));
    int *y = malloc(SIZE * sizeof(int));
    
    /* Initialize with pseudo-random data */
    srand(time(NULL));
    for (int i = 0; i < SIZE; i++) {
        a[i] = rand() % 100;
        b[i] = rand() % 100;
        c[i] = rand() % 100;
        d[i] = rand() % 100;
        x[i] = rand() % 100;
        y[i] = rand() % 100;
    }
    
    /* Call all test functions multiple times with different parameters */
    for (int iter = 0; iter < 10; iter++) {
        loop_carried_deps(SIZE, a, b, c, d);
        nested_loop_carried(SIZE/4, 4, x, y);
        multi_accumulator(SIZE, a, b, c, d);
        variable_trip_loop(SIZE, x, y);
        conditional_loop(SIZE, a, b, 50);
        
        /* Modify data slightly between iterations */
        for (int i = 0; i < SIZE; i++) {
            a[i] = (a[i] + 1) & 0xFF;
            b[i] = (b[i] * 3) & 0xFF;
        }
    }
    
    /* Compute final checksum to prevent dead code elimination */
    int checksum = 0;
    for (int i = 0; i < SIZE; i++) {
        checksum = (checksum + a[i] + b[i] + x[i] + y[i]) & 0xFFFF;
    }
    
    printf("Final checksum: %d\n", checksum);
    printf("Volatile sink: %d\n", g_volatile_sink);
    
    /* Cleanup */
    free(a); free(b); free(c); free(d);
    free(x); free(y);
    
    return checksum != 0 ? 0 : 1;
}
