Device (GPU/Accelerator)
├── Team 0: j = 0..(N/num_teams - 1)
│   ├── Thread 0: SIMD vector of i[0..15]
│   ├── Thread 1: SIMD vector of i[16..31]
│   └── ...
├── Team 1: j = (N/num_teams)..(2N/num_teams - 1)
└── ...
