/* Test case for early rematerialization - main file */
#include <stdint.h>
#include <stdio.h>

/* Prevent optimization */
volatile int g_volatile_counter = 1000;

/* Vector types for register pressure */
typedef int v4si __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));
typedef double v2df __attribute__((vector_size(16)));

/* Complex struct for inter-procedural pressure */
struct ComplexData {
    v4si vec_int;
    v4sf vec_float;
    v2df vec_double;
    long long ints[4];
    double floats[4];
};

/* External helper functions (defined in second file) */
struct ComplexData __attribute__((noinline)) 
process_data(struct ComplexData a, struct ComplexData b, int count);

struct ComplexData __attribute__((noinline))
transform_vector(struct ComplexData input, v4si mask, v4sf scale);

/* Inline assembly to clobber registers and increase pressure */
#define CLOBBER_REGS() \
    asm volatile("" : : : \
        "r0", "r1", "r2", "r3", "r4", "r5", "r6", "r7", \
        "r8", "r9", "r10", "r11", "r12", \
        "memory")

/* Main test function with high register pressure */
static struct ComplexData __attribute__((noinline))
test_function(int seed) {
    /* Declare many local variables of different types */
    int a1, a2, a3, a4, a5, a6, a7, a8, a9, a10;
    float f1, f2, f3, f4, f5, f6, f7, f8;
    double d1, d2, d3, d4, d5, d6;
    long l1, l2, l3, l4, l5;
    v4si v1, v2, v3, v4, v5;
    v4sf vf1, vf2, vf3, vf4;
    v2df vd1, vd2, vd3;
    
    /* Initialize with seed-dependent values */
    a1 = seed * 1;
    a2 = seed * 2;
    a3 = seed * 3;
    a4 = seed * 4;
    a5 = seed * 5;
    f1 = seed * 1.1f;
    f2 = seed * 2.2f;
    f3 = seed * 3.3f;
    f4 = seed * 4.4f;
    d1 = seed * 1.11;
    d2 = seed * 2.22;
    l1 = seed * 100L;
    l2 = seed * 200L;
    
    /* Vector initialization */
    v1 = (v4si){a1, a2, a3, a4};
    v2 = (v4si){a5, a1 + 1, a2 + 2, a3 + 3};
    vf1 = (v4sf){f1, f2, f3, f4};
    vf2 = (v4sf){f2, f3, f4, f1};
    vd1 = (v2df){d1, d2};
    
    /* Long chain of interdependent computations */
    /* This creates many pseudo-registers with multiple uses */
    
    /* Chain 1: Integer operations */
    a6 = a1 + a2;      /* pseudo-reg for a6 */
    a7 = a6 * a3;      /* uses a6, creates a7 */
    a8 = a7 - a4;      /* uses a7, creates a8 */
    a9 = a8 / (a5 ? a5 : 1); /* uses a8, creates a9 */
    a10 = a9 ^ a6;     /* uses a9 and a6, creates a10 */
    
    /* Chain 2: Float operations with integer results */
    f5 = f1 + f2;      /* pseudo-reg for f5 */
    f6 = f5 * f3;      /* uses f5, creates f6 */
    f7 = f6 - f4;      /* uses f6, creates f7 */
    f8 = f7 / (f1 ? f1 : 1.0f); /* uses f7, creates f8 */
    
    /* Chain 3: Mixed operations */
    d3 = d1 + (double)a6;  /* uses a6, creates d3 */
    d4 = d3 * f5;          /* uses d3 and f5, creates d4 */
    d5 = d4 - (double)a7;  /* uses d4 and a7, creates d5 */
    d6 = d5 / (d2 ? d2 : 1.0); /* uses d5, creates d6 */
    
    /* Chain 4: Long operations */
    l3 = l1 + (long)a8;    /* uses a8, creates l3 */
    l4 = l3 * (long)a9;    /* uses l3 and a9, creates l4 */
    l5 = l4 - (long)a10;   /* uses l4 and a10, creates l5 */
    
    /* Vector operations - create more register pressure */
    v3 = v1 + v2;          /* uses v1 and v2, creates v3 */
    v4 = v3 * (v4si){a6, a7, a8, a9}; /* uses v3, creates v4 */
    v5 = v4 - v1;          /* uses v4 and v1, creates v5 */
    
    vf3 = vf1 + vf2;       /* uses vf1 and vf2, creates vf3 */
    vf4 = vf3 * (v4sf){f5, f6, f7, f8}; /* uses vf3, creates vf4 */
    
    vd2 = vd1 + (v2df){d3, d4}; /* uses vd1, creates vd2 */
    vd3 = vd2 * (v2df){d5, d6}; /* uses vd2, creates vd3 */
    
    /* Artificial register pressure spike */
    CLOBBER_REGS();
    
    /* More operations after clobbering - forces re-materialization */
    a1 = a10 + l5;         /* uses a10 and l5 */
    f1 = f8 + (float)d6;   /* uses f8 and d6 */
    d1 = d6 + (double)l5;  /* uses d6 and l5 */
    
    /* Vector cross operations */
    v1 = v5 + (v4si){a1, a2, a3, a4};
    vf1 = vf4 + (v4sf){f1, f2, f3, f4};
    vd1 = vd3 + (v2df){d1, d2};
    
    /* Create complex struct for return */
    struct ComplexData result;
    result.vec_int = v1 + v5;  /* Multiple uses of v1 and v5 */
    result.vec_float = vf1 * vf4; /* Multiple uses of vf1 and vf4 */
    result.vec_double = vd1 - vd3; /* Multiple uses of vd1 and vd3 */
    
    /* Fill arrays with computed values */
    result.ints[0] = a1 + a10;
    result.ints[1] = a2 + a9;
    result.ints[2] = a3 + a8;
    result.ints[3] = a4 + a7;
    
    result.floats[0] = f1 + f8;
    result.floats[1] = f2 + f7;
    result.floats[2] = f3 + f6;
    result.floats[3] = f4 + f5;
    
    /* Final computation using all temporaries */
    result.ints[0] += (int)(d1 + d2 + d3 + d4 + d5 + d6);
    result.ints[1] += (int)(l1 + l2 + l3 + l4 + l5);
    
    return result;
}

/* Hot loop to trigger optimization */
int main() {
    struct ComplexData total = {0};
    int iterations = g_volatile_counter;
    
    /* Create mask and scale for vector operations */
    v4si mask = (v4si){0xFF, 0xFF00, 0xFF0000, 0xFF000000};
    v4sf scale = (v4sf){1.5f, 2.5f, 3.5f, 4.5f};
    
    for (int i = 0; i < iterations; i++) {
        /* Call test function with high register pressure */
        struct ComplexData result1 = test_function(i);
        
        /* Create second input */
        struct ComplexData input2;
        input2.vec_int = mask;
        input2.vec_float = scale;
        input2.vec_double = (v2df){i * 1.0, i * 2.0};
        
        /* Call external helper to increase inter-procedural pressure */
        struct ComplexData result2 = process_data(result1, input2, i % 10);
        
        /* Transform the result */
        struct ComplexData final_result = transform_vector(result2, mask, scale);
        
        /* Accumulate to prevent elimination */
        total.vec_int += final_result.vec_int;
        total.vec_float += final_result.vec_float;
        total.vec_double += final_result.vec_double;
        
        /* Use volatile to prevent loop unrolling */
        if (i % 100 == 0) {
            asm volatile("" : : "r"(total.vec_int), "r"(total.vec_float) : "memory");
        }
    }
    
    /* Print something to prevent dead code elimination */
    printf("Result: %d %f %f\n", 
           total.vec_int[0] + total.vec_int[1] + total.vec_int[2] + total.vec_int[3],
           (double)total.vec_float[0] + total.vec_float[1],
           total.vec_double[0] + total.vec_double[1]);
    
    return 0;
}
