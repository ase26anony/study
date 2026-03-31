#ifndef COMPLEX_TYPES_H
#define COMPLEX_TYPES_H

/* Complex function pointer types with nested parentheses */
void (*signal(int sig, void (*func)(int)))(int);
int (*(*complex_func_ptr)(int (*)(int, int), char))(void);
void (*(*(*nested_func_ptr[3])(int))(float))(char);

/* Array declarations with multiple dimensions and nested initializers */
int arr_3d[2][3][2] = {
    { /* First 3x2 block */
        {1, 2},
        {3, 4},
        {5, 6}
    },
    { /* Second 3x2 block */
        {7, 8},
        {9, 10},
        {11, 12}
    }
};

/* Structures with nested anonymous structures and bit-fields */
struct OuterStruct {
    union {
        struct {
            int a:4;
            int b:4;
            int c:8;
        };
        struct {
            unsigned int x:16;
        };
        char raw[4];
    } inner_union;
    
    struct {
        int (*callback)(struct OuterStruct*, int);
        int data[2];
    } nested_struct;
    
    int d[2][2];
};

/* Even more complex nested structure */
struct UltraNested {
    struct {
        union {
            struct {
                int a:3;
                int b:5;
                int c:8;
            };
            long long value;
        };
        
        struct {
            int (*funcs[2])(int, int);
            struct UltraNested* next;
        } links;
    } core;
    
    int matrix[2][2];
};

#endif /* COMPLEX_TYPES_H */
