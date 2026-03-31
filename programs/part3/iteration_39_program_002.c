/* Test case for early rematerialization - main test file */
#include <stdint.h>
#include <string.h>

/* Force no optimization on specific functions */
#define NOINLINE __attribute__((noinline))
#define VOLATILE_VAR volatile int

/* Vector types to increase register pressure */
typedef int v4si __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));
typedef double v2df __attribute__((vector_size(16)));

/* Complex struct for inter-procedural pressure */
struct MultiRegStruct {
    v4si vec_int;
    v4sf vec_float;
    v2df vec_double;
    long long extra[4];
};

/* External helper functions (defined in second file) */
NOINLINE struct MultiRegStruct helper1(int a, float b, double c, v4si v);
NOINLINE struct MultiRegStruct helper2(struct MultiRegStruct s1, struct MultiRegStruct s2);
NOINLINE v4si vector_op(v4si a, v4si b, v4si c, v4si d);
NOINLINE double mixed_ops(double a, double b, float c, int d);

/* Volatile variables to prevent optimization */
VOLATILE_VAR g_volatile_counter = 0;
VOLATILE_VAR g_volatile_seed = 12345;

/* Main test function with high register pressure */
NOINLINE long long test_function(int seed) {
    /* Declare many variables of different types */
    int i1, i2, i3, i4, i5, i6, i7, i8, i9, i10;
    float f1, f2, f3, f4, f5, f6, f7, f8;
    double d1, d2, d3, d4, d5, d6, d7, d8, d9, d10;
    long long ll1, ll2, ll3, ll4, ll5;
    v4si v1, v2, v3, v4, v5, v6;
    v4sf vf1, vf2, vf3;
    v2df vd1, vd2;
    
    /* Initialize with complex expressions */
    i1 = seed * 3;
    i2 = seed / 2;
    i3 = seed + 0x7FFF;
    i4 = seed ^ 0xABCD;
    i5 = seed | 0x1234;
    
    f1 = (float)seed * 1.5f;
    f2 = (float)seed / 3.7f;
    f3 = f1 + f2;
    f4 = f1 * f2 - f3;
    
    d1 = (double)seed * 2.71828;
    d2 = (double)seed / 3.14159;
    d3 = d1 + d2;
    d4 = d1 * d2 - d3;
    
    /* Vector initialization */
    v1 = (v4si){i1, i2, i3, i4};
    v2 = (v4si){i5, i1, i2, i3};
    v3 = v1 + v2;
    v4 = v1 * v2 - v3;
    
    vf1 = (v4sf){f1, f2, f3, f4};
    vf2 = vf1 * (v4sf){2.0f, 3.0f, 4.0f, 5.0f};
    
    vd1 = (v2df){d1, d2};
    vd2 = vd1 * (v2df){1.5, 2.5};
    
    /* Long chain of interdependent computations */
    /* This creates many pseudo-registers with multiple uses */
    for (int iter = 0; iter < 3; iter++) {
        /* Chain 1: int operations */
        i6 = i1 + i2;
        i7 = i6 * i3;          /* i6 used as operand */
        i8 = i7 - i4;          /* i7 used as operand */
        i9 = i8 ^ i5;          /* i8 used as operand */
        i10 = i9 | i6;         /* i9 and i6 used as operands */
        
        /* Chain 2: float operations */
        f5 = f1 + f2;
        f6 = f5 * f3;          /* f5 used as operand */
        f7 = f6 - f4;          /* f6 used as operand */
        f8 = f7 / f5;          /* f7 and f5 used as operands */
        
        /* Chain 3: double operations - very long chain */
        d5 = d1 + d2;
        d6 = d5 * d3;          /* d5 used as operand */
        d7 = d6 - d4;          /* d6 used as operand */
        d8 = d7 / d5;          /* d7 and d5 used as operands */
        d9 = d8 * d6 + d7;     /* d8, d6, d7 used as operands */
        d10 = d9 - d8 * d5;    /* d9, d8, d5 used as operands */
        
        /* Chain 4: vector operations */
        v5 = v3 + v4;
        v6 = v5 * v1;          /* v5 used as operand */
        v3 = v6 - v2;          /* v6 used as operand */
        v4 = v3 * v5 + v6;     /* v3, v5, v6 used as operands */
        
        /* Chain 5: mixed type operations */
        ll1 = (long long)i10 * (long long)d10;
        ll2 = ll1 + (long long)f8 * 1000;
        ll3 = ll2 ^ (long long)v4[0];
        ll4 = ll3 * ll1 - ll2;
        ll5 = ll4 / (ll1 + 1);
        
        /* Inline assembly to clobber physical registers */
        /* This increases register pressure significantly */
        asm volatile (
            "/* Clobber many registers to force pseudo-reg usage */\n\t"
            :
            :
            : "r0", "r1", "r2", "r3", "r4", "r5", "r6", "r7",
              "r8", "r9", "r10", "r11", "r12",
              "memory"
        );
        
        /* Use helper functions to increase inter-procedural pressure */
        struct MultiRegStruct s1 = helper1(i10, f8, d10, v6);
        struct MultiRegStruct s2 = helper2(s1, (struct MultiRegStruct){
            .vec_int = v5,
            .vec_float = vf2,
            .vec_double = vd2,
            .extra = {ll1, ll2, ll3, ll4}
        });
        
        /* More computations using helper results */
        vf3 = s2.vec_float * vf1;
        vd2 = s2.vec_double + vd1;
        
        /* Vector operation with many parameters */
        v4 = vector_op(v1, v2, v3, v4);
        
        /* Mixed operations */
        d10 = mixed_ops(d9, d8, f7, i9);
        
        /* Update variables for next iteration */
        i1 = i10 ^ iter;
        f1 = f8 + (float)iter;
        d1 = d10 * (1.0 + iter * 0.1);
        v1 = v4 + (v4si){iter, iter, iter, iter};
    }
    
    /* Final computation using all variables */
    long long result = 
        (long long)i1 + (long long)i2 + (long long)i3 +
        (long long)i4 + (long long)i5 + (long long)i6 +
        (long long)i7 + (long long)i8 + (long long)i9 +
        (long long)i10 +
        (long long)(f1 * 1000) + (long long)(f2 * 1000) +
        (long long)(f3 * 1000) + (long long)(f4 * 1000) +
        (long long)(f5 * 1000) + (long long)(f6 * 1000) +
        (long long)(f7 * 1000) + (long long)(f8 * 1000) +
        (long long)d1 + (long long)d2 + (long long)d3 +
        (long long)d4 + (long long)d5 + (long long)d6 +
        (long long)d7 + (long long)d8 + (long long)d9 +
        (long long)d10 +
        ll1 + ll2 + ll3 + ll4 + ll5 +
        v1[0] + v1[1] + v1[2] + v1[3] +
        v2[0] + v2[1] + v2[2] + v2[3] +
        (long long)(vf3[0] * 1000) + (long long)(vd2[0] * 1000);
    
    return result;
}

/* Main function with hot loop */
int main() {
    long long total = 0;
    int iterations = g_volatile_counter + 100; /* Prevent optimization */
    
    /* Hot loop to trigger compilation and optimization */
    for (int i = 0; i < iterations; i++) {
        int seed = g_volatile_seed + i;
        total += test_function(seed);
        
        /* Prevent loop unrolling */
        asm volatile("" : "+r"(total) : : "memory");
    }
    
    /* Use result to prevent dead code elimination */
    g_volatile_counter = (int)(total & 0x7FFFFFFF);
    
    return g_volatile_counter > 0 ? 0 : 1;
}
