// Examples of GTY with options:
struct GTY((chain_next ("chain_next"))) chain_struct {
    struct chain_struct *chain_next;
    int value;
};

struct GTY((desc ("%h.type"))) tagged_union {
    enum tag type;
    union {
        int intval;
        char *strval;
    } GTY((desc ("%1.type"))) u;
};
