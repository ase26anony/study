/* Main test driver with high register pressure loop */
#include <stdint.h>
#include <stdio.h>

/* Volatile to prevent optimization */
volatile int g_iterations = 1000;

/* External helper functions from second compilation unit */
extern struct ComplexResult helper_func1(struct ComplexArgs a, struct ComplexArgs b);
extern struct ComplexResult helper_func2(struct VectorArgs a, struct VectorArgs b);
extern struct ComplexResult helper_func3(struct MixedArgs a, struct MixedArgs b);

/* Complex structs to force register pressure */
struct ComplexArgs {
    long a, b, c, d;
    double x, y;
    float f1, f2;
};

struct VectorArgs {
    int v1[4];
    int v2[4];
    double dv[2];
};

struct MixedArgs {
    long l1, l2;
    double d1, d2;
    float f1, f2, f3, f4;
    int i1, i2, i3, i4;
};

struct ComplexResult {
    long r1, r2;
    double rd1, rd2;
    float rf1, rf2;
    int ri[4];
};

/* Vector types for wide register pressure */
typedef int v4si __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));
typedef double v2df __attribute__((vector_size(16)));

/* Noinline to prevent optimization */
__attribute__((noinline, noipa))
struct ComplexResult test_function(struct ComplexArgs arg1, struct ComplexArgs arg2) {
    /* Declare many local variables of different types */
    long t1, t2, t3, t4, t5, t6, t7, t8, t9, t10;
    double d1, d2, d3, d4, d5, d6, d7, d8, d9, d10;
    float f1, f2, f3, f4, f5, f6, f7, f8, f9, f10;
    int i1, i2, i3, i4, i5, i6, i7, i8, i9, i10;
    
    /* Vector variables for wide register pressure */
    v4si v1, v2, v3, v4, v5;
    v4sf vf1, vf2, vf3, vf4;
    v2df vd1, vd2, vd3;
    
    /* Initialize with complex expressions */
    t1 = arg1.a + arg2.a;
    t2 = arg1.b * arg2.b;
    t3 = t1 - t2;  /* t3 depends on t1 and t2 */
    t4 = t3 * t1;  /* t4 depends on t3 and t1 */
    t5 = t4 / (t2 + 1);
    t6 = t5 ^ t3;  /* XOR operation */
    t7 = t6 | t4;
    t8 = t7 & t5;
    t9 = t8 << 2;
    t10 = t9 >> 1;
    
    /* Floating point computations with dependencies */
    d1 = (double)arg1.x + (double)arg2.x;
    d2 = (double)arg1.y * (double)arg2.y;
    d3 = d1 - d2;  /* d3 depends on d1 and d2 */
    d4 = d3 * d1;  /* d4 depends on d3 and d1 */
    d5 = d4 / (d2 + 1.0);
    d6 = d5 * d3;
    d7 = d6 + d4;
    d8 = d7 - d5;
    d9 = d8 * 2.0;
    d10 = d9 / 3.0;
    
    /* Float computations */
    f1 = arg1.f1 + arg2.f1;
    f2 = arg1.f2 * arg2.f2;
    f3 = f1 - f2;  /* f3 depends on f1 and f2 */
    f4 = f3 * f1;  /* f4 depends on f3 and f1 */
    f5 = f4 / (f2 + 1.0f);
    f6 = f5 * f3;
    f7 = f6 + f4;
    f8 = f7 - f5;
    f9 = f8 * 2.0f;
    f10 = f9 / 3.0f;
    
    /* Integer computations with many dependencies */
    i1 = (int)(t1 & 0xFFFFFFFF);
    i2 = (int)(t2 & 0xFFFFFFFF);
    i3 = i1 + i2;  /* i3 depends on i1 and i2 */
    i4 = i3 * i1;  /* i4 depends on i3 and i1 */
    i5 = i4 / (i2 + 1);
    i6 = i5 ^ i3;
    i7 = i6 | i4;
    i8 = i7 & i5;
    i9 = i8 << 1;
    i10 = i9 >> 2;
    
    /* Vector operations - these use wide registers */
    v1 = (v4si){i1, i2, i3, i4};
    v2 = (v4si){i5, i6, i7, i8};
    v3 = v1 + v2;
    v4 = v1 * v2;
    v5 = v3 - v4;
    
    vf1 = (v4sf){f1, f2, f3, f4};
    vf2 = (v4sf){f5, f6, f7, f8};
    vf3 = vf1 + vf2;
    vf4 = vf1 * vf2;
    
    vd1 = (v2df){d1, d2};
    vd2 = (v2df){d3, d4};
    vd3 = vd1 + vd2;
    
    /* Artificial register pressure with inline assembly */
    /* Clobber many registers to force spilling */
    asm volatile (
        "# Artificial register pressure\n"
        "mov r0, %0\n"
        "mov r1, %1\n"
        "add r2, r0, r1\n"
        :
        : "r" (t1), "r" (t2)
        : "r0", "r1", "r2", "r3", "r4", "r5", "r6", "r7", 
          "r8", "r9", "r10", "r11", "r12", "memory"
    );
    
    /* More complex interdependent computations */
    /* This creates a long dependency chain */
    long chain1 = t1 + t2;
    long chain2 = chain1 * t3;  /* chain2 depends on chain1 and t3 */
    long chain3 = chain2 - t4;  /* chain3 depends on chain2 and t4 */
    long chain4 = chain3 / t5;  /* chain4 depends on chain3 and t5 */
    long chain5 = chain4 ^ t6;  /* chain5 depends on chain4 and t6 */
    long chain6 = chain5 | t7;  /* chain6 depends on chain5 and t7 */
    long chain7 = chain6 & t8;  /* chain7 depends on chain6 and t8 */
    long chain8 = chain7 << t9; /* chain8 depends on chain7 and t9 */
    long chain9 = chain8 >> t10; /* chain9 depends on chain8 and t10 */
    
    /* Mix all computations for final result */
    struct ComplexResult result;
    result.r1 = t10 + chain9;
    result.r2 = t9 - chain8;
    result.rd1 = d10 + (double)chain7;
    result.rd2 = d9 - (double)chain6;
    result.rf1 = f10 + (float)chain5;
    result.rf2 = f9 - (float)chain4;
    
    /* Use vector results */
    int *vp = (int*)&v5;
    for (int j = 0; j < 4; j++) {
        result.ri[j] = vp[j] + i10;
    }
    
    /* Force all variables to be used */
    volatile long sink = t1 + t2 + t3 + t4 + t5 + t6 + t7 + t8 + t9 + t10;
    sink += (long)d1 + (long)d2 + (long)d3 + (long)d4 + (long)d5;
    sink += (long)d6 + (long)d7 + (long)d8 + (long)d9 + (long)d10;
    (void)sink;  /* Prevent unused variable warning */
    
    return result;
}

int main() {
    struct ComplexResult total = {0};
    struct ComplexArgs arg1 = {1, 2, 3, 4, 5.0, 6.0, 7.0f, 8.0f};
    struct ComplexArgs arg2 = {9, 10, 11, 12, 13.0, 14.0, 15.0f, 16.0f};
    
    /* Loop to increase register pressure and trigger rematerialization */
    for (int i = 0; i < g_iterations; i++) {
        /* Modify arguments slightly each iteration */
        arg1.a += i;
        arg2.b -= i;
        
        /* Call test function - this should create high register pressure */
        struct ComplexResult r = test_function(arg1, arg2);
        
        /* Accumulate results to prevent dead code elimination */
        total.r1 += r.r1;
        total.r2 += r.r2;
        total.rd1 += r.rd1;
        total.rd2 += r.rd2;
        total.rf1 += r.rf1;
        total.rf2 += r.rf2;
        for (int j = 0; j < 4; j++) {
            total.ri[j] += r.ri[j];
        }
        
        /* Call helper functions from other compilation unit */
        if (i % 3 == 0) {
            struct ComplexResult h1 = helper_func1(arg1, arg2);
            total.r1 += h1.r1;
        }
        if (i % 5 == 0) {
            struct VectorArgs va1 = {{1,2,3,4}, {5,6,7,8}, {9.0, 10.0}};
            struct VectorArgs va2 = {{11,12,13,14}, {15,16,17,18}, {19.0, 20.0}};
            struct ComplexResult h2 = helper_func2(va1, va2);
            total.rd1 += h2.rd1;
        }
    }
    
    /* Print result to prevent optimization */
    printf("Result: %ld %ld %f %f %f %f %d %d %d %d\n", 
           total.r1, total.r2, total.rd1, total.rd2, 
           total.rf1, total.rf2,
           total.ri[0], total.ri[1], total.ri[2], total.ri[3]);
    
    return 0;
}
