/* reload_test.c - Program to force GCC reload pass to initialize all reload struct fields */
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

/* Volatile variable to prevent optimization of conditional paths */
volatile int volatile_flag = 0;

/* Target function that creates maximum register pressure */
__attribute__((noinline)) 
unsigned long long create_reloads(int N, int init_val) {
    /* Declare many scalar variables of mixed types */
    int a = init_val + 1;
    int b = init_val + 2;
    int c = init_val + 3;
    int d = init_val + 4;
    int e = init_val + 5;
    int f = init_val + 6;
    int g = init_val + 7;
    int h = init_val + 8;
    
    float fa = init_val * 1.1f;
    float fb = init_val * 1.2f;
    float fc = init_val * 1.3f;
    float fd = init_val * 1.4f;
    
    double da = init_val * 2.1;
    double db = init_val * 2.2;
    double dc = init_val * 2.3;
    double dd = init_val * 2.4;
    
    long la = init_val * 3L;
    long lb = init_val * 4L;
    long lc = init_val * 5L;
    long ld = init_val * 6L;
    
    /* Multi-dimensional array for address reloads */
    int arr[128][128];
    
    /* Packed struct for unaligned accesses */
    struct Packed packed;
    packed.d = da;
    packed.i = a;
    packed.c = (char)init_val;
    packed.l = la;
    packed.f = fa;
    
    /* Volatile pointer to packed struct */
    volatile struct Packed *volatile_packed = &packed;
    
    /* Initialize array with non-constant values */
    for (int i = 0; i < 128; i++) {
        for (int j = 0; j < 128; j++) {
            arr[i][j] = (i * 131 + j * 17) % 1000;
        }
    }
    
    /* Main computation loop - creates register pressure and dependencies */
    for (int i = 1; i < N - 1; i++) {
        for (int j = 1; j < N - 1 && j < 127; j++) {
            /* Complex array access pattern requiring address reloads */
            int temp1 = arr[i][j];
            int temp2 = arr[j][i];
            int temp3 = arr[i-1][j];
            int temp4 = arr[i][j-1];
            
            /* Chain of integer computations keeping many variables live */
            a = b + c + temp1;
            b = c + d + temp2;
            c = d + e + temp3;
            d = e + f + temp4;
            e = f + g + a;
            f = g + h + b;
            g = h + a + c;
            h = a + b + d;
            
            /* Floating point computations */
            fa = fb + fc + (float)temp1 * 0.1f;
            fb = fc + fd + (float)temp2 * 0.2f;
            fc = fd + fa + (float)temp3 * 0.3f;
            fd = fa + fb + (float)temp4 * 0.4f;
            
            /* Double computations */
            da = db + dc + (double)temp1 * 0.01;
            db = dc + dd + (double)temp2 * 0.02;
            dc = dd + da + (double)temp3 * 0.03;
            dd = da + db + (double)temp4 * 0.04;
            
            /* Long computations */
            la = lb + lc + (long)temp1 * 10L;
            lb = lc + ld + (long)temp2 * 20L;
            lc = ld + la + (long)temp3 * 30L;
            ld = la + lb + (long)temp4 * 40L;
            
            /* Inline assembly with conflicting constraints to force reloads */
            /* Tied operand constraint (output tied to input 0) */
            asm volatile (
                "add %0, %1, %2\n\t"
                : "=r"(a)
                : "r"(b), "0"(c)
                : "cc"
            );
            
            /* Memory constraint forcing spill/reload */
            asm volatile (
                "imul %0, %1\n\t"
                : "+r"(d)
                : "rm"(e)
                : "cc"
            );
            
            /* Specific register constraints for x86 */
            #ifdef __x86_64__
            asm volatile (
                "addl %%eax, %%ebx\n\t"
                : "+b"(f)
                : "a"(g)
                : "cc"
            );
            #endif
            
            /* Access packed struct through volatile pointer - may need secondary reload */
            volatile_packed->i = a;
            volatile_packed->d = da;
            
            /* Conditional block for optional reloads */
            if (volatile_flag) {
                /* Use different subset of variables inside conditional */
                int cond_temp = a + b + volatile_packed->i;
                float cond_float = fa + fb + volatile_packed->f;
                arr[i][j] = cond_temp + (int)cond_float;
            } else {
                /* Different computation path */
                arr[i][j] = (a * b + c * d) % 1000;
            }
            
            /* More assembly with constraints */
            asm volatile (
                "sub %1, %0\n\t"
                "add %2, %0\n\t"
                : "+r"(h)
                : "r"(a), "r"(b)
                : "cc"
            );
            
            /* Swap array elements - more address computations */
            int swap_temp = arr[i][j];
            arr[i][j] = arr[j][i];
            arr[j][i] = swap_temp;
        }
        
        /* Update volatile flag occasionally */
        if (i % 100 == 0) {
            volatile_flag = (volatile_flag + 1) % 2;
        }
    }
    
    /* Compute checksum to prevent optimization */
    unsigned long long checksum = 0;
    checksum += a + b + c + d + e + f + g + h;
    checksum += (unsigned long long)(fa + fb + fc + fd);
    checksum += (unsigned long long)(da + db + dc + dd);
    checksum += la + lb + lc + ld;
    
    /* Add array elements to checksum */
    for (int i = 0; i < 16; i++) {
        for (int j = 0; j < 16; j++) {
            checksum += arr[i][j];
        }
    }
    
    checksum += volatile_packed->i + (unsigned long long)volatile_packed->d;
    
    return checksum;
}

/* Another function to create cross-function register pressure */
__attribute__((noinline))
void additional_pressure(int N, int *result) {
    double d1 = 1.0, d2 = 2.0, d3 = 3.0, d4 = 4.0, d5 = 5.0;
    float f1 = 1.0f, f2 = 2.0f, f3 = 3.0f, f4 = 4.0f, f5 = 5.0f;
    long l1 = 1, l2 = 2, l3 = 3, l4 = 4, l5 = 5;
    
    for (int i = 0; i < N; i++) {
        /* Chain of floating point operations */
        d1 = d2 * d3 + d4;
        d2 = d3 * d4 + d5;
        d3 = d4 * d5 + d1;
        d4 = d5 * d1 + d2;
        d5 = d1 * d2 + d3;
        
        f1 = f2 * f3 + f4;
        f2 = f3 * f4 + f5;
        f3 = f4 * f5 + f1;
        f4 = f5 * f1 + f2;
        f5 = f1 * f2 + f3;
        
        l1 = l2 + l3 * l4;
        l2 = l3 + l4 * l5;
        l3 = l4 + l5 * l1;
        l4 = l5 + l1 * l2;
        l5 = l1 + l2 * l3;
        
        /* Assembly with memory constraints */
        asm volatile (
            "mov %1, %0\n\t"
            "add $1, %0\n\t"
            : "=r"(l1)
            : "m"(l2)
            : "cc"
        );
    }
    
    *result = (int)(d1 + d2 + f1 + f2 + l1 + l2);
}

int main(int argc, char *argv[]) {
    int N = (argc > 1) ? atoi(argv[1]) : 500;
    
    /* Seed RNG for variable initialization */
    srand(time(NULL));
    int init_val = rand() % 1000;
    
    printf("Starting reload test with N=%d, init=%d\n", N, init_val);
    
    /* Call function that creates reload pressure */
    unsigned long long checksum1 = create_reloads(N, init_val);
    
    /* Create additional pressure in another function */
    int result;
    additional_pressure(N / 2, &result);
    
    /* Use result to affect final checksum */
    checksum1 += result;
    
    printf("Checksum: %llu\n", checksum1);
    
    /* Return non-zero if checksum is "interesting" */
    return (checksum1 % 1000) == 123 ? 1 : 0;
}
