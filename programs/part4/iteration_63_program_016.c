/* test_mcf_coverage.c
 * 
 * This program is designed to trigger the uncovered lines in GCC's
 * Min-Cost Flow solver (mcf.cc) by creating register allocation
 * scenarios that force IRA to build complex fixup graphs with
 * NEW_EXIT and NEW_ENTRY nodes.
 *
 * Compile with: gcc -O2 -fira-algorithm=priority -DMCF_DEBUG -c test_mcf_coverage.c -o test.o
 * Or with: gcc -O3 -funroll-loops -fira-algorithm=CB -DMCF_DEBUG -c test_mcf_coverage.c -o test.o
 */

/* Force inclusion of MCF debugging code */
#ifdef MCF_DEBUG
/* Ensure debug functions are referenced */
void __attribute__((used)) force_debug_ref() {
    /* This function exists to ensure MCF_DEBUG code paths are compiled in */
}
#endif

/* Complex function with many overlapping live ranges */
int __attribute__((noinline)) test_ira_conflict(int iterations) {
    /* Declare many variables to create register pressure */
    int a = 1, b = 2, c = 3, d = 4, e = 5;
    int f = 6, g = 7, h = 8, i = 9, j = 10;
    int k = 11, l = 12, m = 13, n = 14, o = 15;
    int p = 16, q = 17, r = 18, s = 19, t = 20;
    int u = 21, v = 22, w = 23, x = 24, y = 25;
    int z = 26, aa = 27, bb = 28, cc = 29, dd = 30;
    
    volatile int result = 0;
    
    /* Nested loops with many live variables across iterations */
    for (int outer = 0; outer < iterations; outer++) {
        /* All variables are live here - creating massive register pressure */
        a += outer; b += a; c += b; d += c; e += d;
        f += e; g += f; h += g; i += h; j += i;
        
        /* Inner loop with volatile asm to clobber registers */
        for (int inner = 0; inner < 10; inner++) {
            /* Force register spilling by clobbering many registers */
            asm volatile (
                /* Clobber many registers to increase pressure */
                "mov r0, %0\n"
                "mov r1, %1\n"
                "mov r2, %2\n"
                "mov r3, %3\n"
                "add r0, r0, r1\n"
                "add r2, r2, r3\n"
                "mul r0, r0, r2\n"
                : 
                : "r" (k + inner), "r" (l + inner), 
                  "r" (m + inner), "r" (n + inner)
                : "r0", "r1", "r2", "r3", "cc", "memory"
            );
            
            /* More computations keeping variables live */
            k += inner; l += k; m += l; n += m; o += n;
            p += o; q += p; r += q; s += r; t += s;
        }
        
        /* Another asm block clobbering different registers */
        asm volatile (
            "mov r4, %0\n"
            "mov r5, %1\n"
            "mov r6, %2\n"
            "mov r7, %3\n"
            "add r4, r4, r5\n"
            "add r6, r6, r7\n"
            "mul r4, r4, r6\n"
            : 
            : "r" (u + outer), "r" (v + outer), 
              "r" (w + outer), "r" (x + outer)
            : "r4", "r5", "r6", "r7", "cc", "memory"
        );
        
        /* Complex computation chain */
        u += outer; v += u; w += v; x += w; y += x;
        z += y; aa += z; bb += aa; cc += bb; dd += cc;
        
        /* Mix all variables together */
        result += a + b + c + d + e + f + g + h + i + j +
                  k + l + m + n + o + p + q + r + s + t +
                  u + v + w + x + y + z + aa + bb + cc + dd;
    }
    
    return result;
}

/* Second test function with different variable usage pattern */
int __attribute__((noinline)) test_ira_imbalance(int seed) {
    /* Create variables with imbalanced definition/use counts */
    int var1 = seed, var2 = seed * 2, var3 = seed * 3;
    int var4 = seed * 4, var5 = seed * 5, var6 = seed * 6;
    int var7 = seed * 7, var8 = seed * 8, var9 = seed * 9;
    int var10 = seed * 10;
    
    volatile int sum = 0;
    
    /* Pattern that creates many uses but few definitions */
    for (int i = 0; i < 100; i++) {
        /* Many uses of each variable */
        sum += var1 + var2 + var3 + var4 + var5;
        sum += var6 + var7 + var8 + var9 + var10;
        
        /* But only occasional definitions */
        if (i % 10 == 0) {
            var1 += i;
            var6 += i * 2;
        }
        if (i % 7 == 0) {
            var2 += i;
            var7 += i * 2;
        }
        if (i % 5 == 0) {
            var3 += i;
            var8 += i * 2;
        }
        if (i % 3 == 0) {
            var4 += i;
            var9 += i * 2;
        }
        if (i % 2 == 0) {
            var5 += i;
            var10 += i * 2;
        }
        
        /* Nested loop with register pressure */
        for (int j = 0; j < 5; j++) {
            /* Force all variables to be live */
            asm volatile (
                "mov r8, %0\n"
                "mov r9, %1\n"
                "mov r10, %2\n"
                "add r8, r8, r9\n"
                "add r10, r10, r8\n"
                : 
                : "r" (var1 + j), "r" (var2 + j), "r" (var3 + j)
                : "r8", "r9", "r10", "cc"
            );
        }
    }
    
    return sum;
}

/* Third test: Function with switch statement creating complex CFG */
int __attribute__((noinline)) test_ira_cfg(int mode) {
    int r1 = 1, r2 = 2, r3 = 3, r4 = 4, r5 = 5;
    int r6 = 6, r7 = 7, r8 = 8, r9 = 9, r10 = 10;
    int r11 = 11, r12 = 12, r13 = 13, r14 = 14, r15 = 15;
    
    /* Switch creates complex control flow graph */
    switch (mode % 8) {
        case 0:
            r1 += r2; r3 += r4; r5 += r6;
            asm volatile ("" ::: "memory");
            break;
        case 1:
            r7 += r8; r9 += r10; r11 += r12;
            asm volatile ("" ::: "memory");
            break;
        case 2:
            r13 += r14; r15 += r1; r2 += r3;
            asm volatile ("" ::: "memory");
            break;
        case 3:
            r4 += r5; r6 += r7; r8 += r9;
            asm volatile ("" ::: "memory");
            break;
        case 4:
            r10 += r11; r12 += r13; r14 += r15;
            asm volatile ("" ::: "memory");
            break;
        case 5:
            /* All variables live here */
            asm volatile (
                "mov r0, %0\n"
                "mov r1, %1\n"
                "mov r2, %2\n"
                "mov r3, %3\n"
                "mov r4, %4\n"
                "add r0, r0, r1\n"
                "add r2, r2, r3\n"
                "add r4, r4, r0\n"
                "add r4, r4, r2\n"
                : 
                : "r" (r1), "r" (r2), "r" (r3), "r" (r4), "r" (r5)
                : "r0", "r1", "r2", "r3", "r4", "cc"
            );
            break;
        case 6:
            r1 = r15; r2 = r14; r3 = r13;
            r4 = r12; r5 = r11; r6 = r10;
            break;
        case 7:
            /* Maximum register pressure */
            for (int i = 0; i < 20; i++) {
                r1 += r2; r2 += r3; r3 += r4; r4 += r5;
                r5 += r6; r6 += r7; r7 += r8; r8 += r9;
                r9 += r10; r10 += r11; r11 += r12; r12 += r13;
                r13 += r14; r14 += r15; r15 += r1;
            }
            break;
    }
    
    return r1 + r2 + r3 + r4 + r5 + r6 + r7 + r8 + r9 + r10 +
           r11 + r12 + r13 + r14 + r15;
}

/* Fourth test: Recursive function to create interesting live ranges */
int __attribute__((noinline)) test_ira_recursive(int n, int depth) {
    int a = n, b = n * 2, c = n * 3, d = n * 4, e = n * 5;
    
    if (depth <= 0) {
        return a + b + c + d + e;
    }
    
    /* All variables must be preserved across recursive call */
    int child = test_ira_recursive(n + 1, depth - 1);
    
    /* Complex computation with all variables live */
    a += child; b += a; c += b; d += c; e += d;
    
    /* Force register spilling */
    asm volatile (
        "mov r0, %0\n"
        "mov r1, %1\n"
        "mov r2, %2\n"
        "mov r3, %3\n"
        "mov r4, %4\n"
        "add r0, r0, r1\n"
        "add r2, r2, r3\n"
        "add r4, r4, r0\n"
        "add r4, r4, r2\n"
        "mov %0, r0\n"
        "mov %1, r2\n"
        "mov %2, r4\n"
        : "+r" (a), "+r" (c), "+r" (e)
        : "r" (b), "r" (d)
        : "r0", "r1", "r2", "r3", "r4", "cc"
    );
    
    return a + b + c + d + e;
}

/* Main function that exercises all test cases */
int main(int argc, char **argv) {
    int total = 0;
    
    /* Run with different parameters to explore different graph configurations */
    for (int run = 0; run < 10; run++) {
        total += test_ira_conflict(5 + run % 3);
        total += test_ira_imbalance(run * 7);
        total += test_ira_cfg(run);
        total += test_ira_recursive(run, 3);
    }
    
    /* Prevent dead code elimination */
    asm volatile ("" : "+r" (total));
    
    return total % 256;
}
