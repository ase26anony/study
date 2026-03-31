#ifndef COMPLEX_TYPES_H
#define COMPLEX_TYPES_H

/* Test case 1: Function pointer types with nested parameter lists */
void (*signal(int sig, void (*func)(int)))(int);
int (*(*complex_func_ptr)(int (*)(int, int), char**))(float, double);

/* Test case 2: Array declarations with multiple dimensions and nested initializers */
int arr_3d[2][3][2] = {
    { /* First 3x2 matrix */
        {1, 2},
        {3, 4},
        {5, 6}
    },
    { /* Second 3x2 matrix */
        {7, 8},
        {9, 10},
        {11, 12}
    }
};

/* Test case 3: Structures with nested anonymous structures and bit-fields */
struct OuterStruct {
    union {
        struct {
            int a:4;
            int b:4;
            int c:8;
        };
        struct {
            unsigned int x:16;
            unsigned int y:16;
        };
        char raw[4];
    } nested_union;
    
    struct {
        int (*callback)(struct OuterStruct*, int);
        void* data;
    } handler;
    
    int d[2][3];
};

/* Test case 4: Complex typedef chains */
typedef int (*cmp_fn)(int, int);
typedef cmp_fn (*factory_fn)(void);
typedef factory_fn (*registry_fn)(const char*);
typedef registry_fn (*plugin_init_fn)(int, ...);

/* Test case 5: Comments within balanced sequences */
int matrix[2][2] = { 
    {1 /* row1 col1 */, 2 /* row1 col2 */}, 
    {3 /* row2 col1 */, 4 /* row2 col2 */} 
};

void complex_prototype(
    int a, /* First parameter */
    char b, /* Second parameter with comment */
    void (*callback)( /* Callback function pointer */
        int, /* Callback param 1 */
        char* /* Callback param 2 */
    )
);

#endif /* COMPLEX_TYPES_H */
