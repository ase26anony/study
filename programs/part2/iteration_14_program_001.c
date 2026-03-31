/* Program to trigger early rematerialization copy_propagate logic */
#include <stdint.h>
#include <stdlib.h>

#define FORCE_COPY __attribute__((noinline))
#define NO_OPT __attribute__((optimize("O0")))

/* Volatile sinks to prevent elimination */
static volatile int volatile_sink_int;
static volatile double volatile_sink_double;

/* Inline function to force copies */
static inline int32_t force_copy_int(int32_t x, int32_t y) {
    asm volatile("" : "+r"(x), "+r"(y));
    return x + y;
}

static inline double force_copy_double(double x, double y) {
    asm volatile("" : "+f"(x), "+f"(y));
    return x * y;
}

/* Function with mixed operations to create register pressure */
FORCE_COPY
static int process_chunk(int start, int end, 
                         const int8_t* data8, 
                         const int16_t* data16,
                         const float* data_f,
                         double* results) {
    int sum = 0;
    double acc_d = 0.0;
    float acc_f = 0.0f;
    
    /* Multiple loops with different register types */
    for (int i = start; i < end; i++) {
        /* Integer chain with dependencies */
        int t1 = data8[i] * 3;
        int t2 = t1 + data16[i % 1024];
        int t3 = t2 << 2;
        int t4 = t3 - i;
        
        /* Force copy between virtual registers */
        int t5 = force_copy_int(t4, t3);
        
        /* Floating point chain */
        float f1 = data_f[i] * 2.5f;
        float f2 = f1 + (float)t5 * 0.01f;
        float f3 = force_copy_double(f2, f2);  /* Promotion to double */
        
        /* Mixed precision */
        double d1 = (double)f3 * 1.5;
        double d2 = d1 + (double)i * 0.001;
        
        /* Accumulate with different types */
        sum += t5;
        acc_f += f2;
        acc_d += d2;
        
        /* Volatile writes to prevent elimination */
        if (i % 7 == 0) {
            volatile_sink_int = t5;
            volatile_sink_double = d2;
        }
    }
    
    /* More computations to increase pressure */
    for (int i = start; i < end; i += 3) {
        /* Different integer types */
        short s1 = (short)(data8[i] * 2);
        int s2 = s1 * s1;  /* Promotion */
        long s3 = s2 * 3L;
        
        /* Complex expression chain */
        int v1 = s2 + (int)(s3 >> 4);
        int v2 = v1 * v1 - v1;
        int v3 = force_copy_int(v2, v1);
        
        sum += v3;
        
        /* Inline asm to clobber registers */
        asm volatile("" : : "r"(v3), "r"(s2));
    }
    
    results[0] = acc_d;
    results[1] = (double)acc_f;
    return sum;
}

/* Main kernel with nested loops */
NO_OPT
int main(void) {
    const int N = 4096;
    const int CHUNK = 128;
    
    /* Allocate and initialize arrays with different types */
    int8_t* data8 = (int8_t*)malloc(N * sizeof(int8_t));
    int16_t* data16 = (int16_t*)malloc(1024 * sizeof(int16_t));
    float* data_f = (float*)malloc(N * sizeof(float));
    double* results = (double*)malloc(2 * (N/CHUNK) * sizeof(double));
    
    /* Initialize with patterns */
    for (int i = 0; i < N; i++) {
        data8[i] = (int8_t)((i * 13) % 256 - 128);
        data_f[i] = (float)((i * 17) % 1000) * 0.001f;
    }
    for (int i = 0; i < 1024; i++) {
        data16[i] = (int16_t)((i * 23) % 65536 - 32768);
    }
    
    int total_sum = 0;
    double total_double = 0.0;
    
    /* Outer loop creating multiple basic blocks */
    for (int outer = 0; outer < 10; outer++) {
        /* First inner loop */
        for (int chunk = 0; chunk < N; chunk += CHUNK) {
            int start = chunk;
            int end = chunk + CHUNK;
            
            /* Call processing function - forces register saves/restores */
            int chunk_sum = process_chunk(start, end, data8, data16, data_f, 
                                         &results[chunk/CHUNK * 2]);
            
            /* More computations with results */
            double local_acc = 0.0;
            for (int j = 0; j < 2; j++) {
                double r = results[chunk/CHUNK * 2 + j];
                local_acc += r * (double)(chunk_sum % 100);
                
                /* Force mode mixing */
                if (j % 2 == 0) {
                    float f = (float)r;
                    volatile_sink_double = force_copy_double(r, (double)f);
                }
            }
            
            total_sum += chunk_sum;
            total_double += local_acc;
            
            /* Conditional block creating control flow */
            if (chunk_sum % 3 == 0) {
                /* Different computation path */
                long long big_val = (long long)chunk_sum * 1000000LL;
                int truncated = (int)(big_val >> 16);
                total_sum += force_copy_int(truncated, chunk_sum);
            } else if (chunk_sum % 5 == 0) {
                /* Another path with floating point */
                double scaled = (double)chunk_sum * 1.234567;
                total_double += scaled;
                volatile_sink_double = scaled;
            }
        }
        
        /* Second inner loop with different pattern */
        for (int i = 0; i < 100; i++) {
            /* Complex expression tree */
            int a = data8[i] * outer;
            int b = a + i * 7;
            int c = b << (outer % 4);
            int d = c - a;
            int e = force_copy_int(d, c);
            
            /* Floating point intermixed */
            float f = (float)e * 0.5f;
            double g = (double)f * 1.1;
            
            total_sum += e;
            total_double += g;
            
            /* Inline asm with clobbering */
            asm volatile("" : : "r"(e), "r"(d), "f"(g));
        }
    }
    
    /* Final reduction with mixed types */
    float final_float = (float)total_double;
    int final_int = total_sum + (int)final_float;
    
    volatile_sink_int = final_int;
    volatile_sink_double = total_double;
    
    /* Cleanup */
    free(data8);
    free(data16);
    free(data_f);
    free(results);
    
    return final_int & 0xFF;  /* Return non-zero to avoid elimination */
}
