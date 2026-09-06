; RUN: opt < %s -passes=simplifycfg -S | FileCheck %s

declare ptr @malloc(i64)

; CHECK-LABEL: define ptr @same_type(
; CHECK: call ptr @malloc(i64 8), !alloc_token [[SAME_TYPE:![0-9]+]]
define ptr @same_type(i1 %cond) {
  br i1 %cond, label %left, label %right
left:
  %a = call ptr @malloc(i64 8), !alloc_token !0
  ret ptr %a
right:
  %b = call ptr @malloc(i64 8), !alloc_token !1
  ret ptr %b
}

; CHECK-LABEL: define ptr @same_function(
; CHECK: call ptr @malloc(i64 8), !alloc_token [[SAME_FUNCTION:![0-9]+]]
define ptr @same_function(i1 %cond) {
  br i1 %cond, label %left, label %right
left:
  %a = call ptr @malloc(i64 8), !alloc_token !2
  ret ptr %a
right:
  %b = call ptr @malloc(i64 8), !alloc_token !3
  ret ptr %b
}

; CHECK-LABEL: define ptr @different_both(
; CHECK: call ptr @malloc(i64 8), !alloc_token [[DIFFERENT_BOTH:![0-9]+]]
define ptr @different_both(i1 %cond) {
  br i1 %cond, label %left, label %right
left:
  %a = call ptr @malloc(i64 8), !alloc_token !2
  ret ptr %a
right:
  %b = call ptr @malloc(i64 8), !alloc_token !4
  ret ptr %b
}

; CHECK-LABEL: define ptr @legacy(
; CHECK: call ptr @malloc(i64 8), !alloc_token [[LEGACY:![0-9]+]]
define ptr @legacy(i1 %cond) {
  br i1 %cond, label %left, label %right
left:
  %a = call ptr @malloc(i64 8), !alloc_token !0
  ret ptr %a
right:
  %b = call ptr @malloc(i64 8), !alloc_token !5
  ret ptr %b
}

!0 = !{!"int", i1 false, !"foo"}
!1 = !{!"int", i1 true, !"bar"}
!2 = !{!"int", i1 false, !""}
!3 = !{!"char", i1 false, !""}
!4 = !{!"char", i1 true, !"bar"}
!5 = !{!"int", i1 true}

; CHECK-DAG: [[SAME_TYPE]] = !{!"int", i1 true, !"foo|bar"}
; CHECK-DAG: [[SAME_FUNCTION]] = !{!"int|char", i1 false, !""}
; CHECK-DAG: [[DIFFERENT_BOTH]] = !{!"int|char", i1 true, !"|bar"}
; CHECK-DAG: [[LEGACY]] = !{!"int", i1 true}
