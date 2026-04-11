; ModuleID = 'test/vectorize/vect_integer_division.c'
source_filename = "test/vectorize/vect_integer_division.c"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-i128:128-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux-gnu"

; Function Attrs: nofree norecurse nosync nounwind sspstrong memory(argmem: readwrite) uwtable
define dso_local void @test(ptr noundef captures(none) %0, ptr noundef readonly captures(none) %1, i32 noundef %2) local_unnamed_addr #0 !dbg !8 {
  %4 = icmp sgt i32 %2, 0, !dbg !11
  br i1 %4, label %5, label %20, !dbg !12

5:                                                ; preds = %3
  %6 = zext nneg i32 %2 to i64, !dbg !11
  %7 = and i64 %6, 1, !dbg !12
  %8 = icmp eq i32 %2, 1, !dbg !12
  br i1 %8, label %11, label %9, !dbg !12

9:                                                ; preds = %5
  %10 = and i64 %6, 2147483646, !dbg !12
  br label %21, !dbg !12

11:                                               ; preds = %21, %5
  %12 = phi i64 [ 0, %5 ], [ %35, %21 ]
  %13 = icmp eq i64 %7, 0, !dbg !12
  br i1 %13, label %20, label %14, !dbg !12

14:                                               ; preds = %11
  %15 = getelementptr inbounds nuw i32, ptr %1, i64 %12, !dbg !13
  %16 = load i32, ptr %15, align 4, !dbg !13, !tbaa !14
  %17 = getelementptr inbounds nuw i32, ptr %0, i64 %12, !dbg !18
  %18 = load i32, ptr %17, align 4, !dbg !19, !tbaa !14
  %19 = sdiv i32 %18, %16, !dbg !19
  store i32 %19, ptr %17, align 4, !dbg !19, !tbaa !14
  br label %20, !dbg !20

20:                                               ; preds = %14, %11, %3
  ret void, !dbg !20

21:                                               ; preds = %21, %9
  %22 = phi i64 [ 0, %9 ], [ %35, %21 ]
  %23 = phi i64 [ 0, %9 ], [ %36, %21 ]
  %24 = getelementptr inbounds nuw i32, ptr %1, i64 %22, !dbg !13
  %25 = load i32, ptr %24, align 4, !dbg !13, !tbaa !14
  %26 = getelementptr inbounds nuw i32, ptr %0, i64 %22, !dbg !18
  %27 = load i32, ptr %26, align 4, !dbg !19, !tbaa !14
  %28 = sdiv i32 %27, %25, !dbg !19
  store i32 %28, ptr %26, align 4, !dbg !19, !tbaa !14
  %29 = or disjoint i64 %22, 1, !dbg !21
  %30 = getelementptr inbounds nuw i32, ptr %1, i64 %29, !dbg !13
  %31 = load i32, ptr %30, align 4, !dbg !13, !tbaa !14
  %32 = getelementptr inbounds nuw i32, ptr %0, i64 %29, !dbg !18
  %33 = load i32, ptr %32, align 4, !dbg !19, !tbaa !14
  %34 = sdiv i32 %33, %31, !dbg !19
  store i32 %34, ptr %32, align 4, !dbg !19, !tbaa !14
  %35 = add nuw nsw i64 %22, 2, !dbg !21
  %36 = add i64 %23, 2, !dbg !12
  %37 = icmp eq i64 %36, %10, !dbg !12
  br i1 %37, label %11, label %21, !dbg !12, !llvm.loop !22
}

attributes #0 = { nofree norecurse nosync nounwind sspstrong memory(argmem: readwrite) uwtable "min-legal-vector-width"="0" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }

!llvm.dbg.cu = !{!0}
!llvm.module.flags = !{!2, !3, !4, !5, !6}
!llvm.ident = !{!7}

!0 = distinct !DICompileUnit(language: DW_LANG_C11, file: !1, producer: "clang version 21.1.8", isOptimized: true, runtimeVersion: 0, emissionKind: LineTablesOnly, splitDebugInlining: false, nameTableKind: None)
!1 = !DIFile(filename: "test/vectorize/vect_integer_division.c", directory: "/home/serosh/scratch/aion")
!2 = !{i32 2, !"Debug Info Version", i32 3}
!3 = !{i32 1, !"wchar_size", i32 4}
!4 = !{i32 8, !"PIC Level", i32 2}
!5 = !{i32 7, !"PIE Level", i32 2}
!6 = !{i32 7, !"uwtable", i32 2}
!7 = !{!"clang version 21.1.8"}
!8 = distinct !DISubprogram(name: "test", scope: !1, file: !1, line: 1, type: !9, scopeLine: 1, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition | DISPFlagOptimized, unit: !0)
!9 = !DISubroutineType(types: !10)
!10 = !{}
!11 = !DILocation(line: 1, column: 54, scope: !8)
!12 = !DILocation(line: 1, column: 36, scope: !8)
!13 = !DILocation(line: 1, column: 74, scope: !8)
!14 = !{!15, !15, i64 0}
!15 = !{!"int", !16, i64 0}
!16 = !{!"omnipotent char", !17, i64 0}
!17 = !{!"Simple C/C++ TBAA"}
!18 = !DILocation(line: 1, column: 66, scope: !8)
!19 = !DILocation(line: 1, column: 71, scope: !8)
!20 = !DILocation(line: 1, column: 82, scope: !8)
!21 = !DILocation(line: 1, column: 60, scope: !8)
!22 = distinct !{!22, !12, !23, !24}
!23 = !DILocation(line: 1, column: 80, scope: !8)
!24 = !{!"llvm.loop.mustprogress"}
