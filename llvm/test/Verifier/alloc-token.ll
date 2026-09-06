; RUN: not opt -passes=verify -disable-output %s 2>&1 | FileCheck %s
; CHECK: expected function name string
; CHECK: !alloc_token must have 2 or 3 operands

declare ptr @malloc(i64)
define void @test() {
  %a = call ptr @malloc(i64 4), !alloc_token !0
  %b = call ptr @malloc(i64 4), !alloc_token !1
  ret void
}
!0 = !{!"int", i1 false, null}
!1 = !{!"int", i1 false, !"foo", !"bar"}
