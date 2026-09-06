// RUN: %clang_cc1 -triple x86_64-linux-gnu -std=c++17 -fsanitize=alloc-token -falloc-token-mode=typefunchash -emit-llvm -disable-llvm-passes %s -o - | FileCheck %s --check-prefixes=MD,HASHFLAG
// RUN: %clang_cc1 -triple x86_64-linux-gnu -std=c++17 -fsanitize=alloc-token -falloc-token-mode=typefunchashpointersplit -emit-llvm -disable-llvm-passes %s -o - | FileCheck %s --check-prefixes=MD,SPLITFLAG
// RUN: %clang_cc1 -triple x86_64-linux-gnu -std=c++17 -fsanitize=alloc-token -falloc-token-mode=typefunchash -emit-llvm %s -o - | FileCheck %s --check-prefix=LOWER

using size_t = decltype(sizeof(int));
extern "C" void *malloc(size_t);
void *sink;

namespace ns {
// MD-LABEL: define {{.*}} @_ZN2ns3fooEv(
// MD: call {{.*}} @malloc({{.*}}){{.*}}, !alloc_token [[FOO:![0-9]+]]
// MD: call {{.*}} @_Znwm({{.*}}){{.*}}, !alloc_token [[FOO]]
// MD: call {{.*}} @_Znam({{.*}}){{.*}}, !alloc_token [[FOO]]
// LOWER-LABEL: define {{.*}} @_ZN2ns3fooEv(
// LOWER: @__alloc_token_malloc({{.*}}i64 -7809983928432415842)
void foo() {
  sink = malloc(sizeof(int));
  sink = new int;
  sink = new int[2];
}

// Overloads intentionally share the same qualified function name.
// MD-LABEL: define {{.*}} @_ZN2ns3fooEi(
// MD: call {{.*}} @_Znwm({{.*}}){{.*}}, !alloc_token [[FOO]]
void foo(int) { sink = new int; }

// MD-LABEL: define {{.*}} @_ZN2ns3barEv(
// MD: call {{.*}} @_Znwm({{.*}}){{.*}}, !alloc_token [[BAR:![0-9]+]]
void bar() { sink = new int; }

// MD-LABEL: define {{.*}} @_ZN2ns7pointerEv(
// MD: call {{.*}} @_Znwm({{.*}}){{.*}}, !alloc_token [[PTR:![0-9]+]]
void pointer() { sink = new int *; }
}

// Generated global initialization function names are not part of the token.
// MD: call {{.*}} @_Znwm({{.*}}){{.*}}, !alloc_token [[GLOBAL:![0-9]+]]
// LOWER: @__alloc_token__Znwm({{.*}}i64 -6646512383795279905)
int *global = new int;

template<class T> struct Box {
  static void allocate() { sink = new int; }
};
void instantiate() {
  Box<int>::allocate();
  Box<char>::allocate();
}

struct S {
  int *p = new int;
  S() { sink = new int; }
  ~S() { sink = new int; }
};
void construct() { S s; }

int *default_arg(int *p = new int) { return p; }
void caller() { sink = default_arg(); }

void lambda_test() { [] { sink = new int; }(); }

void local_static() {
  static int *p = new int;
  sink = p;
}

// A default member initializer is written outside a function, even though its
// allocation is emitted in the constructor alongside the constructor body.
// MD-LABEL: define {{.*}} @_ZN1SC2Ev(
// MD: call {{.*}} @_Znwm({{.*}}){{.*}}, !alloc_token [[GLOBAL]]
// MD: call {{.*}} @_Znwm({{.*}}){{.*}}, !alloc_token [[CTOR:![0-9]+]]

// HASHFLAG-DAG: !{i32 1, !"alloc-token-mode", !"typefunchash"}
// SPLITFLAG-DAG: !{i32 1, !"alloc-token-mode", !"typefunchashpointersplit"}
// MD-DAG: [[FOO]] = !{!"int", i1 false, !"ns::foo"}
// MD-DAG: [[BAR]] = !{!"int", i1 false, !"ns::bar"}
// MD-DAG: [[PTR]] = !{!"int *", i1 true, !"ns::pointer"}
// MD-DAG: [[GLOBAL]] = !{!"int", i1 false, !""}
// MD-DAG: !{!"int", i1 false, !"Box<int>::allocate"}
// MD-DAG: !{!"int", i1 false, !"Box<char>::allocate"}
// MD-DAG: [[CTOR]] = !{!"int", i1 false, !"S::S"}
// MD-DAG: !{!"int", i1 false, !"S::~S"}
// MD-DAG: !{!"int", i1 false, !"default_arg"}
// MD-DAG: !{!"int", i1 false, !"lambda_test()::(lambda)::operator()"}
// MD-DAG: !{!"int", i1 false, !"local_static"}
