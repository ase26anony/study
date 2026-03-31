/* auto-inc-dec-test.c */
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

#pragma GCC optimize("O3")
#pragma GCC push_options

/* Heterogeneous structure with mixed alignment */
struct S {
    char c;
    short s;
    int a;
    float b;
    double d;
    long l;
} __attribute__((packed));

/* Helper function marked noinline to create separate optimization context */
__attribute__((noinline, optimize("O3")))
static double process_subset(int* base, float* fbase, struct S* sbase, int n) {
    double sum = 0.0;
    int* restrict p1 = base;
    float* restrict p2 = fbase;
    struct S* restrict p3 = sbase;
    
    /* Pattern 1: Multiple base registers with zero offset */
    for (int i = 0; i < n; i++) {
        sum += *p1;      /* [reg + 0] addressing */
        sum += *p2;      /* Another [reg + 0] */
        sum += p3->a;    /* Structure field: base + 0 offset */
        p1++;
        p2++;
        p3++;
    }
    
    /* Insert compiler barrier */
    asm volatile("" ::: "memory");
    
    return sum;
}

/* Function with attribute to force specific optimization level */
__attribute__((optimize("O3"), noinline))
static void mixed_access_patterns(int* arr_i, float* arr_f, 
                                  struct S* arr_s, int size) {
    double total = 0.0;
    
    /* Pattern 2: Post-increment in loop condition */
    register int* p = arr_i;
    register float* q = arr_f;
    int count = size;
    
    while (count-- > 0) {
        total += *p++;  /* Post-increment access */
        total += *q++;  /* Another post-increment */
        
        /* Function call to obscure aliasing */
        if (count % 7 == 0) {
            total += sin(total * 0.01);
        }
    }
    
    /* Pattern 3: Compound expressions with index modification */
    int index = size - 1;
    for (int i = 0; i < size / 2; i++) {
        total += arr_i[index--];    /* Post-decrement in array access */
        total += arr_f[--index];    /* Pre-decrement in array access */
    }
    
    /* Pattern 4: Nested loops with pointer reset */
    for (int outer = 0; outer < 3; outer++) {
        struct S* ptr = arr_s;
        for (int inner = 0; inner < size / 3; inner++) {
            /* Mixed structure field accesses with different offsets */
            total += ptr->a;    /* offset 0 */
            total += ptr->b;    /* non-zero offset */
            total += ptr->d;    /* larger offset */
            ptr++;              /* Pointer increment */
        }
        
        /* Compiler barrier between loop iterations */
        asm volatile("" ::: "memory");
    }
    
    /* Pattern 5: Mixed pointer and index arithmetic */
    for (int i = 0; i < size; i++) {
        /* Base + index arithmetic */
        total += *(arr_i + i);      /* [base_reg + index_reg] */
        total += *(arr_f + i);      /* Different base register */
        
        /* Force zero-offset access through pointer alias */
        int* alias = arr_i + i;
        total += *alias;            /* Should become [reg + 0] */
    }
    
    printf("Subtotal: %f\n", total);
}

int main(void) {
    const int INT_SIZE = 100;
    const int FLOAT_SIZE = 100;
    const int STRUCT_SIZE = 50;
    
    /* Declare arrays with different types and alignments */
    int arr_i[INT_SIZE] __attribute__((aligned(16)));
    float arr_f[FLOAT_SIZE] __attribute__((aligned(8)));
    struct S arr_s[STRUCT_SIZE];
    
    /* Initialize with random values */
    srand(42);
    for (int i = 0; i < INT_SIZE; i++) {
        arr_i[i] = rand() % 1000;
    }
    for (int i = 0; i < FLOAT_SIZE; i++) {
        arr_f[i] = (float)rand() / RAND_MAX * 100.0f;
    }
    for (int i = 0; i < STRUCT_SIZE; i++) {
        arr_s[i].c = rand() % 256;
        arr_s[i].s = rand() % 10000;
        arr_s[i].a = rand() % 1000;
        arr_s[i].b = (float)rand() / RAND_MAX;
        arr_s[i].d = (double)rand() / RAND_MAX * 100.0;
        arr_s[i].l = rand() * 1000L;
    }
    
    double checksum = 0.0;
    
    /* PATTERN A: Simple post-increment loop */
    #pragma GCC unroll 4
    for (int* p = arr_i; p < &arr_i[INT_SIZE]; ) {
        checksum += *p++;  /* Post-increment in access */
        
        /* Insert occasional math call as compiler barrier */
        if ((p - arr_i) % 13 == 0) {
            checksum = cos(checksum);
        }
    }
    
    /* PATTERN B: Post-decrement while loop */
    float* q = &arr_f[FLOAT_SIZE - 1];
    int counter = FLOAT_SIZE;
    while (counter > 0) {
        checksum += *q--;  /* Post-decrement */
        counter--;
        
        /* Reset pointer to create new base register context */
        if (counter == FLOAT_SIZE / 2) {
            q = arr_f;  /* New base register */
        }
    }
    
    /* PATTERN C: Nested loops with structure access */
    for (int outer = 0; outer < 2; outer++) {
        struct S* ptr = arr_s;
        for (int inner = 0; inner < STRUCT_SIZE; inner++) {
            /* Access first field (offset 0) */
            checksum += ptr->a;    /* Should trigger [reg + 0] pattern */
            
            /* Access other fields (non-zero offsets) */
            checksum += ptr->b;
            checksum += ptr->d;
            
            ptr++;  /* Pointer increment */
        }
        
        /* Function call between loops */
        checksum = sqrt(fabs(checksum));
    }
    
    /* PATTERN D: Mixed pointer/index arithmetic */
    for (int i = 0; i < INT_SIZE; i++) {
        /* Different ways to access same memory */
        checksum += arr_i[i];              /* Array indexing */
        checksum += *(arr_i + i);          /* Pointer arithmetic */
        
        /* Create zero-offset access through temporary */
        int* tmp = arr_i + i;
        checksum += *tmp;                  /* [reg + 0] */
    }
    
    /* Call helper function for additional context */
    checksum += process_subset(arr_i, arr_f, arr_s, 25);
    
    /* Call function with attribute optimization */
    mixed_access_patterns(arr_i, arr_f, arr_s, 30);
    
    /* Final computation to prevent dead code elimination */
    checksum = fmod(checksum, 1000000.0);
    printf("Final checksum: %f\n", checksum);
    
    return (int)checksum % 256;
}

#pragma GCC pop_options
