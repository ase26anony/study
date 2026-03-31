/* Test file for gengtype parser - deeply nested structs and unions */

/* Level 1: Basic nested struct */
struct Level1 {
    int a;
    struct {
        char b;
        union {
            short c;
            long d;
            struct {
                float e;
                double f;
            } inner_inner;
        } u;
    } inner;
};

/* Level 2: Array within nested struct */
struct MatrixContainer {
    struct {
        int rows;
        int cols;
        int data[10][20];
        struct {
            char metadata[5][30];
            union {
                int flags[3];
                struct {
                    unsigned char a;
                    unsigned char b;
                    unsigned char c;
                } rgb;
            } info;
        } desc;
    } matrix;
};

/* Level 3: Deeply nested anonymous structs/unions */
struct DeepNest {
    struct {
        union {
            struct {
                int x;
                struct {
                    char y;
                    union {
                        short z;
                        struct {
                            float w;
                            double v;
                            int arr[3][4][5];
                        } deep;
                    } u2;
                } inner2;
            } s1;
            struct {
                long *ptr;
                struct {
                    int (*callback)(int, char);
                    union {
                        void (*vfunc)(void);
                        struct {
                            int count;
                            char name[50];
                        } data;
                    } choice;
                } funcs;
            } s2;
        } choice_union;
    } outer;
};

/* Level 4: Bit-fields with nested structs */
struct BitFieldStruct {
    unsigned int flag1 : 1;
    unsigned int flag2 : 2;
    struct {
        unsigned int nested1 : 3;
        unsigned int nested2 : 4;
        union {
            unsigned int choice1 : 5;
            struct {
                unsigned int deep1 : 2;
                unsigned int deep2 : 3;
            } deep_bits;
        } bit_union;
    } nested_bits;
    int regular_field;
};

/* Level 5: Designated initializers (in type context) */
struct DesignatedInit {
    int a;
    struct {
        int b;
        int c;
        union {
            int d;
            float e;
        } u;
    } inner;
    int arr[3][2];
};

/* Complex array of nested structs */
struct OuterArray {
    struct {
        int id;
        struct {
            char name[20];
            union {
                int value;
                float fvalue;
                struct {
                    int x, y, z;
                } coords;
            } data;
        } entry;
    } items[10][5];
};

/* Mix of all delimiters in one declaration */
struct MixedDelimiters {
    int (*func_ptr_array[5])(int, char);  /* [] then () */
    struct {
        int matrix[3][2];
        union {
            char str[100];
            void (*callbacks[3])(void);
        } u;
    } data;
    int (*(*complex_ptr))(int[10]);  /* Nested () and [] */
};
