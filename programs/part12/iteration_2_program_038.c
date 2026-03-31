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
volatile int volatile_flag = 0;

/* Target function with high register pressure */
__attribute__((noinline))
static long process_data(int N, int init_val) {
    /* Declare many variables to exceed available registers */
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
    struct Packed packed_arr[64];
    
    /* Initialize arrays with non-constant values */
    for (int i = 0; i < 128; i++) {
        for (int j = 0; j < 128; j++) {
            arr[i][j] = (i * 7919 + j * 65537) & 0xFF;
        }
    }
    
    for (int i = 0; i < 64; i++) {
        packed_arr[i].d = i * 3.14159;
        packed_arr[i].i = i * 17;
        packed_arr[i].c = i & 0xFF;
        packed_arr[i].l = i * 1000L;
        packed_arr[i].f = i * 2.71828f;
    }
    
    /* Main computation loop - creates register pressure */
    long total = 0;
    for (int i = 1; i < N; i++) {
        for (int j = 1; j < N; j++) {
            /* Complex array access pattern - forces address reloads */
            int temp = arr[i][j] + arr[j][i];
            
            /* Chain computations to keep variables live */
            a = b + c;
            b = c * d;
            c = d - e;
            d = e + f;
            e = f / (g ? g : 1);
            f = g * h;
            g = h + a;
            h = a * b;
            
            /* Floating point computations */
            fa = fb + fc;
            fb = fc * fd;
            fc = fd - fa;
            fd = fa * 1.5f;
            
            da = db + dc;
            db = dc * dd;
            dc = dd - da;
            dd = da * 1.5;
            
            la = lb + lc;
            lb = lc * ld;
            lc = ld - la;
            ld = la * 2L;
            
            /* Inline assembly with conflicting constraints */
            /* Forces specific register allocation */
            asm volatile (
                "addl %1, %0\n\t"
                : "+r"(a)        /* read-write operand */
                : "r"(b)         /* input operand */
                : "cc"
            );
            
            /* Another asm with tied operand */
            asm volatile (
                "imull %1, %0\n\t"
                : "+r"(c)        /* tied to output */
                : "r"(d)
                : "cc"
            );
            
            /* Floating point asm */
            asm volatile (
                "addss %1, %0\n\t"
                : "+x"(fa)       /* SSE register constraint */
                : "x"(fb)
            );
            
            /* Memory constraint asm */
            asm volatile (
                "movl %1, %%eax\n\t"
                "addl %%eax, %0\n\t"
                : "+m"(arr[i][j])  /* memory operand */
                : "r"(temp)        /* register operand */
                : "%eax", "cc"
            );
            
            /* Access packed struct through volatile pointer */
            volatile struct Packed *p = &packed_arr[(i * j) % 64];
            
            /* This may require secondary reloads due to alignment */
            double packed_d = p->d;
            int packed_i = p->i;
            
            /* Use packed data in computation */
            da += packed_d;
            a += packed_i;
            
            /* Conditional block for optional reloads */
            if (volatile_flag) {
                /* Different computation path */
                a = b - c;
                fa = fb - fc;
                da = db - dc;
                la = lb - lc;
                
                /* More asm in conditional path */
                asm volatile (
                    "subl %1, %0\n\t"
                    : "+r"(a)
                    : "r"(b)
                    : "cc"
                );
            } else {
                /* Original path continues */
                arr[i][j] = a + temp;
                
                /* Another asm with specific register constraints */
                register int r1 asm("ebx") = a;
                register int r2 asm("ecx") = b;
                asm volatile (
                    "xchgl %%ebx, %%ecx\n\t"
                    : "+r"(r1), "+r"(r2)
                    :
                    : "cc"
                );
                a = r1;
                b = r2;
            }
            
            /* Update array with swapped indices */
            arr[j][i] = arr[i][j] + 1;
            
            /* Accumulate total */
            total += a + b + c + d + arr[i][j];
            total += (long)(fa + fb + fc + fd);
            total += (long)(da + db + dc + dd);
            total += la + lb + lc + ld;
        }
        
        /* Cross-iteration dependencies */
        if (i % 2 == 0) {
            /* Force spills by using all variables */
            a = b + c + d + e + f + g + h;
            fa = fb + fc + fd;
            da = db + dc + dd;
            la = lb + lc + ld;
        }
    }
    
    /* Final computation using all variables */
    long result = total + a + b + c + d + e + f + g + h;
    result += (long)(fa + fb + fc + fd);
    result += (long)(da + db + dc + dd);
    result += la + lb + lc + ld;
    
    /* Add array checksum */
    for (int i = 0; i < 16; i++) {
        for (int j = 0; j < 16; j++) {
            result += arr[i][j];
        }
    }
    
    return result;
}

/* Another function to increase compilation unit complexity */
__attribute__((noinline))
static void helper_function(int *ptr, double *dptr, long count) {
    for (long i = 0; i < count; i++) {
        /* Complex addressing modes */
        ptr[i * 2] = ptr[i * 3] + i;
        dptr[i] = dptr[i] * 1.01;
        
        /* Inline asm with memory constraints */
        asm volatile (
            "movq (%1), %%rax\n\t"
            "addq %%rax, %0\n\t"
            : "+m"(*ptr)
            : "r"(dptr)
            : "%rax", "cc"
        );
    }
}

int main(int argc, char *argv[]) {
    /* Use command line argument for loop bound */
    int N = (argc > 1) ? atoi(argv[1]) : 50;
    if (N < 10) N = 10;
    if (N > 100) N = 100;  /* Limit to avoid excessive runtime */
    
    /* Seed RNG for variable initialization */
    srand(time(NULL));
    int init_val = rand() % 1000 + 1;
    
    /* Set volatile flag based on input */
    volatile_flag = (argc > 2) ? atoi(argv[2]) : 0;
    
    printf("Starting computation with N=%d, init=%d\n", N, init_val);
    
    /* Call target function multiple times */
    long total_result = 0;
    for (int iter = 0; iter < 3; iter++) {
        long result = process_data(N + iter, init_val + iter);
        total_result += result;
        printf("Iteration %d: result = %ld\n", iter, result);
    }
    
    /* Additional memory operations */
    int *buffer = malloc(N * N * sizeof(int));
    double *dbuffer = malloc(N * sizeof(double));
    
    if (buffer && dbuffer) {
        for (int i = 0; i < N * N; i++) {
            buffer[i] = i * 3;
        }
        for (int i = 0; i < N; i++) {
            dbuffer[i] = i * 1.5;
        }
        
        helper_function(buffer, dbuffer, N);
        
        /* Use results */
        for (int i = 0; i < N; i++) {
            total_result += buffer[i] + (long)dbuffer[i];
        }
    }
    
    free(buffer);
    free(dbuffer);
    
    printf("Final checksum: %ld\n", total_result);
    return (int)(total_result & 0x7FFFFFFF);
}
