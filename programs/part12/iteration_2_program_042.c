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

/* Volatile variables to prevent optimization */
volatile int flag = 0;
volatile int *volatile vptr;

/* Function with high register pressure and complex operations */
__attribute__((noinline))
unsigned long process_data(int N, int init_val) {
    /* Declare many scalar variables to exceed available registers */
    int a = init_val + 1;
    int b = init_val * 2;
    int c = init_val / 3;
    int d = init_val - 4;
    int e = init_val + 5;
    int f = init_val * 6;
    int g = init_val / 7;
    int h = init_val - 8;
    
    float fa = init_val * 1.1f;
    float fb = init_val * 2.2f;
    float fc = init_val * 3.3f;
    float fd = init_val * 4.4f;
    
    double da = init_val * 1.11;
    double db = init_val * 2.22;
    double dc = init_val * 3.33;
    double dd = init_val * 4.44;
    
    long la = init_val * 100L;
    long lb = init_val * 200L;
    long lc = init_val * 300L;
    long ld = init_val * 400L;
    
    /* Multi-dimensional array for address reloads */
    int arr[128][128];
    
    /* Packed struct for unaligned accesses */
    struct Packed packed = {1.0, 2, 'A', 100L, 3.14f};
    struct Packed *volatile pptr = &packed;
    
    /* Initialize array with non-constant values */
    for (int i = 0; i < 128; i++) {
        for (int j = 0; j < 128; j++) {
            arr[i][j] = (i * 31 + j * 17) % 100;
        }
    }
    
    /* Complex nested loops with array accesses */
    for (int i = 1; i < N; i++) {
        for (int j = 1; j < N; j++) {
            /* Force address reloads with complex array indexing */
            int idx1 = (i * 13 + j * 7) % 127;
            int idx2 = (i * 11 + j * 19) % 127;
            
            /* Array operations that require multiple registers */
            int temp = arr[idx1][idx2];
            arr[idx2][idx1] = arr[i % 127][j % 127] + temp;
            arr[i % 127][j % 127] = temp - arr[idx2][idx1];
            
            /* Inline assembly with conflicting constraints */
            /* Force input/output reloads with tied operands */
            asm volatile (
                "addl %1, %0\n\t"
                : "+r"(a)        /* tied output/input */
                : "r"(b)         /* input in register */
                : "cc"
            );
            
            /* Another asm with different constraints */
            asm volatile (
                "imull %1, %0\n\t"
                : "+r"(c)        /* tied operand */
                : "r"(d)         /* register constraint */
                : "cc"
            );
            
            /* Force memory constraint vs register constraint */
            asm volatile (
                "movl %1, %%eax\n\t"
                "addl %%eax, %0\n\t"
                : "+m"(arr[i % 127][j % 127])  /* memory constraint */
                : "r"(e)                       /* register constraint */
                : "eax", "cc"
            );
            
            /* Float/double operations to use FP registers */
            fa = fb + fc;
            fb = fc * fd;
            fc = fd - fa;
            fd = fa / (fb + 1.0f);
            
            da = db * dc;
            db = dc + dd;
            dc = dd - da;
            dd = da / (db + 1.0);
            
            /* Long operations */
            la = lb + lc;
            lb = lc - ld;
            lc = ld * la;
            ld = la / (lb + 1);
            
            /* Access packed struct through volatile pointer */
            /* This may require secondary reloads due to alignment */
            pptr->i = a + b;
            pptr->d = da + db;
            pptr->f = fa + fb;
            pptr->l = la + lb;
            
            /* Conditional block for optional reloads */
            if (flag) {
                /* Use different variables only in this path */
                asm volatile (
                    "subl %1, %0\n\t"
                    : "+r"(g)
                    : "r"(h)
                    : "cc"
                );
                
                /* Complex computation in conditional path */
                e = f * g + h;
                f = g - h * e;
                g = h + f / (e + 1);
                h = e * f - g;
                
                /* More float operations */
                float ftemp = fa * fb - fc / fd;
                fa = fb + ftemp;
                fb = fc * ftemp;
            } else {
                /* Different computation in else path */
                asm volatile (
                    "xorl %1, %0\n\t"
                    : "+r"(a)
                    : "r"(b)
                    : "cc"
                );
                
                c = d ^ e;
                d = e | f;
                e = f & g;
                f = g ^ h;
            }
            
            /* Chain computations to keep variables live */
            a = b + c;
            b = c * d;
            c = d - e;
            d = e + f;
            e = f * g;
            f = g - h;
            g = h + a;
            h = a * b;
            
            /* Mix types to force moves between register classes */
            fa = (float)a + fb;
            da = (double)b + db;
            la = (long)c + lb;
        }
        
        /* Update flag occasionally */
        if (i % 37 == 0) {
            flag = !flag;
        }
    }
    
    /* Compute checksum using all variables to prevent elimination */
    unsigned long checksum = 0;
    checksum += a + b + c + d + e + f + g + h;
    checksum += (unsigned long)(fa + fb + fc + fd);
    checksum += (unsigned long)(da + db + dc + dd);
    checksum += la + lb + lc + ld;
    
    /* Include array in checksum */
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            checksum += arr[i][j];
        }
    }
    
    /* Include packed struct */
    checksum += pptr->i + (unsigned long)pptr->d + (unsigned long)pptr->f + pptr->l;
    
    return checksum;
}

int main(int argc, char *argv[]) {
    /* Use command line argument for loop bound */
    int N = (argc > 1) ? atoi(argv[1]) : 50;
    
    /* Seed RNG for variable initialization */
    srand(time(NULL));
    
    /* Call processing function multiple times with different inputs */
    unsigned long total = 0;
    for (int i = 0; i < 3; i++) {
        int init = rand() % 100;
        total += process_data(N + i * 10, init);
    }
    
    printf("Checksum: %lu\n", total);
    
    return 0;
}
