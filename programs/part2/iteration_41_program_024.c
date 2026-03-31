Device (e.g., GPU)
├── Team 0
│   ├── Threads process j = 0..k with SIMD vectorization (i-loop)
│   └── Each thread processes multiple i iterations using SIMD
├── Team 1
│   ├── Threads process j = k+1..m with SIMD vectorization
│   └── ...
└── ...
