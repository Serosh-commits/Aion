; ModuleID = 'test/vectorize/vect_mixed_types.c'
source_filename = "test/vectorize/vect_mixed_types.c"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-i128:128-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux-gnu"

; Function Attrs: nofree norecurse nosync nounwind sspstrong memory(argmem: readwrite) uwtable
define dso_local void @test(ptr noundef writeonly captures(none) %0, ptr noundef readonly captures(none) %1, i32 noundef %2) local_unnamed_addr #0 !dbg !8 {
  %4 = icmp sgt i32 %2, 0, !dbg !11
  br i1 %4, label %5, label %28, !dbg !12

5:                                                ; preds = %3
  %6 = zext nneg i32 %2 to i64, !dbg !11
  %7 = icmp ult i32 %2, 4, !dbg !12
  br i1 %7, label %26, label %8, !dbg !12

8:                                                ; preds = %5
  %9 = and i64 %6, 2147483644, !dbg !12
  br label %10, !dbg !12

10:                                               ; preds = %10, %8
  %11 = phi i64 [ 0, %8 ], [ %22, %10 ], !dbg !13
  %12 = getelementptr inbounds nuw i32, ptr %1, i64 %11, !dbg !14
  %13 = getelementptr inbounds nuw i8, ptr %12, i64 8, !dbg !14
  %14 = load <2 x i32>, ptr %12, align 4, !dbg !14, !tbaa !15
  %15 = load <2 x i32>, ptr %13, align 4, !dbg !14, !tbaa !15
  %16 = sitofp <2 x i32> %14 to <2 x double>, !dbg !19
  %17 = sitofp <2 x i32> %15 to <2 x double>, !dbg !19
  %18 = fadd <2 x double> %16, splat (double 1.000000e+00), !dbg !20
  %19 = fadd <2 x double> %17, splat (double 1.000000e+00), !dbg !20
  %20 = getelementptr inbounds nuw double, ptr %0, i64 %11, !dbg !21
  %21 = getelementptr inbounds nuw i8, ptr %20, i64 16, !dbg !22
  store <2 x double> %18, ptr %20, align 8, !dbg !22, !tbaa !23
  store <2 x double> %19, ptr %21, align 8, !dbg !22, !tbaa !23
  %22 = add nuw i64 %11, 4, !dbg !13
  %23 = icmp eq i64 %22, %9, !dbg !13
  br i1 %23, label %24, label %10, !dbg !13, !llvm.loop !25

24:                                               ; preds = %10
  %25 = icmp eq i64 %9, %6, !dbg !12
  br i1 %25, label %28, label %26, !dbg !12

26:                                               ; preds = %5, %24
  %27 = phi i64 [ 0, %5 ], [ %9, %24 ]
  br label %29, !dbg !12

28:                                               ; preds = %29, %24, %3
  ret void, !dbg !30

29:                                               ; preds = %26, %29
  %30 = phi i64 [ %36, %29 ], [ %27, %26 ]
  %31 = getelementptr inbounds nuw i32, ptr %1, i64 %30, !dbg !14
  %32 = load i32, ptr %31, align 4, !dbg !14, !tbaa !15
  %33 = sitofp i32 %32 to double, !dbg !19
  %34 = fadd double %33, 1.000000e+00, !dbg !20
  %35 = getelementptr inbounds nuw double, ptr %0, i64 %30, !dbg !21
  store double %34, ptr %35, align 8, !dbg !22, !tbaa !23
  %36 = add nuw nsw i64 %30, 1, !dbg !13
  %37 = icmp eq i64 %36, %6, !dbg !11
  br i1 %37, label %28, label %29, !dbg !12, !llvm.loop !31
}

attributes #0 = { nofree norecurse nosync nounwind sspstrong memory(argmem: readwrite) uwtable "min-legal-vector-width"="0" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }

!llvm.dbg.cu = !{!0}
!llvm.module.flags = !{!2, !3, !4, !5, !6}
!llvm.ident = !{!7}

!0 = distinct !DICompileUnit(language: DW_LANG_C11, file: !1, producer: "clang version 21.1.8", isOptimized: true, runtimeVersion: 0, emissionKind: LineTablesOnly, splitDebugInlining: false, nameTableKind: None)
!1 = !DIFile(filename: "test/vectorize/vect_mixed_types.c", directory: "/home/serosh/scratch/aion")
!2 = !{i32 2, !"Debug Info Version", i32 3}
!3 = !{i32 1, !"wchar_size", i32 4}
!4 = !{i32 8, !"PIC Level", i32 2}
!5 = !{i32 7, !"PIE Level", i32 2}
!6 = !{i32 7, !"uwtable", i32 2}
!7 = !{!"clang version 21.1.8"}
!8 = distinct !DISubprogram(name: "test", scope: !1, file: !1, line: 1, type: !9, scopeLine: 1, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition | DISPFlagOptimized, unit: !0)
!9 = !DISubroutineType(types: !10)
!10 = !{}
!11 = !DILocation(line: 1, column: 57, scope: !8)
!12 = !DILocation(line: 1, column: 39, scope: !8)
!13 = !DILocation(line: 1, column: 63, scope: !8)
!14 = !DILocation(line: 1, column: 84, scope: !8)
!15 = !{!16, !16, i64 0}
!16 = !{!"int", !17, i64 0}
!17 = !{!"omnipotent char", !18, i64 0}
!18 = !{!"Simple C/C++ TBAA"}
!19 = !DILocation(line: 1, column: 76, scope: !8)
!20 = !DILocation(line: 1, column: 89, scope: !8)
!21 = !DILocation(line: 1, column: 69, scope: !8)
!22 = !DILocation(line: 1, column: 74, scope: !8)
!23 = !{!24, !24, i64 0}
!24 = !{!"double", !17, i64 0}
!25 = distinct !{!25, !12, !26, !27, !28, !29}
!26 = !DILocation(line: 1, column: 96, scope: !8)
!27 = !{!"llvm.loop.mustprogress"}
!28 = !{!"llvm.loop.isvectorized", i32 1}
!29 = !{!"llvm.loop.unroll.runtime.disable"}
!30 = !DILocation(line: 1, column: 98, scope: !8)
!31 = distinct !{!31, !12, !26, !27, !29, !28}
