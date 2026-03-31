/* haifa_sched_trigger.c
 * Designed to trigger GCC's Haifa scheduler state save/restore mechanism
 * and exercise the free_sched_context cleanup logic.
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Force specific microarchitecture for x86 to use detailed scheduling model */
#ifdef __x86_64__
__attribute__((target("arch=nehalem")))
#endif
static void complex_scheduling_kernel(int *arr1, int *arr2, int size) {
    /* High register pressure: many local variables */
    int a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p;
    float fa, fb, fc, fd, fe, ff, fg, fh;
    volatile int v1, v2; /* Prevent optimization */
    
    /* Initialize with volatile reads to create hard dependencies */
    v1 = *arr1;
    v2 = *arr2;
    
    a = v1 ^ 0x55AA55AA;
    b = v2 | 0x33CC33CC;
    fa = (float)a * 1.5f;
    fb = (float)b * 0.75f;
    
    /* Main processing loop with data-dependent branches */
    for (int idx = 0; idx < size; idx++) {
        /* Read with memory barrier to create serialization points */
        asm volatile("" ::: "memory");
        int val1 = arr1[idx];
        int val2 = arr2[idx];
        
        /* Hard-to-predict branch using __builtin_expect */
        if (__builtin_expect((val1 & 0xF) > (val2 & 0x7), 0)) {
            /* Path 1: Integer-heavy operations */
            c = val1 * 3;
            d = val2 / 2;
            e = c ^ d;
            f = e << 3;
            g = f - val1;
            h = g | val2;
            i = h & 0xFF;
            j = i * i;
            k = j + c;
            l = k - d;
            m = l ^ e;
            n = m >> 2;
            o = n * 7;
            p = o % 13;
            
            /* Floating point mix */
            fc = (float)c * 1.1f;
            fd = (float)d * 2.2f;
            fe = fc + fd;
            ff = fe * 0.5f;
            fg = ff - fc;
            fh = fg / fd;
            
            /* Memory barrier before merge point */
            asm volatile("" ::: "memory");
        } else {
            /* Path 2: Different operation mix */
            c = val1 + val2;
            d = val1 - val2;
            e = c * d;
            f = e ^ val1;
            g = f | val2;
            h = g << 1;
            i = h >> 2;
            j = i + 5;
            k = j * 3;
            l = k - 7;
            m = l ^ 0xAA;
            n = m & 0x55;
            o = n + 11;
            p = o * 2;
            
            /* Different floating point pattern */
            fc = (float)val1 * 3.14f;
            fd = (float)val2 * 2.71f;
            fe = fc - fd;
            ff = fe * fe;
            fg = ff + 1.0f;
            fh = fg / 2.0f;
            
            /* Memory barrier before merge point */
            asm volatile("" ::: "memory");
        }
        
        /* Common merge point with complex dependency web */
        a = a ^ p;
        b = b + o;
        fa = fa + fh;
        fb = fb - fg;
        
        /* Create cross-iteration dependencies */
        arr1[idx] = a & 0xFF;
        arr2[idx] = b & 0xFF;
        
        /* Occasional unpredictable branch to another label */
        if ((val1 ^ val2) & 0x10) {
            /* Jump to common label creating complex CFG */
            goto common_merge;
        }
        
        /* Continue normal flow */
        a = a * 2;
        b = b / 2;
        continue;
        
    common_merge:
        /* Common merge point from goto */
        a = a ^ b;
        b = a + b;
    }
    
    /* Use all variables to prevent dead code elimination */
    int checksum = a + b + c + d + e + f + g + h + i + j + k + l + m + n + o + p;
    checksum += (int)fa + (int)fb + (int)fc + (int)fd + (int)fe + (int)ff + (int)fg + (int)fh;
    printf("Checksum: %d\n", checksum);
}

/* Function pointer to inhibit optimization */
typedef void (*kernel_func_t)(int*, int*, int);
static volatile kernel_func_t func_ptr = complex_scheduling_kernel;

/* MIPS-specific pattern if cross-compiling */
#ifdef __mips__
__attribute__((noinline))
static void mips_delay_slot_pattern(int *arr, int size) {
    int sum = 0;
    for (int i = 0; i < size; i++) {
        /* Pattern that might trigger delay slot scheduling */
        asm volatile(
            "lw $t0, 0(%1)\n\t"
            "addiu $t1, $t0, 1\n\t"
            "sw $t1, 0(%1)\n\t"
            : : "r"(arr), "r"(&arr[i]) : "t0", "t1", "memory"
        );
        sum += arr[i];
    }
    printf("MIPS sum: %d\n", sum);
}
#endif

int main(void) {
    const int SIZE = 256;
    int *array1 = malloc(SIZE * sizeof(int));
    int *array2 = malloc(SIZE * sizeof(int));
    
    if (!array1 || !array2) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with pseudo-random values */
    unsigned int seed = 42;
    for (int i = 0; i < SIZE; i++) {
        seed = seed * 1103515245 + 12345;
        array1[i] = (int)(seed & 0x7FFF);
        seed = seed * 1103515245 + 12345;
        array2[i] = (int)(seed & 0x7FFF);
    }
    
    /* Call through volatile function pointer */
    func_ptr(array1, array2, SIZE);
    
    /* MIPS-specific call if compiled for MIPS */
#ifdef __mips__
    mips_delay_slot_pattern(array1, SIZE);
#endif
    
    /* Final computation to use results */
    int final_sum = 0;
    for (int i = 0; i < SIZE; i++) {
        final_sum += array1[i] ^ array2[i];
    }
    printf("Final sum: %d\n", final_sum);
    
    free(array1);
    free(array2);
    
    return 0;
}
