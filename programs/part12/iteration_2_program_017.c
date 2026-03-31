/* reload_trigger.c - Program to trigger GCC reload pass initialization */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

/* Packed struct to force unaligned accesses */
struct __attribute__((packed)) Packed {
    double d;
    int i;
    char c;
    long l;
    float f;
};

/* Volatile variable to prevent optimization */
volatile int global_flag = 0;

/* Target function with high register pressure */
void __attribute__((noinline)) 
force_reloads(int N, int arr[200][200], struct Packed* p) {
    /* Declare many live variables of mixed types */
    int a = rand() % 100, b = rand() % 100, c = rand() % 100;
    int d = rand() % 100, e = rand() % 100, f = rand() % 100;
    int g = rand() % 100, h = rand() % 100, i = rand() % 100;
    int j = rand() % 100, k = rand() % 100, l = rand() % 100;
    
    float fa = rand() / (float)RAND_MAX;
    float fb = rand() / (float)RAND_MAX;
    float fc = rand() / (float)RAND_MAX;
    float fd = rand() / (float)RAND_MAX;
    
    double da = rand() / (double)RAND_MAX;
    double db = rand() / (double)RAND_MAX;
    double dc = rand() / (double)RAND_MAX;
    double dd = rand() / (double)RAND_MAX;
    
    long la = rand() * 100L;
    long lb = rand() * 100L;
    long lc = rand() * 100L;
    long ld = rand() * 100L;
    
    /* Complex addressing mode variables */
    int idx1 = 0, idx2 = 0;
    
    /* Main computation loop - creates many live values */
    for (int iter = 0; iter < N; iter++) {
        /* Update indices with non-trivial computation */
        idx1 = (a * b + c * d) % 199;
        idx2 = (e * f + g * h) % 199;
        
        /* Complex array access with swapping - forces address reloads */
        int temp = arr[idx1][idx2];
        arr[idx1][idx2] = arr[idx2][idx1] + 1;
        arr[idx2][idx1] = temp - 1;
        
        /* Chain computations to keep variables live */
        a = b + c;
        b = c * d;
        c = d - e;
        d = e + f;
        e = f / (g + 1);
        f = g ^ h;
        g = h | i;
        h = i & j;
        i = j << 2;
        j = k >> 1;
        k = l + a;
        l = a * b;
        
        /* Floating point computations */
        fa = fb + fc;
        fb = fc * fd;
        fc = fd - fa;
        fd = fa * 1.1f;
        
        da = db + dc;
        db = dc * dd;
        dc = dd - da;
        dd = da * 1.1;
        
        la = lb + lc;
        lb = lc * ld;
        lc = ld - la;
        ld = la / 3;
        
        /* Inline assembly with conflicting constraints */
        /* Forces specific register allocation and reloads */
        asm volatile (
            "addl %1, %0\n\t"
            : "+r" (a), "+r" (b)
            : "r" (c)
            : "cc"
        );
        
        asm volatile (
            "imull %1, %0\n\t"
            : "+r" (d), "+r" (e)
            : "m" (f)
            : "cc"
        );
        
        /* More assembly with tied operands */
        asm volatile (
            "mov %1, %0\n\t"
            "addl $1, %0\n\t"
            : "=r" (g)
            : "0" (h), "r" (i)
            : "cc"
        );
        
        /* Conditional block for optional reloads */
        if (global_flag) {
            /* Use different subset of variables */
            int t1 = a + b;
            int t2 = c + d;
            float t3 = fa + fb;
            double t4 = da + db;
            
            /* More assembly in conditional path */
            asm volatile (
                "addsd %1, %0\n\t"
                : "+x" (t4)
                : "x" (dc)
            );
            
            arr[iter % 10][0] = t1 + t2;
            p->d = t4;
        } else {
            /* Alternative path with different variables */
            int u1 = e + f;
            int u2 = g + h;
            float u3 = fc + fd;
            double u4 = dc + dd;
            
            asm volatile (
                "addss %1, %0\n\t"
                : "+x" (u3)
                : "x" (fa)
            );
            
            arr[0][iter % 10] = u1 + u2;
            p->i = u1;
        }
        
        /* Access packed struct through volatile pointer */
        volatile struct Packed* vp = p;
        vp->d = da;
        vp->i = a;
        vp->f = fa;
        vp->l = la;
        
        /* More computations to use all variables */
        a = a + (int)fa + (int)da + (int)la;
        b = b + (int)fb + (int)db + (int)lb;
        fa = fa + (float)a + (float)b;
        da = da + (double)fa + (double)fb;
        
        /* Prevent loop unrolling */
        asm volatile ("" : : "r"(iter) : "memory");
    }
    
    /* Use all variables in final computation to prevent elimination */
    int sum_int = a + b + c + d + e + f + g + h + i + j + k + l;
    float sum_float = fa + fb + fc + fd;
    double sum_double = da + db + dc + dd;
    long sum_long = la + lb + lc + ld;
    
    /* Store results to array to ensure they're used */
    arr[0][0] = sum_int + (int)sum_float + (int)sum_double + (int)sum_long;
    arr[0][1] = (int)(sum_float * 100);
    arr[0][2] = (int)(sum_double * 100);
}

int main(int argc, char* argv[]) {
    srand(time(NULL));
    
    /* Non-constant loop bound from command line */
    int N = (argc > 1) ? atoi(argv[1]) : 100;
    if (N <= 0) N = 100;
    
    /* Large multi-dimensional array */
    int arr[200][200];
    
    /* Initialize array with non-constant values */
    for (int i = 0; i < 200; i++) {
        for (int j = 0; j < 200; j++) {
            arr[i][j] = (i * 197 + j * 193) % 1000;
        }
    }
    
    /* Packed struct instance */
    struct Packed p;
    p.d = 3.14159;
    p.i = 42;
    p.c = 'X';
    p.l = 123456789L;
    p.f = 2.71828f;
    
    /* Call the reload-intensive function multiple times */
    for (int outer = 0; outer < 3; outer++) {
        global_flag = outer % 2;  /* Toggle flag for conditional paths */
        force_reloads(N, arr, &p);
    }
    
    /* Compute checksum to prevent dead code elimination */
    long checksum = 0;
    for (int i = 0; i < 200; i++) {
        for (int j = 0; j < 200; j++) {
            checksum += arr[i][j];
        }
    }
    
    checksum += (long)p.d + p.i + p.c + p.l + (long)p.f;
    
    printf("Checksum: %ld\n", checksum);
    return 0;
}
