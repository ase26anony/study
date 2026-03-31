union {
    struct type *pointer_to;           // TYPE_POINTER
    struct type *element_type;         // TYPE_ARRAY
    struct type *return_type;          // TYPE_CALLBACK (function pointer)
    struct field_list *fields;         // TYPE_STRUCT/TYPE_USER_STRUCT/TYPE_UNION
    char *type_name;                   // TYPE_USER_STRUCT name
    // Possibly scalar-specific info
    enum scalar_type scalar_kind;      // TYPE_SCALAR
    size_t array_size;                 // TYPE_ARRAY size
    // ... other fields
} u;
