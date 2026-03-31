/* Primary test file with hot loop and register pressure */
#include <stdint.h>
#include <stdio.h>

/* Force no optimization on helper functions */
#define NOINLINE __attribute__((noinline))
#define VOLATILE_VAR volatile int

/* Vector types to increase register pressure */
typedef int v4si __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));
typedef double v2df __attribute__((vector_size(16)));

/* External helper functions from second compilation unit */
NOINLINE struct ComplexResult compute_complex(int a, int b, float c, double d);
NOINLINE v4si process_vector(v4si vec, int scalar);
NOINLINE struct MixedData mixed_operations(long l, double d, float f);

/* Complex return type to force register pressure */
struct ComplexResult {
    int int_result;
    float float_result;
    double double_result;
    long long_result;
};

struct MixedData {
    v4si vectors[2];
    double doubles[3];
    int ints[4];
};

/* Volatile to prevent dead code elimination */
static VOLATILE_VAR iteration_count = 1000;

/* Main test function with extreme register pressure */
NOINLINE static long test_function(int seed) {
    /* Declare many variables of different types */
    int a = seed + 1;
    int b = seed * 2;
    int c = seed / 3;
    int d = seed - 4;
    int e = seed % 5;
    int f = seed | 0xFF;
    int g = seed & 0x0F;
    int h = seed ^ 0x55;
    
    float fa = seed * 1.1f;
    float fb = seed * 2.2f;
    float fc = seed * 3.3f;
    float fd = seed * 4.4f;
    
    double da = seed * 1.111;
    double db = seed * 2.222;
    double dc = seed * 3.333;
    double dd = seed * 4.444;
    
    long la = seed * 1000L;
    long lb = seed * 2000L;
    long lc = seed * 3000L;
    long ld = seed * 4000L;
    
    /* Vector variables */
    v4si vec1 = {a, b, c, d};
    v4si vec2 = {e, f, g, h};
    v4sf vecf1 = {fa, fb, fc, fd};
    v2df vecd1 = {da, db};
    
    /* Complex chain of interdependent operations */
    /* This creates many pseudo-registers with multiple uses */
    int t1 = a + b;
    int t2 = t1 * c;          /* t1 used here */
    int t3 = t2 - d;          /* t2 used here */
    int t4 = t3 | e;          /* t3 used here */
    int t5 = t4 & f;          /* t4 used here */
    int t6 = t5 ^ g;          /* t5 used here */
    int t7 = t6 + h;          /* t6 used here */
    
    float ft1 = fa + fb;
    float ft2 = ft1 * fc;     /* ft1 used here */
    float ft3 = ft2 - fd;     /* ft2 used here */
    float ft4 = ft3 / fa;     /* ft3 used here */
    
    double dt1 = da + db;
    double dt2 = dt1 * dc;    /* dt1 used here */
    double dt3 = dt2 - dd;    /* dt2 used here */
    double dt4 = dt3 / da;    /* dt3 used here */
    
    long lt1 = la + lb;
    long lt2 = lt1 * lc;      /* lt1 used here */
    long lt3 = lt2 - ld;      /* lt2 used here */
    long lt4 = lt3 / la;      /* lt3 used here */
    
    /* Vector operations */
    v4si vt1 = vec1 + vec2;
    v4si vt2 = vt1 * vec1;    /* vt1 used here */
    v4si vt3 = vt2 - vec2;    /* vt2 used here */
    
    /* More operations creating register pressure */
    int t8 = t7 * 2;
    int t9 = t8 + t4;         /* t4 used again */
    int t10 = t9 - t2;        /* t2 used again */
    int t11 = t10 | t6;       /* t6 used again */
    
    /* Artificial register pressure through inline assembly */
    /* Clobber many registers to force spilling */
    asm volatile(
        "# Artificial register pressure\n"
        "mov r0, %0\n"
        "mov r1, %1\n"
        "add r2, r0, r1\n"
        : 
        : "r"(t11), "r"(t7)
        : "r0", "r1", "r2", "r3", "r4", "r5", "r6", "r7", 
          "r8", "r9", "r10", "r11", "r12", "memory"
    );
    
    /* More operations after assembly clobber */
    float ft5 = ft4 * 2.0f;
    float ft6 = ft5 + ft2;    /* ft2 used again */
    
    double dt5 = dt4 * 2.0;
    double dt6 = dt5 + dt2;   /* dt2 used again */
    
    long lt5 = lt4 * 2L;
    long lt6 = lt5 + lt2;     /* lt2 used again */
    
    /* Call external functions to increase inter-procedural pressure */
    struct ComplexResult cr1 = compute_complex(t11, t10, ft6, dt6);
    v4si processed_vec = process_vector(vt3, t9);
    struct MixedData md = mixed_operations(lt6, dt6, ft6);
    
    /* Final computation using all temporaries */
    long final_result = 
        (long)t11 + (long)t10 + (long)t9 + (long)t8 +
        (long)t7 + (long)t6 + (long)t5 + (long)t4 +
        (long)t3 + (long)t2 + (long)t1 +
        (long)(ft6 * 1000) + (long)(dt6 * 1000) + lt6 +
        cr1.int_result + cr1.long_result +
        processed_vec[0] + processed_vec[1] +
        md.ints[0] + md.ints[1];
    
    /* Force all variables to be used */
    asm volatile("" : : "r"(final_result), "r"(cr1.float_result), 
                  "r"(cr1.double_result), "r"(md.doubles[0]) : "memory");
    
    return final_result;
}

int main(void) {
    long total = 0;
    VOLATILE_VAR count = iteration_count;
    
    /* Hot loop to trigger optimization */
    for (VOLATILE_VAR i = 0; i < count; i++) {
        total += test_function(i);
        
        /* Prevent loop unrolling */
        asm volatile("" : : : "memory");
    }
    
    printf("Result: %ld\n", total);
    return (int)(total % 1000);
}
