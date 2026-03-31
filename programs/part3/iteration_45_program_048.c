/* test-auto-inc-dec.c
 * Program designed to trigger auto-increment/decrement optimization
 * Specifically targets find_inc() logic for post-increment patterns
 */

#include <stddef.h>
#include <string.h>

/* Global volatile to prevent optimization */
volatile int global_sum = 0;

/* Opaque functions to prevent optimization */
extern void use_int(int) __attribute__((noinline));
extern void use_ptr(void*) __attribute__((noinline));
extern void use_char(char) __attribute__((noinline));

/* Prevent inlining */
#define NOINLINE __attribute__((noinline))

/* Test 1: Integer array summation with post-increment pointer */
NOINLINE int test_int_array_sum(int* arr, int n) {
    int sum = 0;
    int* p = arr;
    int* end = arr + n;
    
    /* Create register pressure */
    int r1 = 1, r2 = 2, r3 = 3, r4 = 4, r5 = 5;
    int r6 = 6, r7 = 7, r8 = 8, r9 = 9, r10 = 10;
    
    while (p < end) {
        /* Post-increment access - target pattern */
        sum += *p++;
        
        /* Use many variables to create register pressure */
        r1 += sum; r2 += r1; r3 += r2; r4 += r3; r5 += r4;
        r6 += r5; r7 += r6; r8 += r7; r9 += r8; r10 += r9;
        
        /* Prevent elimination of pointer */
        asm volatile("" : : "r"(p) : "memory");
    }
    
    /* Use all pressure variables */
    sum += r1 + r2 + r3 + r4 + r5 + r6 + r7 + r8 + r9 + r10;
    
    /* Opaque use to prevent optimization */
    use_int(sum);
    return sum;
}

/* Test 2: String copy with post-increment */
NOINLINE void test_char_copy(char* dst, const char* src, size_t len) {
    char* d = dst;
    const char* s = src;
    const char* end = src + len;
    
    /* Register pressure */
    char c1 = 'a', c2 = 'b', c3 = 'c', c4 = 'd', c5 = 'e';
    
    while (s < end) {
        /* Classic post-increment copy pattern */
        *d++ = *s++;
        
        /* Use pressure variables */
        c1 = *d; c2 = *s; c3 = c1 + c2; c4 = c3 - c1; c5 = c4 * 2;
        
        /* Prevent optimization */
        asm volatile("" : : "r"(d), "r"(s) : "memory");
    }
    
    /* Ensure pointers are used */
    use_ptr(d);
    use_ptr((void*)s);
    use_char(c1 + c2 + c3 + c4 + c5);
}

/* Test 3: Struct array traversal */
struct Point {
    int x;
    int y;
    int z;
};

NOINLINE int test_struct_array(struct Point* points, int n) {
    int total = 0;
    struct Point* p = points;
    struct Point* end = points + n;
    
    /* Heavy register pressure */
    int px = 0, py = 0, pz = 0;
    int t1 = 1, t2 = 2, t3 = 3, t4 = 4, t5 = 5;
    
    while (p < end) {
        /* Access struct member with post-increment */
        px += p->x;
        py += p->y;
        pz += p->z;
        
        /* Post-increment the pointer */
        struct Point* current = p++;
        
        /* Complex calculations with pressure variables */
        t1 = px * t1; t2 = py * t2; t3 = pz * t3;
        t4 = t1 + t2; t5 = t3 - t4;
        
        /* Use pointer to prevent optimization */
        use_ptr(current);
        asm volatile("" : : "r"(p) : "memory");
    }
    
    total = px + py + pz + t1 + t2 + t3 + t4 + t5;
    use_int(total);
    return total;
}

/* Test 4: Nested loops with array indexing */
NOINLINE int test_nested_loops(int matrix[][10], int rows) {
    int sum = 0;
    
    /* Multiple index variables for pressure */
    int i = 0, j = 0, k = 0, l = 0, m = 0;
    int* row_ptr;
    
    for (i = 0; i < rows; i++) {
        row_ptr = matrix[i];
        
        /* Inner loop with post-increment */
        for (j = 0; j < 10; j++) {
            /* Access with post-increment */
            sum += *row_ptr++;
            
            /* Additional pressure */
            k = sum + i; l = k + j; m = l * 2;
            
            /* Prevent optimization */
            asm volatile("" : : "r"(row_ptr) : "memory");
        }
        
        /* Use pressure variables */
        sum += k + l + m;
    }
    
    return sum;
}

/* Test 5: Mixed pointer arithmetic forms */
NOINLINE int test_mixed_forms(int* arr, int n, int stride) {
    int sum = 0;
    int* p = arr;
    int* end = arr + n;
    
    /* Various pressure variables */
    int a = 0, b = 0, c = 0, d = 0, e = 0;
    
    while (p < end) {
        /* Different forms that may generate (plus (reg) (const_int 0)) */
        
        /* Form 1: Direct post-increment */
        a += *p++;
        
        /* Form 2: Pointer arithmetic then dereference */
        if (p < end - 1) {
            b += *(p += 0);  /* May create base+0 pattern */
            p++;  /* Manual increment */
        }
        
        /* Form 3: Array indexing with post-increment */
        if (p < end) {
            int idx = 0;
            c += p[idx++];
            p++;  /* Sync pointer */
        }
        
        /* Use all variables */
        d = a + b + c;
        e = d * stride;
        
        /* Heavy memory barrier */
        asm volatile("" : : "r"(p), "r"(a), "r"(b), "r"(c) : "memory");
    }
    
    sum = a + b + c + d + e;
    use_int(sum);
    return sum;
}

/* Test 6: do-while loop with post-decrement */
NOINLINE int test_do_while(int* arr, int n) {
    int sum = 0;
    int* p = arr + n - 1;  /* Start from end */
    
    /* Pressure */
    int x = 0, y = 0, z = 0;
    
    if (n > 0) {
        do {
            /* Post-decrement access */
            sum += *p--;
            
            /* Pressure calculations */
            x = sum * 2;
            y = x - sum;
            z = y + *p;
            
            /* Force pointer usage */
            asm volatile("" : : "r"(p) : "memory");
        } while (p >= arr);
    }
    
    return sum + x + y + z;
}

/* Main function with command-line control */
int main(int argc, char** argv) {
    /* Initialize test data */
    int int_array[100];
    char char_array[100];
    struct Point struct_array[50];
    int matrix[5][10];
    
    /* Initialize arrays */
    for (int i = 0; i < 100; i++) {
        int_array[i] = i;
        char_array[i] = (char)(i % 26 + 'a');
    }
    
    for (int i = 0; i < 50; i++) {
        struct_array[i].x = i * 2;
        struct_array[i].y = i * 3;
        struct_array[i].z = i * 4;
    }
    
    for (int i = 0; i < 5; i++) {
        for (int j = 0; j < 10; j++) {
            matrix[i][j] = i * 10 + j;
        }
    }
    
    /* Use command line to select tests */
    int test_num = (argc > 1) ? atoi(argv[1]) : 0;
    int result = 0;
    
    switch (test_num % 6) {
        case 0:
            result = test_int_array_sum(int_array, 100);
            break;
        case 1:
            test_char_copy(char_array, char_array + 50, 50);
            result = char_array[0];
            break;
        case 2:
            result = test_struct_array(struct_array, 50);
            break;
        case 3:
            result = test_nested_loops(matrix, 5);
            break;
        case 4:
            result = test_mixed_forms(int_array, 100, 2);
            break;
        case 5:
            result = test_do_while(int_array, 100);
            break;
    }
    
    /* Update global to prevent dead code elimination */
    global_sum += result;
    
    return global_sum > 0 ? 0 : 1;
}

/* Dummy implementations of opaque functions */
void use_int(int x) {
    global_sum += x;
}

void use_ptr(void* p) {
    if (p) global_sum += 1;
}

void use_char(char c) {
    global_sum += (int)c;
}
