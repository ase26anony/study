/* test_mcf_coverage.c
 * 
 * This test program is designed to trigger the uncovered lines in GCC's
 * min-cost flow solver (mcf.cc) by creating complex register allocation
 * scenarios that require fixup graph construction with NEW_EXIT and NEW_ENTRY
 * nodes.
 *
 * Compile with: gcc -O2 -fira-algorithm=priority -DMCF_DEBUG -c test_mcf_coverage.c -o test.o
 * Or with: gcc -O3 -funroll-loops -fira-algorithm=CB -DMCF_DEBUG -c test_mcf_coverage.c -o test.o
 */

/* Force inclusion of IRA debugging */
#ifndef MCF_DEBUG
#define MCF_DEBUG 1
#endif

/* Function to create complex overlapping live ranges */
int test_ira_conflict(int iterations) {
    /* Declare many variables to create register pressure */
    int a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p, q, r, s, t;
    int u, v, w, x, y, z;
    int result = 0;
    
    /* Initialize all variables with different values to prevent optimization */
    a = iterations * 1;
    b = iterations * 2;
    c = iterations * 3;
    d = iterations * 4;
    e = iterations * 5;
    f = iterations * 6;
    g = iterations * 7;
    h = iterations * 8;
    i = iterations * 9;
    j = iterations * 10;
    k = iterations * 11;
    l = iterations * 12;
    m = iterations * 13;
    n = iterations * 14;
    o = iterations * 15;
    p = iterations * 16;
    q = iterations * 17;
    r = iterations * 18;
    s = iterations * 19;
    t = iterations * 20;
    u = iterations * 21;
    v = iterations * 22;
    w = iterations * 23;
    x = iterations * 24;
    y = iterations * 25;
    z = iterations * 26;
    
    /* Nested loops with many live variables across iterations */
    for (int outer = 0; outer < iterations; outer++) {
        /* All variables are live here - creates complex interference graph */
        a += b;
        b += c;
        c += d;
        d += e;
        e += f;
        f += g;
        g += h;
        h += i;
        i += j;
        j += k;
        
        /* Inner loop with different live ranges */
        for (int inner = 0; inner < 10; inner++) {
            /* Mix of variables to create overlapping live ranges */
            k += l + m;
            l += m + n;
            m += n + o;
            n += o + p;
            o += p + q;
            
            /* Volatile asm to clobber registers and increase pressure */
            asm volatile (
                "/* Clobber many registers */"
                :
                : 
                : "r0", "r1", "r2", "r3", "r4", "r5", "r6", "r7",
                  "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15",
                  "memory"
            );
            
            /* More computations to keep variables live */
            p += q + r;
            q += r + s;
            r += s + t;
            s += t + u;
            t += u + v;
        }
        
        /* Another computation block with different variable combinations */
        u += v + w;
        v += w + x;
        w += x + y;
        x += y + z;
        y += z + a;  /* Circular dependency to prevent dead code elimination */
        
        /* Another volatile asm with different clobbers */
        asm volatile (
            "/* Clobber more registers */"
            :
            :
            : "eax", "ebx", "ecx", "edx", "esi", "edi", "memory"
        );
        
        /* Final computations */
        z += a + b;
        result += a + b + c + d + e + f + g + h + i + j +
                  k + l + m + n + o + p + q + r + s + t +
                  u + v + w + x + y + z;
    }
    
    /* Force all variables to be used in return to prevent optimization */
    return result + a + b + c + d + e + f + g + h + i + j +
           k + l + m + n + o + p + q + r + s + t +
           u + v + w + x + y + z;
}

/* Second test function with different pattern to explore more graph configurations */
int test_ira_conflict2(int seed) {
    /* Use array to create many pseudo-registers */
    int arr[32];
    int sum = 0;
    
    /* Initialize array with pattern */
    for (int i = 0; i < 32; i++) {
        arr[i] = seed * (i + 1);
    }
    
    /* Complex loop with many overlapping live ranges */
    for (int i = 0; i < 32; i++) {
        for (int j = 0; j < 32; j++) {
            /* Many array elements live simultaneously */
            arr[i] += arr[j];
            arr[j] += arr[(i + j) % 32];
            
            /* Conditional to create control flow complexity */
            if ((i + j) % 3 == 0) {
                asm volatile (
                    "/* Conditional clobber */"
                    :
                    :
                    : "r0", "r1", "r2", "r3", "r4", "memory"
                );
                arr[i] *= 2;
            } else if ((i + j) % 3 == 1) {
                asm volatile (
                    "/* Alternative clobber */"
                    :
                    :
                    : "r5", "r6", "r7", "r8", "memory"
                );
                arr[j] /= 2;
            }
            
            /* More computations to extend live ranges */
            for (int k = 0; k < 4; k++) {
                arr[(i + k) % 32] += arr[(j + k) % 32];
            }
        }
        
        /* Accumulate results */
        sum += arr[i];
    }
    
    return sum;
}

/* Third test: Function with many parameters to create argument passing pressure */
int test_many_args(int a1, int a2, int a3, int a4, int a5, 
                   int a6, int a7, int a8, int a9, int a10,
                   int a11, int a12, int a13, int a14, int a15) {
    /* All arguments are live initially */
    int b1 = a1 + a2;
    int b2 = a3 + a4;
    int b3 = a5 + a6;
    int b4 = a7 + a8;
    int b5 = a9 + a10;
    int b6 = a11 + a12;
    int b7 = a13 + a14;
    int b8 = a15 + a1;
    
    /* Complex computation with all variables live */
    for (int i = 0; i < 100; i++) {
        b1 = b2 + b3;
        b2 = b3 + b4;
        b3 = b4 + b5;
        b4 = b5 + b6;
        b5 = b6 + b7;
        b6 = b7 + b8;
        b7 = b8 + b1;
        b8 = b1 + b2;
        
        /* Force spilling with volatile asm */
        asm volatile (
            "/* Massive clobber to force register pressure */"
            :
            :
            : "r0", "r1", "r2", "r3", "r4", "r5", "r6", "r7",
              "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15",
              "r16", "r17", "r18", "r19", "r20", "r21", "r22", "r23",
              "r24", "r25", "r26", "r27", "r28", "r29", "r30", "r31",
              "memory"
        );
    }
    
    return b1 + b2 + b3 + b4 + b5 + b6 + b7 + b8;
}

/* Main function to drive tests with different parameters */
int main() {
    int total = 0;
    
    /* Test with different iteration counts to explore different graph sizes */
    for (int i = 1; i <= 5; i++) {
        total += test_ira_conflict(i);
        total += test_ira_conflict2(i);
        
        /* Test with many arguments */
        total += test_many_args(i, i*2, i*3, i*4, i*5,
                               i*6, i*7, i*8, i*9, i*10,
                               i*11, i*12, i*13, i*14, i*15);
    }
    
    /* Prevent dead code elimination */
    asm volatile ("" : : "r"(total));
    
    return total % 256;  /* Return small value to avoid overflow */
}
