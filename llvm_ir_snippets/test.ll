declare i32 @printf(ptr, ...)

define i32 @add(i32 %a, i8 %b) {
entry:
  %cond = icmp eq i32 %a, %b
  br i1 %cond, label %IfEqual, label %IfUnequal
IfEqual:
  ret i32 10
IfUnequal:
  ret i32 1
}

define i32 @loop_square(i32 %base) {
entry:
  %base.cond = alloca i32
  store i32 %base, ptr %base.cond
  %a = load i32, ptr %base.cond  
  %sub = sub nsw i32 %a, 1
  store i32 %sub, ptr %base.cond

  %base.addr = alloca i32
  %i = alloca i32
  store i32 %base, ptr %base.addr
  store i32 0, ptr %i
  br label %for.cond

for.cond:
  %0 = load i32, ptr %i
  %1 = load i32, ptr %base.cond
  %cmp = icmp slt i32 %0, %1
  br i1 %cmp, label %for.body, label %for.end

for.body:
  %2 = load i32, ptr %base.addr
  %add = add nsw i32 %2, %base
  store i32 %add, ptr %base.addr
  br label %for.inc

for.inc:
  %4 = load i32, ptr %i
  %inc = add nsw i32 %4, 1
  store i32 %inc, ptr %i
  br label %for.cond
  
for.end:
  %5 = load i32, ptr %base.addr
  ret i32 %5
}

@fmt = private unnamed_addr constant [4 x i8] c"%d\0A\00"

define i32 @main() {
entry:
  %0 = call i32 @add(i32 6, i8 10)
  %1 = call i32 (ptr, ...) @printf(ptr @fmt, i32 %0)
  ret i32 0
}
