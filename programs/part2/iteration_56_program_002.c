#ifndef TYPE_COVERAGE_H
#define TYPE_COVERAGE_H

/* TYPE_UNDEFINED: Forward declarations of incomplete types */
struct IncompleteStruct;          /* Forward declaration */
union IncompleteUnion;            /* Forward declaration */
typedef struct Opaque OpaqueType; /* Opaque typedef */

/* TYPE_STRUCT: Plain C structures */
struct Point {
    int x;
    int y;
};

/* TYPE_UNION: Union declarations */
union Data {
    int i;
    float f;
    double d;
    char c;
};

/* TYPE_USER_STRUCT: Typedef'd structures */
typedef struct {
    int id;
    char name[50];
} UserStruct;

/* TYPE_LANG_STRUCT: GCC attribute extensions */
struct __attribute__((packed)) PackedStruct {
    char a;
    int b;
    double c;
};

struct __attribute__((aligned(16))) AlignedStruct {
    int data[4];
};

/* TYPE_CALLBACK: Function pointer types */
typedef int (*Comparator)(const void*, const void*);
typedef void (*Logger)(const char*);

/* Complex nested type for cross-file testing */
typedef struct ComplexNode {
    void* data;
    struct ComplexNode* next;
    struct ComplexNode* prev;
} ComplexNode;

/* External declarations for multi-file testing */
extern int global_counter;
extern void process_data(void* data, size_t size);

#endif /* TYPE_COVERAGE_H */
