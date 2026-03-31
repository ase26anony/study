/* auto_inc_dec_test.c
 * 
 * This program generates C code patterns that should produce
 * (mem (reg)) RTL expressions followed by pointer increments,
 * targeting the uncovered lines in auto-inc-dec.cc.
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/* Pattern 1: Simple pointer dereference with explicit increment */
int sum_array_int(const int *arr, int n) {
    const int *p = arr;
    int sum = 0;
    
    /* This should generate: (mem (reg p)) followed by p = p + 1 */
    for (int i = 0; i < n; i++) {
        sum += *p;      /* (mem (reg p)) */
        p = p + 1;      /* Increment in separate statement */
    }
    return sum;
}

/* Pattern 2: Pointer traversal with post-increment in loop body */
int sum_array_char(const char *arr, int n) {
    const char *p = arr;
    int sum = 0;
    
    /* Direct post-increment in dereference */
    while (n-- > 0) {
        sum += *p++;    /* Combined dereference and increment */
    }
    return sum;
}

/* Pattern 3: Memory copy with restrict qualifiers */
void copy_buffer(int *restrict dst, const int *restrict src, int n) {
    /* Using restrict helps alias analysis */
    while (n-- > 0) {
        *dst++ = *src++;  /* Two (mem (reg)) patterns with increments */
    }
}

/* Pattern 4: Separate statements in same basic block */
void process_shorts(short *data, int count) {
    short *ptr = data;
    
    for (int i = 0; i < count; i++) {
        short val = *ptr;      /* (mem (reg ptr)) - load */
        val = val * 2;         /* Some operation */
        *ptr = val;            /* (mem (reg ptr)) - store */
        ptr = ptr + 1;         /* Explicit increment */
    }
}

/* Pattern 5: Mixed operations to test pattern recognition */
int find_max(const int *arr, int n) {
    const int *p = arr;
    int max = *p;              /* Initial (mem (reg p)) */
    p = p + 1;                 /* Increment */
    
    for (int i = 1; i < n; i++) {
        if (*p > max) {        /* (mem (reg p)) in condition */
            max = *p;
        }
        p = p + 1;             /* Increment */
    }
    return max;
}

/* Pattern 6: Nested pointer operations */
void reverse_copy(char *restrict dst, const char *restrict src, int len) {
    const char *s = src;
    char *d = dst + len - 1;
    
    while (len-- > 0) {
        *d-- = *s++;    /* Two pointers with auto-decrement/increment */
    }
}

/* Pattern 7: Local pointer in main - direct traversal */
int sum_local_array(void) {
    int local_arr[100];
    int *p = local_arr;
    int sum = 0;
    
    /* Initialize array */
    for (int i = 0; i < 100; i++) {
        local_arr[i] = i;
    }
    
    /* Pointer traversal */
    for (int i = 0; i < 100; i++) {
        sum += *p;      /* (mem (reg p)) */
        p++;            /* Increment */
    }
    return sum;
}

/* Pattern 8: Byte buffer processing */
int checksum(const unsigned char *data, int length) {
    const unsigned char *ptr = data;
    int sum = 0;
    
    /* Process in chunks of 4 for potential unrolling */
    while (length >= 4) {
        sum += ptr[0];  /* Array notation might still generate (mem (reg)) */
        sum += ptr[1];
        sum += ptr[2];
        sum += ptr[3];
        ptr += 4;
        length -= 4;
    }
    
    /* Remainder */
    while (length-- > 0) {
        sum += *ptr++;
    }
    
    return sum;
}

/* Pattern 9: Structure pointer traversal */
struct Point {
    int x;
    int y;
};

int sum_points(const struct Point *points, int n) {
    const struct Point *p = points;
    int total = 0;
    
    for (int i = 0; i < n; i++) {
        total += p->x;      /* (mem (reg p)) with offset */
        p++;                /* Increment by structure size */
    }
    return total;
}

/* Main function to exercise all patterns */
int main(void) {
    /* Test buffers */
    int int_arr[50];
    char char_arr[100];
    int dest_arr[50];
    short short_arr[64];
    unsigned char byte_data[256];
    struct Point points[20];
    
    /* Initialize test data */
    for (int i = 0; i < 50; i++) {
        int_arr[i] = i * 2;
        dest_arr[i] = 0;
    }
    
    for (int i = 0; i < 100; i++) {
        char_arr[i] = 'A' + (i % 26);
    }
    
    for (int i = 0; i < 64; i++) {
        short_arr[i] = i;
    }
    
    for (int i = 0; i < 256; i++) {
        byte_data[i] = i;
    }
    
    for (int i = 0; i < 20; i++) {
        points[i].x = i;
        points[i].y = i * 2;
    }
    
    /* Exercise pattern functions */
    int total = 0;
    
    total += sum_array_int(int_arr, 50);
    printf("Sum of int array: %d\n", sum_array_int(int_arr, 50));
    
    total += sum_array_char(char_arr, 100);
    printf("Sum of char array: %d\n", sum_array_char(char_arr, 100));
    
    copy_buffer(dest_arr, int_arr, 50);
    printf("Copy completed, dest[25] = %d\n", dest_arr[25]);
    
    process_shorts(short_arr, 64);
    printf("Shorts processed, short_arr[32] = %d\n", (int)short_arr[32]);
    
    total += find_max(int_arr, 50);
    printf("Max in int array: %d\n", find_max(int_arr, 50));
    
    char reversed[100];
    reverse_copy(reversed, char_arr, 100);
    printf("Reverse copy done, first char: %c\n", reversed[0]);
    
    total += sum_local_array();
    printf("Local array sum: %d\n", sum_local_array());
    
    total += checksum(byte_data, 256);
    printf("Checksum of byte data: %d\n", checksum(byte_data, 256));
    
    total += sum_points(points, 20);
    printf("Sum of points: %d\n", sum_points(points, 20));
    
    /* Final checksum to verify all computations */
    printf("\nFinal total checksum: %d\n", total);
    
    /* Additional pointer traversal in main itself */
    int *main_ptr = int_arr;
    int main_sum = 0;
    for (int i = 0; i < 10; i++) {
        main_sum += *main_ptr;   /* (mem (reg main_ptr)) */
        main_ptr = main_ptr + 1; /* Separate increment */
    }
    printf("Main pointer traversal sum: %d\n", main_sum);
    
    return 0;
}
