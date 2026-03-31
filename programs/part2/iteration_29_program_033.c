/* test_hw_loops.c - Target hardware loop nesting analysis test */

/* Prevent unwanted optimizations */
#define NO_INLINE __attribute__((noinline, noipa))
#define VOLATILE_DO(x) do { volatile int _sink; _sink = (x); } while(0)

/* Target-specific hardware loop support */
#ifdef __riscv__
  /* RISC-V with Ziloop extension */
  #define TARGET_SUPPORTS_HW_LOOPS 1
#elif defined(__ARC__)
  /* ARC with loop extension */
  #define TARGET_SUPPORTS_HW_LOOPS 1
#elif defined(__XTENSA__) || defined(__MSP430__) || defined(__RL78__)
  /* Other targets known to support hardware loops */
  #define TARGET_SUPPORTS_HW_LOOPS 1
#else
  /* Generic fallback - may still work with appropriate flags */
  #define TARGET_SUPPORTS_HW_LOOPS 0
#endif

/* Global array to prevent dead code elimination */
volatile int results[32];

/* Main test function with carefully structured loops */
NO_INLINE
void test_nested_loops(void) {
    volatile int counter = 0;
    
    /* 
     * Three perfectly nested loops - creates clear parent-child relationships
     * Outer loop (i) contains middle loop (j) contains inner loop (k)
     */
    for (int i = 0; i < 10; ++i) {           /* Loop A - outermost */
        for (int j = 0; j < 8; ++j) {        /* Loop B - middle */
            for (int k = 0; k < 6; ++k) {    /* Loop C - innermost */
                /* Simple body to create basic blocks */
                VOLATILE_DO(i * j + k);
                results[counter++ % 32] = i + j + k;
            }
        }
        
        /*
         * Partially overlapping loop - shares blocks with Loop A but not perfectly nested
         * This loop starts inside Loop A's body but continues after Loop B ends,
         * creating partial overlap that triggers bitmap_intersect_p but not subset checks
         */
        for (int m = 0; m < 5; ++m) {        /* Loop D - partially overlaps with A */
            /* Different computation to create distinct basic blocks */
            VOLATILE_DO(i * m - 1);
            results[counter++ % 32] = i - m;
            
            /* Small conditional to create additional basic blocks */
            if (m % 2 == 0) {
                VOLATILE_DO(m * 2);
            }
        }
    }
    
    /* 
     * Separate loop that doesn't intersect with the nested structure above
     * This ensures we have multiple loops to analyze
     */
    for (int x = 0; x < 7; ++x) {            /* Loop E - separate, non-intersecting */
        VOLATILE_DO(x * x);
        results[counter++ % 32] = x;
        
        /* Another inner loop to create more nesting possibilities */
        for (int y = 0; y < 3; ++y) {        /* Loop F - child of E */
            VOLATILE_DO(x + y);
            results[counter++ % 32] = x * y;
        }
    }
    
    /* Final isolated loop with different structure */
    int n = 0;
    while (n < 4) {                          /* Loop G - while loop variant */
        VOLATILE_DO(n * 10);
        results[counter++ % 32] = n;
        n++;
    }
}

/* Main function */
int main(void) {
    /* Initialize results array */
    for (int i = 0; i < 32; ++i) {
        results[i] = 0;
    }
    
    /* Call the test function */
    test_nested_loops();
    
    /* Use results to prevent dead code elimination */
    volatile int sum = 0;
    for (int i = 0; i < 32; ++i) {
        sum += results[i];
    }
    
    /* Print to create observable side effect */
#ifdef TARGET_SUPPORTS_HW_LOOPS
    /* For embedded targets, we might not have stdio */
    asm volatile ("" : : "r"(sum));
#else
    printf("Result checksum: %d\n", sum);
#endif
    
    return 0;
}
