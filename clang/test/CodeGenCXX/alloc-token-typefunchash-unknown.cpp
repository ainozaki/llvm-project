// RUN: %clang_cc1 -triple x86_64-linux-gnu -fsanitize=alloc-token -falloc-token-mode=typefunchash -falloc-token-max=1000 -mllvm -alloc-token-fallback=7 -emit-llvm %s -o - | FileCheck %s --check-prefix=HASH -DFOO_TOKEN=12 -DBAR_TOKEN=1 -DGLOBAL_TOKEN=1 -DINCOMPLETE_TOKEN=3
// RUN: %clang_cc1 -triple x86_64-linux-gnu -fsanitize=alloc-token -falloc-token-mode=typefunchashpointersplit -falloc-token-max=10 -mllvm -alloc-token-fallback=7 -emit-llvm %s -o - | FileCheck %s --check-prefix=HASH -DFOO_TOKEN=0 -DBAR_TOKEN=1 -DGLOBAL_TOKEN=1 -DINCOMPLETE_TOKEN=1
// RUN: %clang_cc1 -triple x86_64-linux-gnu -fsanitize=alloc-token -falloc-token-mode=typehash -mllvm -alloc-token-fallback=7 -emit-llvm %s -o - | FileCheck %s --check-prefix=FALLBACK --implicit-check-not='!alloc_token'
// RUN: %clang_cc1 -triple x86_64-linux-gnu -fsanitize=alloc-token -falloc-token-mode=typehashpointersplit -mllvm -alloc-token-fallback=7 -emit-llvm %s -o - | FileCheck %s --check-prefix=FALLBACK --implicit-check-not='!alloc_token'

using size_t = decltype(sizeof(int));
extern "C" void *malloc(size_t);
void *sink;

struct Incomplete;

namespace ns {
// HASH-LABEL: define {{.*}} @_ZN2ns3fooEm(
// HASH: @__alloc_token_malloc({{.*}}, i64 [[FOO_TOKEN]]){{.*}} !alloc_token [[FOO:![0-9]+]]
// HASH: @__alloc_token_malloc({{.*}}, i64 [[FOO_TOKEN]]){{.*}} !alloc_token [[FOO]]
// FALLBACK-LABEL: define {{.*}} @_ZN2ns3fooEm(
// FALLBACK-COUNT-2: @__alloc_token_malloc({{.*}}, i64 7)
void foo(size_t size) {
  sink = malloc(size);
  sink = malloc(4096);
}

// HASH-LABEL: define {{.*}} @_ZN2ns3barEm(
// HASH: @__alloc_token_malloc({{.*}}, i64 [[BAR_TOKEN]]){{.*}} !alloc_token [[BAR:![0-9]+]]
// FALLBACK-LABEL: define {{.*}} @_ZN2ns3barEm(
// FALLBACK: @__alloc_token_malloc({{.*}}, i64 7)
void bar(size_t size) { sink = malloc(size); }

// Incomplete types have unknown pointer presence.
// HASH-LABEL: define {{.*}} @_ZN2ns14testIncompleteEv(
// HASH: @__alloc_token_malloc({{.*}}, i64 [[INCOMPLETE_TOKEN]]){{.*}} !alloc_token [[INCOMPLETE:![0-9]+]]
// FALLBACK-LABEL: define {{.*}} @_ZN2ns14testIncompleteEv(
// FALLBACK: @__alloc_token_malloc({{.*}}, i64 7)
void testIncomplete() { sink = (Incomplete *)malloc(16); }
}

// Both the type name and the source function name are empty at file scope.
// HASH: @__alloc_token_malloc({{.*}}, i64 [[GLOBAL_TOKEN]]){{.*}} !alloc_token [[GLOBAL:![0-9]+]]
// FALLBACK: @__alloc_token_malloc({{.*}}, i64 7)
void *global = malloc(4096);

// HASH-DAG: [[FOO]] = !{!"", i1 false, !"ns::foo"}
// HASH-DAG: [[BAR]] = !{!"", i1 false, !"ns::bar"}
// HASH-DAG: [[INCOMPLETE]] = !{!"", i1 false, !"ns::testIncomplete"}
// HASH-DAG: [[GLOBAL]] = !{!"", i1 false, !""}
