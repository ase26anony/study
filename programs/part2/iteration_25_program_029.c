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
        } u1;
    } nested1;
};

/* Level 2: Anonymous structs and unions */
struct Level2 {
    struct {
        int x;
        union {
            char y;
            struct {
                int z;
                union {
                    float w;
                    double v;
                    struct {
                        unsigned char byte;
                        signed short word;
                    } bits;
                } deep_union;
            } deeper;
        } anon_union;
    };
    int arr[3][4][5];
};

/* Level 3: Bit-fields and arrays */
struct Level3 {
    unsigned int flags : 4;
    signed int value : 12;
    struct {
        unsigned char : 2;
        unsigned char enable : 1;
        unsigned char mode : 3;
        union {
            struct {
                int data[8];
                struct {
                    char metadata[16];
                    int (*callback)(void);
                } info;
            } data_block;
            void *ptr;
        } storage;
    } config;
};

/* Level 4: Function pointers in nested structs */
struct Level4 {
    int (*compare)(const void *, const void *);
    struct {
        void (*init)(struct Level4 *);
        int (*process)(int, char **);
        struct {
            char *(*alloc)(size_t);
            void (*free)(void *);
            int (*validate)(const struct {
                int check;
                char *msg;
            } *);
        } mem_ops;
    } ops;
};

/* Level 5: Mixed nested types with designated initializers */
struct Level5 {
    struct {
        union {
            struct {
                int matrix[2][3];
                struct {
                    char name[32];
                    int id;
                } item;
            } data;
            long long big_value;
        } container;
        int count;
    } header;
    struct Level5 *next;
};

/* Array of complex nested structs */
struct Level1 complex_array[10][5];
struct Level2 *ptr_array[3][2][4];
