#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

/* Packed struct to force unaligned accesses and potential secondary reloads */
struct __attribute__((packed)) Packed {
    double d;
    int i;
    char c;
    long l;
    float f;
};

/* Volatile variables to prevent optimization */
volatile int global_flag = 1;
volatile int *volatile volatile_ptr;

/* Function with high register pressure */
__attribute__((noinline))
int reload_heavy_computation(int N, int seed) {
    /* Declare many variables to exceed available registers */
    int a = seed + 1, b = seed + 2, c = seed + 3, d = seed + 4, e = seed + 5;
    int f = seed + 6, g = seed + 7, h = seed + 8, i = seed + 9, j = seed + 10;
    int k = seed + 11, l = seed + 12, m = seed + 13, n = seed + 14, o = seed + 15;
    int p = seed + 16, q = seed + 17, r = seed + 18, s = seed + 19, t = seed + 20;
    
    /* Mixed types to use different register classes */
    float fa = seed * 1.1f, fb = seed * 1.2f, fc = seed * 1.3f, fd = seed * 1.4f;
    double da = seed * 2.1, db = seed * 2.2, dc = seed * 2.3, dd = seed * 2.4;
    long la = seed * 3L, lb = seed * 4L, lc = seed * 5L, ld = seed * 6L;
    
    /* Multi-dimensional array for address reloads */
    int arr[128][128];
    
    /* Packed struct for unaligned accesses */
    struct Packed packed_arr[64];
    
    /* Initialize arrays with non-constant values */
    for (int x = 0; x < 128; x++) {
        for (int y = 0; y < 128; y++) {
            arr[x][y] = (x * y + seed) % 256;
        }
    }
    
    for (int idx = 0; idx < 64; idx++) {
        packed_arr[idx].d = idx * 0.5 + seed;
        packed_arr[idx].i = idx * 3 + seed;
        packed_arr[idx].c = (idx + seed) % 128;
        packed_arr[idx].l = idx * 7L + seed;
        packed_arr[idx].f = idx * 0.25f + seed;
    }
    
    /* Complex nested loops with many live variables */
    for (int iter = 0; iter < N; iter++) {
        /* Create data dependencies between variables */
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
        
        /* Mixed type computations */
        fa = fb * 1.1f + fc;
        fb = fc * 1.2f + fd;
        da = db * 1.01 + dc;
        db = dc * 1.02 + dd;
        la = lb + lc;
        lb = lc + ld;
        
        /* Inline assembly with conflicting constraints */
        /* Force input/output reloads with tied operands */
        asm volatile (
            "add %0, %1, %2\n\t"
            : "=r"(a) 
            : "r"(b), "0"(c)  /* '0' means same as output operand 0 */
            : /* no clobbers */
        );
        
        /* Another asm with memory constraint */
        asm volatile (
            "mov %0, %1\n\t"
            : "=r"(d)
            : "m"(e)  /* Force memory operand */
            : /* no clobbers */
        );
        
        /* Assembly with specific register constraints (x86) */
        #ifdef __x86_64__
        asm volatile (
            "addl %%eax, %%ebx\n\t"
            : "=b"(g)
            : "a"(f), "0"(h)
            : /* no clobbers */
        );
        #endif
        
        /* Complex array access pattern - forces address reloads */
        int idx1 = (iter * 17) % 128;
        int idx2 = (iter * 23) % 128;
        int idx3 = (iter * 31) % 128;
        
        /* Swap array elements using many temporaries */
        int temp1 = arr[idx1][idx2];
        int temp2 = arr[idx2][idx3];
        int temp3 = arr[idx3][idx1];
        
        arr[idx1][idx2] = arr[idx2][idx1] + temp3;
        arr[idx2][idx3] = arr[idx3][idx2] + temp1;
        arr[idx3][idx1] = arr[idx1][idx3] + temp2;
        
        /* Access packed struct through volatile pointer */
        volatile struct Packed *p_ptr = &packed_arr[iter % 64];
        int packed_val = p_ptr->i;
        double packed_dbl = p_ptr->d;
        
        /* Conditional block for optional reloads */
        if (global_flag & (1 << (iter % 8))) {
            /* Use different subset of variables conditionally */
            m = n + o + packed_val;
            n = o + p + (int)packed_dbl;
            o = p + q + (int)(fa * 10);
            
            /* More assembly in conditional path */
            asm volatile (
                "sub %0, %1, %2\n\t"
                : "=r"(p)
                : "r"(q), "r"(r)
                : /* no clobbers */
            );
        } else {
            /* Alternative path with different variable usage */
            r = s + t + (int)(fb * 20);
            s = t + a + (int)(da);
            t = a + b + (int)(db);
            
            /* Force spill/reload with large expression */
            q = (r * s) + (t * a) + (b * c) + (d * e) + (f * g) + 
                (h * i) + (j * k) + (l * m) + (n * o) + (p * q);
        }
        
        /* Chain computations to keep all variables live */
        la = la + lb + a + b + c;
        lb = lb + lc + d + e + f;
        lc = lc + ld + g + h + i;
        ld = ld + la + j + k + l;
        
        da = da + db + fa + fb;
        db = db + dc + fc + fd;
        dc = dc + dd + da * 0.5;
        dd = dd + da + db * 0.25;
        
        /* Prevent loop invariant code motion */
        fa = fa + iter * 0.01f;
        fb = fb + iter * 0.02f;
        fc = fc + iter * 0.03f;
        fd = fd + iter * 0.04f;
    }
    
    /* Compute checksum using all variables */
    int sum = a + b + c + d + e + f + g + h + i + j + 
              k + l + m + n + o + p + q + r + s + t;
    
    sum += (int)fa + (int)fb + (int)fc + (int)fd;
    sum += (int)da + (int)db + (int)dc + (int)dd;
    sum += (int)la + (int)lb + (int)lc + (int)ld;
    
    /* Add array checksum */
    for (int x = 0; x < 16; x++) {
        for (int y = 0; y < 16; y++) {
            sum += arr[x][y];
        }
    }
    
    /* Add packed struct checksum */
    for (int idx = 0; idx < 16; idx++) {
        sum += packed_arr[idx].i + (int)packed_arr[idx].d + 
               packed_arr[idx].c + (int)packed_arr[idx].l;
    }
    
    return sum;
}

int main(int argc, char *argv[]) {
    int N = (argc > 1) ? atoi(argv[1]) : 50;
    int seed = (argc > 2) ? atoi(argv[2]) : time(NULL);
    
    srand(seed);
    
    /* Call reload-heavy function multiple times */
    int total = 0;
    for (int i = 0; i < 3; i++) {
        total += reload_heavy_computation(N, seed + i);
    }
    
    printf("Result: %d\n", total);
    return 0;
}
