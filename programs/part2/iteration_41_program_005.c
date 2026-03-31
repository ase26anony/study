Device (GPU/Accelerator)
├── Team 0
│   ├── Thread 0: j = 0..(N/teams)-1
│   │   └── SIMD vectorized: i = 0..M-1 (vector length ≤ 16)
│   ├── Thread 1: j = (N/teams)..(2N/teams)-1
│   │   └── SIMD vectorized: i = 0..M-1
│   └── ...
├── Team 1
│   └── Similar distribution
└── ...
