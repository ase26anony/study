#ifndef COMPLEX_TYPES_H
#define COMPLEX_TYPES_H

/* Complex function pointer types with nested parentheses */
void (*signal(int sig, void (*func)(int)))(int);
int (*(*complex_func_ptr)(int (*)(int, int), char*))(float, double);

/* Multi-dimensional arrays with nested initializers */
int matrix_3d[2][3][4] = {
    {
        {1, 2, 3, 4},
        {5, 6, 7, 8},
        {9, 10, 11, 12}
    },
    {
        {13, 14, 15, 16},
        {17, 18, 19, 20},
        {21, 22, 23, 24}
    }
};

/* Nested structures with anonymous unions and bit-fields */
struct Outer {
    union {
        struct {
            int a:4;
            int b:4;
            int c:8;
        } bits;
        struct {
            long x;
            long y;
        } coords;
        char raw[4];
    } inner;
    
    struct {
        int (*callback)(struct Outer*, int);
        void* data;
    } meta;
    
    int arr[2][3];
};

/* Function with complex parameter list containing comments */
void process_data(
    int count,              /* Number of elements */
    char** strings,         /* Array of strings */
    void (*callback)(       /* Callback function */
        int status,         /* Status code */
        const char* msg     /* Status message */
    )
);

#endif /* COMPLEX_TYPES_H */
