/* early-remat-test.c - Test case for GCC early rematerialization coverage */

/* Force no-inline attribute for helper functions */
#define NOINLINE __attribute__((noinline))

/* Vector type to increase register pressure */
typedef int v4si __attribute__((vector_size(16)));

/* Volatile sink to prevent optimizations */
volatile int global_sink;
volatile float float_sink;
volatile double double_sink;

/* Non-inline function with many arguments */
NOINLINE int use_values(int a, int b, float c, double d, 
                        long e, short f, char g, int h) {
    /* Use all arguments to prevent elimination */
    return (a * b) + (int)c + (int)d + (int)e + f + g + h;
}

/* Another non-inline function for FP operations */
NOINLINE double fp_operations(double a, double b, double c, 
                              double d, double e, double f) {
    return ((a * b) + (c / d) - (e * f)) * 0.5;
}

/* Function to create complex expressions */
NOINLINE int complex_expr(int x, int y, int z, int w) {
    return ((x * y) << 2) + (z / (w ? w : 1)) - (x ^ y ^ z ^ w);
}

int main(void) {
    /* Initialize arrays with volatile reads to force memory ops */
    volatile int array[256];
    volatile float farray[256];
    volatile double darray[256];
    
    for (int i = 0; i < 256; i++) {
        array[i] = i;
        farray[i] = i * 1.5f;
        darray[i] = i * 2.5;
    }
    
    int result = 0;
    
    /* Nested loops to create register pressure */
    for (int outer = 0; outer < 100; outer++) {
        for (int i = 0; i < 100; i++) {
            /* Many independent arithmetic operations creating temporaries */
            int t1 = i * 3 + outer;
            int t2 = i / 2 - outer;
            int t3 = t1 ^ t2;
            int t4 = t1 * t2 + t3;
            int t5 = (t4 << 3) | (t3 & 0xFF);
            int t6 = t5 - t4 + t3 - t2 + t1;
            
            /* Floating point operations - different register class */
            float f1 = farray[i & 255] * 1.1f;
            float f2 = farray[(i + 1) & 255] / 2.2f;
            float f3 = f1 + f2 * 3.3f - 4.4f;
            float f4 = f3 * f2 / f1 + 5.5f;
            
            /* Double precision - more register pressure */
            double d1 = darray[i & 255] * 1.01;
            double d2 = darray[(i + 2) & 255] / 2.02;
            double d3 = d1 * 1.5 + d2 * 2.5;
            double d4 = d3 / d1 - d2 * 0.75;
            double d5 = d4 * 3.14159 + d3 * 2.71828;
            
            /* Vector operations */
            v4si v1 = {i, i+1, i+2, i+3};
            v4si v2 = {outer, outer+1, outer+2, outer+3};
            v4si v3 = v1 + v2;
            v4si v4 = v1 * v2;
            v4si v5 = v3 - v4;
            
            /* Inline assembly that clobbers registers */
            /* For x86-64 */
            asm volatile("" : : : "rax", "rbx", "rcx", "rdx", 
                                     "xmm0", "xmm1", "xmm2", "xmm3",
                                     "xmm4", "xmm5", "memory");
            
            /* More computations after clobber - forces rematerialization */
            int t7 = t6 * 2 + (i & 0xF);
            int t8 = t7 / (outer ? outer : 1) + t5;
            int t9 = (t8 << 1) | (t7 >> 1);
            
            float f5 = f4 * 6.6f + (i * 0.1f);
            float f6 = f5 / 7.7f - f3 * 0.5f;
            
            double d6 = d5 * 1.23456 + d4 * 0.98765;
            double d7 = d6 / 3.14159 - d3 * 0.31831;
            
            /* Call non-inline function with many arguments */
            int func_result = use_values(t7, t8, f5, d6, 
                                        (long)t9, (short)i, 
                                        (char)outer, t6);
            
            /* More FP function calls */
            double fp_result = fp_operations(d1, d2, d3, d4, d5, d6);
            
            /* Complex expression function */
            int complex_result = complex_expr(t1, t2, t3, t4);
            
            /* Volatile writes to prevent elimination */
            global_sink = t7 + t8 + t9;
            float_sink = f5 + f6;
            double_sink = d6 + d7;
            
            /* Use array with volatile access */
            array[i & 255] = func_result;
            farray[i & 255] = (float)fp_result;
            darray[i & 255] = fp_result;
            
            /* Another inline assembly clobber */
            /* For x86-64 SSE/AVX registers */
            asm volatile("" : : : "xmm6", "xmm7", "xmm8", "xmm9",
                                     "xmm10", "xmm11", "xmm12", "xmm13",
                                     "memory");
            
            /* Final computations that depend on many values */
            int final1 = (t7 * 3 + t8 * 5 - t9 * 7) & 0xFFF;
            int final2 = (func_result ^ complex_result) | (i << 16);
            float final3 = f5 * 2.0f + f6 * 3.0f - (float)outer * 0.5f;
            double final4 = d6 * 1.5 + d7 * 2.5 - fp_result * 0.5;
            
            /* Vector operation result extraction */
            int vsum = v5[0] + v5[1] + v5[2] + v5[3];
            
            /* Accumulate to final result (prevents dead code elimination) */
            result += final1 + final2 + (int)final3 + (int)final4 + vsum;
            
            /* Additional computations to increase pressure */
            for (int j = 0; j < 4; j++) {
                int temp1 = (i * j + outer) * 11;
                int temp2 = (i - j * outer) / 3;
                int temp3 = temp1 ^ temp2;
                float temp4 = (float)temp1 * 0.33f + (float)temp2 * 0.67f;
                double temp5 = (double)temp3 * 1.234;
                
                /* Small inline assembly */
                asm volatile("" : : : "rsi", "rdi", "memory");
                
                result += temp1 + temp2 + temp3 + (int)temp4 + (int)temp5;
            }
        }
        
        /* Outer loop computations */
        int outer_temp1 = outer * 17;
        int outer_temp2 = outer / 3;
        float outer_temp3 = (float)outer * 3.14f;
        double outer_temp4 = (double)outer * 2.71828;
        
        /* Call function from outer loop too */
        int outer_func = use_values(outer_temp1, outer_temp2, 
                                   outer_temp3, outer_temp4,
                                   outer, outer, outer, outer);
        
        result += outer_func;
    }
    
    /* Print result to prevent elimination */
    printf("Result: %d\n", result);
    
    return 0;
}
