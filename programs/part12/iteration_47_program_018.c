   struct tagged_string {
       int length_bits;    // 4 bytes (typically)
       char data[];        // Flexible array - size determined at allocation
   };
