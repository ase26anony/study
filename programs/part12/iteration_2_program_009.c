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
void trigger_reloads(int N, int *checksum) {
    /* Declare many variables to exceed available registers */
    int a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p, q, r, s, t;
    float fa, fb, fc, fd, fe, ff, fg, fh, fi, fj;
    double da, db, dc, dd, de, df, dg, dh, di, dj;
    long la, lb, lc, ld, le, lf, lg, lh, li, lj;
    
    /* Initialize with non-constant values */
    a = N * 1; b = N * 2; c = N * 3; d = N * 4; e = N * 5;
    f = N * 6; g = N * 7; h = N * 8; i = N * 9; j = N * 10;
    k = N * 11; l = N * 12; m = N * 13; n = N * 14; o = N * 15;
    p = N * 16; q = N * 17; r = N * 18; s = N * 19; t = N * 20;
    
    fa = N * 1.1f; fb = N * 1.2f; fc = N * 1.3f; fd = N * 1.4f; fe = N * 1.5f;
    ff = N * 1.6f; fg = N * 1.7f; fh = N * 1.8f; fi = N * 1.9f; fj = N * 2.0f;
    
    da = N * 1.1; db = N * 1.2; dc = N * 1.3; dd = N * 1.4; de = N * 1.5;
    df = N * 1.6; dg = N * 1.7; dh = N * 1.8; di = N * 1.9; dj = N * 2.0;
    
    la = N * 21L; lb = N * 22L; lc = N * 23L; ld = N * 24L; le = N * 25L;
    lf = N * 26L; lg = N * 27L; lh = N * 28L; li = N * 29L; lj = N * 30L;
    
    /* Multi-dimensional array for address reloads */
    int arr[200][200];
    for (int x = 0; x < 200; x++) {
        for (int y = 0; y < 200; y++) {
            arr[x][y] = x * y + N;
        }
    }
    
    /* Packed struct for secondary reloads */
    struct Packed packed_arr[100];
    for (int idx = 0; idx < 100; idx++) {
        packed_arr[idx].d = idx * 1.5;
        packed_arr[idx].i = idx * 2;
        packed_arr[idx].f = idx * 0.5f;
        packed_arr[idx].l = idx * 3L;
        packed_arr[idx].c = idx % 256;
    }
    
    /* Complex loop with many live variables and interdependencies */
    for (int iter = 0; iter < N; iter++) {
        /* Create complex data dependencies between variables */
        a = b + c;
        b = c + d;
        c = d + e;
        d = e + f;
        e = f + g;
        f = g + h;
        g = h + i;
        h = i + j;
        i = j + k;
        j = k + l;
        
        /* Floating point dependencies */
        fa = fb + fc;
        fb = fc + fd;
        fc = fd + fe;
        fd = fe + ff;
        fe = ff + fg;
        
        /* Double precision dependencies */
        da = db + dc;
        db = dc + dd;
        dc = dd + de;
        dd = de + df;
        de = df + dg;
        
        /* Long integer dependencies */
        la = lb + lc;
        lb = lc + ld;
        lc = ld + le;
        ld = le + lf;
        le = lf + lg;
        
        /* Inline assembly with conflicting constraints to force reloads */
        /* Tied operand constraint (output tied to input) */
        asm volatile (
            "add %0, %1, %2\n\t"
            : "=r"(a)
            : "r"(b), "0"(c)
            : "cc"
        );
        
        /* Different register class constraints */
        asm volatile (
            "imul %0, %1, %2\n\t"
            : "=r"(d)
            : "r"(e), "r"(f)
            : "cc"
        );
        
        /* Memory constraint forcing spill/reload */
        asm volatile (
            "mov %0, %1\n\t"
            : "=r"(g)
            : "m"(h)
        );
        
        /* Complex addressing mode in array access - forces address reloads */
        int idx1 = (a + b) % 100;
        int idx2 = (c + d) % 100;
        arr[idx1][idx2] = arr[idx2][idx1] + 1;
        
        /* Access packed struct through volatile pointer - forces secondary reloads */
        volatile_ptr = &global_flag;
        if (*volatile_ptr) {
            /* Conditional block for optional reloads */
            struct Packed *p = &packed_arr[iter % 100];
            
            /* Access misaligned members */
            da += p->d;
            a += p->i;
            fa += p->f;
            la += p->l;
            
            /* More complex arithmetic in conditional path */
            m = n + o + p + q;
            n = o + p + q + r;
            o = p + q + r + s;
            
            /* Another inline assembly with specific constraints */
            asm volatile (
                "sub %0, %1, %2\n\t"
                : "=r"(p)
                : "r"(q), "r"(r)
                : "cc"
            );
        } else {
            /* Alternative path with different variable usage */
            s = t + a + b;
            t = a + b + c;
            
            /* Force different register allocation pattern */
            asm volatile (
                "xor %0, %1, %2\n\t"
                : "=r"(s)
                : "r"(t), "r"(a)
                : "cc"
            );
        }
        
        /* More array operations with complex indices */
        for (int x = 0; x < 10; x++) {
            int idx = (a + x) % 200;
            arr[idx][x] = arr[x][idx] * 2;
            
            /* Mix in floating point operations */
            fa = fb * arr[idx][x];
            da = db / (arr[x][idx] + 1);
        }
        
        /* Chain computations using all variable types */
        l = (int)(fa + fb + fc);
        m = (int)(da + db + dc);
        n = (int)((double)la / 100.0);
        
        /* Force spill by using all variables in a complex expression */
        *checksum += a + b + c + d + e + f + g + h + i + j +
                    k + l + m + n + o + p + q + r + s + t +
                    (int)fa + (int)fb + (int)fc + (int)fd + (int)fe +
                    (int)da + (int)db + (int)dc + (int)dd + (int)de +
                    (int)la + (int)lb + (int)lc + (int)ld + (int)le +
                    arr[iter % 200][(iter * 7) % 200];
    }
    
    /* Final computation using all variables to prevent dead code elimination */
    *checksum += a + b + c + d + e + f + g + h + i + j +
                k + l + m + n + o + p + q + r + s + t +
                (int)fa + (int)fb + (int)fc + (int)fd + (int)fe +
                (int)ff + (int)fg + (int)fh + (int)fi + (int)fj +
                (int)da + (int)db + (int)dc + (int)dd + (int)de +
                (int)df + (int)dg + (int)dh + (int)di + (int)dj +
                (int)la + (int)lb + (int)lc + (int)ld + (int)le +
                (int)lf + (int)lg + (int)lh + (int)li + (int)lj;
}

int main(int argc, char *argv[]) {
    int N = (argc > 1) ? atoi(argv[1]) : 100;
    int checksum = 0;
    
    srand(time(NULL));
    
    /* Call the reload-intensive function multiple times */
    for (int i = 0; i < 3; i++) {
        trigger_reloads(N + i, &checksum);
    }
    
    printf("Checksum: %d\n", checksum);
    return 0;
}
