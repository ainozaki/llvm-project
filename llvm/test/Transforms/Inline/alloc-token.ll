; RUN: opt < %s -passes='cgscc(inline)' -S | FileCheck %s

declare ptr @malloc(i64)
declare ptr @helper()

define internal ptr @wrapper(i64 %size) alwaysinline {
  ; Not returned: must not inherit the token.
  %h = call ptr @helper()
  %p = call ptr @malloc(i64 %size)
  ret ptr %p
}

; CHECK-LABEL: define ptr @inherits(
; CHECK: call ptr @helper(){{$}}
; CHECK: call ptr @malloc(i64 4){{.*}}, !alloc_token [[MD:![0-9]+]]
define ptr @inherits() {
  %c = call ptr @wrapper(i64 4), !alloc_token !0
  ret ptr %c
}

define internal ptr @wrapper_own(i64 %size) alwaysinline {
  %p = call ptr @malloc(i64 %size), !alloc_token !1
  ret ptr %p
}

; Metadata the wrapper set itself is never overwritten.
; CHECK-LABEL: define ptr @no_overwrite(
; CHECK: call ptr @malloc(i64 4){{.*}}, !alloc_token [[OWN:![0-9]+]]
define ptr @no_overwrite() {
  %c = call ptr @wrapper_own(i64 4), !alloc_token !0
  ret ptr %c
}

; All three operands are inherited when the inner allocation has no metadata.
; CHECK-LABEL: define ptr @inherits_function(
; CHECK: call ptr @malloc(i64 4){{.*}}, !alloc_token [[FUNC:![0-9]+]]
define ptr @inherits_function() {
  %p = call ptr @wrapper(i64 4), !alloc_token !2
  ret ptr %p
}

define internal ptr @wrapper_empty_type(i64 %size) alwaysinline {
  %p = call ptr @malloc(i64 %size), !alloc_token !3
  ret ptr %p
}

; Metadata with an empty type name is overwritten.
; CHECK-LABEL: define ptr @overwrites_empty_type(
; CHECK: call ptr @malloc(i64 4){{.*}}, !alloc_token [[FUNC]]
define ptr @overwrites_empty_type() {
  %p = call ptr @wrapper_empty_type(i64 4), !alloc_token !2
  ret ptr %p
}

; CHECK-DAG: [[MD]] = !{!"Outer", i1 true}
; CHECK-DAG: [[OWN]] = !{!"Inner", i1 false}
; CHECK-DAG: [[FUNC]] = !{!"int", i1 false, !"ns::foo"}
!0 = !{!"Outer", i1 true}
!1 = !{!"Inner", i1 false}
!2 = !{!"int", i1 false, !"ns::foo"}
!3 = !{!"", i1 false, !"wrapper"}
