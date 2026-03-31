/* Primary test file with hot loop and register pressure */
#include <stdint.h>
#include <string.h>

/* Force pseudo-register creation through complex expressions */
#define FORCE_REG(N) asm volatile("" : : "r"(N) : "memory")

/* Volatile variables to prevent optimization */
volatile int g_volatile_counter = 1000;
volatile float g_volatile_float = 3.14159f;
volatile double g_volatile_double = 2.71828;

/* Vector types for wide register pressure */
typedef int v4si __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));
typedef double v2df __attribute__((vector_size(16)));

/* Complex struct for inter-procedural pressure */
struct MultiRegStruct {
    long a, b, c, d;
    double x, y;
    v4si vec;
};

/* Forward declarations from helper.c */
struct MultiRegStruct __attribute__((noinline)) 
helper_func1(struct MultiRegStruct s1, struct MultiRegStruct s2);

struct MultiRegStruct __attribute__((noinline))
helper_func2(struct MultiRegStruct s1, int a, float b, double c, v4si v);

int __attribute__((noinline))
helper_func3(int a, int b, int c, int d, int e, int f, int g, int h);

/* Main test function with extreme register pressure */
static long __attribute__((noinline, optimize("O3")))
test_function(int seed) {
    /* Declare many variables of different types */
    int t1 = seed + 1;
    int t2 = seed * 2;
    int t3 = t1 ^ t2;
    int t4 = t3 << 3;
    int t5 = t4 | 0xFF;
    int t6 = t5 - t2;
    int t7 = t6 * t1;
    int t8 = t7 / (t3 ? t3 : 1);
    int t9 = t8 & 0xFFFF;
    int t10 = t9 >> 2;
    
    float f1 = g_volatile_float * t1;
    float f2 = f1 + t2;
    float f3 = f2 * 2.5f;
    float f4 = f3 - f1;
    float f5 = f4 / (f2 ? f2 : 1.0f);
    
    double d1 = g_volatile_double * t3;
    double d2 = d1 + t4;
    double d3 = d2 * 1.618034;
    double d4 = d3 - d1;
    double d5 = d4 / (d2 ? d2 : 1.0);
    
    long l1 = (long)t1 * t2;
    long l2 = l1 + t3;
    long l3 = l2 << 4;
    long l4 = l3 ^ 0xAAAAAAAA;
    long l5 = l4 - l1;
    
    /* Vector operations for wide register pressure */
    v4si v1 = {t1, t2, t3, t4};
    v4si v2 = {t5, t6, t7, t8};
    v4si v3 = v1 + v2;
    v4si v4 = v1 * v2;
    v4si v5 = v3 & v4;
    v4si v6 = v5 | ~v4;
    
    v4sf vf1 = {f1, f2, f3, f4};
    v4sf vf2 = vf1 * 2.0f;
    v4sf vf3 = vf1 + vf2;
    
    v2df vd1 = {d1, d2};
    v2df vd2 = {d3, d4};
    v2df vd3 = vd1 * vd2;
    
    /* Create complex dependency chain */
    int chain1 = t1 + t2;
    int chain2 = chain1 * t3;      /* Pseudo-reg for chain1 used here */
    int chain3 = chain2 - t4;      /* And potentially here */
    int chain4 = chain3 ^ t5;
    int chain5 = chain4 | t6;
    int chain6 = chain5 << t7;
    int chain7 = chain6 >> t8;
    int chain8 = chain7 & t9;
    int chain9 = chain8 + t10;
    
    /* Force register pressure with inline assembly */
    asm volatile(
        "/* Clobber physical registers to increase pressure */\n\t"
        "nop"
        : 
        : 
        : "r0", "r1", "r2", "r3", "r4", "r5", "r6", "r7",
          "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15",
          "memory"
    );
    
    /* More interdependent computations */
    float f6 = f5 * chain1;
    float f7 = f6 + chain2;
    float f8 = f7 * chain3;
    
    double d6 = d5 * chain4;
    double d7 = d6 + chain5;
    double d8 = d7 * chain6;
    
    /* Use all temporaries in final computation */
    long result = l5;
    result += (long)(f8 * 1000);
    result += (long)(d8 * 1000);
    result += chain9;
    
    /* Use vector results */
    int vsum = v6[0] + v6[1] + v6[2] + v6[3];
    result += vsum;
    
    float vfsum = vf3[0] + vf3[1] + vf3[2] + vf3[3];
    result += (long)(vfsum * 100);
    
    double vdsum = vd3[0] + vd3[1];
    result += (long)(vdsum * 1000);
    
    /* Force pseudo-register references through memory operations */
    FORCE_REG(t1); FORCE_REG(t2); FORCE_REG(t3); FORCE_REG(t4);
    FORCE_REG(t5); FORCE_REG(t6); FORCE_REG(t7); FORCE_REG(t8);
    FORCE_REG(t9); FORCE_REG(t10);
    
    return result;
}

/* Function with inter-procedural register pressure */
static long __attribute__((noinline))
cross_function_pressure(int base) {
    struct MultiRegStruct s1 = {
        .a = base, .b = base + 1, .c = base + 2, .d = base + 3,
        .x = base * 1.5, .y = base * 2.5,
        .vec = {base, base+1, base+2, base+3}
    };
    
    struct MultiRegStruct s2 = {
        .a = base * 2, .b = base * 3, .c = base * 4, .d = base * 5,
        .x = base * 3.5, .y = base * 4.5,
        .vec = {base*2, base*3, base*4, base*5}
    };
    
    /* Call helper functions that use many registers */
    struct MultiRegStruct s3 = helper_func1(s1, s2);
    
    int multi_int = helper_func3(
        s1.a, s1.b, s1.c, s1.d,
        s2.a, s2.b, s2.c, s2.d
    );
    
    struct MultiRegStruct s4 = helper_func2(
        s3, multi_int, g_volatile_float, g_volatile_double, s1.vec
    );
    
    /* Use all results */
    long result = s4.a + s4.b + s4.c + s4.d;
    result += (long)(s4.x + s4.y);
    result += s4.vec[0] + s4.vec[1] + s4.vec[2] + s4.vec[3];
    
    return result;
}

int main() {
    long total = 0;
    int iterations = g_volatile_counter;
    
    /* Hot loop to trigger early rematerialization */
    for (int i = 0; i < iterations; i++) {
        /* Mix both test functions */
        total += test_function(i);
        
        if (i % 10 == 0) {
            total += cross_function_pressure(i);
        }
        
        /* Prevent loop unrolling */
        asm volatile("" : "+r"(total) : : "memory");
    }
    
    /* Use result to prevent elimination */
    volatile long sink = total;
    return sink > 0 ? 0 : 1;
}
