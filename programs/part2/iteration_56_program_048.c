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
    int i;
    float f;
    double d;
    char c;
};

union TaggedUnion {
    int type;
    struct {
        int x, y;
    } point;
    struct {
        float radius;
    } circle;
};

/* TYPE_LANG_STRUCT: GCC attribute extensions */
struct __attribute__((packed)) PackedStruct {
    char a;
    int b;
    double c;
};

struct __attribute__((aligned(16))) AlignedStruct {
    int data[4];
};

/* Function pointer types for callbacks */
typedef int (*CompareFunc)(const void*, const void*);
typedef void (*LoggerFunc)(const char*);

/* External declarations for multi-file testing */
extern struct Point global_point;
extern UserStruct global_user;

#endif /* TEST_TYPES_H */
