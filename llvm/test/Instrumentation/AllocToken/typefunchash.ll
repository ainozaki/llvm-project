; RUN: opt < %s -passes='inferattrs,alloc-token<mode=typefunchash>' -alloc-token-max=5 -alloc-token-fallback=7 -S | FileCheck %s --check-prefixes=CHECK,HASH
; RUN: opt < %s -passes='inferattrs,alloc-token<mode=typefunchashpointersplit>' -alloc-token-max=5 -alloc-token-fallback=7 -S | FileCheck %s --check-prefixes=CHECK,SPLIT
; RUN: opt < %s -passes='inferattrs,alloc-token<mode=typefunchashpointersplit>' -alloc-token-max=1 -alloc-token-fallback=7 -S | FileCheck %s --check-prefixes=CHECK,ONE
; RUN: opt < %s -passes='inferattrs,alloc-token<mode=typehash>' -S | FileCheck %s --check-prefix=LEGACY

target datalayout = "e-p:64:64"
declare ptr @malloc(i64)

; Source names come from metadata, not the current IR function. The first two
; calls differ only in function name; the third differs only in the pointer flag.
; An empty function name is valid, while legacy or absent metadata falls back.
; CHECK-LABEL: define void @test(
; HASH: call ptr @__alloc_token_malloc(i64 4, i64 4)
; HASH: call ptr @__alloc_token_malloc(i64 4, i64 0)
; HASH: call ptr @__alloc_token_malloc(i64 4, i64 4)
; HASH: call ptr @__alloc_token_malloc(i64 4, i64 1)
; SPLIT: call ptr @__alloc_token_malloc(i64 4, i64 0)
; SPLIT: call ptr @__alloc_token_malloc(i64 4, i64 1)
; SPLIT: call ptr @__alloc_token_malloc(i64 4, i64 2)
; SPLIT: call ptr @__alloc_token_malloc(i64 4, i64 1)
; ONE-COUNT-4: call ptr @__alloc_token_malloc(i64 4, i64 0)
; CHECK-COUNT-2: call ptr @__alloc_token_malloc(i64 4, i64 7)
; LEGACY-LABEL: define void @test(
; LEGACY-COUNT-5: call ptr @__alloc_token_malloc(i64 4, i64 2689373973731826898)
; LEGACY: call ptr @__alloc_token_malloc(i64 4, i64 0)
define void @test() sanitize_alloc_token {
  call ptr @malloc(i64 4), !alloc_token !0
  call ptr @malloc(i64 4), !alloc_token !1
  call ptr @malloc(i64 4), !alloc_token !2
  call ptr @malloc(i64 4), !alloc_token !3
  call ptr @malloc(i64 4), !alloc_token !4
  call ptr @malloc(i64 4)
  ret void
}

!0 = !{!"int", i1 false, !"ns::foo"}
!1 = !{!"int", i1 false, !"ns::bar"}
!2 = !{!"int", i1 true, !"ns::foo"}
!3 = !{!"int", i1 false, !""}
!4 = !{!"int", i1 false}
