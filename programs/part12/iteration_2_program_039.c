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

/* Function with high register pressure and complex operations */
void reload_inducing_function(int N, int *checksum) {
    /* Declare many scalar variables of mixed types */
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
    
    long la = rand() * 1000L;
    long lb = rand() * 1000L;
    long lc = rand() * 1000L;
    long ld = rand() * 1000L;
    
    /* Multi-dimensional array for address reloads */
    int arr[200][200];
    for (int x = 0; x < 200; x++) {
        for (int y = 0; y < 200; y++) {
            arr[x][y] = (x * y) % 100;
        }
    }
    
    /* Packed struct with volatile pointer */
    struct Packed packed;
    packed.d = da;
    packed.i = a;
    packed.f = fa;
    packed.l = la;
    packed.c = 'A';
    
    volatile struct Packed *volatile_packed = &packed;
    
    /* Complex nested loops with array accesses */
    for (int iter = 0; iter < N; iter++) {
        /* Create data dependencies between all variables */
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
        
        /* Floating point computations */
        fa = fb + fc;
        fb = fc + fd;
        fc = fd + fa;
        fd = fa + fb;
        
        /* Double computations */
        da = db + dc;
        db = dc + dd;
        dc = dd + da;
        dd = da + db;
        
        /* Long computations */
        la = lb + lc;
        lb = lc + ld;
        lc = ld + la;
        ld = la + lb;
        
        /* Inline assembly with conflicting constraints */
        /* Force input reloads with "r" constraints */
        asm volatile (
            "add %0, %1, %2\n\t"
            : "=r" (a)
            : "r" (b), "0" (c)
            : "cc"
        );
        
        /* Force output reload with memory constraint */
        asm volatile (
            "mov %0, %1\n\t"
            : "=m" (arr[iter % 200][0])
            : "r" (d)
        );
        
        /* Assembly with tied operand (forces reload) */
        int temp = e;
        asm volatile (
            "add %0, %1\n\t"
            : "+r" (temp)
            : "r" (f)
            : "cc"
        );
        e = temp;
        
        /* Complex array addressing - forces address reloads */
        int idx1 = iter % 199;
        int idx2 = (iter * 7) % 199;
        arr[idx1][idx2] = arr[idx2][idx1] + a;
        
        /* Access packed struct through volatile pointer - may need secondary reload */
        volatile_packed->i = arr[idx1][idx2];
        volatile_packed->d = da + db;
        
        /* Conditional block for optional reloads */
        if (global_flag) {
            /* Use different subset of variables */
            int t1 = a + b + c;
            int t2 = d + e + f;
            float t3 = fa + fb;
            double t4 = da + db;
            
            /* More inline assembly in conditional path */
            asm volatile (
                "imul %0, %1, %2\n\t"
                : "=r" (t1)
                : "r" (t2), "r" (iter)
                : "cc"
            );
            
            /* Update array with conditional results */
            arr[(iter + 1) % 200][(iter + 2) % 200] = t1 + t2;
            
            /* Mix types in computation */
            da = t4 + (double)t3;
        } else {
            /* Alternative path using other variables */
            long t5 = la + lb;
            float t6 = fc + fd;
            
            asm volatile (
                "sub %0, %1, %2\n\t"
                : "=r" (t5)
                : "r" (lc), "r" (ld)
                : "cc"
            );
            
            arr[(iter + 3) % 200][(iter + 4) % 200] = (int)(t5 % 1000);
            fb = t6 * 2.0f;
        }
        
        /* More computations to keep all variables live */
        k = l + a;
        l = a + b;
        
        /* Force spill/reload by using all variables in a big expression */
        *checksum += a + b + c + d + e + f + g + h + i + j + k + l
                   + (int)fa + (int)fb + (int)fc + (int)fd
                   + (int)da + (int)db + (int)dc + (int)dd
                   + (int)la + (int)lb + (int)lc + (int)ld
                   + arr[iter % 200][(iter + 1) % 200];
    }
    
    /* Final computations using all variables */
    *checksum += a + b + c + d + e + f + g + h + i + j + k + l;
    *checksum += (int)(fa * 100) + (int)(fb * 100) + (int)(fc * 100) + (int)(fd * 100);
    *checksum += (int)(da * 100) + (int)(db * 100) + (int)(dc * 100) + (int)(dd * 100);
    *checksum += (int)(la % 1000) + (int)(lb % 1000) + (int)(lc % 1000) + (int)(ld % 1000);
}

int main(int argc, char *argv[]) {
    srand(time(NULL));
    
    /* Non-constant loop bound from command line */
    int N = (argc > 1) ? atoi(argv[1]) : 100;
    if (N < 10) N = 10;
    if (N > 1000) N = 1000;
    
    int checksum = 0;
    
    /* Call the function multiple times to increase pressure */
    for (int repeat = 0; repeat < 3; repeat++) {
        reload_inducing_function(N, &checksum);
        
        /* Toggle global flag to exercise both conditional paths */
        global_flag = !global_flag;
    }
    
    printf("Checksum: %d\n", checksum);
    return 0;
}
