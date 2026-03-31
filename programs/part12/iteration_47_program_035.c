struct tagged_string {
    int length_bits;  // Typically 4 bytes (platform-dependent)
    char data[];      // Variable length, starts immediately after length_bits
};
