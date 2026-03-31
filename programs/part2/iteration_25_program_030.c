/* Test file 1: Deeply nested structs and unions */

/* Basic nested struct */
struct Level1 {
    int a;
    struct Level2 {
        char b;
        struct Level3 {
            long c;
            union Level4 {
                int d;
                struct Level5 {
                    short e;
                    struct {
                        unsigned char f;
                        union {
                            double g;
                            struct Level6 {
                                float h;
                                struct Level7 {
                                    int i[3][2];
                                    union Level8 {
                                        long j;
                                        struct Level9 {
                                            char k;
                                            struct {
                                                int l;
                                            } anonymous;
                                        } nested9;
                                    } u8;
                                } nested7;
                            } nested6;
                        } u_inner;
                    } anonymous_inner;
                } nested5;
            } u4;
        } nested3;
    } nested2;
};

/* Struct with arrays and bit-fields */
struct ComplexArrayStruct {
    int matrix[3][4][5];
    struct {
        unsigned int flags : 3;
        unsigned int : 5;  /* Unnamed bit-field */
        signed int value : 12;
    } bitfield_container;
    union ArrayUnion {
        char str[10][20];
        int nums[5][3][2];
        struct {
            float data[2][2];
        } matrix2d;
    } array_union;
};

/* Designated initializers in type context */
struct WithDesignatedInit {
    struct {
        int x;
        int y;
    } point;
    int arr[4];
} global_var = { .point = { .x = 1, .y = 2 }, .arr = { [0] = 10, [3] = 20 } };

/* Anonymous struct/union combinations */
struct AnonymousNest {
    struct {
        union {
            struct {
                int depth1;
                struct {
                    char depth2;
                };
            };
            long alt;
        };
        int after;
    } container;
};

/* Function returning complex nested struct */
struct Outer {
    struct Middle {
        struct Inner {
            int value;
        } inner;
    } middle;
};

struct Outer (*get_complex_struct(void))[5] {
    return 0;
}
