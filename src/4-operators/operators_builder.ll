; ModuleID = 'operators.c'
source_filename = "operators.c"
target triple = "arm64-apple-darwin25.6.0"

define i32 @main() {
entry:
  %a = alloca i32, align 4
  store i32 17, ptr %a, align 4
  %b = alloca i32, align 4
  store i32 5, ptr %b, align 4
  %ua = alloca i32, align 4
  store i32 17, ptr %ua, align 4
  %ub = alloca i32, align 4
  store i32 5, ptr %ub, align 4
  %x = alloca double, align 8
  store double 3.140000e+00, ptr %x, align 8
  %y = alloca double, align 8
  store double 2.000000e+00, ptr %y, align 8
  %a.val = load i32, ptr %a, align 4
  %b.val = load i32, ptr %b, align 4
  %ua.val = load i32, ptr %ua, align 4
  %ub.val = load i32, ptr %ub, align 4
  %x.val = load double, ptr %x, align 8
  %y.val = load double, ptr %y, align 8
  %sum = alloca i32, align 4
  %add = add i32 %a.val, %b.val
  store i32 %add, ptr %sum, align 4
  %diff = alloca i32, align 4
  %sub = sub i32 %a.val, %b.val
  store i32 %sub, ptr %diff, align 4
  %prod = alloca i32, align 4
  %mul = mul i32 %a.val, %b.val
  store i32 %mul, ptr %prod, align 4
  %quot = alloca i32, align 4
  %sdiv = sdiv i32 %a.val, %b.val
  store i32 %sdiv, ptr %quot, align 4
  %rem = alloca i32, align 4
  %srem = srem i32 %a.val, %b.val
  store i32 %srem, ptr %rem, align 4
  %uquot = alloca i32, align 4
  %udiv = udiv i32 %ua.val, %ub.val
  store i32 %udiv, ptr %uquot, align 4
  %band = alloca i32, align 4
  %and = and i32 %a.val, %b.val
  store i32 %and, ptr %band, align 4
  %bor = alloca i32, align 4
  %or = or i32 %a.val, %b.val
  store i32 %or, ptr %bor, align 4
  %bxor = alloca i32, align 4
  %xor = xor i32 %a.val, %b.val
  store i32 %xor, ptr %bxor, align 4
  %lsh = alloca i32, align 4
  %shl = shl i32 %a.val, 2
  store i32 %shl, ptr %lsh, align 4
  %rsh = alloca i32, align 4
  %ashr = ashr i32 %a.val, 2
  store i32 %ashr, ptr %rsh, align 4
  %ursh = alloca i32, align 4
  %lshr = lshr i32 %ua.val, 2
  store i32 %lshr, ptr %ursh, align 4
  %lt = alloca i32, align 4
  %cmp.slt = icmp slt i32 %a.val, %b.val
  %lt.conv = zext i1 %cmp.slt to i32
  store i32 %lt.conv, ptr %lt, align 4
  %eq = alloca i32, align 4
  %cmp.eq = icmp eq i32 %a.val, %b.val
  %eq.conv = zext i1 %cmp.eq to i32
  store i32 %eq.conv, ptr %eq, align 4
  %flt_lt = alloca i32, align 4
  %cmp.olt = fcmp olt double %x.val, %y.val
  %flt_lt.conv = zext i1 %cmp.olt to i32
  store i32 %flt_lt.conv, ptr %flt_lt, align 4
  %neg = alloca i32, align 4
  %neg.val = sub nsw i32 0, %a.val
  store i32 %neg.val, ptr %neg, align 4
  %bnot = alloca i32, align 4
  %not.val = xor i32 %a.val, -1
  store i32 %not.val, ptr %bnot, align 4
  %lnot = alloca i32, align 4
  %tobool = icmp ne i32 %a.val, 0
  %lnot.i1 = xor i1 %tobool, true
  %lnot.ext = zext i1 %lnot.i1 to i32
  store i32 %lnot.ext, ptr %lnot, align 4
  %dneg = alloca double, align 8
  %fneg = fneg double %x.val
  store double %fneg, ptr %dneg, align 8
  %dsum = alloca double, align 8
  %fadd = fadd double %x.val, %y.val
  store double %fadd, ptr %dsum, align 8
  %ddiff = alloca double, align 8
  %fsub = fsub double %x.val, %y.val
  store double %fsub, ptr %ddiff, align 8
  %dprod = alloca double, align 8
  %fmul = fmul double %x.val, %y.val
  store double %fmul, ptr %dprod, align 8
  %dquot = alloca double, align 8
  %fdiv = fdiv double %x.val, %y.val
  store double %fdiv, ptr %dquot, align 8
  %uadd = alloca i32, align 4
  %uadd.val = add i32 %ua.val, 1
  store i32 %uadd.val, ptr %uadd, align 4
  %sadd = alloca i32, align 4
  %sadd.val = add nsw i32 %a.val, 1
  store i32 %sadd.val, ptr %sadd, align 4
  %ssub = alloca i32, align 4
  %ssub.val = sub nsw i32 %a.val, 1
  store i32 %ssub.val, ptr %ssub, align 4
  %smul = alloca i32, align 4
  %smul.val = mul nsw i32 %a.val, 2
  store i32 %smul.val, ptr %smul, align 4
  %ret.val = load i32, ptr %sum, align 4
  ret i32 %ret.val
}
