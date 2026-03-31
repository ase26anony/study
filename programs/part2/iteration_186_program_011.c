/* mcf_coverage.c - Program to trigger GCC's Minimum Cost Flow debug output */
#include <stdio.h>
#include <stdlib.h>

/* Force compiler to keep all variables alive */
#define USE(x) asm volatile("" : : "r"(x))

/* Complex test function with high register pressure */
static int __attribute__((noinline)) 
test_mcf_pressure(int seed, int iterations) {
    /* Declare many variables to create register pressure */
    volatile int v0 = seed, v1 = seed + 1, v2 = seed + 2, v3 = seed + 3;
    volatile int v4 = seed + 4, v5 = seed + 5, v6 = seed + 6, v7 = seed + 7;
    volatile int v8 = seed + 8, v9 = seed + 9, v10 = seed + 10, v11 = seed + 11;
    volatile int v12 = seed + 12, v13 = seed + 13, v14 = seed + 14, v15 = seed + 15;
    volatile int v16 = seed + 16, v17 = seed + 17, v18 = seed + 18, v19 = seed + 19;
    volatile int v20 = seed + 20, v21 = seed + 21, v22 = seed + 22, v23 = seed + 23;
    volatile int v24 = seed + 24, v25 = seed + 25, v26 = seed + 26, v27 = seed + 27;
    volatile int v28 = seed + 28, v29 = seed + 29;
    
    /* Mix different types to increase pressure across register classes */
    volatile float f0 = seed * 0.1f, f1 = seed * 0.2f, f2 = seed * 0.3f;
    volatile double d0 = seed * 0.01, d1 = seed * 0.02;
    volatile void* p0 = &v0, *p1 = &v1, *p2 = &v2;
    
    int result = 0;
    
    /* Complex loop with overlapping live ranges */
    for (int i = 0; i < iterations; i++) {
        /* Chain of dependent operations */
        v0 = v1 + v2;
        v1 = v3 * v4;
        v2 = v5 - v6;
        v3 = v7 ^ v8;
        v4 = v9 | v10;
        v5 = v11 & v12;
        v6 = v13 << 2;
        v7 = v14 >> 1;
        v8 = v15 + v16;
        v9 = v17 * v18;
        v10 = v19 - v20;
        v11 = v21 ^ v22;
        v12 = v23 | v24;
        v13 = v25 & v26;
        v14 = v27 << 3;
        v15 = v28 >> 2;
        
        /* Floating point operations mixed in */
        f0 = f1 * 1.1f;
        f1 = f2 + 0.5f;
        f2 = f0 - 0.3f;
        d0 = d1 * 1.01;
        d1 = d0 + 0.02;
        
        /* Pointer arithmetic */
        p0 = (char*)p1 + v0;
        p1 = (char*)p2 + v1;
        p2 = (char*)p0 + v2;
        
        /* Complex conditional structure creating many basic blocks */
        switch (i % 12) {
            case 0:
                v16 = v0 + v1;
                /* Inline asm with register clobbering */
                asm volatile("# MCF Test" : : : "eax", "ebx", "ecx", "edx");
                break;
            case 1:
                v17 = v2 * v3;
                asm volatile("# MCF Test" : : : "esi", "edi", "ebp");
                break;
            case 2:
                v18 = v4 - v5;
                /* Force spill/reload */
                asm volatile("# MCF Test" : "+r"(v18) : : "memory");
                break;
            case 3:
                v19 = v6 ^ v7;
                asm volatile("# MCF Test" : : : "xmm0", "xmm1", "xmm2");
                break;
            case 4:
                v20 = v8 | v9;
                asm volatile("# MCF Test" : : : "xmm3", "xmm4", "xmm5");
                break;
            case 5:
                v21 = v10 & v11;
                break;
            case 6:
                v22 = v12 << 1;
                asm volatile("# MCF Test" : : : "rax", "rbx", "rcx", "rdx");
                break;
            case 7:
                v23 = v13 >> 2;
                break;
            case 8:
                v24 = v14 + v15;
                asm volatile("# MCF Test" : : : "r8", "r9", "r10", "r11");
                break;
            case 9:
                v25 = v16 * v17;
                break;
            case 10:
                v26 = v18 - v19;
                asm volatile("# MCF Test" : : : "r12", "r13", "r14", "r15");
                break;
            case 11:
                v27 = v20 ^ v21;
                /* Complex expression with many operands */
                v28 = ((v22 | v23) & (v24 ^ v25)) + ((v26 << 1) | (v27 >> 2));
                break;
        }
        
        /* Nested conditionals creating control flow merges */
        if (i % 3 == 0) {
            v29 = v0 + v1;
            if (v29 > 1000) {
                v29 = v29 % 1000;
                goto label_a;
            } else {
                v29 = v29 * 2;
            }
        } else if (i % 3 == 1) {
            v29 = v2 * v3;
            if (v29 < 0) {
                v29 = -v29;
            label_a:
                v29 = v29 + 1;
            }
        } else {
            v29 = v4 - v5;
            /* Another asm clobber */
            asm volatile("# MCF Test" : : : "eax", "ebx", "ecx", "edx", "esi", "edi");
        }
        
        /* Loop with break/continue creating exit edges */
        for (int j = 0; j < 5; j++) {
            if (j == 2 && (i % 7 == 0)) {
                v29 += j * 10;
                continue;
            }
            if (j == 4 && (i % 11 == 0)) {
                v29 -= j * 5;
                break;
            }
            v29 += j;
        }
        
        /* Accumulate result */
        result += v0 + v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9;
        result += v10 + v11 + v12 + v13 + v14 + v15 + v16 + v17 + v18 + v19;
        result += v20 + v21 + v22 + v23 + v24 + v25 + v26 + v27 + v28 + v29;
        
        /* Force all variables to be considered live */
        USE(v0); USE(v1); USE(v2); USE(v3); USE(v4); USE(v5);
        USE(v6); USE(v7); USE(v8); USE(v9); USE(v10); USE(v11);
        USE(v12); USE(v13); USE(v14); USE(v15); USE(v16); USE(v17);
        USE(v18); USE(v19); USE(v20); USE(v21); USE(v22); USE(v23);
        USE(v24); USE(v25); USE(v26); USE(v27); USE(v28); USE(v29);
        USE(f0); USE(f1); USE(f2); USE(d0); USE(d1);
        USE(p0); USE(p1); USE(p2);
    }
    
    return result;
}

/* Second complex function to create interprocedural pressure */
static int __attribute__((noinline))
another_complex_function(int base) {
    volatile int a = base, b = base + 1, c = base + 2;
    volatile int d = base + 3, e = base + 4, f = base + 5;
    int sum = 0;
    
    /* Different control flow pattern */
    for (int i = 0; i < 50; i++) {
        if (i % 2 == 0) {
            a = b + c;
            b = c * d;
            asm volatile("# MCF Test 2" : : : "eax", "ebx", "ecx");
        } else {
            c = d - e;
            d = e ^ f;
            asm volatile("# MCF Test 2" : : : "edx", "esi", "edi");
        }
        
        /* Switch with many cases */
        switch (i % 8) {
            case 0: e = a << 1; break;
            case 1: f = b >> 2; break;
            case 2: a = c + d; break;
            case 3: b = e * f; break;
            case 4: c = a - b; break;
            case 5: d = c ^ e; break;
            case 6: e = f | a; break;
            case 7: f = b & d; break;
        }
        
        sum += a + b + c + d + e + f;
    }
    
    return sum;
}

int main(void) {
    int total = 0;
    
    /* Call test function multiple times with different seeds */
    for (int i = 0; i < 10; i++) {
        total += test_mcf_pressure(i * 100, 100);
        total += another_complex_function(i * 50);
    }
    
    printf("Result: %d\n", total);
    return 0;
}
