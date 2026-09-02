// RUN: %clang_cc1 -fsanitize=alloc-token -falloc-token-mode=typesitehashpointersplit -triple x86_64-linux-gnu -std=c++20 -emit-llvm -disable-llvm-passes %s -o - | FileCheck %s
// RUN: %clang_cc1 -fsanitize=alloc-token -falloc-token-mode=typesitehashpointersplit -fmacro-prefix-map=alloc-token-site-input.cpp=remapped.cpp -triple x86_64-linux-gnu -std=c++20 -emit-llvm -disable-llvm-passes %s -o - | FileCheck %s --check-prefix=REMAP

using size_t = decltype(sizeof(0));
extern "C" void *malloc(size_t) __attribute__((malloc, alloc_size(1)));

#line 100 "alloc-token-site-input.cpp"
int *first() {
  return static_cast<int *>(malloc(sizeof(int)));
}

int *second() {
  return static_cast<int *>(malloc(sizeof(int)));
}

#define ALLOC_INT() static_cast<int *>(malloc(sizeof(int)))
int *from_macro() {
  return ALLOC_INT();
}

int *from_new() {
  return new int;
}

// CHECK-LABEL: define{{.*}} ptr @_Z5firstv()
// CHECK: call{{.*}} @malloc{{.*}} !alloc_token [[FIRST:![0-9]+]]
// CHECK-LABEL: define{{.*}} ptr @_Z6secondv()
// CHECK: call{{.*}} @malloc{{.*}} !alloc_token [[SECOND:![0-9]+]]
// CHECK-LABEL: define{{.*}} ptr @_Z10from_macrov()
// CHECK: call{{.*}} @malloc{{.*}} !alloc_token [[MACRO:![0-9]+]]
// CHECK-LABEL: define{{.*}} ptr @_Z8from_newv()
// CHECK: call{{.*}} @_Znwm{{.*}} !alloc_token [[NEW:![0-9]+]]

// CHECK-DAG: [[FIRST]] = !{!"int", i1 false, !"alloc-token-site-input.cpp:101:29"}
// CHECK-DAG: [[SECOND]] = !{!"int", i1 false, !"alloc-token-site-input.cpp:105:29"}
// CHECK-DAG: [[MACRO]] = !{!"int", i1 false, !"alloc-token-site-input.cpp:110:10@alloc-token-site-input.cpp:108:40"}
// CHECK-DAG: [[NEW]] = !{!"int", i1 false, !"alloc-token-site-input.cpp:114:10"}

// REMAP: !{!"int", i1 false, !"remapped.cpp:101:29"}
