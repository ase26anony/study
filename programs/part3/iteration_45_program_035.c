/* test-auto-inc-dec.c
 * Program designed to trigger auto-increment/decrement optimization patterns
 * Specifically targets find_inc() logic with mem_loc = address_of_x, reg0 = XEXP(x,0), reg1_is_const = true
 */

#include <stddef.h>
#include <string.h>

/* Global volatile to prevent dead code elimination */
volatile int global_counter = 0;

/* Opaque functions to create aliasing concerns */
extern void use_int(int) __attribute__((noinline));
extern void use_ptr(void*) __attribute__((noinline));
extern void use_char(char) __attribute__((noinline));

void use_int(int x) {
    global_counter += x;
}

void use_ptr(void* p) {
    global_counter += (int)((size_t)p & 0xFF);
}

void use_char(char c) {
    global_counter += c;
}

/* Test 1: Sum integer array with post-incrementing pointer in loop */
int test1_sum_int_array(int* arr, int n) {
    int sum = 0;
    int* p = arr;
    int* end = arr + n;
    
    /* Create register pressure with many live variables */
    int temp1 = 0, temp2 = 0, temp3 = 0, temp4 = 0;
    int temp5 = 0, temp6 = 0, temp7 = 0, temp8 = 0;
    
    while (p < end) {
        /* Post-increment access: should generate (mem (plus (reg) (const_int 0))) */
        sum += *p++;
        
        /* Use all temp variables to increase register pressure */
        temp1 += sum & 1;
        temp2 += sum & 2;
        temp3 += sum & 4;
        temp4 += sum & 8;
        temp5 += sum & 16;
        temp6 += sum & 32;
        temp7 += sum & 64;
        temp8 += sum & 128;
        
        /* Prevent optimization with asm barrier */
        asm volatile("" : : "r"(p), "r"(sum) : "memory");
    }
    
    /* Use all temps to prevent elimination */
    use_int(temp1 + temp2 + temp3 + temp4 + temp5 + temp6 + temp7 + temp8);
    return sum;
}

/* Test 2: String copy with post-increment pointers */
void test2_str_copy(char* dst, const char* src, size_t len) {
    char* d = dst;
    const char* s = src;
    const char* end = src + len;
    
    /* Register pressure variables */
    char c1 = 0, c2 = 0, c3 = 0, c4 = 0;
    int count = 0;
    
    while (s < end) {
        /* Classic post-increment copy pattern */
        *d++ = *s++;
        
        /* Additional operations on the copied value */
        char current = *(d - 1);
        c1 += current & 1;
        c2 += current & 2;
        c3 += current & 4;
        c4 += current & 8;
        count++;
        
        /* Make pointers appear used */
        asm volatile("" : : "r"(d), "r"(s) : "memory");
    }
    
    /* Use variables to prevent elimination */
    use_char(c1 + c2 + c3 + c4);
    use_int(count);
    
    /* Opaque use of pointers */
    use_ptr(dst);
    use_ptr((void*)src);
}

/* Test 3: Struct traversal with post-increment */
struct Point3D {
    int x, y, z;
    char label[4];
};

int test3_sum_struct_array(struct Point3D* points, int n) {
    int sum = 0;
    struct Point3D* p = points;
    struct Point3D* end = points + n;
    
    /* High register pressure */
    int t1 = 0, t2 = 0, t3 = 0, t4 = 0, t5 = 0;
    int t6 = 0, t7 = 0, t8 = 0, t9 = 0, t10 = 0;
    
    while (p < end) {
        /* Access struct members via post-incrementing pointer */
        sum += p->x + p->y + p->z;
        
        /* Post-increment the pointer */
        struct Point3D* current = p++;
        
        /* Complex calculations to maintain register pressure */
        t1 += current->x;
        t2 += current->y;
        t3 += current->z;
        t4 += current->label[0];
        t5 += current->label[1];
        t6 += current->label[2];
        t7 += current->label[3];
        t8 += sum & 0xFF;
        t9 += (int)current & 0xFF;
        t10 += n;
        
        /* Barrier to prevent reordering */
        asm volatile("" : : "r"(p), "r"(sum) : "memory");
    }
    
    /* Use all temps */
    use_int(t1 + t2 + t3 + t4 + t5 + t6 + t7 + t8 + t9 + t10);
    return sum;
}

/* Test 4: Nested loops with array indexing and post-increment */
int test4_nested_loops(int matrix[10][10], int rows, int cols) {
    int total = 0;
    
    /* Multiple index variables for register pressure */
    int i = 0, j = 0, k = 0, l = 0;
    int tmp1 = 0, tmp2 = 0, tmp3 = 0, tmp4 = 0;
    
    for (i = 0; i < rows; i++) {
        int* row = matrix[i];
        int* row_end = row + cols;
        
        /* Inner loop with pointer arithmetic */
        while (row < row_end) {
            /* Combined form that may decompose to base+offset */
            total += *(row++);
            
            /* Additional computations */
            tmp1 += i;
            tmp2 += j;
            tmp3 += total & 1;
            tmp4 += (int)row & 0xF;
            
            /* Index update in loop condition */
            j = (j + 1) % cols;
        }
        
        /* More register pressure */
        k += i * 2;
        l += i * 3;
        
        /* Barrier */
        asm volatile("" : : "r"(row), "r"(total) : "memory");
    }
    
    /* Use all variables */
    use_int(tmp1 + tmp2 + tmp3 + tmp4 + k + l);
    return total;
}

/* Test 5: Mixed pointer types and stride access */
int test5_mixed_pointers(char* data, int size, int stride) {
    int sum = 0;
    char* p = data;
    char* end = data + size;
    
    /* Multiple typed pointers for different access patterns */
    int* int_ptr;
    short* short_ptr;
    
    /* High register pressure */
    register int r1 asm("r1") = 0;
    register int r2 asm("r2") = 0;
    register int r3 asm("r3") = 0;
    int v1 = 0, v2 = 0, v3 = 0, v4 = 0, v5 = 0;
    
    while (p < end) {
        /* Cast to different pointer types with post-increment */
        int_ptr = (int*)p;
        sum += *int_ptr++;
        
        short_ptr = (short*)p;
        sum += *short_ptr++;
        
        /* Pointer arithmetic with stride */
        p += stride;
        
        /* Register pressure calculations */
        r1 += sum & 0xFF;
        r2 += (int)p & 0xFF;
        r3 += stride;
        v1 += *data;
        v2 += *(data + 1);
        v3 += *(data + 2);
        v4 += *(data + 3);
        v5 += size;
        
        /* Compiler barrier */
        asm volatile("" : : "r"(p), "r"(int_ptr), "r"(short_ptr) : "memory");
    }
    
    /* Use all variables */
    use_int(r1 + r2 + r3 + v1 + v2 + v3 + v4 + v5);
    return sum;
}

/* Test 6: Do-while loop with post-decrement */
int test6_post_decrement(int* arr, int n) {
    int sum = 0;
    int* p = arr + n - 1;
    
    /* Register pressure */
    int a = 0, b = 0, c = 0, d = 0, e = 0;
    
    if (n > 0) {
        do {
            /* Post-decrement access */
            sum += *p--;
            
            /* Additional computations */
            a += sum & 1;
            b += sum & 2;
            c += sum & 4;
            d += sum & 8;
            e += n;
            
            /* Barrier */
            asm volatile("" : : "r"(p), "r"(sum) : "memory");
        } while (p >= arr);
    }
    
    use_int(a + b + c + d + e);
    return sum;
}

/* Main function with command-line control */
int main(int argc, char** argv) {
    /* Initialize test data */
    int int_array[100];
    char char_array[200];
    struct Point3D struct_array[50];
    int matrix[10][10];
    
    /* Fill with pseudo-random data */
    for (int i = 0; i < 100; i++) {
        int_array[i] = (i * 37) & 0xFF;
    }
    
    for (int i = 0; i < 200; i++) {
        char_array[i] = (i * 13) & 0x7F;
    }
    
    for (int i = 0; i < 50; i++) {
        struct_array[i].x = i * 3;
        struct_array[i].y = i * 5;
        struct_array[i].z = i * 7;
        for (int j = 0; j < 4; j++) {
            struct_array[i].label[j] = (i + j) & 0xFF;
        }
    }
    
    for (int i = 0; i < 10; i++) {
        for (int j = 0; j < 10; j++) {
            matrix[i][j] = (i * 10 + j) * 11;
        }
    }
    
    /* Select test based on command line argument */
    int test_num = 0;
    if (argc > 1) {
        test_num = atoi(argv[1]) % 7;
    }
    
    int result = 0;
    
    switch (test_num) {
        case 0:
            result = test1_sum_int_array(int_array, 100);
            break;
        case 1:
            test2_str_copy(char_array + 100, char_array, 100);
            result = global_counter;
            break;
        case 2:
            result = test3_sum_struct_array(struct_array, 50);
            break;
        case 3:
            result = test4_nested_loops(matrix, 10, 10);
            break;
        case 4:
            result = test5_mixed_pointers(char_array, 200, 4);
            break;
        case 5:
            result = test6_post_decrement(int_array, 100);
            break;
        default:
            /* Run all tests */
            result = test1_sum_int_array(int_array, 100);
            result += test3_sum_struct_array(struct_array, 50);
            test2_str_copy(char_array + 100, char_array, 100);
            result += test4_nested_loops(matrix, 10, 10);
            result += test5_mixed_pointers(char_array, 200, 4);
            result += test6_post_decrement(int_array, 100);
            break;
    }
    
    /* Final use to prevent elimination */
    use_int(result);
    
    return (result + global_counter) & 0xFF;
}
