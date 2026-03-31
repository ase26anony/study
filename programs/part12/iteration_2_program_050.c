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

/* Volatile variables to prevent optimization */
volatile int global_flag = 1;
volatile int *volatile volatile_ptr;

/* Function with high register pressure and complex operations */
__attribute__((noinline))
int trigger_reloads(int N, int init_val) {
    /* Declare many variables to exceed available registers */
    int a = init_val + 1;
    int b = init_val * 2;
    int c = init_val / 3;
    int d = init_val - 4;
    int e = init_val + 5;
    int f = init_val * 6;
    int g = init_val / 7;
    int h = init_val - 8;
    
    /* Floating point variables - different register class */
    float fa = init_val * 1.1f;
    float fb = init_val * 2.2f;
    float fc = init_val * 3.3f;
    float fd = init_val * 4.4f;
    
    /* Double variables - another register class */
    double da = init_val * 1.11;
    double db = init_val * 2.22;
    double dc = init_val * 3.33;
    double dd = init_val * 4.44;
    
    /* Long variables */
    long la = init_val * 100L;
    long lb = init_val * 200L;
    long lc = init_val * 300L;
    long ld = init_val * 400L;
    
    /* Multi-dimensional array to force address reloads */
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
        packed_arr[i].i = i * 12345;
        packed_arr[i].c = i & 0xFF;
        packed_arr[i].l = i * 987654321L;
        packed_arr[i].f = i * 2.71828f;
    }
    
    /* Complex nested loops with many live variables */
    int sum = 0;
    for (int i = 1; i < N; i++) {
        for (int j = 1; j < N; j++) {
            /* Force register pressure by using all variables */
            a = b + c;
            b = c + d;
            c = d + e;
            d = e + f;
            e = f + g;
            f = g + h;
            g = h + a;
            
            /* Floating point computations */
            fa = fb + fc;
            fb = fc + fd;
            fc = fd + fa;
            
            /* Double computations */
            da = db + dc;
            db = dc + dd;
            dc = dd + da;
            
            /* Long computations */
            la = lb + lc;
            lb = lc + ld;
            lc = ld + la;
            
            /* Array access with complex addressing - forces address reloads */
            int temp = arr[i][j];
            arr[j][i] = arr[i][j] + arr[i-1][j-1];
            arr[i][j] = temp + arr[j][i];
            
            /* Access packed struct through volatile pointer */
            volatile_ptr = &packed_arr[i % 64].i;
            int packed_val = *volatile_ptr;
            
            /* Inline assembly with conflicting constraints */
            /* Force input/output reloads with tied operands */
            asm volatile (
                "addl %1, %0\n\t"
                : "+r"(a)        /* tied output/input */
                : "r"(b)         /* input in register */
                : "cc"
            );
            
            /* Another asm with memory constraint */
            asm volatile (
                "movl %1, %%eax\n\t"
                "addl %%eax, %0\n\t"
                : "+m"(arr[i][j])  /* memory output */
                : "r"(c)           /* register input */
                : "%eax", "cc"
            );
            
            /* Floating point asm */
            asm volatile (
                "addss %1, %0\n\t"
                : "+x"(fa)        /* SSE register constraint */
                : "x"(fb)
            );
            
            /* Conditional block for optional reloads */
            if (global_flag & 1) {
                /* Use different variables in conditional path */
                h = a + packed_val;
                fd = fa * 1.5f;
                dd = da * 1.5;
                ld = la >> 2;
                
                /* Another asm in conditional path */
                asm volatile (
                    "imull %1, %0\n\t"
                    : "+r"(h)
                    : "r"(packed_val)
                    : "cc"
                );
            } else {
                /* Alternative path with different variables */
                a = h + packed_val;
                fa = fd * 0.5f;
                da = dd * 0.5;
                la = ld << 2;
            }
            
            /* More computations to keep variables live */
            sum += a + b + c + d + e + f + g + h;
            sum += (int)fa + (int)fb + (int)fc + (int)fd;
            sum += (int)da + (int)db + (int)dc + (int)dd;
            sum += (int)la + (int)lb + (int)lc + (int)ld;
            sum += arr[i][j] + packed_val;
            
            /* Access packed struct member - may need secondary reload */
            packed_arr[j % 64].d = packed_arr[i % 64].d * 1.01;
            double packed_d = packed_arr[j % 64].d;
            
            /* Use the packed double value */
            sum += (int)packed_d;
        }
        
        /* Change global_flag occasionally */
        if (i % 13 == 0) {
            global_flag ^= 1;
        }
    }
    
    /* Final computation using all variables to prevent elimination */
    int result = sum;
    result += a + b + c + d;
    result += (int)(fa + fb + fc + fd);
    result += (int)(da + db + dc + dd);
    result += (int)(la + lb + lc + ld);
    
    /* One more asm to force final reloads */
    asm volatile (
        "movl %1, %%eax\n\t"
        "addl %%eax, %0\n\t"
        : "+r"(result)
        : "r"(arr[N/2][N/2])
        : "%eax", "cc"
    );
    
    return result;
}

/* Another function to increase compilation complexity */
__attribute__((noinline))
int helper_function(int x, int y) {
    int result = 0;
    for (int i = 0; i < 100; i++) {
        /* More inline asm with constraints */
        asm volatile (
            "leal (%1, %2), %0\n\t"
            : "=r"(result)
            : "r"(x), "r"(y)
        );
        x = result ^ y;
        y = result ^ x;
    }
    return result;
}

int main(int argc, char *argv[]) {
    int N = (argc > 1) ? atoi(argv[1]) : 50;
    int seed = (argc > 2) ? atoi(argv[2]) : time(NULL);
    
    srand(seed);
    int init_val = rand() % 1000;
    
    printf("Starting reload trigger test with N=%d, seed=%d\n", N, seed);
    
    int result1 = trigger_reloads(N, init_val);
    printf("trigger_reloads result: %d\n", result1);
    
    int result2 = helper_function(result1, init_val);
    printf("helper_function result: %d\n", result2);
    
    /* Final checksum */
    int final_result = result1 + result2;
    printf("Final checksum: %d\n", final_result);
    
    return final_result != 0 ? 0 : 1;
}
