/* mcf_coverage.c - Program to trigger GCC's Minimum Cost Flow debugging output */
#include <stdio.h>
#include <stdlib.h>

/* Force compiler to preserve all computations */
static volatile int sink;

/* Complex test function with high register pressure */
int __attribute__((noinline)) test_mcf(int seed, int iterations) {
    /* Declare many variables to create register pressure */
    int v0, v1, v2, v3, v4, v5, v6, v7, v8, v9;
    int v10, v11, v12, v13, v14, v15, v16, v17, v18, v19;
    int v20, v21, v22, v23, v24, v25, v26, v27, v28, v29;
    int v30, v31, v32, v33, v34, v35, v36, v37, v38, v39;
    int result = seed;
    
    /* Initialize variables with complex dependencies */
    v0 = seed * 1;
    v1 = seed * 2 + v0;
    v2 = seed * 3 + v1;
    v3 = seed * 4 + v2;
    v4 = seed * 5 + v3;
    
    /* Create artificial register pressure with inline asm */
    asm volatile ("" : "=r"(v0), "=r"(v1), "=r"(v2) : "0"(v0), "1"(v1), "2"(v2));
    
    /* Complex control flow with many basic blocks */
    for (int i = 0; i < iterations; i++) {
        /* Nested conditionals creating control flow splits */
        if (i % 3 == 0) {
            v5 = v0 + v1;
            v6 = v2 * v3;
            /* Inline asm with register clobbers forces graph modifications */
            asm volatile ("# Force register pressure" 
                         : "+r"(v5), "+r"(v6)
                         : 
                         : "eax", "ebx", "ecx", "edx");
        } else if (i % 3 == 1) {
            v7 = v4 - v0;
            v8 = v1 * v2;
            /* More clobbers to force NEW_ENTRY/NEW_EXIT creation */
            asm volatile ("# More pressure" 
                         : "+r"(v7), "+r"(v8)
                         : 
                         : "esi", "edi", "ebp");
        } else {
            v9 = v3 / (v0 + 1);
            v10 = v4 % (v2 + 1);
        }
        
        /* Switch with many cases creates multiple basic blocks */
        switch (i % 11) {
            case 0: v11 = v0 + i; break;
            case 1: v12 = v1 - i; break;
            case 2: v13 = v2 * i; break;
            case 3: v14 = v3 ^ i; break;
            case 4: v15 = v4 | i; break;
            case 5: v16 = v5 & i; break;
            case 6: v17 = v6 << (i & 3); break;
            case 7: v18 = v7 >> (i & 3); break;
            case 8: v19 = v8 + (i * 2); break;
            case 9: v20 = v9 - (i * 3); break;
            case 10: v21 = v10 * (i + 1); break;
        }
        
        /* More variables with overlapping live ranges */
        v22 = v11 + v12;
        v23 = v13 - v14;
        v24 = v15 * v16;
        v25 = v17 / (v18 + 1);
        v26 = v19 % (v20 + 1);
        v27 = v21 ^ v22;
        v28 = v23 | v24;
        v29 = v25 & v26;
        
        /* Goto labels create additional control flow edges */
        if (v27 > 1000) goto compute_heavy;
        if (v28 < -1000) goto compute_light;
        
        v30 = v27 * v28;
        goto continue_loop;
        
    compute_heavy:
        v30 = v27 / (v28 + 1);
        v31 = v29 * 2;
        goto continue_loop;
        
    compute_light:
        v30 = v27 + v28;
        v31 = v29 / 2;
        
    continue_loop:
        /* Mix different types to increase pressure */
        float f1 = v30 * 0.5f;
        float f2 = v31 * 0.25f;
        int v32 = (int)(f1 + f2);
        
        /* Pointer operations create different register class pressure */
        int *ptr1 = &v32;
        int *ptr2 = &v33;
        *ptr2 = *ptr1 + i;
        
        /* Complex arithmetic chain with dependencies */
        v34 = v32 + v33;
        v35 = v34 * v0;
        v36 = v35 - v1;
        v37 = v36 ^ v2;
        v38 = v37 | v3;
        v39 = v38 & v4;
        
        /* Update result with all computations */
        result += v39;
        
        /* Rotate values to create loop-carried dependencies */
        v0 = v39;
        v1 = v38;
        v2 = v37;
        v3 = v36;
        v4 = v35;
    }
    
    /* Final complex computation */
    result = result ^ v0 ^ v1 ^ v2 ^ v3 ^ v4;
    
    /* Use sink to prevent elimination */
    sink = result;
    return result;
}

/* Another complex function to ensure interprocedural analysis doesn't simplify */
int __attribute__((noinline)) test_mcf2(int base) {
    int a = base, b = base + 1, c = base + 2;
    
    /* Unrolled loop with many operations */
    for (int i = 0; i < 50; i++) {
        a = a * 3 + i;
        b = b / 2 - i;
        c = c ^ a ^ b;
        
        /* Conditional with both sides needing registers */
        if (a > b) {
            asm volatile ("# Side A" : "+r"(a), "+r"(c) :: "memory");
        } else {
            asm volatile ("# Side B" : "+r"(b), "+r"(c) :: "memory");
        }
    }
    
    return a + b + c;
}

int main(void) {
    int total = 0;
    
    /* Call test functions multiple times with different inputs */
    for (int i = 0; i < 100; i++) {
        total += test_mcf(i, 10 + (i % 5));
        total += test_mcf2(i);
    }
    
    printf("Result: %d\n", total);
    return 0;
}
