#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

/* Packed struct to force unaligned accesses and secondary reloads */
struct __attribute__((packed)) Packed {
    double d;
    int i;
    float f;
    long l;
    char c;
};

/* Volatile variable to prevent optimization and create conditional paths */
volatile int global_flag = 1;

/* Target function that will induce many reloads */
void induce_reloads(int N, int argc) {
    /* Create many live variables to exceed register file */
    int a = rand() % 100, b = rand() % 100, c = rand() % 100;
    int d = rand() % 100, e = rand() % 100, f = rand() % 100;
    int g = rand() % 100, h = rand() % 100, i = rand() % 100;
    int j = rand() % 100, k = rand() % 100, l = rand() % 100;
    
    /* Mixed types to use different register classes */
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
    
    /* Multi-dimensional array for address reloads */
    int arr[200][200];
    
    /* Packed struct accessed through volatile pointer */
    struct Packed packed;
    volatile struct Packed *volatile_packed = &packed;
    
    /* Initialize array with non-constant values */
    for (int x = 0; x < 200; x++) {
        for (int y = 0; y < 200; y++) {
            arr[x][y] = (x * y + rand()) % 1000;
        }
    }
    
    /* Main computation loop - creates register pressure */
    for (int iter = 0; iter < N; iter++) {
        /* Complex array access with computed indices - forces address reloads */
        int idx1 = (a + b + iter) % 199;
        int idx2 = (c + d + iter) % 199;
        int idx3 = (e + f + iter) % 199;
        int idx4 = (g + h + iter) % 199;
        
        /* Swapping array elements using many live variables */
        int temp1 = arr[idx1][idx2];
        int temp2 = arr[idx3][idx4];
        arr[idx1][idx2] = arr[idx4][idx3] + temp2;
        arr[idx3][idx4] = arr[idx2][idx1] + temp1;
        
        /* Inline assembly with conflicting constraints - forces reloads */
        /* Tied operand constraint (0) forces input/output to same register */
        asm volatile (
            "add %0, %1, %2\n\t"
            : "=r"(a)
            : "r"(b), "0"(a)
            : "cc"
        );
        
        /* Another asm with different constraints on same variable */
        asm volatile (
            "imul %0, %1\n\t"
            : "+r"(b)
            : "r"(c)
            : "cc"
        );
        
        /* Float/double operations using inline asm */
        asm volatile (
            "addsd %0, %1, %2\n\t"
            : "=x"(da)
            : "x"(db), "0"(da)
        );
        
        asm volatile (
            "addss %0, %1, %2\n\t"
            : "=x"(fa)
            : "x"(fb), "0"(fa)
        );
        
        /* Long operations */
        asm volatile (
            "add %0, %1, %2\n\t"
            : "=r"(la)
            : "r"(lb), "0"(la)
            : "cc"
        );
        
        /* Create complex data dependencies to keep variables live */
        c = a + b + d;
        d = c + e + f;
        e = d + g + h;
        f = e + i + j;
        g = f + k + l;
        
        /* Float dependency chain */
        fb = fa + fc + fd;
        fc = fb + fa * 0.5f;
        fd = fc + fb * 0.3f;
        
        /* Double dependency chain */
        db = da + dc * 1.1;
        dc = db + dd * 0.9;
        dd = dc + da * 1.2;
        
        /* Long dependency chain */
        lb = la + lc;
        lc = lb + ld;
        ld = lc + la;
        
        /* Access packed struct through volatile pointer - may need secondary reload */
        volatile_packed->d = da;
        volatile_packed->i = a;
        volatile_packed->f = fa;
        volatile_packed->l = la;
        
        /* Conditional block for optional reloads */
        if (global_flag || (iter % 100 == 0)) {
            /* Use different subset of variables inside conditional */
            int cond_var = h + i + j;
            float cond_float = fc + fd;
            double cond_double = dc + dd;
            
            /* More inline asm in conditional path */
            asm volatile (
                "sub %0, %1, %2\n\t"
                : "=r"(h)
                : "r"(i), "0"(h)
                : "cc"
            );
            
            /* Complex computation using conditional variables */
            i = cond_var + k + l;
            j = i * 2 - cond_var;
            
            /* Update array based on conditional */
            arr[iter % 199][(iter * 7) % 199] = cond_var;
        } else {
            /* Alternative path with different variable usage */
            k = l + m + n;
            l = k * 3 / 2;
        }
        
        /* More array manipulation with different indices */
        int idx5 = (iter * 13) % 199;
        int idx6 = (iter * 17) % 199;
        int idx7 = (iter * 19) % 199;
        int idx8 = (iter * 23) % 199;
        
        arr[idx5][idx6] = arr[idx6][idx5] + a;
        arr[idx7][idx8] = arr[idx8][idx7] + b;
        
        /* Force spill by using all variables in one expression */
        int complex_expr = a + b + c + d + e + f + g + h + i + j + k + l;
        float float_expr = fa + fb + fc + fd;
        double double_expr = da + db + dc + dd;
        long long_expr = la + lb + lc + ld;
        
        /* Store results back to prevent elimination */
        arr[iter % 199][0] = complex_expr;
        arr[0][iter % 199] = (int)(float_expr * 100);
        packed.d = double_expr;
        packed.l = long_expr;
    }
    
    /* Compute checksum to prevent dead code elimination */
    long checksum = 0;
    for (int x = 0; x < 100; x++) {
        for (int y = 0; y < 100; y++) {
            checksum += arr[x][y];
        }
    }
    
    checksum += a + b + c + d + e + f + g + h + i + j + k + l;
    checksum += (long)(fa * 1000 + fb * 1000 + fc * 1000 + fd * 1000);
    checksum += (long)(da * 1000 + db * 1000 + dc * 1000 + dd * 1000);
    checksum += la + lb + lc + ld;
    
    printf("Checksum: %ld\n", checksum);
}

int main(int argc, char *argv[]) {
    srand(time(NULL));
    
    /* Use command line argument for loop count */
    int N = (argc > 1) ? atoi(argv[1]) : 100;
    
    /* Additional variables in main to increase pressure */
    int m = rand() % 100, n = rand() % 100, o = rand() % 100;
    int p = rand() % 100, q = rand() % 100, r = rand() % 100;
    float fe = rand() / (float)RAND_MAX;
    float ff = rand() / (float)RAND_MAX;
    double de = rand() / (double)RAND_MAX;
    double df = rand() / (double)RAND_MAX;
    long le = rand() * 100L;
    long lf = rand() * 100L;
    
    /* Call the reload-inducing function multiple times */
    for (int call = 0; call < 3; call++) {
        induce_reloads(N, argc);
        
        /* Use the additional variables between calls */
        m = n + o + p;
        n = m + q + r;
        fe = ff * 1.1f;
        ff = fe * 0.9f;
        de = df * 1.05;
        df = de * 0.95;
        le = lf + 1000;
        lf = le - 500;
    }
    
    return 0;
}
