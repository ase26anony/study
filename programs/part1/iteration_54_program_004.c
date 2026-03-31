/* early-remat-test.c */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <xmmintrin.h>

/* Prevent optimization of helper functions */
#define NOINLINE __attribute__((noinline, noipa))

/* Vector type to increase register pressure */
typedef int v4si __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));

/* Volatile sink to prevent optimizations */
volatile int global_sink;
volatile float float_sink;

/* Non-inline function with many arguments */
NOINLINE int use_values(int a, int b, float c, double d, 
                        int e, int f, float g, double h,
                        v4si* vec_int, v4sf* vec_float) {
    /* Force side effects */
    global_sink = a + b;
    float_sink = c + g;
    
    /* Use vector arguments */
    v4si temp_vec = *vec_int + a;
    v4sf temp_fvec = *vec_float + c;
    
    return (temp_vec[0] + temp_vec[1] + temp_vec[2] + temp_vec[3]) +
           (int)(temp_fvec[0] + temp_fvec[1]);
}

/* Another non-inline function to force register shuffling */
NOINLINE double compute_polynomial(double x, double y, double z,
                                   int i, int j, int k) {
    /* Complex polynomial with many temporaries */
    double t1 = x * x + y * y;
    double t2 = z * z - x * y;
    double t3 = t1 * t2 + x * z;
    double t4 = y * z - t1 * t2;
    double t5 = t3 * t4 + t2 * t3;
    double t6 = t4 * t5 - t3 * t4;
    
    /* Use loop indices to make each computation unique */
    t1 += i * 0.1;
    t2 += j * 0.2;
    t3 += k * 0.3;
    t4 -= i * 0.4;
    t5 -= j * 0.5;
    t6 -= k * 0.6;
    
    return t1 + t2 + t3 + t4 + t5 + t6;
}

int main(void) {
    /* Initialize arrays with varying values */
    int array_int[256];
    float array_float[256];
    double array_double[256];
    
    for (int i = 0; i < 256; i++) {
        array_int[i] = i * 3 + 1;
        array_float[i] = i * 0.7f + 2.3f;
        array_double[i] = i * 1.3 + 3.7;
    }
    
    /* Initialize vector data */
    v4si vec_int_data = {1, 2, 3, 4};
    v4sf vec_float_data = {1.1f, 2.2f, 3.3f, 4.4f};
    
    /* Accumulator to prevent dead code elimination */
    int64_t total_sum = 0;
    
    /* Nested loops to create register pressure */
    for (int outer = 0; outer < 100; outer++) {
        for (int inner = 0; inner < 100; inner++) {
            /* Many independent computations with different data types */
            /* Each computation creates its own temporaries */
            
            /* Integer computations - each creates unique temporaries */
            int idx = (outer * 17 + inner * 13) % 256;
            int idx2 = (outer * 19 + inner * 11) % 256;
            int idx3 = (outer * 23 + inner * 7) % 256;
            
            /* Computation 1: Complex integer expression */
            int a = array_int[idx] * 3 + array_int[idx2] / 7;
            int b = array_int[idx2] * 5 - array_int[idx3] / 11;
            int c = a * b + array_int[idx] - array_int[idx2];
            int d = b * c - a * array_int[idx3];
            int e = (c << 3) | (d >> 2);
            int f = (e ^ a) & (b | c);
            
            /* Computation 2: Floating point expression */
            float fa = array_float[idx] * 1.7f + array_float[idx2] / 3.3f;
            float fb = array_float[idx2] * 2.9f - array_float[idx3] / 4.1f;
            float fc = fa * fb + array_float[idx] - array_float[idx2];
            float fd = fb * fc - fa * array_float[idx3];
            float fe = fc / 1.3f + fd * 0.7f;
            float ff = (fe > fa) ? fe - fa : fa - fe;
            
            /* Computation 3: Double precision expression */
            double da = array_double[idx] * 1.11 + array_double[idx2] / 3.33;
            double db = array_double[idx2] * 2.22 - array_double[idx3] / 4.44;
            double dc = da * db + array_double[idx] - array_double[idx2];
            double dd = db * dc - da * array_double[idx3];
            double de = dc / 1.55 + dd * 0.66;
            double df = (de > da) ? de - da : da - de;
            
            /* Inline assembly that clobbers registers */
            /* For x86-64, clobber commonly used registers */
            asm volatile(
                "# Force register pressure\n"
                "mov $0, %%eax\n"
                "mov $0, %%ebx\n"
                "mov $0, %%ecx\n"
                "mov $0, %%edx\n"
                "mov $0, %%esi\n"
                "mov $0, %%edi\n"
                :
                :
                : "eax", "ebx", "ecx", "edx", "esi", "edi", "memory"
            );
            
            /* More computations after assembly to force rematerialization */
            int g = a + b + c + d + e + f + idx + idx2 + idx3;
            float fg = fa + fb + fc + fd + fe + ff + idx * 0.1f;
            double dg = da + db + dc + dd + de + df + idx * 0.01;
            
            /* Call function with many arguments - forces register shuffling */
            int result1 = use_values(a, b, fa, da, 
                                    c, d, fb, db,
                                    &vec_int_data, &vec_float_data);
            
            /* Another assembly clobber */
            asm volatile(
                "# Clobber more registers\n"
                "pxor %%xmm0, %%xmm0\n"
                "pxor %%xmm1, %%xmm1\n"
                "pxor %%xmm2, %%xmm2\n"
                "pxor %%xmm3, %%xmm3\n"
                :
                :
                : "xmm0", "xmm1", "xmm2", "xmm3", "memory"
            );
            
            /* More independent computations */
            int h = g * 3 - result1;
            float fh = fg * 1.5f - result1 * 0.5f;
            double dh = dg * 2.0 - result1 * 0.25;
            
            /* Call polynomial function */
            double poly_result = compute_polynomial(da, db, dc, 
                                                   outer, inner, idx);
            
            /* Volatile writes to prevent optimization */
            global_sink = h;
            float_sink = fh;
            
            /* Use SSE intrinsics for vector operations */
            __m128i vec1 = _mm_set_epi32(a, b, c, d);
            __m128i vec2 = _mm_set_epi32(e, f, g, h);
            __m128i vec3 = _mm_add_epi32(vec1, vec2);
            
            /* Extract and use results */
            int vec_results[4];
            _mm_storeu_si128((__m128i*)vec_results, vec3);
            
            /* Final accumulation with all computed values */
            total_sum += result1 + h + (int)fh + (int)dh + 
                        (int)poly_result + vec_results[0] + 
                        vec_results[1] + vec_results[2] + vec_results[3];
            
            /* Another assembly barrier */
            asm volatile("# Memory barrier" ::: "memory");
            
            /* Modify vector data slightly to prevent loop invariant removal */
            vec_int_data[0] += 1;
            vec_float_data[1] += 0.1f;
        }
        
        /* Prevent loop unrolling from reducing register pressure */
        asm volatile("# Outer loop barrier" ::: "memory");
    }
    
    printf("Result: %ld\n", (long)total_sum);
    return 0;
}
