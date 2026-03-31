/* Compile with: gcc -O2 -fno-expensive-optimizations -fdump-rtl-early_remat -fdump-rtl-all -o test test.c */

#include <stdint.h>
#include <stdlib.h>

/* Helper functions to force copies and prevent optimization */
static inline int32_t copy_and_add(int32_t a, int32_t b, int32_t c) {
    volatile int32_t temp = a + b;
    asm volatile("" : "+r"(temp) : : "memory");
    return temp + c;
}

static inline double float_copy_mul(double x, double y, int scale) {
    volatile double v = x * y;
    asm volatile("" : "+f"(v) : : "memory");
    return v * scale;
}

static inline int16_t narrow_compute(int32_t a, int32_t b, char c) {
    volatile int16_t narrow = (int16_t)(a - b);
    asm volatile("" : "+r"(narrow) : : "memory");
    return narrow + c;
}

/* Function with complex control flow to create many basic blocks */
static int process_data(int8_t* data1, int16_t* data2, int32_t* data3, 
                       float* fdata1, double* fdata2, int len) {
    int32_t acc_int = 0;
    double acc_float = 0.0;
    volatile int sink_int = 0;
    volatile double sink_float = 0.0;
    
    /* Outer loop - creates register pressure */
    for (int i = 0; i < len; i++) {
        int32_t base = data3[i % len];
        double fbase = fdata2[i % len];
        
        /* First inner loop with integer operations */
        for (int j = 0; j < 8; j++) {
            /* Create many short-lived recomputable values */
            int32_t t1 = base + j * 17;
            int32_t t2 = t1 - data1[(i + j) % len];
            int32_t t3 = t2 * 3;
            
            /* Force copy propagation context */
            int32_t copied = copy_and_add(t3, data2[j % len], i);
            
            /* Mixed precision */
            int16_t narrow = narrow_compute(copied, t1, data1[j % len]);
            
            /* Volatile sink to prevent elimination */
            sink_int = narrow;
            acc_int += (int32_t)narrow + t2;
            
            /* Conditional block to split control flow */
            if (j % 3 == 0) {
                int32_t t4 = t3 + copied;
                acc_int -= t4;
                asm volatile("" : : "r"(t4) : "memory");
            }
        }
        
        /* Second inner loop with floating point */
        for (int k = 0; k < 4; k++) {
            /* Floating point recomputable values */
            double ft1 = fbase * (k + 1);
            double ft2 = ft1 + fdata1[(i + k) % len];
            
            /* Force floating point copy */
            double ft3 = float_copy_mul(ft2, 1.5, 2);
            
            /* Mixed integer/float */
            int32_t ift = (int32_t)ft3;
            double ft4 = ft3 * ift;
            
            /* More register pressure with conversions */
            float ft5 = (float)ft4;
            double ft6 = (double)ft5 * 0.75;
            
            sink_float = ft6;
            acc_float += ft6;
            
            /* Another conditional */
            if (k % 2 == 1) {
                double ft7 = ft6 * ft3;
                acc_float -= ft7;
                asm volatile("" : : "f"(ft7) : "memory");
            }
        }
        
        /* Cross-type computation to force different register modes */
        if (i % 5 == 0) {
            int64_t big = (int64_t)acc_int * (int64_t)acc_float;
            double dbig = (double)big * 0.01;
            float fbig = (float)dbig;
            
            /* Inline asm that clobbers registers */
            asm volatile("" : : "r"(big), "f"(dbig), "r"(fbig) : 
                        "%rax", "%rbx", "%xmm0", "%xmm1");
        }
    }
    
    /* Final mixing to prevent dead code elimination */
    asm volatile("" : "+r"(acc_int), "+f"(acc_float) : : "memory");
    return acc_int + (int)acc_float;
}

/* Another layer to increase compilation unit complexity */
static inline int process_chunk(int8_t* d1, int16_t* d2, int32_t* d3,
                               float* f1, double* f2, int start, int end) {
    int total = 0;
    for (int i = start; i < end; i += 4) {
        int chunk_len = (end - i) < 4 ? (end - i) : 4;
        total += process_data(d1 + i, d2 + i, d3 + i, 
                             f1 + i, f2 + i, chunk_len);
        
        /* More copy contexts */
        volatile int vt = total;
        asm volatile("" : "+r"(vt) : : "memory");
        total = vt;
    }
    return total;
}

int main(void) {
    const int SIZE = 128;
    
    /* Allocate and initialize arrays with different patterns */
    int8_t* data1 = (int8_t*)malloc(SIZE * sizeof(int8_t));
    int16_t* data2 = (int16_t*)malloc(SIZE * sizeof(int16_t));
    int32_t* data3 = (int32_t*)malloc(SIZE * sizeof(int32_t));
    float* fdata1 = (float*)malloc(SIZE * sizeof(float));
    double* fdata2 = (double*)malloc(SIZE * sizeof(double));
    
    /* Initialize with varying patterns to prevent optimization */
    for (int i = 0; i < SIZE; i++) {
        data1[i] = (i * 37) & 0xFF;
        data2[i] = (i * 51) & 0xFFFF;
        data3[i] = i * 73;
        fdata1[i] = (float)(i * 0.123);
        fdata2[i] = (double)(i * 0.456);
    }
    
    /* Volatile writes to prevent array optimization */
    volatile int8_t* v1 = data1;
    volatile int16_t* v2 = data2;
    asm volatile("" : : "r"(v1), "r"(v2) : "memory");
    
    /* Process in overlapping chunks to create complex DFG */
    int result = 0;
    for (int chunk = 0; chunk < 4; chunk++) {
        int start = chunk * 32;
        int end = start + 64;  /* Overlap chunks */
        if (end > SIZE) end = SIZE;
        
        result += process_chunk(data1, data2, data3, fdata1, fdata2, start, end);
        
        /* Force spill/reload context */
        volatile int vr = result;
        asm volatile("" : "+r"(vr) : : "%rcx", "%rdx", "memory");
        result = vr;
    }
    
    /* Final use to prevent elimination */
    asm volatile("" : : "r"(result) : "memory");
    
    free(data1);
    free(data2);
    free(data3);
    free(fdata1);
    free(fdata2);
    
    return result & 0xFF;  /* Return non-zero to be useful */
}
