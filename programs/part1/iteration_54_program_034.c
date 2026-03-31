/* early-remat-test.c - Test case for GCC early rematerialization coverage */

/* Prevent inlining of helper functions */
#define NOINLINE __attribute__((noinline))

/* Vector type to increase register pressure */
typedef int v4si __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));

/* Volatile sink to prevent optimizations */
volatile int global_sink;
volatile float float_sink;

/* Non-inline function with many arguments */
NOINLINE int use_values(int a, int b, float c, double d, 
                        int e, int f, float g, double h) {
    return (a * b) + (int)(c * d) + e - f + (int)(g * h);
}

/* Another non-inline function with vector arguments */
NOINLINE v4si use_vector(v4si a, v4si b, v4si c, v4si d) {
    return a + b * c - d;
}

/* Complex computation to create many temporaries */
NOINLINE int complex_computation(int base, int iter) {
    /* Many independent computations creating register pressure */
    int t1 = base * iter * 3;
    int t2 = iter << 4;
    int t3 = t1 / (iter + 1);
    int t4 = t2 - t3;
    int t5 = t4 * 7;
    int t6 = t5 + (iter % 13);
    int t7 = t6 ^ 0xABCD;
    int t8 = t7 & 0xFFF;
    int t9 = t8 | 0x1000;
    int t10 = t9 << 2;
    int t11 = t10 >> 1;
    int t12 = t11 * 3;
    int t13 = t12 / 5;
    int t14 = t13 + 17;
    int t15 = t14 - 8;
    int t16 = t15 * 11;
    int t17 = t16 % 19;
    int t18 = t17 ^ t1;
    int t19 = t18 & t2;
    int t20 = t19 | t3;
    
    return t20;
}

int main(void) {
    /* Initialize arrays to feed computations */
    int array1[256];
    float array2[256];
    double array3[256];
    
    for (int i = 0; i < 256; i++) {
        array1[i] = i * 2;
        array2[i] = i * 0.5f;
        array3[i] = i * 0.25;
    }
    
    /* Accumulator to prevent dead code elimination */
    int total = 0;
    float ftotal = 0.0f;
    double dtotal = 0.0;
    
    /* Nested loops with high register pressure */
    for (int outer = 0; outer < 100; outer++) {
        for (int inner = 0; inner < 256; inner++) {
            /* Many temporary variables with complex expressions */
            int idx = (outer * 256 + inner) % 256;
            
            /* Force register pressure with independent computations */
            int a = array1[idx] * 3 + outer;
            int b = inner * 7 - array1[(idx + 1) % 256];
            int c = a ^ b;
            int d = (a * b) / (inner + 1);
            int e = (c << 3) | (d >> 2);
            int f = complex_computation(e, inner);
            
            float fa = array2[idx] * 2.5f + outer * 0.1f;
            float fb = array2[(idx + 2) % 256] * 1.7f;
            float fc = fa / (fb + 0.001f);
            float fd = fc * 3.14f;
            
            double da = array3[idx] * 1.234;
            double db = array3[(idx + 3) % 256] * 0.987;
            double dc = da - db;
            double dd = dc * 2.71828;
            
            /* Inline assembly to clobber registers and force rematerialization */
            /* For x86-64 */
            asm volatile("" : : : "rax", "rbx", "rcx", "rdx", "rsi", "rdi", 
                         "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15",
                         "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5",
                         "xmm6", "xmm7", "xmm8", "xmm9", "xmm10", "xmm11",
                         "xmm12", "xmm13", "xmm14", "xmm15", "memory");
            
            /* Call function with many arguments - forces argument passing */
            int result = use_values(a, b, fa, da, c, d, fb, db);
            
            /* More computations after call */
            int g = result * 2 + f;
            float fg = fd * 1.5f + fc;
            double dg = dd * 1.1 + dc;
            
            /* Vector operations for additional register pressure */
            v4si vec1 = {a, b, c, d};
            v4si vec2 = {e, f, g, result};
            v4si vec3 = vec1 + vec2;
            v4si vec4 = vec1 * vec2;
            
            /* Another assembly clobber */
            asm volatile("" : : : "rax", "rbx", "rcx", "rdx", 
                         "xmm0", "xmm1", "xmm2", "xmm3", "memory");
            
            /* Use vector function */
            v4si vec_result = use_vector(vec1, vec2, vec3, vec4);
            
            /* Volatile writes to prevent optimization */
            global_sink = g;
            float_sink = fg;
            
            /* More independent computations */
            int h = (g * 13) % 17;
            float fh = fg * 2.0f - 1.0f;
            double dh = dg / 3.0;
            
            int i = h + (inner & 0xF);
            float fi = fh + (outer * 0.01f);
            double di = dh * 1.01;
            
            /* Final assembly clobber */
            asm volatile("" : : : "rax", "rbx", "rcx", "rdx", "rsi", "rdi",
                         "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5", "memory");
            
            /* Accumulate results */
            total += i + vec_result[0] + vec_result[1];
            ftotal += fi;
            dtotal += di;
            
            /* Additional complex expression chain */
            int j = (i * 3 + 7) % 31;
            int k = (j << 2) ^ 0xDEADBEEF;
            int l = (k >> 1) & 0x7FFF;
            int m = l * 5 - 3;
            int n = (m % 23) + 11;
            
            float fj = fi * 1.1f + 0.5f;
            float fk = fj / 2.0f - 0.25f;
            float fl = fk * 3.14f;
            
            total += n;
            ftotal += fl;
        }
        
        /* Periodic function call with different arguments */
        if (outer % 10 == 0) {
            int temp1 = outer * 100;
            int temp2 = total % 1000;
            float temp3 = ftotal * 0.01f;
            double temp4 = dtotal * 0.001;
            
            int call_result = use_values(temp1, temp2, temp3, temp4,
                                         total & 0xFF, outer, ftotal, dtotal);
            total += call_result;
        }
    }
    
    /* Print results to prevent optimization */
    printf("Result: %d, %f, %lf\n", total, ftotal, dtotal);
    
    return total > 0 ? 0 : 1;
}
