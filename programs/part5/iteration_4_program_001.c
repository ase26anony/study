struct Example {
    unsigned int flag : 1;  // bitfield
};

// This would be illegal:
// &example.flag;  // ERROR: cannot take address of bitfield
