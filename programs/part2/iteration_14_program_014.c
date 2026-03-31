/* Compile with: gcc -O2 -fno-expensive-optimizations -fdump-rtl-early_remat -fdump-rtl-all -fno-omit-frame-pointer -fno-schedule-insns -finline-small-functions -fno-tree-pre -fdump-rtl-expand -o test test.c */

#include <stdint.h>
#include <stdlib.h>

/* Volatile sinks to prevent elimination */
static volatile int volatile_sink_int;
static volatile float volatile_sink_float;
static volatile double volatile_sink_double;

/* Inline helper functions to force copies */
static inline int8_t process_int8(int8_t a, int8_t b, int8_t c) {
    asm volatile("" : "+r"(a), "+r"(b), "+r"(c) : :);
    int8_t res = (a * b) + (c >> 1);
    asm volatile("" : : "r"(res));
    return res;
}

static inline int16_t process_int16(int16_t a, int16_t b, int16_t c) {
    asm volatile("" : "+r"(a), "+r"(b), "+r"(c) : :);
    int16_t res = (a + b) * 3 - c;
    asm volatile("" : : "r"(res));
    return res;
}

static inline int32_t process_int32(int32_t a, int32_t b, int32_t c) {
    asm volatile("" : "+r"(a), "+r"(b), "+r"(c) : :);
    int32_t res = (a << 2) + (b >> 1) - c;
    asm volatile("" : : "r"(res));
    return res;
}

static inline float process_float(float a, float b, float c) {
    asm volatile("" : "+f"(a), "+f"(b), "+f"(c) : :);
    float res = (a * 1.5f) + (b / 2.0f) - c;
    asm volatile("" : : "f"(res));
    return res;
}

static inline double process_double(double a, double b, double c) {
    asm volatile("" : "+f"(a), "+f"(b), "+f"(c) : :);
    double res = (a * 2.5) + (b / 3.0) - c;
    asm volatile("" : : "f"(res));
    return res;
}

/* Mixed precision computation with many virtual registers */
static void compute_kernel(int8_t *arr8, int16_t *arr16, int32_t *arr32, 
                          float *arrf, double *arrd, int size) {
    for (int outer = 0; outer < 3; ++outer) {
        /* Create register pressure with many short-lived values */
        int32_t acc32 = outer * 100;
        float accf = outer * 0.5f;
        double accd = outer * 0.25;
        
        for (int i = 0; i < size; ++i) {
            /* Integer computations with different precisions */
            int8_t t1 = arr8[i] + (i & 0x7F);
            int16_t t2 = arr16[i] - (i & 0x7FFF);
            int32_t t3 = arr32[i] + (i * 2);
            
            /* Force copies through inline functions */
            int8_t r1 = process_int8(t1, arr8[(i + 1) % size], t1 >> 2);
            int16_t r2 = process_int16(t2, arr16[(i + 2) % size], t2 >> 3);
            int32_t r3 = process_int32(t3, arr32[(i + 3) % size], t3 >> 4);
            
            /* Mix computations to create dependencies */
            acc32 = (acc32 + r1 + r2 + r3) & 0xFFFF;
            
            /* Floating point computations */
            float f1 = arrf[i] * 1.1f + i;
            double d1 = arrd[i] * 1.2 + i;
            
            float fr = process_float(f1, arrf[(i + 1) % size], f1 * 0.5f);
            double dr = process_double(d1, arrd[(i + 2) % size], d1 * 0.6);
            
            /* Mixed precision calculations */
            accf = accf + fr + (float)dr;
            accd = accd + dr + (double)fr;
            
            /* Conditional block to split control flow */
            if (i % 7 == 0) {
                /* Different computation path */
                int32_t tmp = r1 * r2 + r3;
                float ftmp = (float)tmp * 0.01f;
                double dtmp = (double)tmp * 0.02;
                
                acc32 ^= tmp;
                accf += ftmp;
                accd -= dtmp;
                
                /* Force more register pressure */
                asm volatile("" : : "r"(tmp), "f"(ftmp), "f"(dtmp));
            } else if (i % 13 == 0) {
                /* Another path with different operations */
                float fcomb = fr * 2.0f - (float)dr;
                double dcomb = dr * 3.0 - (double)fr;
                
                accf = accf * 0.9f + fcomb;
                accd = accd * 0.8 + dcomb;
                
                asm volatile("" : : "f"(fcomb), "f"(dcomb));
            }
            
            /* Volatile sinks to prevent elimination */
            if (i % 100 == 0) {
                volatile_sink_int = acc32;
                volatile_sink_float = accf;
                volatile_sink_double = accd;
            }
        }
        
        /* Inner loop with different pattern */
        for (int j = 0; j < size / 2; ++j) {
            /* Create cross-type computations */
            int idx = j * 2;
            int32_t mix1 = arr8[idx] + arr16[idx] + arr32[idx];
            float mix2 = arrf[idx] * (float)arr8[idx];
            double mix3 = arrd[idx] * (double)arr16[idx];
            
            /* Chain computations */
            for (int k = 0; k < 3; ++k) {
                mix1 = (mix1 << 1) + k;
                mix2 = mix2 * 1.1f + k;
                mix3 = mix3 * 1.2 - k;
                
                /* Force register copies */
                int32_t cmix1 = process_int32(mix1, mix1 >> 2, k);
                float cmix2 = process_float(mix2, mix2 * 0.5f, (float)k);
                double cmix3 = process_double(mix3, mix3 * 0.6, (double)k);
                
                mix1 = cmix1;
                mix2 = cmix2;
                mix3 = cmix3;
            }
            
            /* Use results to prevent elimination */
            asm volatile("" : : "r"(mix1), "f"(mix2), "f"(mix3));
        }
    }
}

/* Another computation kernel with different pattern */
static void compute_kernel2(char *carr, short *sarr, int *iarr, 
                           float *farr, double *darr, int size) {
    int counter = 0;
    
    for (int phase = 0; phase < 4; ++phase) {
        /* Initialize accumulators */
        int32_t phase_acc_int = phase * 1000;
        float phase_acc_float = phase * 100.0f;
        double phase_acc_double = phase * 1000.0;
        
        for (int i = 0; i < size; i += 2) {
            /* Load and compute with different strides */
            char c1 = carr[i];
            char c2 = carr[i + 1];
            short s1 = sarr[i];
            short s2 = sarr[i + 1];
            int i1 = iarr[i];
            int i2 = iarr[i + 1];
            float f1 = farr[i];
            float f2 = farr[i + 1];
            double d1 = darr[i];
            double d2 = darr[i + 1];
            
            /* Complex dependency chain */
            int32_t v1 = (c1 * s1) + i1;
            int32_t v2 = (c2 * s2) + i2;
            float v3 = f1 * (float)v1 + f2;
            double v4 = d1 * (double)v2 + d2;
            
            /* Cross-type conversions and back */
            int32_t v5 = (int32_t)v3 + (int32_t)v4;
            float v6 = (float)v1 + (float)v2;
            double v7 = (double)v1 - (double)v2;
            
            /* More computations */
            for (int step = 0; step < 2; ++step) {
                v5 = v5 * 3 + step;
                v6 = v6 * 1.5f - step;
                v7 = v7 / 2.0 + step;
                
                /* Force spills/reloads with inline asm */
                asm volatile("" : "+r"(v5), "+f"(v6), "+f"(v7) : : "memory");
            }
            
            /* Update accumulators */
            phase_acc_int += v5;
            phase_acc_float += v6;
            phase_acc_double += v7;
            
            counter++;
            
            /* Periodically use volatile sink */
            if (counter % 50 == 0) {
                volatile_sink_int = phase_acc_int;
                volatile_sink_float = phase_acc_float;
                volatile_sink_double = phase_acc_double;
            }
        }
    }
}

int main(void) {
    const int SIZE = 1000;
    
    /* Allocate and initialize arrays with different patterns */
    int8_t *arr8 = (int8_t*)malloc(SIZE * sizeof(int8_t));
    int16_t *arr16 = (int16_t*)malloc(SIZE * sizeof(int16_t));
    int32_t *arr32 = (int32_t*)malloc(SIZE * sizeof(int32_t));
    float *arrf = (float*)malloc(SIZE * sizeof(float));
    double *arrd = (double*)malloc(SIZE * sizeof(double));
    
    char *carr = (char*)malloc(SIZE * sizeof(char));
    short *sarr = (short*)malloc(SIZE * sizeof(short));
    int *iarr = (int*)malloc(SIZE * sizeof(int));
    float *farr = (float*)malloc(SIZE * sizeof(float));
    double *darr = (double*)malloc(SIZE * sizeof(double));
    
    /* Initialize with patterns */
    for (int i = 0; i < SIZE; ++i) {
        arr8[i] = (i * 3) & 0xFF;
        arr16[i] = (i * 5) & 0xFFFF;
        arr32[i] = i * 7;
        arrf[i] = i * 0.123f;
        arrd[i] = i * 0.456;
        
        carr[i] = (i * 11) & 0xFF;
        sarr[i] = (i * 13) & 0xFFFF;
        iarr[i] = i * 17;
        farr[i] = i * 0.789f;
        darr[i] = i * 1.234;
    }
    
    /* Run computation kernels multiple times */
    for (int iter = 0; iter < 10; ++iter) {
        compute_kernel(arr8, arr16, arr32, arrf, arrd, SIZE);
        compute_kernel2(carr, sarr, iarr, farr, darr, SIZE);
        
        /* Shuffle data slightly to change patterns */
        for (int i = 0; i < SIZE - 1; ++i) {
            arr8[i] ^= arr8[i + 1];
            arr16[i] += arr16[i + 1];
            arrf[i] *= 0.99f;
            arrd[i] *= 0.995;
        }
    }
    
    /* Final volatile store */
    volatile_sink_int = arr32[SIZE - 1];
    volatile_sink_float = arrf[SIZE - 1];
    volatile_sink_double = arrd[SIZE - 1];
    
    /* Cleanup */
    free(arr8);
    free(arr16);
    free(arr32);
    free(arrf);
    free(arrd);
    free(carr);
    free(sarr);
    free(iarr);
    free(farr);
    free(darr);
    
    return 0;
}
