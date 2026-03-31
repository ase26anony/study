#ifndef TEST_TYPES_H
#define TEST_TYPES_H

/* TYPE_UNDEFINED: Forward declarations of incomplete types */
struct IncompleteStruct;
union IncompleteUnion;
typedef struct OpaqueType OpaqueType;

/* TYPE_STRUCT: Plain C structures */
struct Point {
    int x;
    int y;
};

struct ComplexData {
    struct Point position;
    double timestamp;
};

/* TYPE_USER_STRUCT: Typedef'd structures */
typedef struct {
    int id;
    char name[50];
} UserStruct;

typedef struct Node {
    int data;
    struct Node* next;
} ListNode;

/* TYPE_UNION: Union declarations */
union DataUnion {
    int int_val;
    float float_val;
    double double_val;
    char* string_val;
};

/* TYPE_CALLBACK: Function pointer types */
typedef int (*Comparator)(const void*, const void*);
typedef void (*Logger)(const char* message);

/* TYPE_LANG_STRUCT: GCC attribute extensions */
struct __attribute__((packed)) PackedStruct {
    char flag;
    int value;
    double precision;
};

struct __attribute__((aligned(16))) AlignedStruct {
    long long data[2];
};

/* External declarations for multi-file testing */
extern struct Point global_point;
extern UserStruct global_user;
extern union DataUnion global_union;

#endif /* TEST_TYPES_H */
