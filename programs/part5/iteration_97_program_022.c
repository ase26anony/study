#ifndef COMPLEX_TYPES_H
#define COMPLEX_TYPES_H

/* Complex function pointer types with nested parentheses */
void (*signal(int sig, void (*func)(int)))(int);
int (*(*complex_func_ptr)(int (*)(int, int), char))(float, double);

/* Multi-dimensional arrays with nested initializers */
int matrix_3d[2][3][2] = {
    { /* First 2D slice */
        {1, 2},   /* row 0 */
        {3, 4},   /* row 1 */
        {5, 6}    /* row 2 */
    },
    { /* Second 2D slice */
        {7, 8},   /* row 0 */
        {9, 10},  /* row 1 */
        {11, 12}  /* row 2 */
    }
};

/* Structures with nested anonymous structures and bit-fields */
struct OuterStruct {
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
    } data;
    
    int (*callback)(struct OuterStruct*, int);
    int array[2][3];
};

/* Nested union within struct with bit-fields */
struct DeviceRegister {
    union {
        struct {
            unsigned int enable:1;
            unsigned int mode:3;
            unsigned int reserved:4;
            unsigned int data:8;
        } fields;
        unsigned short raw_value;
    } reg;
    
    void (*interrupt_handler)(struct DeviceRegister*);
};

#endif /* COMPLEX_TYPES_H */
