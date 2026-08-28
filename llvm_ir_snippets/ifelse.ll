declare i32 @printf(ptr, ...)

@fmt = private unnamed_addr constant [4 x i8] c"%d\0A\00"

define i32 @main() {
entry:
  %int_val = alloca i32
  store i32 22, ptr %int_val
  %val = load i32, ptr %int_val

  %cond = icmp slt i32 %val, 3
  br i1 %cond, label %if.then, label %if.else

if.then:
  call i32 (ptr, ...) @printf(ptr @fmt, i32 30) 
  br label %if.end

if.else:
  %cond2 = icmp sgt i32 %val, 3
  br i1 %cond, label %if.then2, label %if.else2

  call i32 (ptr, ...) @printf(ptr @fmt, i32 99) 
  br label %if.end

if.then2:
  call i32 (ptr, ...) @printf(ptr @fmt, i32 -123) 
  br label %if.end

if.else2:
  call i32 (ptr, ...) @printf(ptr @fmt, i32 -10) 
  br label %if.end

if.end:
  ret i32 0
}
