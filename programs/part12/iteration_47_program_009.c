   struct tagged_string {
       int length_bits;    // Fixed-size member (typically 4 bytes)
       char data[];        // Variable-length array (0+ bytes)
   };
