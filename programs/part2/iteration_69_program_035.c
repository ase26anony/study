typedef enum {
    TYPE_NONE,
    TYPE_UNDEFINED,
    TYPE_STRUCT,
    // ... other types
} TypeCategory;

void serialize_type(TypeInfo *current) {
    switch (current->category) {
        case TYPE_NONE:
            gcc_unreachable();  // Should never happen
        case TYPE_UNDEFINED:
            write_state_undefined_type(current);
            break;
        // ... other cases as shown
    }
}
