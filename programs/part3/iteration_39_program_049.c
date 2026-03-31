/* Main test driver with hot loop */
#include <stdint.h>
#include <stdio.h>

/* Volatile to prevent optimization */
volatile int loop_count = 1000;

/* External helper functions */
extern struct MultiArg helper1(struct MultiArg a, struct MultiArg b);
extern struct MultiArg helper2(struct MultiArg a, struct MultiArg b);
extern struct MultiArg helper3(struct MultiArg a, struct MultiArg b);
extern struct MultiArg helper4(struct MultiArg a, struct MultiArg b);

/* Vector types for register pressure */
typedef int v4si __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));
typedef double v2df __attribute__((vector_size(16)));

/* Complex struct for cross-function pressure */
struct MultiArg {
    v4si vec_int;
    v4sf vec_float;
    v2df vec_double;
    long long_val;
    double double_val;
    int int_val;
    float float_val;
};

/* Noinline to prevent inlining */
__attribute__((noinline, noipa))
struct MultiArg test_function(struct MultiArg input) {
    /* Many local variables to create register pressure */
    v4si v1, v2, v3, v4, v5, v6, v7, v8, v9, v10;
    v4sf f1, f2, f3, f4, f5, f6, f7, f8, f9, f10;
    v2df d1, d2, d3, d4, d5, d6, d7, d8, d9, d10;
    int i1, i2, i3, i4, i5, i6, i7, i8, i9, i10;
    long l1, l2, l3, l4, l5, l6, l7, l8, l9, l10;
    float fl1, fl2, fl3, fl4, fl5, fl6, fl7, fl8, fl9, fl10;
    double db1, db2, db3, db4, db5, db6, db7, db8, db9, db10;
    
    /* Initialize from input */
    v1 = input.vec_int;
    f1 = input.vec_float;
    d1 = input.vec_double;
    i1 = input.int_val;
    l1 = input.long_val;
    fl1 = input.float_val;
    db1 = input.double_val;
    
    /* Long chain of interdependent computations */
    /* This creates many pseudo-registers with multiple uses */
    
    /* Vector operations */
    v2 = v1 + (v4si){1, 2, 3, 4};
    v3 = v2 * v1;  /* v2 used as operand */
    v4 = v3 - v2;  /* v3 used as operand, v2 used again */
    v5 = v4 >> 1;
    v6 = v5 | v4;  /* v5 used as operand, v4 used again */
    v7 = v6 & v5;  /* v6 used as operand, v5 used again */
    v8 = v7 ^ v6;  /* v7 used as operand, v6 used again */
    v9 = v8 << 2;
    v10 = v9 + v8; /* v9 used as operand, v8 used again */
    
    /* Float vector operations */
    f2 = f1 + (v4sf){1.0f, 2.0f, 3.0f, 4.0f};
    f3 = f2 * f1;  /* f2 used as operand */
    f4 = f3 - f2;  /* f3 used as operand, f2 used again */
    f5 = f4 / (v4sf){2.0f, 2.0f, 2.0f, 2.0f};
    f6 = f5 * f4;  /* f5 used as operand, f4 used again */
    f7 = f6 + f5;  /* f6 used as operand, f5 used again */
    f8 = f7 - f6;  /* f7 used as operand, f6 used again */
    f9 = f8 * (v4sf){0.5f, 0.5f, 0.5f, 0.5f};
    f10 = f9 + f8; /* f9 used as operand, f8 used again */
    
    /* Double vector operations */
    d2 = d1 + (v2df){1.0, 2.0};
    d3 = d2 * d1;  /* d2 used as operand */
    d4 = d3 - d2;  /* d3 used as operand, d2 used again */
    d5 = d4 / (v2df){2.0, 2.0};
    d6 = d5 * d4;  /* d5 used as operand, d4 used again */
    d7 = d6 + d5;  /* d6 used as operand, d5 used again */
    d8 = d7 - d6;  /* d7 used as operand, d6 used again */
    d9 = d8 * (v2df){0.5, 0.5};
    d10 = d9 + d8; /* d9 used as operand, d8 used again */
    
    /* Scalar integer operations with dependencies */
    i2 = i1 + 1;
    i3 = i2 * i1;  /* i2 used as operand */
    i4 = i3 - i2;  /* i3 used as operand, i2 used again */
    i5 = i4 >> 1;
    i6 = i5 | i4;  /* i5 used as operand, i4 used again */
    i7 = i6 & i5;  /* i6 used as operand, i5 used again */
    i8 = i7 ^ i6;  /* i7 used as operand, i6 used again */
    i9 = i8 << 2;
    i10 = i9 + i8; /* i9 used as operand, i8 used again */
    
    /* Scalar long operations */
    l2 = l1 + 1L;
    l3 = l2 * l1;  /* l2 used as operand */
    l4 = l3 - l2;  /* l3 used as operand, l2 used again */
    l5 = l4 >> 1;
    l6 = l5 | l4;  /* l5 used as operand, l4 used again */
    l7 = l6 & l5;  /* l6 used as operand, l5 used again */
    l8 = l7 ^ l6;  /* l7 used as operand, l6 used again */
    l9 = l8 << 2;
    l10 = l9 + l8; /* l9 used as operand, l8 used again */
    
    /* Scalar float operations */
    fl2 = fl1 + 1.0f;
    fl3 = fl2 * fl1;  /* fl2 used as operand */
    fl4 = fl3 - fl2;  /* fl3 used as operand, fl2 used again */
    fl5 = fl4 / 2.0f;
    fl6 = fl5 * fl4;  /* fl5 used as operand, fl4 used again */
    fl7 = fl6 + fl5;  /* fl6 used as operand, fl5 used again */
    fl8 = fl7 - fl6;  /* fl7 used as operand, fl6 used again */
    fl9 = fl8 * 0.5f;
    fl10 = fl9 + fl8; /* fl9 used as operand, fl8 used again */
    
    /* Scalar double operations */
    db2 = db1 + 1.0;
    db3 = db2 * db1;  /* db2 used as operand */
    db4 = db3 - db2;  /* db3 used as operand, db2 used again */
    db5 = db4 / 2.0;
    db6 = db5 * db4;  /* db5 used as operand, db4 used again */
    db7 = db6 + db5;  /* db6 used as operand, db5 used again */
    db8 = db7 - db6;  /* db7 used as operand, db6 used again */
    db9 = db8 * 0.5;
    db10 = db9 + db8; /* db9 used as operand, db8 used again */
    
    /* Inline assembly to clobber physical registers and increase pressure */
    /* Clobber multiple registers to force pseudo-register usage */
    asm volatile (
        "/* Clobber physical registers */\n\t"
        "mov r0, %0\n\t"
        "mov r1, %1\n\t"
        "add r0, r1\n\t"
        : 
        : "r" (i10), "r" (l10)
        : "r0", "r1", "r2", "r3", "r4", "r5", "r6", "r7", 
          "r8", "r9", "r10", "r11", "r12", "memory"
    );
    
    /* More operations after assembly to force register reloading */
    v1 = v10 + v9;
    f1 = f10 + f9;
    d1 = d10 + d9;
    i1 = i10 + i9;
    l1 = l10 + l9;
    fl1 = fl10 + fl9;
    db1 = db10 + db9;
    
    /* Complex expression mixing all types */
    struct MultiArg result;
    result.vec_int = v1 + (v4si){i1, i2, i3, i4};
    result.vec_float = f1 * (v4sf){fl1, fl2, fl3, fl4};
    result.vec_double = d1 + (v2df){db1, db2};
    result.long_val = l1 + (long)db3;
    result.double_val = db1 + (double)l2;
    result.int_val = i1 + (int)fl5;
    result.float_val = fl1 + (float)i5;
    
    return result;
}

int main() {
    struct MultiArg init = {
        .vec_int = {1, 2, 3, 4},
        .vec_float = {1.0f, 2.0f, 3.0f, 4.0f},
        .vec_double = {1.0, 2.0},
        .long_val = 1000L,
        .double_val = 3.14159,
        .int_val = 42,
        .float_val = 2.71828f
    };
    
    struct MultiArg result = init;
    
    /* Hot loop to increase compilation significance */
    for (int i = 0; i < loop_count; i++) {
        /* Call test function repeatedly */
        result = test_function(result);
        
        /* Call helper functions to increase inter-procedural pressure */
        result = helper1(result, init);
        result = helper2(result, init);
        result = helper3(result, init);
        result = helper4(result, init);
    }
    
    /* Use result to prevent dead code elimination */
    volatile int dummy = result.int_val + (int)result.float_val;
    
    printf("Result: %d\n", dummy);
    return 0;
}
