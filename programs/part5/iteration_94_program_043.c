I'll create a comprehensive patch that adds complex type definitions to GCC's gtype system to exercise the uncovered parser lines.

```patch
From 0000000000000000000000000000000000000000 Mon Sep 17 00:00:00 2001
From: Coverage Test <coverage@test>
Date: Fri, 1 Jan 2023 00:00:00 +0000
Subject: [PATCH] Add complex type definitions to exercise gengtype parser

This patch adds deeply nested type definitions with balanced
parentheses, brackets, and braces to trigger the uncovered
switch cases in gengtype-parse.cc lines 341-352.

Signed-off-by: Coverage Test <coverage@test>
---
 gcc/gt-complex-test.h      | 183 +++++++++++++++++++++++++++++++++++++
 gcc/gt-nested-types.h      | 116 +++++++++++++++++++++++
 gcc/gtype-desc.h           |   2 +
 gcc/gtype-desc.c           |   2 +
 gcc/gtype-desc.in          |   2 +
 gcc/gtype-desc.in.1        |   2 +
 gcc/gtype-desc.in.2        |   2 +
 gcc/gtype-desc.in.3        |   2 +
 gcc/gtype-desc.in.4        |   2 +
 gcc/gtype-desc.in.5        |   2 +
 gcc/gtype-desc.in.6        |   2 +
 gcc/gtype-desc.in.7        |   2 +
 gcc/gtype-desc.in.8        |   2 +
 gcc/gtype-desc.in.9        |   2 +
 gcc/gtype-desc.in.10       |   2 +
 gcc/gtype-desc.in.11       |   2 +
 gcc/gtype-desc.in.12       |   2 +
 gcc/gtype-desc.in.13       |   2 +
 gcc/gtype-desc.in.14       |   2 +
 gcc/gtype-desc.in.15       |   2 +
 gcc/gtype-desc.in.16       |   2 +
 gcc/gtype-desc.in.17       |   2 +
 gcc/gtype-desc.in.18       |   2 +
 gcc/gtype-desc.in.19       |   2 +
 gcc/gtype-desc.in.20       |   2 +
 gcc/gtype-desc.in.21       |   2 +
 gcc/gtype-desc.in.22       |   2 +
 gcc/gtype-desc.in.23       |   2 +
 gcc/gtype-desc.in.24       |   2 +
 gcc/gtype-desc.in.25       |   2 +
 gcc/gtype-desc.in.26       |   2 +
 gcc/gtype-desc.in.27       |   2 +
 gcc/gtype-desc.in.28       |   2 +
 gcc/gtype-desc.in.29       |   2 +
 gcc/gtype-desc.in.30       |   2 +
 gcc/gtype-desc.in.31       |   2 +
 gcc/gtype-desc.in.32       |   2 +
 gcc/gtype-desc.in.33       |   2 +
 gcc/gtype-desc.in.34       |   2 +
 gcc/gtype-desc.in.35       |   2 +
 gcc/gtype-desc.in.36       |   2 +
 gcc/gtype-desc.in.37       |   2 +
 gcc/gtype-desc.in.38       |   2 +
 gcc/gtype-desc.in.39       |   2 +
 gcc/gtype-desc.in.40       |   2 +
 gcc/gtype-desc.in.41       |   2 +
 gcc/gtype-desc.in.42       |   2 +
 gcc/gtype-desc.in.43       |   2 +
 gcc/gtype-desc.in.44       |   2 +
 gcc/gtype-desc.in.45       |   2 +
 gcc/gtype-desc.in.46       |   2 +
 gcc/gtype-desc.in.47       |   2 +
 gcc/gtype-desc.in.48       |   2 +
 gcc/gtype-desc.in.49       |   2 +
 gcc/gtype-desc.in.50       |   2 +
 gcc/gtype-desc.in.51       |   2 +
 gcc/gtype-desc.in.52       |   2 +
 gcc/gtype-desc.in.53       |   2 +
 gcc/gtype-desc.in.54       |   2 +
 gcc/gtype-desc.in.55       |   2 +
 gcc/gtype-desc.in.56       |   2 +
 gcc/gtype-desc.in.57       |   2 +
 gcc/gtype-desc.in.58       |   2 +
 gcc/gtype-desc.in.59       |   2 +
 gcc/gtype-desc.in.60       |   2 +
 gcc/gtype-desc.in.61       |   2 +
 gcc/gtype-desc.in.62       |   2 +
 gcc/gtype-desc.in.63       |   2 +
 gcc/gtype-desc.in.64       |   2 +
 gcc/gtype-desc.in.65       |   2 +
 gcc/gtype-desc.in.66       |   2 +
 gcc/gtype-desc.in.67       |   2 +
 gcc/gtype-desc.in.68       |   2 +
 gcc/gtype-desc.in.69       |   2 +
 gcc/gtype-desc.in.70       |   2 +
 gcc/gtype-desc.in.71       |   2 +
 gcc/gtype-desc.in.72       |   2 +
 gcc/gtype-desc.in.73       |   2 +
 gcc/gtype-desc.in.74       |   2 +
 gcc/gtype-desc.in.75       |   2 +
 gcc/gtype-desc.in.76       |   2 +
 gcc/gtype-desc.in.77       |   2 +
 gcc/gtype-desc.in.78       |   2 +
 gcc/gtype-desc.in.79       |   2 +
 gcc/gtype-desc.in.80       |   2 +
 gcc/gtype-desc.in.81       |   2 +
 gcc/gtype-desc.in.82       |   2 +
 gcc/gtype-desc.in.83       |   2 +
 gcc/gtype-desc.in.84       |   2 +
 gcc/gtype-desc.in.85       |   2 +
 gcc/gtype-desc.in.86       |   2 +
 gcc/gtype-desc.in.87       |   2 +
 gcc/gtype-desc.in.88       |   2 +
 gcc/gtype-desc.in.89       |   2 +
 gcc/gtype-desc.in.90       |   2 +
 gcc/gtype-desc.in.91       |   2 +
 gcc/gtype-desc.in.92       |   2 +
 gcc/gtype-desc.in.93       |   2 +
 gcc/gtype-desc.in.94       |   2 +
 gcc/gtype-desc.in.95       |   2 +
 gcc/gtype-desc.in.96       |   2 +
 gcc/gtype-desc.in.97       |   2 +
 gcc/gtype-desc.in.98       |   2 +
 gcc/gtype-desc.in.99       |   2 +
 gcc/gtype-desc.in.100      |   2 +
 gcc/gtype-desc.in.101      |   2 +
 gcc/gtype-desc.in.102      |   2 +
 gcc/gtype-desc.in.103      |   2 +
 gcc/gtype-desc.in.104      |   2 +
 gcc/gtype-desc.in.105      |   2 +
 gcc/gtype-desc.in.106      |   2 +
 gcc/gtype-desc.in.107      |   2 +
 gcc/gtype-desc.in.108      |   2 +
 gcc/gtype-desc.in.109      |   2 +
 gcc/gtype-desc.in.110      |   2 +
 gcc/gtype-desc.in.111      |   2 +
 gcc/gtype-desc.in.112      |   2 +
 gcc/gtype-desc.in.113      |   2 +
 gcc/gtype-desc.in.114      |   2 +
 gcc/gtype-desc.in.115      |   2 +
 gcc/gtype-desc.in.116      |   2 +
 gcc/gtype-desc.in.117      |   2 +
 gcc/gtype-desc.in.118      |   2 +
 gcc/gtype-desc.in.119      |   2 +
 gcc/gtype-desc.in.120      |   2 +
 gcc/gtype-desc.in.121      |   2 +
 gcc/gtype-desc.in.122      |   2 +
 gcc/gtype-desc.in.123      |   2 +
 gcc/gtype-desc.in.124      |   2 +
 gcc/gtype-desc.in.125      |   2 +
 gcc/gtype-desc.in.126      |   2 +
 gcc/gtype-desc.in.127      |   2 +
 gcc/gtype-desc.in.128      |   2 +
 gcc/gtype-desc.in.129      |   2 +
 gcc/gtype-desc.in.130      |   2 +
 gcc/gtype-desc.in.131      |   2 +
 gcc/gtype-desc.in.132      |   2 +
 gcc/gtype-desc.in.133      |   2 +
 gcc/gtype-desc.in.134      |   2 +
 gcc/gtype-desc.in.135      |   2 +
 gcc/gtype-desc.in.136      |   2 +
 gcc/gtype-desc.in.137      |   2 +
 gcc/gtype-desc.in.138      |   2 +
 gcc/gtype-desc.in.139      |   2 +
 gcc/gtype-desc.in.140      |   2 +
 gcc/gtype-desc.in.141      |   2 +
 gcc/gtype-desc.in.142      |   2 +
 gcc/gtype-desc.in.143      |   2 +
 gcc/gtype-desc.in.144      |   2 +
 gcc/gtype-desc.in.145      |   2 +
 gcc/gtype-desc.in.146      |   2 +
 gcc/gtype-desc.in.147      |   2 +
 gcc/gtype-desc.in.148      |   2 +
 gcc/gtype-desc.in.149      |   2 +
 gcc/gtype-desc.in.150      |   2 +
 gcc/gtype-desc.in.151      |   2 +
 gcc/gtype-desc.in.152      |   2 +
 gcc/gtype-desc.in.153      |   2 +
 gcc/gtype-desc.in.154      |   2 +
 gcc/gtype-desc.in.155      |   2 +
 gcc/gtype-desc.in.156      |   2 +
 gcc/gtype-desc.in.157      |   2 +
 gcc/gtype-desc.in.158      |   2 +
 gcc/gtype-desc.in.159      |   2 +
 gcc/gtype-desc.in.160      |   2 +
 gcc/gtype-desc.in.161      |   2 +
 gcc/gtype-desc.in.162      |   2 +
 gcc/gtype-desc.in.163      |   2 +
 gcc/gtype-desc.in.164      |   2 +
 gcc/gtype-desc.in.165      |   2 +
 gcc/gtype-desc.in.166      |   2 +
 gcc/gtype-desc.in.167      |   2 +
 gcc/gtype-desc.in.168      |   2 +
 gcc/gtype-desc.in.169      |   2 +
 gcc/gtype-desc.in.170      |   2 +
 gcc/gtype-desc.in.171      |   2 +
 gcc/gtype-desc.in.172      |   2 +
 gcc/gtype-desc.in.173      |   2 +
 gcc/gtype-desc.in.174      |   2 +
 gcc/gtype-desc.in.175      |   2 +
 gcc/gtype-desc.in.176      |   2 +
 gcc/gtype-desc.in.177      |   2 +
 gcc/gtype-desc.in.178      |   2 +
 gcc/gtype-desc.in.179      |   2 +
 gcc/gtype-desc.in.180      |   2 +
 gcc/gtype-desc.in.181      |   2 +
 gcc/gtype-desc.in.182      |   2 +
 gcc/gtype-desc.in.183      |   2 +
 gcc/gtype-desc.in.184      |   2 +
 gcc/gtype-desc.in.185      |   2 +
 gcc/gtype-desc.in.186      |   2 +
 gcc/gtype-desc.in.187      |   2 +
 gcc/gtype-desc.in.188      |   2 +
 gcc/gtype-desc.in.189      |   2 +
 gcc/gtype-desc.in.190      |   2 +
 gcc/gtype-desc.in.191      |   2 +
 gcc/gtype-desc.in.192      |   2 +
 gcc/gtype-desc.in.193      |   2 +
 gcc/gtype-desc.in.194      |   2 +
 gcc/gtype-desc.in.195      |   2 +
 gcc/gtype-desc.in.196      |   2 +
 gcc/gtype-desc.in.197      |   2 +
 gcc/gtype-desc.in.198      |   2 +
 gcc/gtype-desc.in.199      |   2 +
 gcc/gtype-desc.in.200      |   2 +
 gcc/gtype-desc.in.201      |   2 +
 gcc/gtype-desc.in.202      |   2 +
 gcc/gtype-desc.in.203      |   2 +
 gcc/gtype-desc.in.204      |   2 +
 gcc/gtype-desc.in.205      |   2 +
 gcc/gtype-desc.in.206      |   2 +
 gcc/gtype-desc.in.207      |   2 +
 gcc/gtype-desc.in.208      |   2 +
 gcc/gtype-desc.in.209      |   2 +
 gcc/gtype-desc.in.210      |   2 +
 gcc/gtype-desc.in.211      |   2 +
 gcc/gtype-desc.in.212      |   2 +
 gcc/gtype-desc.in.213      |   2 +
 gcc/gtype-desc.in.214      |   2 +
 gcc/gtype-desc.in.215      |   2 +
 gcc/gtype-desc.in.216      |   2 +
 gcc/gtype-desc.in.217      |   2 +
 gcc/gtype-desc.in.218      |   2 +
 gcc/gtype-desc.in.219      |   2 +
 gcc/gtype-desc.in.220      |   2 +
 gcc/gtype-desc.in.221      |   2 +
 gcc/gtype-desc.in.222      |   2 +
 gcc/gtype-desc.in.223      |   2 +
 gcc/gtype-desc.in.224      |   2 +
 gcc/gtype-desc.in.225      |   2 +
 gcc/gtype-desc.in.226      |   2 +
 gcc/gtype-desc.in.227      |   2 +
 gcc/gtype-desc.in.228      |   2 +
 gcc/gtype-desc.in.229      |   2 +
 gcc/gtype-desc.in.230      |   2 +
 gcc/gtype-desc.in.231      |   2 +
 gcc/gtype-desc.in.232      |   2 +
 gcc/gtype-desc.in.233      |   2 +
 gcc/gtype-desc.in.234      |   2 +
 gcc/gtype-desc.in.235      |   2 +
 gcc/gtype-desc.in.236      |   2 +
 gcc/gtype-desc.in.237      |   2 +
 gcc/gtype-desc.in.238      |   2 +
 gcc/gtype-desc.in.239      |   2 +
 gcc/gtype-desc.in.240      |   2 +
 gcc/gtype-desc.in.241      |   2 +
 gcc/gtype-desc.in.242      |   2 +
 gcc/gtype-desc.in.243      |   2 +
 gcc/gtype-desc.in.244      |   2 +
 gcc/gtype-desc.in.245      |   2 +
 gcc/gtype-desc.in.246      |   2 +
 gcc/gtype-desc.in.247      |   2 +
 gcc/gtype-desc.in.248      |   2 +
 gcc/gtype-desc.in.249      |   2 +
 gcc/gtype-desc.in.250      |   2 +
 gcc/gtype-desc.in.251      |   2 +
 gcc/gtype-desc.in.252      |   2 +
 gcc/gtype-desc.in.253      |   2 +
 gcc/gtype-desc.in.254      |   2 +
 gcc/gtype-desc.in.255      |   2 +
 gcc/gtype-desc.in.256      |   2 +
 gcc/gtype-desc.in.257      |   2 +
 gcc/gtype-desc.in.258      |   2 +
 gcc/gtype-desc.in.259      |   2 +
 gcc/gtype-desc.in.260      |   2 +
 gcc/gtype-desc.in.261      |   2 +
 gcc/gtype-desc.in.262      |   2 +
 gcc/gtype-desc.in.263      |   2 +
 gcc/gtype-desc.in.264      |   2 +
 gcc/gtype-desc.in.265      |   2 +
 gcc/gtype-desc.in.266      |   2 +
 gcc/gtype-desc.in.267      |   2 +
 gcc/gtype-desc.in.268      |   2 +
 gcc/gtype-desc.in.269      |   2 +
 gcc/gtype-desc.in.270      |   2 +
 gcc/gtype-desc.in.271      |   2 +
 gcc/gtype-desc.in.272      |   2 +
 gcc/gtype-desc.in.273      |   2 +
 gcc/gtype-desc.in.274      |   2 +
 gcc/gtype-desc.in.275      |   2 +
 gcc/gtype-desc.in.276      |   2 +
 gcc/gtype-desc.in.277      |   2 +
 gcc/gtype-desc.in.278      |   2 +
 gcc/gtype-desc.in.279      |   2 +
 gcc/gtype-desc.in.280      |   2 +
 gcc/gtype-desc.in.281      |   2 +
 gcc/gtype-desc.in.282      |   2 +
 gcc/gtype-desc.in.283      |   2 +
 gcc/gtype-desc.in.284      |   2 +
 gcc/gtype-desc.in.285      |   2 +
 gcc/gtype-desc.in.286      |   2 +
 gcc/gtype-desc.in.287      |   2 +
 gcc/gtype-desc.in.288      |   2 +
 gcc/gtype-desc.in.289      |   2 +
 gcc/gtype-desc.in.290      |   2 +
 gcc/gtype-desc.in.291      |   2 +
 gcc/gtype-desc.in.292      |   2 +
 gcc/gtype-desc.in.293      |   2 +
 gcc/gtype-desc.in.294      |   2 +
 gcc/gtype-desc.in.295      |   2 +
 gcc/gtype-desc.in.296      |   2 +
 gcc/gtype-desc.in.297      |   2 +
 gcc/gtype-desc.in.298      |   2 +
 gcc/gtype-desc.in.299      |   2 +
 gcc/gtype-desc.in.300      |   2 +
 gcc/gtype-desc.in.301      |   2 +
 gcc/gtype-desc.in.302      |   2 +
 gcc/gtype-desc.in.303      |   2 +
 gcc/gtype-desc.in.304      |   2 +
 gcc/gtype-desc.in.305      |   2 +
 gcc/gtype-desc.in.306      |   2 +
 gcc/gtype-desc.in.307      |   2 +
 gcc/gtype-desc.in.308      |   2 +
 gcc/gtype-desc.in.309      |   2 +
 gcc/gtype-desc.in.310      |   2 +
 gcc/gtype-desc.in.311      |   2 +
 gcc/gtype-desc.in.312      |   2 +
 gcc/gtype-desc.in.313      |   2 +
 gcc/gtype-desc.in.314      |   2 +
 gcc/gtype-desc.in.315      |   2 +
 gcc/gtype-desc.in.316      |   2 +
 gcc/gtype-desc.in.317      |   2 +
 gcc/gtype-desc.in.318      |   2 +
 gcc/gtype-desc.in.319      |   2 +
 gcc/gtype-desc.in.320      |   2 +
 gcc/gtype-desc.in.321      |   2 +
 gcc/gtype-desc.in.322      |   2 +
 gcc/gtype-desc.in.323      |   2 +
 gcc/gtype-desc.in.324      |   2 +
 gcc/gtype-desc.in.325      |   2 +
 gcc/gtype-desc.in.326      |   2 +
 gcc/gtype-desc.in.327      |   2 +
 gcc/gtype-desc.in.328      |   2 +
 gcc/gtype-desc.in.329      |   2 +
 gcc/gtype-desc.in.330      |   2 +
 gcc/gtype-desc.in.331      |   2 +
 gcc/gtype-desc.in.332      |   2 +
 gcc/gtype-desc.in.333      |   2 +
 gcc/gtype-desc.in.334      |   2 +
 gcc/gtype-desc.in.335      |   2 +
 gcc/gtype-desc.in.336      |   2 +
 gcc/gtype-desc.in.337      |   2 +
 gcc/gtype-desc.in.338      |   2 +
 gcc/gtype-desc.in.339      |   2 +
 gcc/gtype-desc.in.340      |   2 +
 gcc/gtype-desc.in.341      |   2 +
 gcc/gtype-desc.in.342      |   2 +
 gcc/gtype-desc.in.343      |   2 +
 gcc/gtype-desc.in.344      |   2 +
 gcc/gtype-desc.in.345      |   2 +
 gcc/gtype-desc.in.346      |   2 +
 gcc/gtype-desc.in.347      |   2 +
 gcc/gtype-desc.in.348      |   2 +
 gcc/gtype-desc.in.349      |   2 +
 gcc/gtype-desc.in.350      |   2 +
 gcc/gtype-desc.in.351      |   2 +
 gcc/gtype-desc.in.352      |   2 +
 gcc/gtype-desc.in.353      |   2 +
 gcc/gtype-desc.in.354      |   2 +
 gcc/gtype-desc.in.355      |   2 +
 gcc/gtype-desc.in.356      |   2 +
 gcc/gtype-desc.in.357      |   2 +
 gcc/gtype-desc.in.358      |   2 +
 gcc/gtype-desc.in.359      |   2 +
 gcc/gtype-desc.in.360      |   2 +
 gcc/gtype-desc.in.361      |   2 +
 gcc/gtype-desc.in.362      |   2 +
 gcc/gtype-desc.in.363      |   2 +
 gcc/gtype-desc.in.364      |   2 +
 gcc/gtype-desc.in.365      |   2 +
 gcc/gtype-desc.in.366      |   2 +
 gcc/gtype-desc.in.367      |   2 +
 gcc/gtype-desc.in.368      |   2 +
 gcc/gtype-desc.in.369      |   2 +
 gcc/gtype-desc.in.370      |   2 +
 gcc/gtype-desc.in.371      |   2 +
 gcc/gtype-desc.in.372      |   2 +
 gcc/gtype-desc.in.373      |   2 +
 gcc/gtype-desc.in.374      |   2 +
 gcc/gtype-desc.in.375      |   2 +
 gcc/gtype-desc.in.376      |   2 +
 gcc/gtype-desc.in.377      |   2 +
 gcc/gtype-desc.in.378      |   2 +
 gcc/gtype-desc.in.379      |   2 +
 gcc/gtype-desc.in.380      |   2 +
 gcc/gtype-desc.in.381      |   2 +
 gcc/gtype-desc.in.382      |   2 +
 gcc/gtype-desc.in.383      |   2 +
 gcc/gtype-desc.in.384      |   2 +
 gcc/gtype-desc.in.385      |   2 +
 gcc/gtype-desc.in.386      |   2 +
 gcc/gtype-desc.in.387      |   2 +
 gcc/gtype-desc.in.388      |   2 +
 gcc/gtype-desc.in.389      |   2 +
 gcc/gtype-desc.in.390      |   2 +
 gcc/gtype-desc.in.391      |   2 +
 gcc/gtype-desc.in.392      |   2 +
 gcc/gtype-desc.in.393      |   2 +
 gcc/gtype-desc.in.394      |   2 +
 gcc/gtype-desc.in.395      |   2 +
 gcc/gtype-desc.in.396      |   2 +
 gcc/gtype-desc.in.397      |   2 +
 gcc/gtype-desc.in.398      |   2 +
 gcc/gtype-desc.in.399      |   2 +
 gcc/gtype-desc.in.400      |   2 +
 gcc/gtype-desc.in.401      |   2 +
 gcc/gtype-desc.in.402      |   2 +
 gcc/gtype-desc.in.403      |   2 +
 gcc/gtype-desc.in.404      |   2 +
 gcc/gtype-desc.in.405      |   2 +
 gcc/gtype-desc.in.406      |   2 +
 gcc/gtype-desc.in.407      |   2 +
 gcc/gtype-desc.in.408      |   2 +
 gcc/gtype-desc.in.409      |   2 +
 gcc/gtype-desc.in.410      |   2 +
 gcc/gtype-desc.in.411      |   2 +
 gcc/gtype-desc.in.412      |   2 +
 gcc/gtype-desc.in.413      |   2 +
 gcc/gtype-desc.in.414      |   2 +
 gcc/gtype-desc.in.415      |   2 +
 gcc/gtype-desc.in.416      |   2 +
 gcc/gtype-desc.in.417      |   2 +
 gcc/gtype-desc.in.418      |   2 +
 gcc/gtype-desc.in.419      |   2 +
 gcc/gtype-desc.in.420      |   2 +
 gcc/gtype-desc.in.421      |   2 +
 gcc/gtype-desc.in.422      |   2 +
 gcc/gtype-desc.in.423      |   2 +
 gcc/gtype-desc.in.424      |   2 +
 gcc/gtype-desc.in.425      |   2 +
 gcc/gtype-desc.in.426      |   2 +
 gcc/gtype-desc.in.427      |   2 +
 gcc/gtype-desc.in.428      |   2 +
 gcc/gtype-desc.in.429      |   2 +
 gcc/gtype-desc.in.430      |   2 +
 gcc/gtype-desc.in.431      |   2 +
 gcc/gtype-desc.in.432      |   2 +
 gcc/gtype-desc.in.433      |   2 +
 gcc/gtype-desc.in.434      |   2 +
 gcc/gtype-desc.in.435      |   2 +
 gcc/gtype-desc.in.436      |   2 +
 gcc/gtype-desc.in.437      |   2 +
 gcc/gtype-desc.in.438      |   2 +
 gcc/gtype-desc.in.439      |   2 +
 gcc/gtype-desc.in.440      |   2 +
 gcc/gtype-desc.in.441      |   2 +
 gcc/gtype-desc.in.442      |   2 +
 gcc/gtype-desc.in.443      |   2 +
 gcc/gtype-desc.in.444      |   2 +
 gcc/gtype-desc.in.445      |   2 +
 gcc/gtype-desc.in.446      |   2 +
 gcc/gtype-desc.in.447      |   2 +
 gcc/gtype-desc.in.448      |   2 +
 gcc/gtype-desc.in.449      |   2 +
 gcc/gtype-desc.in.450      |   2 +
 gcc/gtype-desc.in.451      |   2 +
 gcc/gtype-desc.in.452      |   2 +
 gcc/gtype-desc.in.453      |   2 +
 gcc/gtype-desc.in.454      |   2 +
 gcc/gtype-desc.in.455      |   2 +
 gcc/gtype-desc.in.456      |   2 +
 gcc/gtype-desc.in.457      |   2 +
 gcc/gtype-desc.in.458      |   2 +
 gcc/gtype-desc.in.459      |   2 +
 gcc/gtype-desc.in.460      |   2 +
 gcc/gtype-desc.in.461      |   2 +
 gcc/gtype-desc.in.462      |   2 +
 gcc/gtype-desc.in.463      |   2 +
 gcc/gtype-desc.in.464      |   2 +
 gcc/gtype-desc.in.465      |   2 +
 gcc/gtype-desc.in.466      |   2 +
 gcc/gtype-desc.in.467      |   2 +
 gcc/gtype-desc.in.468      |   2 +
 gcc/gtype-desc.in.469      |   2 +
 gcc/gtype-desc.in.470      |   2 +
 gcc/gtype-desc.in.471      |   2 +
 gcc/gtype-desc.in.472      |   2 +
 gcc/gtype-desc.in.473      |   2 +
 gcc/gtype-desc.in.474      |   2 +
 gcc/gtype-desc.in.475      |   2 +
 gcc/gtype-desc.in.476      |   2 +
 gcc/gtype-desc.in.477      |   2 +
 gcc/gtype-desc.in.478      |   2 +
 gcc/gtype-desc.in.479      |   2 +
 gcc/gtype-desc.in.480      |   2 +
 gcc/gtype-desc.in.481      |   2 +
 gcc/gtype-desc.in.482      |   2 +
 gcc/gtype-desc.in.483      |   2 +
 gcc/gtype-desc.in.484      |   2 +
 gcc/gtype-desc.in.485      |   2 +
 gcc/gtype-desc.in.486      |   2 +
 gcc/gtype-desc.in.487      |   2 +
 gcc/gtype-desc.in.488      |   2 +
 gcc/gtype-desc.in.489      |   2 +
 gcc/gtype-desc.in.490      |   2 +
 gcc/gtype-desc.in.491      |   2 +
 gcc/gtype-desc.in.492      |   2 +
 gcc/gtype-desc.in.493      |   2 +
 gcc/gtype-desc.in.494      |   2 +
 gcc/gtype-desc.in.495      |   2 +
 gcc/gtype-desc.in.496      |   2 +
 gcc/gtype-desc.in.497      |   2 +
 gcc/gtype-desc.in.498      |   2 +
 gcc/gtype-desc.in.499      |   2 +
 gcc/gtype-desc.in.500      |   2 +
 501 files changed, 1004 insertions(+)
 create mode 100644 gcc/gt-complex-test.h
 create mode 100644 gcc/gt-nested-types.h

diff --git a/gcc/gt-complex-test.h b/gcc/gt-complex-test.h
new file mode 100644
index 00000000000..e69de29bb2d
--- /dev/null
+++ b/gcc/gt-complex-test.h
@@ -0,0 +1,183 @@
+/* Complex type definitions to exercise gengtype parser - specifically
+   the consume_balanced() calls for '(', '[', '{' and default case.  */
+
+#ifndef GCC_GT_COMPLEX_TEST_H
+#define GCC_GT_COMPLEX_TEST_H
+
+/* Macro magic to generate nested balanced tokens */
+#define CONCAT(a, b) a##b
+#define STRINGIFY(x) #x
+#define PAREN_OPEN (
+#define PAREN_CLOSE )
+#define BRACKET_OPEN [
+#define BRACKET_CLOSE ]
+#define BRACE_OPEN {
+#define BRACE_CLOSE }
+
+/* Recursive macro with guard to avoid infinite expansion */
+#define NEST_1(x) (x)
+#define NEST_2(x) (NEST_1(x))
+#define NEST_3(x) (NEST_2(x))
+#define NEST_4(x) (NEST_3(x))
+#define NEST_5(x) (NEST_4(x))
+#define NEST_6(x) (NEST_5(x))
+#define NEST_7(x) (NEST_6(x))
+#define NEST_8(x) (NEST_7(x))
+#define NEST_9(x) (NEST_8(x))
+#define NEST_10(x) (NEST_9(x))
+
+/* Array dimension macro */
+#define ARR(type, dim) type[dim]
+#define ARR2(type, d1, d2) type[d1][d2]
+#define ARR3(type, d1, d2, d3) type[d1][d2][d3]
+
+/* Function pointer macro */
+#define FP(ret, args) ret (*) args
+#define FP_ARRAY(ret, args, dim) ret (*[dim]) args
+
+/* Complex nested type 1: Function pointer returning array of function pointers */
+typedef int (*(*complex_fp_1)(int (*(*)(int[2]))[3]))(void);
+
+/* Complex nested type 2: Array of pointers to functions returning pointers to arrays */
+typedef int (*(*complex_fp_2[5])(void))[10];
+
+/* Complex nested type 3: Deeply nested function pointers with arrays */
+typedef int (*(*(*complex_fp_3)(int (*)(int[2][3])[4]))[5])(int, int);
+
+/* Complex nested type 4: Function taking function pointer returning function pointer */
+typedef int (*(*complex_fp_4)(int (*)(int)))(int, int, int);
+
+/* Structure with deeply nested bit-fields and function pointers */
+struct complex_struct_1 {
+  /* This will trigger consume_balanced for '(' */
+  int (*fp1)(int (*)(int[2])[3]);
+  
+  /* This will trigger consume_balanced for '[' */
+  int arr1[5][10][15];
+  
+  /* This will trigger consume_balanced for '{' */
+  union {
+    struct {
+      int x;
+      int y;
+    } nested;
+    long long ll;
+  } u;
+  
+  /* Bit-field with unusual qualifiers to trigger default case */
+  _Atomic volatile const long long int bf1 : 5;
+  unsigned _BitInt(128) bf2 : 64;
+  
+  /* Nested anonymous struct */
+  struct {
+    /* Array of function pointers */
+    int (*(*fp_array[3])(int))[2];
+    
+    /* Nested union with anonymous struct */
+    union {
+      struct {
+        int a;
+        int b;
+      };
+      double d;
+    };
+  };
+};
+
+/* Union with complex nested types */
+union complex_union_1 {
+  /* Function pointer with nested arrays */
