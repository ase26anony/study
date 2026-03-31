/* reload_trigger.c - Program to trigger GCC reload pass initialization */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

/* Packed struct to force unaligned accesses and secondary reloads */
struct __attribute__((packed)) Packed {
    double d;
    int i;
    char c;
    long l;
    float f;
};

/* Volatile flag for conditional execution */
volatile int reload_flag = 1;

/* Target function with high register pressure */
__attribute__((noinline))
static long trigger_reloads(int N, struct Packed* p, int arr[200][200]) {
    /* Declare many scalar variables to exceed available registers */
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
    
    long la = rand() * rand();
    long lb = rand() * rand();
    long lc = rand() * rand();
    long ld = rand() * rand();
    
    /* Complex addressing mode variables */
    int idx1 = 0, idx2 = 0;
    
    /* Main computation loop - creates register pressure */
    for (int iter = 0; iter < N; iter++) {
        /* Update indices for array access */
        idx1 = (iter * 17) % 199;
        idx2 = (iter * 23) % 199;
        
        /* Complex array access with computed indices - forces address reloads */
        int temp = arr[idx1][idx2];
        arr[idx2][idx1] = arr[idx1][idx2] + 1;
        arr[idx1][idx2] = temp;
        
        /* Chain of arithmetic operations keeping many variables live */
        a = b + c;
        d = e * f;
        g = h - i;
        j = k / (l + 1);
        
        /* Floating point chains */
        fa = fb * fc;
        fb = fc + fd;
        fc = fd - fa;
        fd = fa * fb;
        
        /* Double precision chains */
        da = db + dc;
        db = dc - dd;
        dc = dd * da;
        dd = da / (db + 1.0);
        
        /* Long integer chains */
        la = lb + lc;
        lb = lc - ld;
        lc = ld * la;
        ld = la / (lb + 1);
        
        /* Inline assembly with conflicting constraints - forces reloads */
        /* Tied operand constraint (output tied to input) */
        asm volatile (
            "add %0, %1, %2\n\t"
            : "=r"(a)      /* Output in register */
            : "r"(b), "0"(c)  /* Inputs, with '0' meaning same as output 0 */
            : "cc"
        );
        
        /* Another asm with memory constraint */
        asm volatile (
            "imul %0, %1\n\t"
            : "+r"(d)      /* Read-write operand */
            : "rm"(e)      /* Register or memory */
            : "cc"
        );
        
        /* Floating point asm with specific constraints */
        asm volatile (
            "addss %0, %1\n\t"
            : "=x"(fa)     /* SSE register constraint */
            : "x"(fb), "0"(fc)
        );
        
        /* Conditional block for optional reloads */
        if (reload_flag) {
            /* Use different subset of variables inside conditional */
            int t1 = a + g;
            int t2 = d + j;
            float t3 = fa + fc;
            double t4 = da + dc;
            
            /* More asm in conditional path */
            asm volatile (
                "sub %0, %1, %2\n\t"
                : "=r"(t1)
                : "r"(t2), "0"(iter)
                : "cc"
            );
            
            /* Update array based on conditional computation */
            arr[iter % 199][(iter + 1) % 199] = t1 + t2;
            
            /* Mix types */
            da = t3 + t4;
        } else {
            /* Alternative path with different variable usage */
            arr[(iter + 1) % 199][iter % 199] = b + h;
            fb = dc + 1.0;
        }
        
        /* Access packed struct through volatile pointer - forces secondary reloads */
        volatile struct Packed* volatile_p = p;
        volatile_p->d = da;
        volatile_p->i = a;
        volatile_p->f = fa;
        volatile_p->l = la;
        
        /* Read back from packed struct - unaligned access */
        double packed_d = volatile_p->d;
        int packed_i = volatile_p->i;
        
        /* Use packed struct values in computation */
        da = da * packed_d;
        a = a ^ packed_i;
        
        /* More arithmetic to create data dependencies */
        b = c + d;
        c = e + f;
        e = g + h;
        f = i + j;
        h = k + l;
        
        /* Cross-type operations */
        fa = (float)da + fb;
        db = (double)fa + dc;
        
        /* Long operations */
        la = lb << 2;
        lb = lc >> 1;
        lc = ld * 3;
        ld = la + lb;
        
        /* Prevent loop invariant code motion */
        reload_flag = iter % 7;
    }
    
    /* Compute checksum using all variables to prevent elimination */
    long checksum = a + b + c + d + e + f + g + h + i + j + k + l;
    checksum += (long)fa + (long)fb + (long)fc + (long)fd;
    checksum += (long)da + (long)db + (long)dc + (long)dd;
    checksum += la + lb + lc + ld;
    
    /* Add array elements to checksum */
    for (int i = 0; i < 10; i++) {
        for (int j = 0; j < 10; j++) {
            checksum += arr[i][j];
        }
    }
    
    return checksum;
}

int main(int argc, char* argv[]) {
    /* Use command line argument for loop bound */
    int N = (argc > 1) ? atoi(argv[1]) : 100;
    
    /* Initialize random seed */
    srand(time(NULL));
    
    /* Large multi-dimensional array */
    int arr[200][200];
    
    /* Initialize array with random values */
    for (int i = 0; i < 200; i++) {
        for (int j = 0; j < 200; j++) {
            arr[i][j] = rand() % 1000;
        }
    }
    
    /* Packed struct instance */
    struct Packed p;
    p.d = rand() / (double)RAND_MAX;
    p.i = rand();
    p.c = rand() % 256;
    p.l = rand() * rand();
    p.f = rand() / (float)RAND_MAX;
    
    /* Call the reload-intensive function multiple times */
    long total_checksum = 0;
    for (int iteration = 0; iteration < 3; iteration++) {
        total_checksum += trigger_reloads(N, &p, arr);
        
        /* Modify inputs slightly each iteration */
        N = (N + 17) % 200 + 50;
        reload_flag = iteration % 2;
    }
    
    /* Print result to prevent optimization */
    printf("Checksum: %ld\n", total_checksum);
    
    return 0;
}
