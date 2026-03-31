/* ddg_coverage.c - Program to trigger DDG edge creation in GCC scheduler */

#include <math.h>
#include <stdlib.h>

/* Non-inlineable function to force latency modeling */
static double external_computation(double x, double y) __attribute__((noinline));
static double external_computation(double x, double y) {
    return x * y + (x - y) / (x + y + 1.0);
}

/* Complex addressing function to confuse alias analysis */
static int get_offset(int i, double* coeff) __attribute__((noinline));
static int get_offset(int i, double* coeff) {
    return (int)(coeff[i & 3] * 10.0) & 7;
}

/* Main computation with mixed dependencies */
void compute_loop(double* src, double* dest, double* coeff, 
                  double* aux, int n, int base_offset) {
    volatile double sink; /* Prevent optimizations */
    double acc = 1.0;
    double tmp1, tmp2;
    int i;
    
    /* Initialize some values */
    dest[0] = src[0] * coeff[0];
    
    /* Main loop with complex dependencies */
    for (i = 1; i < n; ++i) {
        /* 1. Loop-carried true dependency (RAW) with high latency */
        acc = external_computation(acc, src[i]);
        
        /* 2. Anti-dependency (WAR) through memory */
        tmp1 = dest[i-1];               /* Read dest[i-1] */
        dest[i] = acc + tmp1 * coeff[i]; /* Write dest[i] */
        
        /* 3. Output dependency (WAW) with control flow */
        if (i % 16 == 0) {
            acc = sqrt(fabs(acc));      /* High latency operation */
        } else if (i % 8 == 0) {
            acc = 1.0 / (acc + 1.0);    /* Division - variable latency */
        }
        
        /* 4. Complex aliasing with pointer arithmetic */
        int offset = get_offset(i, coeff) + base_offset;
        aux[i + offset] = src[i] * 0.5;
        
        /* 5. Another output dependency chain */
        if (i % 3 == 0) {
            tmp2 = aux[i] + coeff[i % 4];
            aux[i] = tmp2 * 2.0;        /* Overwrites aux[i] */
        }
        
        /* 6. Mixed integer/floating point with division */
        int idx = (i * 13) % n;
        if (idx > 0) {
            double div_result = (double)i / (idx + 1); /* Integer division */
            dest[i] += div_result;
        }
        
        /* 7. Control-dependent memory access */
        double* target = (i % 5 == 0) ? dest : aux;
        target[i % 32] += src[i] * 0.1;
    }
    
    /* Force result usage */
    sink = acc + dest[n/2] + aux[n/4];
}

/* Secondary loop with different pattern */
void compute_loop2(float* fa, float* fb, float* fc, int m) {
    float local_acc = 0.0f;
    int j;
    
    for (j = 0; j < m; ++j) {
        /* Recurrence with loop-carried dependency */
        local_acc = local_acc + fa[j] * fb[j];
        
        /* Anti-dependency chain */
        float old = fc[j];
        fc[j] = local_acc + sinf((float)j);
        
        /* Output dependency with condition */
        if (j % 7 == 0) {
            local_acc = cosf(local_acc);
        }
        
        /* Complex addressing */
        int idx = (j * 17) % m;
        if (idx > 10) {
            fa[idx] = fb[j] * 0.3f;
        }
    }
    
    volatile float vsink = local_acc + fc[m/3];
}

int main(void) {
    const int N = 1024;
    const int M = 512;
    
    /* Allocate and initialize arrays */
    double* src = (double*)malloc(N * sizeof(double));
    double* dest = (double*)malloc(N * sizeof(double));
    double* coeff = (double*)malloc(N * sizeof(double));
    double* aux = (double*)malloc(N * 2 * sizeof(double));
    
    float* fa = (float*)malloc(M * sizeof(float));
    float* fb = (float*)malloc(M * sizeof(float));
    float* fc = (float*)malloc(M * sizeof(float));
    
    /* Initialize with simple patterns */
    for (int i = 0; i < N; ++i) {
        src[i] = (i % 37) * 0.1;
        coeff[i] = (i % 19) * 0.05;
        dest[i] = 0.0;
        aux[i] = 0.0;
    }
    
    for (int i = 0; i < M; ++i) {
        fa[i] = (i % 23) * 0.02f;
        fb[i] = (i % 29) * 0.03f;
        fc[i] = 0.0f;
    }
    
    /* Execute loops with different parameters */
    compute_loop(src, dest, coeff, aux, N, 3);
    compute_loop2(fa, fb, fc, M);
    
    /* Another call with different offset */
    compute_loop(src + 100, dest + 100, coeff, aux + 50, N - 100, 5);
    
    /* Use results to prevent dead code elimination */
    volatile double final_sink = dest[N/3] + aux[N/2] + src[N/4];
    volatile float final_fsink = fc[M/2] + fa[M/3];
    
    /* Cleanup */
    free(src);
    free(dest);
    free(coeff);
    free(aux);
    free(fa);
    free(fb);
    free(fc);
    
    return (int)(final_sink + final_fsink) & 1;
}
