#ifndef COMPLEX_TYPES_H
#define COMPLEX_TYPES_H

/* Test case 1: Function pointer types with nested parameter lists */
void (*signal(int sig, void (*func)(int)))(int);
int (*(*complex_func_ptr)(int (*)(int, int), char*))(float, double);

/* Test case 2: Array declarations with multiple dimensions and nested initializers */
int arr_3d[2][3][2] = {
    { /* Layer 0 */
        {1, 2}, /* row 0 */
        {3, 4}, /* row 1 */
        {5, 6}  /* row 2 */
    },
    { /* Layer 1 */
        {7, 8}, /* row 0 */
        {9, 10}, /* row 1 */
        {11, 12} /* row 2 */
    }
};

/* Test case 3: Structures/unions with nested anonymous structures and bit-fields */
struct OuterStruct {
    union {
        struct {
            int a:4; /* bit-field a */
            int b:4; /* bit-field b */
            int c:8; /* bit-field c */
        };
        struct {
            unsigned int x:8; /* different layout */
            unsigned int y:8;
        };
        char raw[2];
    } nested_union;
    
    struct {
        int (*callback)(int, char);
        float matrix[2][2];
    } inner_struct;
    
    int d[3];
};

/* Nested structure with multiple levels */
struct DeeplyNested {
    struct Level1 {
        struct Level2 {
            struct Level3 {
                int value;
                struct {
                    char flag:1;
                    char mode:3;
                } bits;
            } l3;
        } l2[2];
    } l1;
};

#endif /* COMPLEX_TYPES_H */
