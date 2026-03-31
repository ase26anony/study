/* test-annotated-loops.c */
#include <stdio.h>
#include <stdlib.h>

/* Dummy function to prevent optimization */
extern void use(int val) {
    volatile int sink = val;
    (void)sink;
}

/* Another dummy function with side effects */
static void side_effect(void) {
    static int counter = 0;
    counter++;
}

int main(void) {
    const int N = 100;
    int i, j;
    int result = 0;
    
    /* Arrays for loop computations */
    int a[N], b[N], c[N];
    int partial_sums[4] = {0};
    
    /* Initialize arrays */
    for (i = 0; i < N; i++) {
        a[i] = i + 1;
        b[i] = 0;
        c[i] = (i * 3) % 7;
    }
    
    /* 1. Loop with no-vector annotation (ivdep pragma) */
    /* This pragma tells GCC there are no loop-carried dependencies */
    #pragma GCC ivdep
    for (i = 1; i < N; i++) {
        /* Potential dependency that ivdep says we can ignore */
        a[i] = a[i] + a[i-1] * 2;
        side_effect();
    }
    use(a[N-1]);
    
    /* 2. Loop with vector annotation */
    /* Hint that this loop is a good candidate for vectorization */
    #pragma GCC vector
    for (i = 0; i < N; i++) {
        b[i] = a[i] * 3 + c[i];
        /* Prevent elimination */
        if (b[i] < 0) side_effect();
    }
    use(b[N/2]);
    
    /* 3. Loop with parallel annotation */
    /* Hint that loop iterations are independent and can be parallelized */
    #pragma GCC parallel
    for (i = 0; i < N; i++) {
        /* Independent accumulation into different buckets */
        partial_sums[i % 4] += b[i] * 2;
        /* Call to prevent optimization */
        if (i % 16 == 0) side_effect();
    }
    for (j = 0; j < 4; j++) {
        result += partial_sums[j];
    }
    
    /* 4. Loop with maybe-infinite annotation (unroll pragma without count) */
    /* Suggests the loop should be unrolled, potentially infinitely */
    {
        int iterations = 8; /* Small number to allow unrolling */
        int product = 1;
        
        #pragma GCC unroll
        for (i = 1; i <= iterations; i++) {
            product *= i;
            /* Volatile access to prevent elimination */
            volatile int vol = product;
            (void)vol;
        }
        result += product;
    }
    
    /* 5. Nested loop with annotation on inner loop */
    {
        int matrix[10][10];
        int sum = 0;
        
        /* Initialize */
        for (i = 0; i < 10; i++) {
            for (j = 0; j < 10; j++) {
                matrix[i][j] = i * 10 + j;
            }
        }
        
        /* Annotated inner loop */
        for (i = 0; i < 10; i++) {
            #pragma GCC ivdep
            for (j = 1; j < 10; j++) {
                matrix[i][j] += matrix[i][j-1];
                if (matrix[i][j] % 7 == 0) side_effect();
            }
            sum += matrix[i][9];
        }
        result += sum;
    }
    
    /* 6. Loop with annotation inside conditional */
    {
        int extra[N];
        int use_annotation = 1;
        
        for (i = 0; i < N; i++) {
            extra[i] = i * 2;
        }
        
        if (use_annotation) {
            #pragma GCC vector
            for (i = 0; i < N; i += 2) {
                extra[i] = extra[i] * extra[i];
                if (extra[i] > 1000) side_effect();
            }
        }
        
        for (i = 0; i < N; i++) {
            result += extra[i];
        }
    }
    
    /* 7. While loop with annotation */
    {
        int count = 20;
        int acc = 0;
        
        #pragma GCC unroll
        while (count > 0) {
            acc += count;
            count--;
            /* Prevent optimization */
            volatile int v = acc;
            (void)v;
        }
        result += acc;
    }
    
    printf("Final result: %d\n", result);
    
    /* Verify result is non-zero */
    if (result == 0) {
        fprintf(stderr, "Error: All computations eliminated!\n");
        return 1;
    }
    
    return 0;
}
