/* fixed-point-test.c */
/* Compile with: gcc -O1 -std=gnu11 -Wno-psabi fixed-point-test.c -o fixed-point-test */

#include <stdio.h>
#include <stdint.h>

/* Opaque functions to prevent constant propagation */
__attribute__((noinline)) signed short _Fract get_sfract_max(void) {
    return 0.999969482421875r;  /* MAX for signed short _Fract (Q0.15) */
}

__attribute__((noinline)) unsigned short _Fract get_ufract_max(void) {
    return 0.999969482421875ur; /* MAX for unsigned short _Fract (U0.16) */
}

__attribute__((noinline)) signed long _Accum get_saccum_max(void) {
    return 32767.999969482421875lk; /* MAX for signed long _Accum (Q15.16) */
}

__attribute__((noinline)) signed long _Accum get_saccum_min(void) {
    return -32768.0lk; /* MIN for signed long _Accum */
}

/* Function to consume values and prevent dead code elimination */
volatile int sink;
__attribute__((noinline)) void use_value(int val) {
    sink = val;
}

int main(void) {
    int checksum = 0;
    
    /* Test 1: Conversion from wider to narrower type with overflow */
    printf("Test 1: Wide to narrow conversions\n");
    {
        /* These should trigger range checks during constant folding */
        const signed long _Accum wide_max = get_saccum_max();
        const signed long _Accum wide_min = get_saccum_min();
        
        /* Conversion that should require range checking */
        volatile signed short _Fract narrow1 = (signed short _Fract)wide_max;
        volatile signed short _Fract narrow2 = (signed short _Fract)wide_min;
        
        /* Complex constant expression forcing range check */
        const signed long _Accum c1 = (signed long _Accum)0.5lk * 3.1415926535lk;
        const signed short _Fract c2 = (signed short _Fract)c1;
        
        use_value((int)(narrow1 * 1000));
        use_value((int)(narrow2 * 1000));
        checksum += (int)(c2 * 1000);
    }
    
    /* Test 2: Boundary value testing for signed types */
    printf("Test 2: Signed boundary testing\n");
    {
        /* Values at and beyond MAX for signed short _Fract */
        const signed short _Fract sf_max = get_sfract_max();
        const signed short _Fract sf_near_max = 0.999r;  /* Just below max */
        
        /* Operations that might overflow */
        volatile signed short _Fract r1 = sf_max;
        volatile signed short _Fract r2 = sf_near_max * 1.1r;  /* Should overflow */
        
        /* Test with _Sat qualifier */
        volatile signed short _Sat _Fract sat1 = sf_max;
        volatile signed short _Sat _Fract sat2 = sf_near_max * 1.1r;
        
        /* Conversion that requires range checking */
        volatile signed char _Fract cf1 = (signed char _Fract)sf_max;
        
        checksum += (int)(r1 * 1000);
        checksum += (int)(r2 * 1000);
        checksum += (int)(sat1 * 1000);
        checksum += (int)(sat2 * 1000);
    }
    
    /* Test 3: Unsigned type overflow testing */
    printf("Test 3: Unsigned overflow testing\n");
    {
        const unsigned short _Fract uf_max = get_ufract_max();
        const unsigned short _Fract uf_half = 0.5ur;
        
        /* Operations near unsigned maximum */
        volatile unsigned short _Fract u1 = uf_max;
        volatile unsigned short _Fract u2 = uf_max + 0.0001ur;  /* Should overflow */
        
        /* Mixed signed/unsigned conversion */
        volatile signed short _Fract s1 = -0.5r;
        volatile unsigned short _Fract u3 = (unsigned short _Fract)s1;  /* Should underflow */
        
        /* Unsigned saturated arithmetic */
        volatile unsigned short _Sat _Fract usat1 = uf_max;
        volatile unsigned short _Sat _Fract usat2 = uf_max + 0.0001ur;
        
        checksum += (int)(u1 * 1000);
        checksum += (int)(u2 * 1000);
        checksum += (int)(u3 * 1000);
    }
    
    /* Test 4: Complex constant expressions with different precisions */
    printf("Test 4: Complex constant expressions\n");
    {
        /* Multi-step constant expression forcing range checks */
        const signed long _Accum a = 10000.123lk;
        const signed long _Accum b = 20000.456lk;
        const signed long _Accum c = a * b / 1000.0lk;
        
        /* Conversion to narrower type */
        const signed short _Accum d = (signed short _Accum)c;
        const signed short _Fract e = (signed short _Fract)d;
        
        /* Another complex expression */
        const signed _Fract f = 0.7r;
        const signed _Fract g = 0.8r;
        const signed _Fract h = f * g * 2.0r;
        
        /* Force materialization */
        volatile signed short _Fract v1 = e;
        volatile signed _Fract v2 = h;
        
        checksum += (int)(v1 * 1000);
        checksum += (int)(v2 * 1000);
    }
    
    /* Test 5: Loop with fixed-point computations */
    printf("Test 5: Loop-based computations\n");
    {
        signed short _Fract accum = 0.0r;
        const signed short _Fract step = 0.1r;
        
        /* Small loop that still allows some constant folding */
        for (int i = 0; i < 5; i++) {
            accum += step;
            
            /* Conversion inside loop */
            volatile signed char _Fract narrow = (signed char _Fract)accum;
            
            /* Multiplication that could overflow */
            volatile signed short _Fract scaled = accum * 1.5r;
            
            checksum += (int)(narrow * 100);
            checksum += (int)(scaled * 100);
        }
        
        /* Final conversion that might overflow */
        volatile signed char _Fract final_narrow = (signed char _Fract)(accum * 10.0r);
        checksum += (int)(final_narrow * 100);
    }
    
    /* Test 6: Mixed precision arithmetic */
    printf("Test 6: Mixed precision\n");
    {
        /* Mix different fixed-point types */
        const signed long _Accum la = 15000.5lk;
        const signed short _Accum sa = 200.5k;
        const signed short _Fract sf = 0.9r;
        
        /* Operations mixing types */
        volatile signed long _Accum r1 = la + (signed long _Accum)sa;
        volatile signed short _Accum r2 = (signed short _Accum)la + sa;
        volatile signed short _Fract r3 = (signed short _Fract)la + sf;
        
        /* Chain of conversions */
        volatile signed char _Fract cf = (signed char _Fract)((signed short _Fract)la);
        
        checksum += (int)(r1 / 100);
        checksum += (int)(r2 * 10);
        checksum += (int)(r3 * 1000);
        checksum += (int)(cf * 1000);
    }
    
    printf("Final checksum: %d\n", checksum);
    return checksum != 0 ? 0 : 1;
}
