; ModuleID = 'test/vectorize/vect_conditional_store.c'
source_filename = "test/vectorize/vect_conditional_store.c"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-i128:128-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux-gnu"

; Function Attrs: nofree norecurse nosync nounwind sspstrong memory(argmem: readwrite) uwtable
define dso_local void @test(ptr noundef readonly captures(none) %0, ptr noundef writeonly captures(none) %1, i32 noundef %2) local_unnamed_addr #0 !dbg !8 {
  %4 = icmp sgt i32 %2, 0, !dbg !11
  br i1 %4, label %5, label %90, !dbg !12

5:                                                ; preds = %3
  %6 = ptrtoint ptr %1 to i64, !dbg !11
  %7 = ptrtoint ptr %0 to i64, !dbg !11
  %8 = zext nneg i32 %2 to i64, !dbg !11
  %9 = icmp ult i32 %2, 8, !dbg !12
  %10 = sub i64 %6, %7, !dbg !12
  %11 = icmp ult i64 %10, 32, !dbg !12
  %12 = or i1 %9, %11, !dbg !12
  br i1 %12, label %74, label %13, !dbg !12

13:                                               ; preds = %5
  %14 = and i64 %8, 2147483640, !dbg !12
  %15 = getelementptr i8, ptr %1, i64 4, !dbg !12
  %16 = getelementptr i8, ptr %1, i64 8, !dbg !12
  %17 = getelementptr i8, ptr %1, i64 12, !dbg !12
  %18 = getelementptr i8, ptr %1, i64 16, !dbg !12
  %19 = getelementptr i8, ptr %1, i64 20, !dbg !12
  %20 = getelementptr i8, ptr %1, i64 24, !dbg !12
  %21 = getelementptr i8, ptr %1, i64 28, !dbg !12
  br label %22, !dbg !12

22:                                               ; preds = %69, %13
  %23 = phi i64 [ 0, %13 ], [ %70, %69 ], !dbg !13
  %24 = getelementptr inbounds nuw i32, ptr %0, i64 %23, !dbg !14
  %25 = getelementptr inbounds nuw i8, ptr %24, i64 16, !dbg !14
  %26 = load <4 x i32>, ptr %24, align 4, !dbg !14, !tbaa !15
  %27 = load <4 x i32>, ptr %25, align 4, !dbg !14, !tbaa !15
  %28 = icmp sgt <4 x i32> %26, zeroinitializer, !dbg !19
  %29 = icmp sgt <4 x i32> %27, zeroinitializer, !dbg !19
  %30 = extractelement <4 x i1> %28, i64 0, !dbg !19
  br i1 %30, label %31, label %34, !dbg !19

31:                                               ; preds = %22
  %32 = getelementptr inbounds nuw i32, ptr %1, i64 %23, !dbg !20
  %33 = extractelement <4 x i32> %26, i64 0, !dbg !21
  store i32 %33, ptr %32, align 4, !dbg !21, !tbaa !15
  br label %34, !dbg !19

34:                                               ; preds = %31, %22
  %35 = extractelement <4 x i1> %28, i64 1, !dbg !19
  br i1 %35, label %36, label %39, !dbg !19

36:                                               ; preds = %34
  %37 = getelementptr i32, ptr %15, i64 %23, !dbg !20
  %38 = extractelement <4 x i32> %26, i64 1, !dbg !21
  store i32 %38, ptr %37, align 4, !dbg !21, !tbaa !15
  br label %39, !dbg !19

39:                                               ; preds = %36, %34
  %40 = extractelement <4 x i1> %28, i64 2, !dbg !19
  br i1 %40, label %41, label %44, !dbg !19

41:                                               ; preds = %39
  %42 = getelementptr i32, ptr %16, i64 %23, !dbg !20
  %43 = extractelement <4 x i32> %26, i64 2, !dbg !21
  store i32 %43, ptr %42, align 4, !dbg !21, !tbaa !15
  br label %44, !dbg !19

44:                                               ; preds = %41, %39
  %45 = extractelement <4 x i1> %28, i64 3, !dbg !19
  br i1 %45, label %46, label %49, !dbg !19

46:                                               ; preds = %44
  %47 = getelementptr i32, ptr %17, i64 %23, !dbg !20
  %48 = extractelement <4 x i32> %26, i64 3, !dbg !21
  store i32 %48, ptr %47, align 4, !dbg !21, !tbaa !15
  br label %49, !dbg !19

49:                                               ; preds = %46, %44
  %50 = extractelement <4 x i1> %29, i64 0, !dbg !19
  br i1 %50, label %51, label %54, !dbg !19

51:                                               ; preds = %49
  %52 = getelementptr i32, ptr %18, i64 %23, !dbg !20
  %53 = extractelement <4 x i32> %27, i64 0, !dbg !21
  store i32 %53, ptr %52, align 4, !dbg !21, !tbaa !15
  br label %54, !dbg !19

54:                                               ; preds = %51, %49
  %55 = extractelement <4 x i1> %29, i64 1, !dbg !19
  br i1 %55, label %56, label %59, !dbg !19

56:                                               ; preds = %54
  %57 = getelementptr i32, ptr %19, i64 %23, !dbg !20
  %58 = extractelement <4 x i32> %27, i64 1, !dbg !21
  store i32 %58, ptr %57, align 4, !dbg !21, !tbaa !15
  br label %59, !dbg !19

59:                                               ; preds = %56, %54
  %60 = extractelement <4 x i1> %29, i64 2, !dbg !19
  br i1 %60, label %61, label %64, !dbg !19

61:                                               ; preds = %59
  %62 = getelementptr i32, ptr %20, i64 %23, !dbg !20
  %63 = extractelement <4 x i32> %27, i64 2, !dbg !21
  store i32 %63, ptr %62, align 4, !dbg !21, !tbaa !15
  br label %64, !dbg !19

64:                                               ; preds = %61, %59
  %65 = extractelement <4 x i1> %29, i64 3, !dbg !19
  br i1 %65, label %66, label %69, !dbg !19

66:                                               ; preds = %64
  %67 = getelementptr i32, ptr %21, i64 %23, !dbg !20
  %68 = extractelement <4 x i32> %27, i64 3, !dbg !21
  store i32 %68, ptr %67, align 4, !dbg !21, !tbaa !15
  br label %69, !dbg !19

69:                                               ; preds = %66, %64
  %70 = add nuw i64 %23, 8, !dbg !13
  %71 = icmp eq i64 %70, %14, !dbg !13
  br i1 %71, label %72, label %22, !dbg !13, !llvm.loop !22

72:                                               ; preds = %69
  %73 = icmp eq i64 %14, %8, !dbg !12
  br i1 %73, label %90, label %74, !dbg !12

74:                                               ; preds = %5, %72
  %75 = phi i64 [ 0, %5 ], [ %14, %72 ]
  %76 = and i64 %8, 1, !dbg !12
  %77 = icmp eq i64 %76, 0, !dbg !12
  br i1 %77, label %86, label %78, !dbg !12

78:                                               ; preds = %74
  %79 = getelementptr inbounds nuw i32, ptr %0, i64 %75, !dbg !14
  %80 = load i32, ptr %79, align 4, !dbg !14, !tbaa !15
  %81 = icmp sgt i32 %80, 0, !dbg !19
  br i1 %81, label %82, label %84, !dbg !19

82:                                               ; preds = %78
  %83 = getelementptr inbounds nuw i32, ptr %1, i64 %75, !dbg !20
  store i32 %80, ptr %83, align 4, !dbg !21, !tbaa !15
  br label %84, !dbg !20

84:                                               ; preds = %82, %78
  %85 = or disjoint i64 %75, 1, !dbg !13
  br label %86, !dbg !12

86:                                               ; preds = %84, %74
  %87 = phi i64 [ %75, %74 ], [ %85, %84 ]
  %88 = add nsw i64 %8, -1, !dbg !12
  %89 = icmp eq i64 %75, %88, !dbg !12
  br i1 %89, label %90, label %91, !dbg !12

90:                                               ; preds = %86, %105, %72, %3
  ret void, !dbg !27

91:                                               ; preds = %86, %105
  %92 = phi i64 [ %106, %105 ], [ %87, %86 ]
  %93 = getelementptr inbounds nuw i32, ptr %0, i64 %92, !dbg !14
  %94 = load i32, ptr %93, align 4, !dbg !14, !tbaa !15
  %95 = icmp sgt i32 %94, 0, !dbg !19
  br i1 %95, label %96, label %98, !dbg !19

96:                                               ; preds = %91
  %97 = getelementptr inbounds nuw i32, ptr %1, i64 %92, !dbg !20
  store i32 %94, ptr %97, align 4, !dbg !21, !tbaa !15
  br label %98, !dbg !20

98:                                               ; preds = %91, %96
  %99 = add nuw nsw i64 %92, 1, !dbg !13
  %100 = getelementptr inbounds nuw i32, ptr %0, i64 %99, !dbg !14
  %101 = load i32, ptr %100, align 4, !dbg !14, !tbaa !15
  %102 = icmp sgt i32 %101, 0, !dbg !19
  br i1 %102, label %103, label %105, !dbg !19

103:                                              ; preds = %98
  %104 = getelementptr inbounds nuw i32, ptr %1, i64 %99, !dbg !20
  store i32 %101, ptr %104, align 4, !dbg !21, !tbaa !15
  br label %105, !dbg !20

105:                                              ; preds = %103, %98
  %106 = add nuw nsw i64 %92, 2, !dbg !13
  %107 = icmp eq i64 %106, %8, !dbg !11
  br i1 %107, label %90, label %91, !dbg !12, !llvm.loop !28
}

attributes #0 = { nofree norecurse nosync nounwind sspstrong memory(argmem: readwrite) uwtable "min-legal-vector-width"="0" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }

!llvm.dbg.cu = !{!0}
!llvm.module.flags = !{!2, !3, !4, !5, !6}
!llvm.ident = !{!7}

!0 = distinct !DICompileUnit(language: DW_LANG_C11, file: !1, producer: "clang version 21.1.8", isOptimized: true, runtimeVersion: 0, emissionKind: LineTablesOnly, splitDebugInlining: false, nameTableKind: None)
!1 = !DIFile(filename: "test/vectorize/vect_conditional_store.c", directory: "/home/serosh/scratch/aion")
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
!13 = !DILocation(line: 1, column: 60, scope: !8)
!14 = !DILocation(line: 1, column: 70, scope: !8)
!15 = !{!16, !16, i64 0}
!16 = !{!"int", !17, i64 0}
!17 = !{!"omnipotent char", !18, i64 0}
!18 = !{!"Simple C/C++ TBAA"}
!19 = !DILocation(line: 1, column: 75, scope: !8)
!20 = !DILocation(line: 1, column: 80, scope: !8)
!21 = !DILocation(line: 1, column: 85, scope: !8)
!22 = distinct !{!22, !12, !23, !24, !25, !26}
!23 = !DILocation(line: 1, column: 93, scope: !8)
!24 = !{!"llvm.loop.mustprogress"}
!25 = !{!"llvm.loop.isvectorized", i32 1}
!26 = !{!"llvm.loop.unroll.runtime.disable"}
!27 = !DILocation(line: 1, column: 95, scope: !8)
!28 = distinct !{!28, !12, !23, !24, !25}
