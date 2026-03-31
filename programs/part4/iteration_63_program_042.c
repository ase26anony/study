/* test_mcf_coverage.c
 * 
 * This test is designed to trigger the uncovered lines in GCC's mcf.cc
 * Specifically, the dump_fixup_edge function's handling of NEW_EXIT and NEW_ENTRY
 * nodes in the fixup graph during register allocation.
 *
 * Compile with: gcc -O2 -fira-algorithm=priority -DMCF_DEBUG -c test_mcf_coverage.c -o test.o
 * Or with: gcc -O3 -funroll-loops -fira-algorithm=CB -DMCF_DEBUG -c test_mcf_coverage.c -o test.o
 */

/* Force inclusion of IRA and MCF debugging */
#ifndef MCF_DEBUG
#define MCF_DEBUG 1
#endif

/* Function with many overlapping live ranges to create complex conflict graph */
int test_ira_conflict(int iterations) {
    /* Declare many variables that will have overlapping live ranges */
    int v1, v2, v3, v4, v5, v6, v7, v8, v9, v10;
    int v11, v12, v13, v14, v15, v16, v17, v18, v19, v20;
    int v21, v22, v23, v24, v25, v26, v27, v28, v29, v30;
    int sum = 0;
    
    /* Initialize variables to create different live ranges */
    v1 = iterations;
    v2 = v1 * 2;
    v3 = v2 + 1;
    v4 = v3 - v1;
    v5 = v4 * v2;
    
    /* Nested loops to create complex liveness patterns */
    for (int i = 0; i < iterations; i++) {
        /* Many variables live across loop iterations */
        v6 = v1 + i;
        v7 = v2 * i;
        v8 = v3 - i;
        v9 = v4 + v6;
        v10 = v5 * v7;
        
        /* Inner loop with more live variables */
        for (int j = 0; j < 3; j++) {
            /* These variables are live within inner loop */
            v11 = v6 + j;
            v12 = v7 - j;
            v13 = v8 * j;
            v14 = v9 + v11;
            v15 = v10 - v12;
            
            /* More computations creating dependencies */
            v16 = v11 * v12;
            v17 = v13 + v14;
            v18 = v15 - v16;
            v19 = v17 * v18;
            v20 = v19 / (v14 + 1);
            
            /* Use volatile asm to clobber registers and increase pressure */
            asm volatile ("# Force register clobbering" 
                         : "=r"(v21), "=r"(v22), "=r"(v23), "=r"(v24), "=r"(v25)
                         : "0"(v16), "1"(v17), "2"(v18), "3"(v19), "4"(v20)
                         : "memory", "cc");
            
            /* More variables to increase register pressure */
            v26 = v21 + v22;
            v27 = v23 * v24;
            v28 = v25 - v26;
            v29 = v27 + v28;
            v30 = v29 * 2;
            
            sum += v30;
        }
        
        /* Cross-iteration dependencies */
        v1 = v21 % 7;
        v2 = v22 + 1;
        v3 = v23 - 2;
        v4 = v24 * 3;
        v5 = v25 / 4;
    }
    
    /* Final computation using all variables */
    int result = v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10 +
                 v11 + v12 + v13 + v14 + v15 + v16 + v17 + v18 + v19 + v20 +
                 v21 + v22 + v23 + v24 + v25 + v26 + v27 + v28 + v29 + v30;
    
    return sum + result;
}

/* Second test function with different pattern to explore more graph configurations */
int test_ira_conflict2(int seed) {
    /* Use array to create many pseudo-registers */
    int arr[20];
    int brr[15];
    
    /* Initialize arrays with complex pattern */
    for (int i = 0; i < 20; i++) {
        arr[i] = seed * i + i * i;
    }
    
    for (int i = 0; i < 15; i++) {
        brr[i] = seed - i * 2;
    }
    
    /* Complex loop with many live variables */
    int total = 0;
    for (int i = 0; i < 10; i++) {
        int a = arr[i];
        int b = arr[i + 5];
        int c = arr[i + 10];
        
        /* Multiple live variables in nested scope */
        {
            int d = brr[i % 15];
            int e = a * b;
            int f = c + d;
            int g = e - f;
            int h = g * 2;
            
            /* Force register pressure with inline asm */
            asm volatile ("# More register pressure %0 %1 %2 %3 %4"
                         : "+r"(a), "+r"(b), "+r"(c), "+r"(d), "+r"(e)
                         :: "memory");
            
            int j = h + a;
            int k = b - c;
            int l = d * e;
            int m = j + k;
            int n = l - m;
            
            total += n;
        }
        
        /* Another nested loop */
        for (int j = 0; j < 3; j++) {
            int x = arr[(i + j) % 20];
            int y = brr[j % 15];
            int z = x * y + j;
            
            /* Volatile asm that clobbers many registers */
            asm volatile ("# Clobber registers %0 %1 %2"
                         : "=r"(x), "=r"(y), "=r"(z)
                         : "0"(x), "1"(y), "2"(z)
                         : "cc");
            
            total += z;
        }
    }
    
    return total;
}

/* Third test: Function with switch case to create control flow complexity */
int test_ira_conflict3(int mode) {
    int r1 = 1, r2 = 2, r3 = 3, r4 = 4, r5 = 5;
    int r6 = 6, r7 = 7, r8 = 8, r9 = 9, r10 = 10;
    int result = 0;
    
    switch (mode % 4) {
        case 0:
            r1 = r2 * r3;
            r4 = r5 + r6;
            r7 = r8 - r9;
            /* Many live variables */
            result = r1 + r4 + r7 + r10;
            /* Force spill/reload */
            asm volatile ("# Case 0 clobber" ::: "r0", "r1", "r2", "r3", "r4", "r5", "r6", "r7", "r8", "r9", "r10", "memory");
            break;
            
        case 1:
            r2 = r3 * r4;
            r5 = r6 + r7;
            r8 = r9 - r10;
            r1 = r2 + r5;
            result = r1 + r8;
            asm volatile ("# Case 1 clobber" ::: "r0", "r1", "r2", "r3", "r4", "r5", "r6", "r7", "r8", "r9", "r10", "memory");
            break;
            
        case 2:
            r3 = r4 * r5;
            r6 = r7 + r8;
            r9 = r10 - r1;
            r2 = r3 + r6;
            r4 = r9 * 2;
            result = r2 + r4;
            asm volatile ("# Case 2 clobber" ::: "r0", "r1", "r2", "r3", "r4", "r5", "r6", "r7", "r8", "r9", "r10", "memory");
            break;
            
        case 3:
            /* All variables live */
            r10 = r1 * r2 + r3 * r4 + r5 * r6 + r7 * r8 + r9;
            result = r10;
            asm volatile ("# Case 3 clobber" ::: "r0", "r1", "r2", "r3", "r4", "r5", "r6", "r7", "r8", "r9", "r10", "memory");
            break;
    }
    
    return result;
}

/* Main function to exercise all test cases */
int main() {
    int total = 0;
    
    /* Call test functions with different parameters to explore
     * different register allocation scenarios */
    for (int i = 0; i < 10; i++) {
        total += test_ira_conflict(i + 1);
        total += test_ira_conflict2(i * 3);
        total += test_ira_conflict3(i);
    }
    
    /* Prevent dead code elimination */
    asm volatile ("# Final total: %0" : : "r"(total));
    
    return total % 256;
}
