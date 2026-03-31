/* test-auto-inc-dec.c */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Opaque functions to prevent optimization */
extern void use_int(int) __attribute__((noinline));
extern void use_ptr(void*) __attribute__((noinline));
extern void use_char(char) __attribute__((noinline));

/* Global volatile to prevent dead code elimination */
volatile int global_checksum = 0;

/* Test 1: Integer array summation with post-increment pointer */
int test1_int_array_sum(int* arr, int n) {
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
        r1 += r2; r3 += r4; r5 += r6;
        r7 += r8; r9 += r10;
        r2 += r1; r4 += r3; r6 += r5;
        r8 += r7; r10 += r9;
    }
    
    /* Use all pressure variables to prevent elimination */
    asm volatile("" : : "r"(r1), "r"(r2), "r"(r3), "r"(r4), "r"(r5),
                     "r"(r6), "r"(r7), "r"(r8), "r"(r9), "r"(r10));
    
    return sum;
}

/* Test 2: String copy with post-increment pointers */
void test2_string_copy(char* dst, const char* src, int n) {
    char* d = dst;
    const char* s = src;
    int i = 0;
    
    /* Register pressure variables */
    char c1 = 'a', c2 = 'b', c3 = 'c', c4 = 'd';
    int t1 = 1, t2 = 2, t3 = 3, t4 = 4;
    
    do {
        /* Classic post-increment copy pattern */
        *d++ = *s++;
        
        /* Mix operations to prevent over-optimization */
        c1 = c2 + t1;
        c2 = c3 + t2;
        c3 = c4 + t3;
        c4 = c1 + t4;
        t1 = t2 + i;
        t2 = t3 + i;
        t3 = t4 + i;
        t4 = t1 + i;
        
        i++;
    } while (i < n);
    
    /* Opaque use of pointers */
    use_ptr(dst);
    use_ptr((void*)src);
    
    /* Use pressure variables */
    asm volatile("" : : "r"(c1), "r"(c2), "r"(c3), "r"(c4),
                     "r"(t1), "r"(t2), "r"(t3), "r"(t4));
}

/* Test 3: Struct array traversal with post-increment */
struct Point {
    int x;
    int y;
    int z;
};

int test3_struct_array_sum(struct Point* points, int n) {
    int sum = 0;
    struct Point* p = points;
    int count = n;
    
    /* High register pressure */
    int a1 = 1, a2 = 2, a3 = 3, a4 = 4, a5 = 5;
    int b1 = 6, b2 = 7, b3 = 8, b4 = 9, b5 = 10;
    struct Point temp;
    
    while (count-- > 0) {
        /* Access struct member with post-increment */
        sum += p->x + p->y + p->z;
        
        /* Post-increment the pointer */
        struct Point* old_p = p++;
        
        /* Complex operations with pressure variables */
        temp.x = a1 + b1;
        temp.y = a2 + b2;
        temp.z = a3 + b3;
        a1 = a2 + old_p->x;
        a2 = a3 + old_p->y;
        a3 = a4 + old_p->z;
        a4 = a5 + temp.x;
        a5 = b1 + temp.y;
        b1 = b2 + temp.z;
        b2 = b3 + a1;
        b3 = b4 + a2;
        b4 = b5 + a3;
        b5 = a4 + b4;
    }
    
    /* Force use of all variables */
    asm volatile("" : : "r"(a1), "r"(a2), "r"(a3), "r"(a4), "r"(a5),
                     "r"(b1), "r"(b2), "r"(b3), "r"(b4), "r"(b5),
                     "r"(temp.x), "r"(temp.y), "r"(temp.z));
    
    return sum;
}

/* Test 4: Nested loops with array indexing and post-increment */
int test4_nested_loops(int** matrix, int rows, int cols) {
    int total = 0;
    
    /* Multiple index variables for pressure */
    int i = 0, j = 0, k = 0, l = 0, m = 0;
    int t1 = 0, t2 = 0, t3 = 0, t4 = 0, t5 = 0;
    
    for (i = 0; i < rows; i++) {
        int* row = matrix[i];
        int* p = row;
        int* end = row + cols;
        
        /* Inner loop with pointer post-increment */
        while (p < end) {
            /* Target pattern in nested context */
            total += *p++;
            
            /* Index update that might create base+0 pattern */
            j = (j + 1) % 8;
            k = (k + 2) % 8;
            l = (l + 3) % 8;
            m = (m + 4) % 8;
            
            /* More pressure operations */
            t1 += i + j;
            t2 += j + k;
            t3 += k + l;
            t4 += l + m;
            t5 += m + i;
        }
        
        /* Inter-loop operations */
        asm volatile("" : : "r"(j), "r"(k), "r"(l), "r"(m),
                         "r"(t1), "r"(t2), "r"(t3), "r"(t4), "r"(t5));
    }
    
    return total + t1 + t2 + t3 + t4 + t5;
}

/* Test 5: Mixed pointer types and stride access */
int test5_mixed_pointers(char* data, int size, int stride) {
    int sum = 0;
    char* p = data;
    int* ip;
    short* sp;
    
    /* Extreme register pressure */
    register int r0 asm("r0") = 0;
    register int r1 asm("r1") = 1;
    register int r2 asm("r2") = 2;
    register int r3 asm("r3") = 3;
    register int r4 asm("r4") = 4;
    
    for (int i = 0; i < size; i += stride) {
        /* Different pointer types with post-increment */
        ip = (int*)(p + i);
        sp = (short*)(p + i + 4);
        
        /* Combined form that may decompose to base+0 */
        sum += *(ip++);
        sum += *(sp++);
        
        /* Pointer arithmetic that might create (plus (reg) (const_int 0)) */
        char* q = p + i;
        sum += *(q++);
        
        /* Use stride in pointer update */
        p += stride;
        
        /* Heavy register usage */
        r0 = r1 + r2;
        r1 = r2 + r3;
        r2 = r3 + r4;
        r3 = r4 + r0;
        r4 = r0 + r1;
        
        /* Opause use to prevent elimination */
        use_ptr(ip);
        use_ptr(sp);
        use_ptr(q);
    }
    
    /* Ensure all registers are marked as used */
    asm volatile("" : : "r"(r0), "r"(r1), "r"(r2), "r"(r3), "r"(r4));
    
    return sum;
}

/* Main function with command-line control */
int main(int argc, char** argv) {
    const int N = 1024;
    
    /* Initialize test data */
    int* int_array = malloc(N * sizeof(int));
    char* char_array = malloc(N * sizeof(char));
    struct Point* point_array = malloc(N * sizeof(struct Point));
    int** matrix = malloc(16 * sizeof(int*));
    
    for (int i = 0; i < N; i++) {
        int_array[i] = i % 100;
        char_array[i] = 'A' + (i % 26);
        point_array[i].x = i;
        point_array[i].y = i * 2;
        point_array[i].z = i * 3;
    }
    
    for (int i = 0; i < 16; i++) {
        matrix[i] = malloc(16 * sizeof(int));
        for (int j = 0; j < 16; j++) {
            matrix[i][j] = i * 16 + j;
        }
    }
    
    /* Select test based on command line */
    int test_num = (argc > 1) ? atoi(argv[1]) : 0;
    int result = 0;
    
    switch (test_num) {
        case 1:
            result = test1_int_array_sum(int_array, N);
            break;
        case 2:
            test2_string_copy(char_array, "Source string for testing", 100);
            result = char_array[0];
            break;
        case 3:
            result = test3_struct_array_sum(point_array, N);
            break;
        case 4:
            result = test4_nested_loops(matrix, 16, 16);
            break;
        case 5:
            result = test5_mixed_pointers(char_array, N, 4);
            break;
        default:
            /* Run all tests */
            result = test1_int_array_sum(int_array, N);
            test2_string_copy(char_array + 100, char_array, 100);
            result += test3_struct_array_sum(point_array, N);
            result += test4_nested_loops(matrix, 16, 16);
            result += test5_mixed_pointers(char_array, N, 4);
            break;
    }
    
    /* Update global volatile to prevent elimination */
    global_checksum = result;
    
    /* Cleanup */
    for (int i = 0; i < 16; i++) {
        free(matrix[i]);
    }
    free(matrix);
    free(int_array);
    free(char_array);
    free(point_array);
    
    printf("Result: %d\n", result);
    return 0;
}

/* Dummy implementations of opaque functions */
void __attribute__((noinline)) use_int(int x) {
    global_checksum += x;
}

void __attribute__((noinline)) use_ptr(void* p) {
    global_checksum += (int)((long)p & 0xFF);
}

void __attribute__((noinline)) use_char(char c) {
    global_checksum += c;
}
