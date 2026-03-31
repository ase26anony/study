#ifndef TEST_TYPES_H
#define TEST_TYPES_H

/* TYPE_UNDEFINED: Forward declarations of incomplete types */
struct IncompleteStruct;
union IncompleteUnion;
typedef struct Opaque OpaqueType;

/* TYPE_STRUCT: Plain C structures */
struct Point {
    int x;
    int y;
};

struct ComplexData {
    char name[50];
    struct Point location;
    void *metadata;
};

/* TYPE_USER_STRUCT: Typedef'd structures */
typedef struct {
    int id;
    char name[32];
    float score;
} UserStruct;

typedef struct Node {
    int data;
    struct Node *next;
} ListNode;

/* TYPE_UNION: Union declarations */
union DataUnion {
    int i;
    float f;
    double d;
    char *str;
};

union BitFieldUnion {
    struct {
        unsigned int flag1 : 1;
        unsigned int flag2 : 3;
        unsigned int flag3 : 4;
    } bits;
    unsigned short all_flags;
};

/* TYPE_LANG_STRUCT: GCC-specific attributes */
struct __attribute__((packed)) PackedStruct {
    char a;
    int b;
    double c;
};

struct __attribute__((aligned(16))) AlignedStruct {
    long long data[2];
    char padding;
};

/* TYPE_CALLBACK: Function pointer types */
typedef int (*Comparator)(const void *, const void *);
typedef void (*Logger)(const char *, ...);

/* External declarations for multi-file testing */
extern struct ComplexData global_complex;
extern UserStruct *get_user_struct(int id);

#endif /* TEST_TYPES_H */
