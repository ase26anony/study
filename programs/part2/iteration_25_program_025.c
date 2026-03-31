/* Test file 1: Deeply nested struct/union definitions */

/* Level 1 nesting */
struct Outer1 {
    int a;
    struct {
        char b;
        union {
            int c;
            long d;
            struct {
                short e;
                union {
                    char f;
                    double g;
                } inner_union;
            } inner_struct;
        } u1;
    } inner1;
    float arr1[3][4];
};

/* Level 2: Anonymous structs/unions */
struct Outer2 {
    struct {
        int x;
        union {
            struct {
                char y;
                int z[2][3];
            } s;
            long w;
        };
    };
    struct {
        union {
            struct {
                int a;
                struct {
                    char b;
                    union {
                        int c;
                        double d;
                    };
                } nested;
            };
            float f;
        };
    } anon;
};

/* Level 3: Bit-fields and arrays */
struct ComplexBitfield {
    unsigned int a : 3;
    unsigned int b : 5;
    struct {
        unsigned int c : 2;
        unsigned int d : 4;
        union {
            struct {
                unsigned int e : 1;
                unsigned int f : 7;
            } bits;
            unsigned char byte;
        } u;
    } inner;
    int matrix[2][3][4];
};

/* Designated initializers in type context */
struct WithDesignatedInit {
    int a;
    struct {
        int b;
        int c;
    } inner;
    int arr[5];
} global_var = { 
    .a = 1, 
    .inner = { .b = 2, .c = 3 },
    .arr = { [0] = 10, [4] = 50, [2] = 30 }
};

/* Nested array of structs */
struct Node {
    int value;
    struct Node* children[10];
    struct {
        struct Node* left;
        struct Node* right;
    } pointers;
};

struct Container {
    struct Node nodes[5][3];
    union {
        struct Node* ptr_array[10];
        struct {
            struct Node* head;
            struct Node* tail;
        };
    } node_union;
};
