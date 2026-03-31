/* reload_trigger.c - Program to force GCC reload pass to initialize all reload struct fields */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

/* Packed struct to force unaligned accesses and potential secondary reloads */
struct __attribute__((packed)) PackedStruct {
    double d;
    int i;
    float f;
    long l;
    char c;
    short s;
};

/* Volatile variables to prevent optimization */
volatile int global_flag = 1;
volatile int *volatile volatile_ptr;

/* Target function with high register pressure */
__attribute__((noinline))
unsigned long long trigger_reloads(int N, int seed) {
    /* Declare many scalar variables to exceed available registers */
    int a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p, q, r, s, t;
    float fa, fb, fc, fd, fe, ff, fg, fh, fi, fj;
    double da, db, dc, dd, de, df, dg, dh;
    long la, lb, lc, ld, le, lf;
    
    /* Multi-dimensional array for address reloads */
    int arr[128][128];
    
    /* Packed struct for unaligned access reloads */
    struct PackedStruct ps[16];
    
    /* Initialize with non-constant values */
    srand(seed);
    a = rand() % 100; b = rand() % 100; c = rand() % 100; d = rand() % 100;
    e = rand() % 100; f = rand() % 100; g = rand() % 100; h = rand() % 100;
    i = rand() % 100; j = rand() % 100; k = rand() % 100; l = rand() % 100;
    m = rand() % 100; n = rand() % 100; o = rand() % 100; p = rand() % 100;
    q = rand() % 100; r = rand() % 100; s = rand() % 100; t = rand() % 100;
    
    fa = (float)rand() / RAND_MAX; fb = (float)rand() / RAND_MAX;
    fc = (float)rand() / RAND_MAX; fd = (float)rand() / RAND_MAX;
    fe = (float)rand() / RAND_MAX; ff = (float)rand() / RAND_MAX;
    fg = (float)rand() / RAND_MAX; fh = (float)rand() / RAND_MAX;
    fi = (float)rand() / RAND_MAX; fj = (float)rand() / RAND_MAX;
    
    da = (double)rand() / RAND_MAX; db = (double)rand() / RAND_MAX;
    dc = (double)rand() / RAND_MAX; dd = (double)rand() / RAND_MAX;
    de = (double)rand() / RAND_MAX; df = (double)rand() / RAND_MAX;
    dg = (double)rand() / RAND_MAX; dh = (double)rand() / RAND_MAX;
    
    la = rand() % 1000; lb = rand() % 1000; lc = rand() % 1000;
    ld = rand() % 1000; le = rand() % 1000; lf = rand() % 1000;
    
    /* Initialize array with non-constant pattern */
    for (int x = 0; x < 128; x++) {
        for (int y = 0; y < 128; y++) {
            arr[x][y] = (x * 31 + y * 17 + seed) % 256;
        }
    }
    
    /* Initialize packed structs */
    for (int idx = 0; idx < 16; idx++) {
        ps[idx].d = (double)rand() / RAND_MAX;
        ps[idx].i = rand() % 100;
        ps[idx].f = (float)rand() / RAND_MAX;
        ps[idx].l = rand() % 1000;
        ps[idx].c = rand() % 256;
        ps[idx].s = rand() % 65536;
    }
    
    volatile_ptr = &global_flag;
    
    unsigned long long checksum = 0;
    
    /* Main computation loop - creates register pressure and various reloads */
    for (int iter = 0; iter < N; iter++) {
        /* Complex addressing mode - forces address register reloads */
        int idx1 = (iter * 13) % 128;
        int idx2 = (iter * 17) % 128;
        
        /* Chain of integer computations - keeps many variables live */
        a = b + c;
        d = e * f - g;
        h = i ^ j;
        k = l | m;
        n = o & p;
        q = r << 2;
        s = t >> 1;
        
        /* Floating point computations - uses FP registers */
        fa = fb * fc + fd;
        fe = ff - fg * fh;
        fi = fj / (fa + 1.0f);
        
        /* Double precision computations */
        da = db + dc * dd;
        de = df - dg / (dh + 1.0);
        
        /* Long integer computations */
        la = lb * lc + ld;
        le = lf ^ la;
        
        /* Inline assembly with conflicting constraints - forces reloads */
        /* Tied operand constraint (output = input 0) */
        asm volatile (
            "addl %1, %0\n\t"
            : "+r"(a)        /* read-write operand, tied */
            : "r"(b)         /* input only */
            : "cc"
        );
        
        /* Another asm with memory constraint */
        asm volatile (
            "movl %1, %%eax\n\t"
            "addl %%eax, %0\n\t"
            : "+m"(arr[idx1][idx2])  /* memory operand */
            : "r"(c)                  /* register operand */
            : "%eax", "cc"
        );
        
        /* Asm with specific register constraints (x86) */
        asm volatile (
            "imull %%edx, %%ecx\n\t"
            "addl %%ecx, %0\n\t"
            : "+r"(d)
            : "d"(e), "c"(f)         /* edx and ecx specifically */
            : "cc"
        );
        
        /* Access packed struct through volatile pointer - may cause secondary reloads */
        if (*volatile_ptr) {
            /* Conditional block - may create optional reload contexts */
            struct PackedStruct *volatile vps = &ps[iter % 16];
            
            /* Access unaligned members */
            vps->i = vps->i + a;
            vps->f = vps->f + fa;
            vps->d = vps->d + da;
            
            /* More computations only in conditional path */
            int tmp = vps->i * 3;
            float ftmp = vps->f * 2.5f;
            double dtmp = vps->d / 1.5;
            
            /* Use these in subsequent computations */
            g = tmp + h;
            fb = ftmp + fc;
            db = dtmp + dc;
        }
        
        /* Array operation with complex addressing - forces address reloads */
        arr[idx2][idx1] = arr[idx1][idx2] + arr[idx2][(idx1 + 1) % 128];
        
        /* More chained dependencies */
        b = c + d;
        e = f + g;
        i = j + k;
        l = m + n;
        o = p + q;
        r = s + t;
        
        fb = fc + fd;
        ff = fg + fh;
        db = dc + dd;
        dg = dh + da;
        
        lb = lc + ld;
        lf = la + le;
        
        /* Another asm with multiple constraints */
        asm volatile (
            "mov %1, %%rax\n\t"
            "add %2, %%rax\n\t"
            "mov %%rax, %0\n\t"
            : "=r"(t)
            : "r"(a), "r"(b)
            : "%rax", "cc"
        );
        
        /* Update checksum to prevent elimination */
        checksum += a + b + c + d + e + f + g + h + i + j;
        checksum += (unsigned long long)(fa * 1000);
        checksum += (unsigned long long)(da * 1000);
        checksum += la + lb + lc + ld + le + lf;
        checksum += arr[idx1][idx2];
    }
    
    /* Final aggregation to use all variables */
    int final_int = a + b + c + d + e + f + g + h + i + j + k + l + m + n + o + p + q + r + s + t;
    float final_float = fa + fb + fc + fd + fe + ff + fg + fh + fi + fj;
    double final_double = da + db + dc + dd + de + df + dg + dh;
    long final_long = la + lb + lc + ld + le + lf;
    
    checksum += final_int + (unsigned long long)(final_float * 1000) + 
                (unsigned long long)(final_double * 1000) + final_long;
    
    /* Sum array elements */
    for (int x = 0; x < 128; x += 8) {
        for (int y = 0; y < 128; y += 8) {
            checksum += arr[x][y];
        }
    }
    
    /* Sum packed struct elements */
    for (int idx = 0; idx < 16; idx++) {
        checksum += ps[idx].i + (unsigned long long)(ps[idx].f * 1000) + 
                   (unsigned long long)(ps[idx].d * 1000) + ps[idx].l;
    }
    
    return checksum;
}

int main(int argc, char *argv[]) {
    int N = (argc > 1) ? atoi(argv[1]) : 100;
    int seed = (argc > 2) ? atoi(argv[2]) : time(NULL);
    
    printf("Running with N=%d, seed=%d\n", N, seed);
    
    unsigned long long result = trigger_reloads(N, seed);
    
    printf("Result checksum: %llu\n", result);
    
    return 0;
}
