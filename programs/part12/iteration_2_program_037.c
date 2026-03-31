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

/* Volatile variables to prevent optimization */
volatile int global_flag = 1;
volatile int *volatile volatile_ptr;

/* Function with high register pressure and complex operations */
void reload_inducing_function(int N, int *checksum) {
    /* Declare many variables to exceed available registers */
    int a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p, q, r, s, t;
    float fa, fb, fc, fd, fe, ff, fg, fh, fi, fj;
    double da, db, dc, dd, de, df, dg, dh;
    long la, lb, lc, ld, le, lf, lg, lh;
    
    /* Initialize with non-constant values */
    a = *checksum + 1; b = a * 2; c = b + 3; d = c - 4; e = d * 5;
    f = e / 2; g = f + 7; h = g - 8; i = h * 9; j = i + 10;
    k = j - 11; l = k * 12; m = l + 13; n = m - 14; o = n * 15;
    p = o + 16; q = p - 17; r = q * 18; s = r + 19; t = s - 20;
    
    fa = a * 0.1f; fb = b * 0.2f; fc = c * 0.3f; fd = d * 0.4f;
    fe = e * 0.5f; ff = f * 0.6f; fg = g * 0.7f; fh = h * 0.8f;
    fi = i * 0.9f; fj = j * 1.0f;
    
    da = a * 0.01; db = b * 0.02; dc = c * 0.03; dd = d * 0.04;
    de = e * 0.05; df = f * 0.06; dg = g * 0.07; dh = h * 0.08;
    
    la = a * 100L; lb = b * 200L; lc = c * 300L; ld = d * 400L;
    le = e * 500L; lf = f * 600L; lg = g * 700L; lh = h * 800L;
    
    /* Multi-dimensional array for address reloads */
    int arr[128][128];
    for (int x = 0; x < 128; x++) {
        for (int y = 0; y < 128; y++) {
            arr[x][y] = x * y + *checksum;
        }
    }
    
    /* Packed struct for unaligned accesses */
    struct Packed packed;
    packed.d = da;
    packed.i = a;
    packed.f = fa;
    packed.l = la;
    packed.c = 'A';
    
    /* Volatile pointer to packed struct */
    volatile struct Packed *vol_packed = &packed;
    
    /* Main computation loop - creates complex data dependencies */
    for (int iter = 0; iter < N; iter++) {
        /* Complex array access pattern forcing address reloads */
        for (int x = 1; x < 127; x++) {
            for (int y = 1; y < 127; y++) {
                /* Non-constant indices force address computation */
                int idx1 = (x + iter) % 126 + 1;
                int idx2 = (y + iter * 2) % 126 + 1;
                
                /* Complex addressing with multiple variables */
                arr[idx1][idx2] = arr[idx2][idx1] + a + b + c;
                
                /* More complex computation with all variables */
                int temp = arr[x][y];
                temp = temp * a + b - c * d + e / (f + 1);
                arr[x][y] = temp;
            }
        }
        
        /* Inline assembly with conflicting constraints */
        /* Force input reloads with 'r' constraints */
        asm volatile (
            "addl %1, %0\n\t"
            "subl %2, %0\n\t"
            : "+r"(a), "+r"(b)
            : "r"(c)
            : "cc"
        );
        
        /* Another asm with tied operand (forces reload) */
        asm volatile (
            "imull %1, %0\n\t"
            : "+r"(d)
            : "r"(e), "0"(d)
            : "cc"
        );
        
        /* Asm with memory constraint */
        asm volatile (
            "movl %1, %%eax\n\t"
            "addl %%eax, %0\n\t"
            : "+m"(arr[iter % 127][0])
            : "r"(f)
            : "%eax", "cc"
        );
        
        /* Float/double asm operations */
        asm volatile (
            "addss %1, %0\n\t"
            : "+x"(fa)
            : "x"(fb)
        );
        
        asm volatile (
            "addsd %1, %0\n\t"
            : "+x"(da)
            : "x"(db)
        );
        
        /* Conditional block for optional reloads */
        if (global_flag || (iter % 7 == 0)) {
            /* Use different subset of variables */
            g = h + i * j - k;
            m = n * o + p / (q + 1);
            
            /* Access packed struct through volatile pointer */
            int packed_val = vol_packed->i;
            float packed_float = vol_packed->f;
            
            /* Complex computation with packed data */
            g += packed_val;
            m += (int)packed_float;
            
            /* More inline asm in conditional path */
            asm volatile (
                "xorl %1, %0\n\t"
                : "+r"(g)
                : "r"(m)
                : "cc"
            );
        } else {
            /* Alternative path with different variables */
            r = s * t - a;
            fa = fb + fc - fd;
            
            /* Another asm with different constraints */
            asm volatile (
                "orl %1, %0\n\t"
                : "+r"(r)
                : "r"(s)
                : "cc"
            );
        }
        
        /* Chain computations to keep all variables live */
        a = b + c - d * e / (f + 1);
        b = c + d - e * f / (g + 1);
        c = d + e - f * g / (h + 1);
        d = e + f - g * h / (i + 1);
        e = f + g - h * i / (j + 1);
        
        fa = fb + fc * fd - fe;
        fb = fc + fd * fe - ff;
        fc = fd + fe * ff - fg;
        
        da = db + dc * dd - de;
        db = dc + dd * de - df;
        
        la = lb + lc * ld - le;
        lb = lc + ld * le - lf;
        
        /* Update checksum with all variables */
        *checksum += a + b + c + d + e + f + g + h + i + j;
        *checksum += k + l + m + n + o + p + q + r + s + t;
        *checksum += (int)fa + (int)fb + (int)fc + (int)fd + (int)fe;
        *checksum += (int)da + (int)db;
        *checksum += (int)la + (int)lb;
        
        /* Access array with complex index */
        *checksum += arr[iter % 127][(iter * 3) % 127];
        
        /* Access packed struct member */
        *checksum += vol_packed->i;
    }
    
    /* Final aggregation to prevent optimization */
    *checksum = *checksum + a + b + c + (int)fa + (int)da + (int)la;
}

int main(int argc, char *argv[]) {
    int N = (argc > 1) ? atoi(argv[1]) : 50;
    int checksum = 0;
    
    /* Seed RNG for variable initialization */
    srand(time(NULL));
    
    /* Call reload-inducing function */
    reload_inducing_function(N, &checksum);
    
    /* Print result to prevent dead code elimination */
    printf("Checksum: %d\n", checksum);
    
    return 0;
}
