   struct tagged_string {
       int length_bits;    // 4 bytes (typically)
       char data[];        // Variable size, allocated at runtime
   };
