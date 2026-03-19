; ModuleID = 'test/vectorize/vect_pointer_iv.c'
source_filename = "test/vectorize/vect_pointer_iv.c"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-i128:128-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux-gnu"

; Function Attrs: nofree norecurse nosync nounwind sspstrong memory(write, argmem: readwrite, inaccessiblemem: none) uwtable
define dso_local void @test(ptr noundef readonly captures(none) %0, i32 noundef %1) local_unnamed_addr #0 !dbg !8 {
  %3 = icmp sgt i32 %1, 0, !dbg !11
  br i1 %3, label %4, label %22, !dbg !12

4:                                                ; preds = %2
  %5 = zext nneg i32 %1 to i64, !dbg !11
  %6 = and i64 %5, 7, !dbg !12
  %7 = icmp ult i32 %1, 8, !dbg !12
  br i1 %7, label %10, label %8, !dbg !12

8:                                                ; preds = %4
  %9 = and i64 %5, 2147483640, !dbg !12
  br label %23, !dbg !12

10:                                               ; preds = %23, %4
  %11 = phi i64 [ 0, %4 ], [ %57, %23 ]
  %12 = icmp eq i64 %6, 0, !dbg !12
  br i1 %12, label %22, label %13, !dbg !12

13:                                               ; preds = %10, %13
  %14 = phi i64 [ %19, %13 ], [ %11, %10 ]
  %15 = phi i64 [ %20, %13 ], [ 0, %10 ]
  %16 = getelementptr inbounds nuw ptr, ptr %0, i64 %14, !dbg !13
  %17 = load ptr, ptr %16, align 8, !dbg !13, !tbaa !14
  %18 = trunc nuw nsw i64 %14 to i32, !dbg !19
  store i32 %18, ptr %17, align 4, !dbg !19, !tbaa !20
  %19 = add nuw nsw i64 %14, 1, !dbg !22
  %20 = add i64 %15, 1, !dbg !12
  %21 = icmp eq i64 %20, %6, !dbg !12
  br i1 %21, label %22, label %13, !dbg !12, !llvm.loop !23

22:                                               ; preds = %10, %13, %2
  ret void, !dbg !25

23:                                               ; preds = %23, %8
  %24 = phi i64 [ 0, %8 ], [ %57, %23 ]
  %25 = phi i64 [ 0, %8 ], [ %58, %23 ]
  %26 = getelementptr inbounds nuw ptr, ptr %0, i64 %24, !dbg !13
  %27 = load ptr, ptr %26, align 8, !dbg !13, !tbaa !14
  %28 = trunc nuw nsw i64 %24 to i32, !dbg !19
  store i32 %28, ptr %27, align 4, !dbg !19, !tbaa !20
  %29 = or disjoint i64 %24, 1, !dbg !22
  %30 = getelementptr inbounds nuw ptr, ptr %0, i64 %29, !dbg !13
  %31 = load ptr, ptr %30, align 8, !dbg !13, !tbaa !14
  %32 = trunc nuw nsw i64 %29 to i32, !dbg !19
  store i32 %32, ptr %31, align 4, !dbg !19, !tbaa !20
  %33 = or disjoint i64 %24, 2, !dbg !22
  %34 = getelementptr inbounds nuw ptr, ptr %0, i64 %33, !dbg !13
  %35 = load ptr, ptr %34, align 8, !dbg !13, !tbaa !14
  %36 = trunc nuw nsw i64 %33 to i32, !dbg !19
  store i32 %36, ptr %35, align 4, !dbg !19, !tbaa !20
  %37 = or disjoint i64 %24, 3, !dbg !22
  %38 = getelementptr inbounds nuw ptr, ptr %0, i64 %37, !dbg !13
  %39 = load ptr, ptr %38, align 8, !dbg !13, !tbaa !14
  %40 = trunc nuw nsw i64 %37 to i32, !dbg !19
  store i32 %40, ptr %39, align 4, !dbg !19, !tbaa !20
  %41 = or disjoint i64 %24, 4, !dbg !22
  %42 = getelementptr inbounds nuw ptr, ptr %0, i64 %41, !dbg !13
  %43 = load ptr, ptr %42, align 8, !dbg !13, !tbaa !14
  %44 = trunc nuw nsw i64 %41 to i32, !dbg !19
  store i32 %44, ptr %43, align 4, !dbg !19, !tbaa !20
  %45 = or disjoint i64 %24, 5, !dbg !22
  %46 = getelementptr inbounds nuw ptr, ptr %0, i64 %45, !dbg !13
  %47 = load ptr, ptr %46, align 8, !dbg !13, !tbaa !14
  %48 = trunc nuw nsw i64 %45 to i32, !dbg !19
  store i32 %48, ptr %47, align 4, !dbg !19, !tbaa !20
  %49 = or disjoint i64 %24, 6, !dbg !22
  %50 = getelementptr inbounds nuw ptr, ptr %0, i64 %49, !dbg !13
  %51 = load ptr, ptr %50, align 8, !dbg !13, !tbaa !14
  %52 = trunc nuw nsw i64 %49 to i32, !dbg !19
  store i32 %52, ptr %51, align 4, !dbg !19, !tbaa !20
  %53 = or disjoint i64 %24, 7, !dbg !22
  %54 = getelementptr inbounds nuw ptr, ptr %0, i64 %53, !dbg !13
  %55 = load ptr, ptr %54, align 8, !dbg !13, !tbaa !14
  %56 = trunc nuw nsw i64 %53 to i32, !dbg !19
  store i32 %56, ptr %55, align 4, !dbg !19, !tbaa !20
  %57 = add nuw nsw i64 %24, 8, !dbg !22
  %58 = add i64 %25, 8, !dbg !12
  %59 = icmp eq i64 %58, %9, !dbg !12
  br i1 %59, label %10, label %23, !dbg !12, !llvm.loop !26
}

attributes #0 = { nofree norecurse nosync nounwind sspstrong memory(write, argmem: readwrite, inaccessiblemem: none) uwtable "min-legal-vector-width"="0" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }

!llvm.dbg.cu = !{!0}
!llvm.module.flags = !{!2, !3, !4, !5, !6}
!llvm.ident = !{!7}

!0 = distinct !DICompileUnit(language: DW_LANG_C11, file: !1, producer: "clang version 21.1.8", isOptimized: true, runtimeVersion: 0, emissionKind: LineTablesOnly, splitDebugInlining: false, nameTableKind: None)
!1 = !DIFile(filename: "test/vectorize/vect_pointer_iv.c", directory: "/home/serosh/scratch/aion")
!2 = !{i32 2, !"Debug Info Version", i32 3}
!3 = !{i32 1, !"wchar_size", i32 4}
!4 = !{i32 8, !"PIC Level", i32 2}
!5 = !{i32 7, !"PIE Level", i32 2}
!6 = !{i32 7, !"uwtable", i32 2}
!7 = !{!"clang version 21.1.8"}
!8 = distinct !DISubprogram(name: "test", scope: !1, file: !1, line: 1, type: !9, scopeLine: 1, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition | DISPFlagOptimized, unit: !0)
!9 = !DISubroutineType(types: !10)
!10 = !{}
!11 = !DILocation(line: 1, column: 47, scope: !8)
!12 = !DILocation(line: 1, column: 29, scope: !8)
!13 = !DILocation(line: 1, column: 60, scope: !8)
!14 = !{!15, !15, i64 0}
!15 = !{!"p1 int", !16, i64 0}
!16 = !{!"any pointer", !17, i64 0}
!17 = !{!"omnipotent char", !18, i64 0}
!18 = !{!"Simple C/C++ TBAA"}
!19 = !DILocation(line: 1, column: 65, scope: !8)
!20 = !{!21, !21, i64 0}
!21 = !{!"int", !17, i64 0}
!22 = !DILocation(line: 1, column: 53, scope: !8)
!23 = distinct !{!23, !24}
!24 = !{!"llvm.loop.unroll.disable"}
!25 = !DILocation(line: 1, column: 72, scope: !8)
!26 = distinct !{!26, !12, !27, !28}
!27 = !DILocation(line: 1, column: 70, scope: !8)
!28 = !{!"llvm.loop.mustprogress"}
