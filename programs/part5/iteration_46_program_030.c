#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

/* Simple PRNG for reproducible results */
static uint32_t seed = 123456789;
static inline uint32_t prng() {
    seed = seed * 1103515245 + 12345;
    return seed;
}

/* Vector types for different architectures */
#ifdef __AVX512F__
#include <immintrin.h>
typedef __m512i v16si;
typedef __m512 v16sf;
typedef __mmask16 kmask;
#elif defined(__AVX2__)
#include <immintrin.h>
typedef __m256i v8si;
typedef __m256 v8sf;
#elif defined(__ARM_NEON)
#include <arm_neon.h>
typedef int32x4_t v4si;
typedef float32x4_t v4sf;
#else
/* Fallback to generic types */
typedef int32_t v4si __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));
#endif

/* Function to inhibit optimization */
static inline void inhibit_opt(volatile int* var) {
    asm volatile("" : "+r"(*var));
}

/* Complex expression with many temporaries */
__attribute__((noinline, target("avx2")))
void test_many_args(int* restrict out, const int* restrict in1, 
                    const int* restrict in2, const int* restrict in3,
                    int n) {
    volatile int iter_counter = 0;
    
    for (int i = 0; i < n; i += 8) {
        inhibit_opt(&iter_counter);
        
        /* Load multiple vectors - creates many temporaries */
        v8si v1 = _mm256_loadu_si256((const __m256i*)(in1 + i));
        v8si v2 = _mm256_loadu_si256((const __m256i*)(in2 + i));
        v8si v3 = _mm256_loadu_si256((const __m256i*)(in3 + i));
        v8si v4 = _mm256_loadu_si256((const __m256i*)(in1 + i + 8));
        v8si v5 = _mm256_loadu_si256((const __m256i*)(in2 + i + 8));
        
        /* Complex multi-statement expression with many intermediates */
        v8si t1 = _mm256_add_epi32(v1, v2);
        v8si t2 = _mm256_sub_epi32(v3, v4);
        v8si t3 = _mm256_mullo_epi32(t1, t2);
        v8si t4 = _mm256_slli_epi32(v5, 2);
        v8si t5 = _mm256_xor_si256(t3, t4);
        
        /* Extended inline asm with 11 operands */
        v8si result;
        asm volatile (
            "vpaddd %0, %1, %2\n\t"
            "vpsubd %0, %0, %3\n\t"
            "vpmulld %0, %0, %4\n\t"
            "vpslld $2, %5, %6\n\t"
            "vpxor %0, %0, %6\n\t"
            : "=&x"(result)
            : "x"(v1), "x"(v2), "x"(v3), "x"(v4), 
              "x"(v5), "i"(2), "m"(*(const int(*)[8])(in1 + i)),
              "m"(*(const int(*)[8])(in2 + i)), 
              "m"(*(const int(*)[8])(in3 + i)),
              "m"(*(const int(*)[8])(in1 + i + 8))
            : "memory"
        );
        
        /* Another complex shuffle-like operation using builtins */
        int indices[16] = {0,8,1,9,2,10,3,11,4,12,5,13,6,14,7,15};
        v8si shuffled;
        
        /* This may expand to optab with many arguments */
        for (int j = 0; j < 8; j++) {
            int idx = indices[j];
            int val = (idx < 8) ? 
                     ((int*)&v1)[idx] : 
                     ((int*)&v2)[idx - 8];
            ((int*)&shuffled)[j] = val;
        }
        
        /* Final blend operation */
        v8si final_result = _mm256_blendv_epi8(result, shuffled, t5);
        
        _mm256_storeu_si256((__m256i*)(out + i), final_result);
        
        iter_counter++;
    }
}

/* Alternative function using vector builtins with many arguments */
__attribute__((noinline, target("default")))
void test_vector_builtins(float* restrict out, const float* restrict in,
                         int n) {
    typedef float v8f __attribute__((vector_size(32)));
    
    for (int i = 0; i < n; i += 8) {
        /* Load 4 vectors */
        v8f v1 = *(const v8f*)(in + i);
        v8f v2 = *(const v8f*)(in + i + 8);
        v8f v3 = *(const v8f*)(in + i + 16);
        v8f v4 = *(const v8f*)(in + i + 24);
        
        /* Complex conversion chain that might use many-argument optab */
        v8f t1 = __builtin_convertvector(
                 __builtin_convertvector(v1, __typeof__((int){0})), 
                 __typeof__((float){0}));
        
        /* Extended asm with 10 operands for float operations */
        v8f result;
        asm volatile (
            "vaddps %0, %1, %2\n\t"
            "vmulps %0, %0, %3\n\t"
            "vfmadd132ps %0, %4, %5\n\t"
            : "=&x"(result)
            : "x"(v1), "x"(v2), "x"(v3), "x"(v4),
              "m"(*(const float(*)[8])(in + i)),
              "m"(*(const float(*)[8])(in + i + 8)),
              "m"(*(const float(*)[8])(in + i + 16)),
              "m"(*(const float(*)[8])(in + i + 24)),
              "i"(0x3F800000)  /* 1.0f as immediate */
            : "memory"
        );
        
        *(v8f*)(out + i) = result;
    }
}

/* ARM NEON version */
#ifdef __ARM_NEON
__attribute__((noinline))
void test_neon_many_args(int32_t* restrict out, const int32_t* restrict in,
                        int n) {
    volatile int iter = 0;
    
    for (int i = 0; i < n; i += 4) {
        inhibit_opt(&iter);
        
        int32x4_t v1 = vld1q_s32(in + i);
        int32x4_t v2 = vld1q_s32(in + i + 4);
        int32x4_t v3 = vld1q_s32(in + i + 8);
        int32x4_t v4 = vld1q_s32(in + i + 12);
        
        /* Complex table lookup - may require many arguments */
        int32x4_t result;
        uint8x16_t mask = {0,1,2,3, 4,5,6,7, 8,9,10,11, 12,13,14,15};
        
        /* Extended asm with 11 operands for ARM */
        asm volatile (
            "vadd.i32 %0, %1, %2\n\t"
            "vsub.i32 %0, %0, %3\n\t"
            "vmul.i32 %0, %0, %4\n\t"
            : "=&w"(result)
            : "w"(v1), "w"(v2), "w"(v3), "w"(v4),
              "m"(*(const int32_t(*)[4])(in + i)),
              "m"(*(const int32_t(*)[4])(in + i + 4)),
              "m"(*(const int32_t(*)[4])(in + i + 8)),
              "m"(*(const int32_t(*)[4])(in + i + 12)),
              "I"(2), "I"(3)  /* immediates */
            : "memory"
        );
        
        vst1q_s32(out + i, result);
        iter++;
    }
}
#endif

int main() {
    const int N = 1024;
    int* data1 = aligned_alloc(32, N * sizeof(int));
    int* data2 = aligned_alloc(32, N * sizeof(int));
    int* data3 = aligned_alloc(32, N * sizeof(int));
    int* output = aligned_alloc(32, N * sizeof(int));
    
    /* Initialize with pseudo-random data */
    for (int i = 0; i < N; i++) {
        data1[i] = (int)prng();
        data2[i] = (int)prng();
        data3[i] = (int)prng();
    }
    
    /* Test the many-argument functions */
    test_many_args(output, data1, data2, data3, N);
    
    /* Compute checksum */
    uint64_t checksum = 0;
    for (int i = 0; i < N; i++) {
        checksum += (uint64_t)output[i];
    }
    
    printf("Checksum: %llu\n", (unsigned long long)checksum);
    
    /* Test float version */
    float* fdata = aligned_alloc(32, N * sizeof(float));
    float* fout = aligned_alloc(32, N * sizeof(float));
    
    for (int i = 0; i < N; i++) {
        fdata[i] = (float)prng() / 1000.0f;
    }
    
    test_vector_builtins(fout, fdata, N);
    
    float fsum = 0;
    for (int i = 0; i < N; i++) {
        fsum += fout[i];
    }
    printf("Float checksum: %f\n", fsum);
    
    free(data1);
    free(data2);
    free(data3);
    free(output);
    free(fdata);
    free(fout);
    
    return 0;
}
