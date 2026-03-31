/* test_modulo_sched.c
 * Designed to trigger GCC's modulo scheduler debugging output
 * Compile with: gcc -O3 -fmodulo-sched -fdump-rtl-sms -c test_modulo_sched.c
 */

#include <stdio.h>
#include <stdlib.h>

#define SIZE 1024

/* Function 1: Basic loop with distance-1 dependence and accumulator */
void func1_distance1(int n, int* restrict a, int* restrict b, int* restrict c) {
    volatile int sum = 0;  /* volatile to preserve dependence */
    for (int i = 1; i < n; i++) {
        /* Distance-1 dependence: a[i] depends on a[i-1] */
        a[i] = a[i-1] * b[i] + c[i];
        
        /* Another distance-1 dependence through accumulator */
        sum = sum + a[i] * 3;
        
        /* Memory barrier to prevent optimization */
        asm volatile("" : : "r"(sum) : "memory");
    }
    /* Use result to prevent dead code elimination */
    a[0] = sum;
}

/* Function 2: Multiple interleaved distance-1 dependencies */
void func2_multiple_deps(int n, int* restrict x, int* restrict y, 
                         int* restrict z, int* restrict w) {
    volatile int acc1 = x[0];
    volatile int acc2 = y[0];
    
    for (int i = 1; i < n; i++) {
        /* Two separate distance-1 dependencies */
        acc1 = acc1 * 2 + z[i];
        acc2 = acc2 + w[i] * acc1;
        
        /* Cross-iteration array access with distance 1 */
        x[i] = y[i-1] + acc2;
        y[i] = x[i-1] * 3 - acc1;
        
        /* Force dependencies to be preserved */
        asm volatile("" : : "r"(acc1), "r"(acc2) : "memory");
    }
}

/* Function 3: Nested loops with inner loop carried dependency */
void func3_nested(int n, int m, int* restrict mat, int* restrict vec) {
    for (int i = 0; i < n; i++) {
        volatile int carry = 0;
        
        /* Inner loop with distance-1 dependence */
        for (int j = 1; j < m; j++) {
            int idx = i * m + j;
            int prev_idx = i * m + (j - 1);
            
            /* Distance-1 dependence within inner loop */
            mat[idx] = mat[prev_idx] * vec[j] + carry;
            carry = mat[idx] % 7;
            
            /* Additional operation to create more ILP */
            vec[j] = (vec[j] + carry) * 2;
        }
        
        /* Prevent optimization */
        asm volatile("" : : "r"(carry) : "memory");
    }
}

/* Function 4: Complex recurrence with multiple uses */
void func4_complex_recurrence(int n, int* restrict p, int* restrict q, 
                              int* restrict r, int* restrict s) {
    volatile int t1 = p[0];
    volatile int t2 = q[0];
    
    for (int i = 1; i < n; i++) {
        /* Multiple interleaved recurrences */
        int temp = t1 * t2 + r[i];
        t1 = t2 + s[i] * 3;
        t2 = temp - p[i-1];  /* Distance-1 use of p */
        
        /* Array with distance-1 dependence */
        p[i] = t1 + t2;
        q[i] = q[i-1] * 2 + t1;  /* Another distance-1 use */
        
        /* Create more scheduling complexity */
        r[i] = (r[i-1] + r[i]) / 2;  /* Yet another distance-1 */
        
        asm volatile("" : : "r"(t1), "r"(t2) : "memory");
    }
}

/* Function 5: Loop with unknown trip count (prevents unrolling) */
void func5_unknown_trip(int* restrict a, int* restrict b, 
                        int* restrict c, int n) {
    volatile int sum = a[0];
    
    /* n is parameter - unknown at compile time */
    for (int i = 1; i < n; i++) {
        /* Classic distance-1 recurrence */
        sum = sum * 17 + b[i];
        
        /* Multiple uses of sum to create pressure */
        a[i] = sum + c[i];
        b[i] = a[i-1] + sum;  /* Distance-1 use of a */
        c[i] = b[i] * 2 - sum;
        
        /* Memory clobber to preserve all dependencies */
        asm volatile("" : : "r"(sum) : "memory");
    }
}

/* Function 6: Software pipelining candidate with high ILP */
void func6_high_ilp(int n, int* restrict out, int* restrict in1, 
                    int* restrict in2, int* restrict in3) {
    volatile int acc1 = 1;
    volatile int acc2 = 2;
    volatile int acc3 = 3;
    
    for (int i = 1; i < n; i++) {
        /* Three parallel accumulators with distance-1 deps */
        acc1 = acc1 * in1[i] + 5;
        acc2 = acc2 + in2[i] * acc1;
        acc3 = acc3 * 3 - in3[i] + acc2;
        
        /* Interdependent array operations */
        out[i] = out[i-1] + acc1 + acc2 + acc3;  /* Distance-1 use */
        in1[i] = in1[i-1] * 2;                   /* Another distance-1 */
        in2[i] = acc2 - out[i-1];                /* And another */
        
        /* Force all accumulators to be live */
        asm volatile("" : : "r"(acc1), "r"(acc2), "r"(acc3) : "memory");
    }
}

/* Main driver that calls all functions */
int main(int argc, char** argv) {
    int n = SIZE;
    
    /* Allocate and initialize arrays */
    int* a = (int*)malloc(SIZE * sizeof(int));
    int* b = (int*)malloc(SIZE * sizeof(int));
    int* c = (int*)malloc(SIZE * sizeof(int));
    int* d = (int*)malloc(SIZE * sizeof(int));
    int* e = (int*)malloc(SIZE * sizeof(int));
    int* f = (int*)malloc(SIZE * sizeof(int));
    int* mat = (int*)malloc(SIZE * SIZE * sizeof(int));
    
    /* Initialize with pseudo-random values */
    for (int i = 0; i < SIZE; i++) {
        a[i] = i * 3 + 1;
        b[i] = i * 5 + 2;
        c[i] = i * 7 + 3;
        d[i] = i * 11 + 4;
        e[i] = i * 13 + 5;
        f[i] = i * 17 + 6;
    }
    
    for (int i = 0; i < SIZE * SIZE; i++) {
        mat[i] = i % 19;
    }
    
    /* Call all functions to ensure they're compiled */
    func1_distance1(n, a, b, c);
    func2_multiple_deps(n, d, e, f, a);
    func3_nested(16, 64, mat, b);
    func4_complex_recurrence(n, c, d, e, f);
    
    /* Vary the trip count */
    func5_unknown_trip(a, b, c, n / 2);
    func5_unknown_trip(d, e, f, n / 4);
    
    func6_high_ilp(n, a, b, c, d);
    
    /* Compute checksum to prevent dead code elimination */
    int checksum = 0;
    for (int i = 0; i < SIZE; i++) {
        checksum ^= a[i] ^ b[i] ^ c[i] ^ d[i] ^ e[i] ^ f[i];
    }
    
    for (int i = 0; i < SIZE * SIZE; i += 256) {
        checksum ^= mat[i];
    }
    
    printf("Checksum: %d\n", checksum);
    
    /* Cleanup */
    free(a); free(b); free(c); free(d); free(e); free(f); free(mat);
    
    return checksum != 0 ? 0 : 1;
}
