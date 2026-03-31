/* early-remat-test.c - Test case for GCC early rematerialization coverage */

/* Force no inlining for helper functions */
#define NOINLINE __attribute__((noinline))

/* Vector type to increase register pressure */
typedef int v4si __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));

/* Volatile sink to prevent optimizations */
volatile int global_sink;
volatile float float_sink;

/* Non-inline function with many arguments */
NOINLINE int use_many_args(int a, int b, int c, float d, double e, 
                           long f, short g, char h, v4si *vec) {
    /* Use all arguments to prevent elimination */
    int sum = a + b + c;
    sum += (int)d + (int)e + (int)f + g + h;
    if (vec) sum += (*vec)[0];
    global_sink = sum;
    return sum & 1;
}

/* Another non-inline function for floating point */
NOINLINE float fp_operations(float a, float b, float c, float d,
                             float e, float f, float g, float h) {
    /* Complex FP expression that can't be easily optimized */
    float t1 = a * b + c / d;
    float t2 = e - f * g + h;
    float t3 = t1 * t2 - a / (b + 0.1f);
    float t4 = c * d - e / (f + 0.2f);
    float result = t3 + t4 - (a * c) / (e * g + 1.0f);
    float_sink = result;
    return result;
}

/* Function to create register pressure with mixed operations */
NOINLINE int compute_value(int base, int iter) {
    /* Many independent computations to create temporaries */
    int t1 = base * 3 + iter;
    int t2 = base / 2 - iter;
    int t3 = t1 * t2 + base;
    int t4 = (base << 3) | (iter & 0xFF);
    int t5 = t3 ^ t4;
    int t6 = t5 * 7 - t2;
    int t7 = t6 + (t1 >> 2);
    int t8 = t7 * 11 % 1023;
    int t9 = t8 | (t3 & t4);
    int t10 = t9 * 13 + t6;
    
    return t10;
}

int main(void) {
    /* Initialize arrays to feed computations */
    int array1[256];
    float array2[256];
    double array3[256];
    
    for (int i = 0; i < 256; i++) {
        array1[i] = i * 3;
        array2[i] = i * 1.5f;
        array3[i] = i * 2.7;
    }
    
    /* Volatile pointer to force memory operations */
    volatile int* volatile_ptr = array1;
    
    /* Vector variables for wide register usage */
    v4si vec1 = {1, 2, 3, 4};
    v4si vec2 = {5, 6, 7, 8};
    v4sf fvec1 = {1.0f, 2.0f, 3.0f, 4.0f};
    v4sf fvec2 = {5.0f, 6.0f, 7.0f, 8.0f};
    
    int total = 0;
    
    /* Outer loop to increase pressure */
    for (int outer = 0; outer < 100; outer++) {
        /* Nested loop with high register pressure */
        for (int i = 0; i < 100; i++) {
            /* Many independent integer computations */
            int a = compute_value(i, outer);
            int b = compute_value(outer, i);
            int c = a * b + i;
            int d = (a << 2) | (b & 0xF);
            int e = c ^ d;
            int f = e * 3 - a;
            int g = f + (b >> 1);
            int h = g * 5 % 511;
            int j = h | (c & d);
            int k = j * 7 + f;
            int l = k * 11 - g;
            int m = l ^ h;
            int n = m * 13 + j;
            int o = n / (a + 1) + k;
            int p = o * 17 % 1023;
            int q = p | (l & m);
            int r = q * 19 + n;
            int s = r - o * 3;
            int t = s ^ p;
            int u = t * 23 + q;
            int v = u / (b + 1) - r;
            int w = v * 29 % 2047;
            int x = w | (s & t);
            int y = x * 31 + u;
            int z = y - v * 5;
            
            /* Floating point computations mixed in */
            float fa = array2[i & 255] * 1.1f + outer;
            float fb = array2[(i + 1) & 255] * 0.9f - outer;
            float fc = fa * fb + array2[(i + 2) & 255];
            float fd = fa / (fb + 0.5f) - array2[(i + 3) & 255];
            float fe = fc * fd - fa / (fb * 2.0f);
            float ff = fe + fc / (fd + 0.3f);
            float fg = ff * 1.7f - fe;
            float fh = fg / (ff + 0.7f) + fd;
            
            /* Vector operations */
            v4si vec_tmp1 = vec1 * vec2 + i;
            v4si vec_tmp2 = vec1 << (i & 3);
            v4si vec_tmp3 = vec_tmp1 ^ vec_tmp2;
            
            v4sf fvec_tmp1 = fvec1 * fvec2 + fa;
            v4sf fvec_tmp2 = fvec1 / (fvec2 + 0.1f);
            v4sf fvec_tmp3 = fvec_tmp1 - fvec_tmp2 * 0.5f;
            
            /* Inline assembly that clobbers registers */
            /* For x86-64 */
            asm volatile("" : : : "rax", "rbx", "rcx", "rdx", 
                                       "xmm0", "xmm1", "xmm2", "xmm3",
                                       "memory");
            
            /* Call function with many arguments */
            int result1 = use_many_args(a, b, c, fa, array3[i & 255],
                                       (long)z, (short)w, (char)x, &vec_tmp3);
            
            /* More computations after call (forces rematerialization) */
            int a2 = a * 2 + result1;
            int b2 = b / 2 - result1;
            int c2 = c ^ result1;
            float fa2 = fa * 2.0f + result1;
            float fb2 = fb / 2.0f - result1;
            
            /* Another assembly clobber */
            asm volatile("" : : : "rsi", "rdi", "r8", "r9",
                                       "xmm4", "xmm5", "xmm6", "xmm7",
                                       "memory");
            
            /* Call floating point function */
            float fp_result = fp_operations(fa2, fb2, fc, fd, 
                                           fe, ff, fg, fh);
            
            /* Volatile memory access to prevent optimization */
            global_sink = z;
            *volatile_ptr = y;
            
            /* More independent computations */
            int final1 = a2 * b2 + c2;
            int final2 = (a2 << 3) | (b2 & 0xFF);
            int final3 = final1 ^ final2;
            int final4 = final3 * 37 - a2;
            int final5 = final4 + (int)(fp_result * 100);
            
            /* Use vector results */
            vec_tmp3 = vec_tmp3 + final5;
            fvec_tmp3 = fvec_tmp3 * (float)final5;
            
            /* Accumulate to total (prevents dead code elimination) */
            total += final5 + vec_tmp3[0] + (int)fvec_tmp3[0];
            
            /* Another assembly barrier */
            asm volatile("" : : : "r10", "r11", "r12", "r13",
                                       "xmm8", "xmm9", "xmm10", "xmm11",
                                       "memory");
        }
        
        /* Modify vectors to prevent loop invariant removal */
        vec1 = vec1 + outer;
        vec2 = vec2 - outer;
        fvec1 = fvec1 * (1.0f + outer * 0.01f);
        fvec2 = fvec2 / (1.0f + outer * 0.01f);
    }
    
    /* Print result to prevent optimization */
    printf("Result: %d\n", total);
    
    return total & 0xFF;
}
