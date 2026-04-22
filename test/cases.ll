; ModuleID = 'test/cases.cpp'
source_filename = "test/cases.cpp"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-i128:128-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux-gnu"

; Function Attrs: mustprogress nofree norecurse nosync nounwind sspstrong memory(argmem: readwrite) uwtable
define dso_local void @_Z15loop_dependencyPiS_i(ptr noundef captures(none) %0, ptr noundef readonly captures(none) %1, i32 noundef %2) local_unnamed_addr #0 !dbg !14 {
    #dbg_value(ptr %0, !20, !DIExpression(), !25)
    #dbg_value(ptr %1, !21, !DIExpression(), !25)
    #dbg_value(i32 %2, !22, !DIExpression(), !25)
    #dbg_value(i32 1, !23, !DIExpression(), !26)
  %4 = icmp sgt i32 %2, 1, !dbg !27
  br i1 %4, label %5, label %31, !dbg !29

5:                                                ; preds = %3
  %6 = zext nneg i32 %2 to i64, !dbg !30
  %7 = load i32, ptr %0, align 4
  %8 = add nsw i64 %6, -1, !dbg !31
  %9 = and i64 %8, 3, !dbg !31
  %10 = add nsw i32 %2, -2, !dbg !31
  %11 = icmp ult i32 %10, 3, !dbg !31
  br i1 %11, label %16, label %12, !dbg !31

12:                                               ; preds = %5
  %13 = and i64 %8, -4, !dbg !31
  br label %32, !dbg !31

14:                                               ; preds = %32
  %15 = icmp eq i64 %9, 0, !dbg !31
  br i1 %15, label %31, label %16, !dbg !31

16:                                               ; preds = %14, %5
  %17 = phi i32 [ %7, %5 ], [ %54, %14 ]
  %18 = phi i64 [ 1, %5 ], [ %55, %14 ]
  %19 = icmp ne i64 %9, 0, !dbg !32
  tail call void @llvm.assume(i1 %19), !dbg !32
  br label %20, !dbg !32

20:                                               ; preds = %20, %16
  %21 = phi i32 [ %17, %16 ], [ %27, %20 ], !dbg !33
  %22 = phi i64 [ %18, %16 ], [ %28, %20 ]
  %23 = phi i64 [ 0, %16 ], [ %29, %20 ]
    #dbg_value(i64 %22, !23, !DIExpression(), !26)
  %24 = getelementptr i32, ptr %0, i64 %22, !dbg !33
  %25 = getelementptr inbounds nuw i32, ptr %1, i64 %22, !dbg !35
  %26 = load i32, ptr %25, align 4, !dbg !35, !tbaa !10
  %27 = add nsw i32 %26, %21, !dbg !36
  store i32 %27, ptr %24, align 4, !dbg !37, !tbaa !10
  %28 = add nuw nsw i64 %22, 1, !dbg !38
    #dbg_value(i64 %28, !23, !DIExpression(), !26)
  %29 = add i64 %23, 1, !dbg !39
  %30 = icmp eq i64 %29, %9, !dbg !39
  br i1 %30, label %31, label %20, !dbg !39, !llvm.loop !40

31:                                               ; preds = %14, %20, %3
  ret void, !dbg !42

32:                                               ; preds = %32, %12
  %33 = phi i32 [ %7, %12 ], [ %54, %32 ], !dbg !33
  %34 = phi i64 [ 1, %12 ], [ %55, %32 ]
  %35 = phi i64 [ 0, %12 ], [ %56, %32 ]
    #dbg_value(i64 %34, !23, !DIExpression(), !26)
  %36 = getelementptr i32, ptr %0, i64 %34, !dbg !33
  %37 = getelementptr inbounds nuw i32, ptr %1, i64 %34, !dbg !35
  %38 = load i32, ptr %37, align 4, !dbg !35, !tbaa !10
  %39 = add nsw i32 %38, %33, !dbg !43
  store i32 %39, ptr %36, align 4, !dbg !44, !tbaa !10
  %40 = add nuw nsw i64 %34, 1, !dbg !45
    #dbg_value(i64 %40, !23, !DIExpression(), !26)
  %41 = getelementptr i32, ptr %0, i64 %40, !dbg !33
  %42 = getelementptr inbounds nuw i32, ptr %1, i64 %40, !dbg !35
  %43 = load i32, ptr %42, align 4, !dbg !35, !tbaa !10
  %44 = add nsw i32 %43, %39, !dbg !46
  store i32 %44, ptr %41, align 4, !dbg !47, !tbaa !10
  %45 = add nuw nsw i64 %34, 2, !dbg !48
    #dbg_value(i64 %45, !23, !DIExpression(), !26)
  %46 = getelementptr i32, ptr %0, i64 %45, !dbg !33
  %47 = getelementptr inbounds nuw i32, ptr %1, i64 %45, !dbg !35
  %48 = load i32, ptr %47, align 4, !dbg !35, !tbaa !10
  %49 = add nsw i32 %48, %44, !dbg !49
  store i32 %49, ptr %46, align 4, !dbg !50, !tbaa !10
  %50 = add nuw nsw i64 %34, 3, !dbg !51
    #dbg_value(i64 %50, !23, !DIExpression(), !26)
  %51 = getelementptr i32, ptr %0, i64 %50, !dbg !33
  %52 = getelementptr inbounds nuw i32, ptr %1, i64 %50, !dbg !35
  %53 = load i32, ptr %52, align 4, !dbg !35, !tbaa !10
  %54 = add nsw i32 %53, %49, !dbg !52
  store i32 %54, ptr %51, align 4, !dbg !53, !tbaa !10
  %55 = add nuw nsw i64 %34, 4, !dbg !54
    #dbg_value(i64 %55, !23, !DIExpression(), !26)
  %56 = add i64 %35, 4, !dbg !55
  %57 = icmp eq i64 %56, %13, !dbg !55
  br i1 %57, label %14, label %32, !dbg !55, !llvm.loop !56
}

; Function Attrs: mustprogress nocallback nofree nosync nounwind willreturn memory(argmem: readwrite)
declare void @llvm.lifetime.start.p0(ptr captures(none)) #1

; Function Attrs: mustprogress nocallback nofree nosync nounwind willreturn memory(argmem: readwrite)
declare void @llvm.lifetime.end.p0(ptr captures(none)) #1

; Function Attrs: mustprogress nofree noinline norecurse nosync nounwind sspstrong willreturn memory(none) uwtable
define dso_local noundef i32 @_Z6squarei(i32 noundef %0) local_unnamed_addr #2 !dbg !59 {
    #dbg_value(i32 %0, !63, !DIExpression(), !64)
  %2 = mul nsw i32 %0, %0, !dbg !65
  ret i32 %2, !dbg !66
}

; Function Attrs: mustprogress nofree norecurse nosync nounwind sspstrong willreturn memory(none) uwtable
define dso_local noundef i32 @_Z10use_squarei(i32 noundef %0) local_unnamed_addr #3 !dbg !67 {
    #dbg_value(i32 %0, !69, !DIExpression(), !70)
  %2 = tail call noundef i32 @_Z6squarei(i32 noundef %0), !dbg !71
  %3 = add nsw i32 %0, 1, !dbg !72
  %4 = tail call noundef i32 @_Z6squarei(i32 noundef %3), !dbg !73
  %5 = add nsw i32 %4, %2, !dbg !74
  ret i32 %5, !dbg !75
}

; Function Attrs: mustprogress nofree norecurse nosync nounwind sspstrong willreturn memory(argmem: read) uwtable
define dso_local noundef i32 @_Z14loop_with_exitPiii(ptr noundef readonly captures(none) %0, i32 noundef %1, i32 noundef %2) local_unnamed_addr #4 !dbg !76 {
    #dbg_value(ptr %0, !80, !DIExpression(), !86)
    #dbg_value(i32 %1, !81, !DIExpression(), !86)
    #dbg_value(i32 %2, !82, !DIExpression(), !86)
    #dbg_value(i32 0, !83, !DIExpression(), !86)
    #dbg_value(i32 0, !84, !DIExpression(), !87)
  %4 = icmp sgt i32 %1, 0, !dbg !88
  br i1 %4, label %5, label %17, !dbg !90

5:                                                ; preds = %3
  %6 = zext nneg i32 %1 to i64, !dbg !91
  br label %7, !dbg !92

7:                                                ; preds = %5, %13
  %8 = phi i64 [ 0, %5 ], [ %15, %13 ]
  %9 = phi i32 [ 0, %5 ], [ %14, %13 ]
    #dbg_value(i64 %8, !84, !DIExpression(), !87)
    #dbg_value(i32 %9, !83, !DIExpression(), !86)
  %10 = getelementptr inbounds nuw i32, ptr %0, i64 %8, !dbg !93
  %11 = load i32, ptr %10, align 4, !dbg !93, !tbaa !10
  %12 = icmp sgt i32 %11, %2, !dbg !96
  br i1 %12, label %17, label %13, !dbg !97

13:                                               ; preds = %7
  %14 = add nsw i32 %11, %9, !dbg !98
    #dbg_value(i32 %14, !83, !DIExpression(), !86)
  %15 = add nuw nsw i64 %8, 1, !dbg !99
    #dbg_value(i64 %15, !84, !DIExpression(), !87)
  %16 = icmp eq i64 %15, %6, !dbg !91
  br i1 %16, label %17, label %7, !dbg !100, !llvm.loop !101

17:                                               ; preds = %13, %7, %3
  %18 = phi i32 [ 0, %3 ], [ %9, %7 ], [ %14, %13 ], !dbg !86
  ret i32 %18, !dbg !103
}

; Function Attrs: mustprogress nofree norecurse nosync nounwind sspstrong willreturn memory(argmem: readwrite) uwtable
define dso_local void @_Z10slp_missedPfS_S_(ptr noundef writeonly captures(none) initializes((0, 4), (8, 12), (16, 20)) %0, ptr noundef readonly captures(none) %1, ptr noundef readonly captures(none) %2) local_unnamed_addr #5 !dbg !104 {
    #dbg_value(ptr %0, !110, !DIExpression(), !113)
    #dbg_value(ptr %1, !111, !DIExpression(), !113)
    #dbg_value(ptr %2, !112, !DIExpression(), !113)
  %4 = load float, ptr %1, align 4, !dbg !114, !tbaa !115
  %5 = load float, ptr %2, align 4, !dbg !117, !tbaa !115
  %6 = fadd float %4, %5, !dbg !118
  store float %6, ptr %0, align 4, !dbg !119, !tbaa !115
  %7 = getelementptr inbounds nuw i8, ptr %1, i64 4, !dbg !120
  %8 = load float, ptr %7, align 4, !dbg !120, !tbaa !115
  %9 = getelementptr inbounds nuw i8, ptr %2, i64 4, !dbg !121
  %10 = load float, ptr %9, align 4, !dbg !121, !tbaa !115
  %11 = fadd float %8, %10, !dbg !122
  %12 = getelementptr inbounds nuw i8, ptr %0, i64 8, !dbg !123
  store float %11, ptr %12, align 4, !dbg !124, !tbaa !115
  %13 = getelementptr inbounds nuw i8, ptr %1, i64 8, !dbg !125
  %14 = load float, ptr %13, align 4, !dbg !125, !tbaa !115
  %15 = getelementptr inbounds nuw i8, ptr %2, i64 8, !dbg !126
  %16 = load float, ptr %15, align 4, !dbg !126, !tbaa !115
  %17 = fadd float %14, %16, !dbg !127
  %18 = getelementptr inbounds nuw i8, ptr %0, i64 16, !dbg !128
  store float %17, ptr %18, align 4, !dbg !129, !tbaa !115
  ret void, !dbg !130
}

; Function Attrs: mustprogress sspstrong uwtable
define dso_local void @_Z9sroa_failv() local_unnamed_addr #6 !dbg !131 {
  %1 = alloca i32, align 4, !DIAssignID !136
    #dbg_assign(i1 poison, !135, !DIExpression(), !136, ptr %1, !DIExpression(), !137)
  call void @llvm.lifetime.start.p0(ptr nonnull %1) #9, !dbg !138
  store i32 42, ptr %1, align 4, !dbg !139, !tbaa !10, !DIAssignID !140
    #dbg_assign(i32 42, !135, !DIExpression(), !140, ptr %1, !DIExpression(), !137)
  call void @_Z6escapePi(ptr noundef nonnull %1), !dbg !141
  call void @llvm.lifetime.end.p0(ptr nonnull %1) #9, !dbg !142
  ret void, !dbg !143
}

declare !dbg !144 void @_Z6escapePi(ptr noundef) local_unnamed_addr #7

; Function Attrs: mustprogress nofree norecurse nosync nounwind sspstrong memory(argmem: readwrite) uwtable
define dso_local void @_Z9licm_failPiS_S_(ptr noundef writeonly captures(none) %0, ptr noundef readonly captures(none) %1, ptr noundef readonly captures(none) %2) local_unnamed_addr #0 !dbg !147 {
    #dbg_value(ptr %0, !151, !DIExpression(), !156)
    #dbg_value(ptr %1, !152, !DIExpression(), !156)
    #dbg_value(ptr %2, !153, !DIExpression(), !156)
    #dbg_value(i32 0, !154, !DIExpression(), !157)
  %4 = load i32, ptr %2, align 4, !dbg !158, !tbaa !10
  %5 = icmp sgt i32 %4, 0, !dbg !160
  br i1 %5, label %7, label %6, !dbg !161

6:                                                ; preds = %7, %3
  ret void, !dbg !162

7:                                                ; preds = %3, %7
  %8 = phi i64 [ %13, %7 ], [ 0, %3 ]
    #dbg_value(i64 %8, !154, !DIExpression(), !157)
  %9 = load i32, ptr %1, align 4, !dbg !163, !tbaa !10
  %10 = trunc nuw nsw i64 %8 to i32, !dbg !165
  %11 = add nsw i32 %9, %10, !dbg !165
  %12 = getelementptr inbounds nuw i32, ptr %0, i64 %8, !dbg !166
  store i32 %11, ptr %12, align 4, !dbg !167, !tbaa !10
  %13 = add nuw nsw i64 %8, 1, !dbg !168
    #dbg_value(i64 %13, !154, !DIExpression(), !157)
  %14 = load i32, ptr %2, align 4, !dbg !158, !tbaa !10
  %15 = sext i32 %14 to i64, !dbg !169
  %16 = icmp slt i64 %13, %15, !dbg !169
  br i1 %16, label %7, label %6, !dbg !170, !llvm.loop !171
}

; Function Attrs: nocallback nofree nosync nounwind willreturn memory(inaccessiblemem: write)
declare void @llvm.assume(i1 noundef) #8

attributes #0 = { mustprogress nofree norecurse nosync nounwind sspstrong memory(argmem: readwrite) uwtable "min-legal-vector-width"="0" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #1 = { mustprogress nocallback nofree nosync nounwind willreturn memory(argmem: readwrite) }
attributes #2 = { mustprogress nofree noinline norecurse nosync nounwind sspstrong willreturn memory(none) uwtable "min-legal-vector-width"="0" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #3 = { mustprogress nofree norecurse nosync nounwind sspstrong willreturn memory(none) uwtable "min-legal-vector-width"="0" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #4 = { mustprogress nofree norecurse nosync nounwind sspstrong willreturn memory(argmem: read) uwtable "min-legal-vector-width"="0" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #5 = { mustprogress nofree norecurse nosync nounwind sspstrong willreturn memory(argmem: readwrite) uwtable "min-legal-vector-width"="0" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #6 = { mustprogress sspstrong uwtable "min-legal-vector-width"="0" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #7 = { "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #8 = { nocallback nofree nosync nounwind willreturn memory(inaccessiblemem: write) }
attributes #9 = { nounwind }

!llvm.dbg.cu = !{!0}
!llvm.module.flags = !{!2, !3, !4, !5, !6, !7, !8}
!llvm.ident = !{!9}
!llvm.errno.tbaa = !{!10}

!0 = distinct !DICompileUnit(language: DW_LANG_C_plus_plus_14, file: !1, producer: "clang version 22.1.3", isOptimized: true, runtimeVersion: 0, emissionKind: FullDebug, splitDebugInlining: false, nameTableKind: None)
!1 = !DIFile(filename: "test/cases.cpp", directory: "/home/serosh/scratch/aion", checksumkind: CSK_MD5, checksum: "1875efa122c2dfe0e2f458aa615c5d35")
!2 = !{i32 7, !"Dwarf Version", i32 5}
!3 = !{i32 2, !"Debug Info Version", i32 3}
!4 = !{i32 1, !"wchar_size", i32 4}
!5 = !{i32 8, !"PIC Level", i32 2}
!6 = !{i32 7, !"PIE Level", i32 2}
!7 = !{i32 7, !"uwtable", i32 2}
!8 = !{i32 7, !"debug-info-assignment-tracking", i1 true}
!9 = !{!"clang version 22.1.3"}
!10 = !{!11, !11, i64 0}
!11 = !{!"int", !12, i64 0}
!12 = !{!"omnipotent char", !13, i64 0}
!13 = !{!"Simple C++ TBAA"}
!14 = distinct !DISubprogram(name: "loop_dependency", linkageName: "_Z15loop_dependencyPiS_i", scope: !1, file: !1, line: 2, type: !15, scopeLine: 2, flags: DIFlagPrototyped | DIFlagAllCallsDescribed, spFlags: DISPFlagDefinition | DISPFlagOptimized, unit: !0, retainedNodes: !19, keyInstructions: true)
!15 = !DISubroutineType(types: !16)
!16 = !{null, !17, !17, !18}
!17 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !18, size: 64)
!18 = !DIBasicType(name: "int", size: 32, encoding: DW_ATE_signed)
!19 = !{!20, !21, !22, !23}
!20 = !DILocalVariable(name: "a", arg: 1, scope: !14, file: !1, line: 2, type: !17)
!21 = !DILocalVariable(name: "b", arg: 2, scope: !14, file: !1, line: 2, type: !17)
!22 = !DILocalVariable(name: "n", arg: 3, scope: !14, file: !1, line: 2, type: !18)
!23 = !DILocalVariable(name: "i", scope: !24, file: !1, line: 3, type: !18)
!24 = distinct !DILexicalBlock(scope: !14, file: !1, line: 3, column: 5)
!25 = !DILocation(line: 0, scope: !14)
!26 = !DILocation(line: 0, scope: !24)
!27 = !DILocation(line: 3, column: 23, scope: !28, atomGroup: 12, atomRank: 1)
!28 = distinct !DILexicalBlock(scope: !24, file: !1, line: 3, column: 5)
!29 = !DILocation(line: 3, column: 5, scope: !24, atomGroup: 13, atomRank: 1)
!30 = !DILocation(line: 3, column: 23, scope: !28, atomGroup: 2, atomRank: 1)
!31 = !DILocation(line: 3, column: 5, scope: !24)
!32 = !DILocation(line: 3, column: 5, scope: !24, atomGroup: 3, atomRank: 1)
!33 = !DILocation(line: 4, column: 16, scope: !34)
!34 = distinct !DILexicalBlock(scope: !28, file: !1, line: 3, column: 33)
!35 = !DILocation(line: 4, column: 25, scope: !34)
!36 = !DILocation(line: 4, column: 23, scope: !34, atomGroup: 18, atomRank: 2)
!37 = !DILocation(line: 4, column: 14, scope: !34, atomGroup: 18, atomRank: 1)
!38 = !DILocation(line: 3, column: 29, scope: !28, atomGroup: 19, atomRank: 2)
!39 = !DILocation(line: 3, column: 5, scope: !24, atomGroup: 21, atomRank: 1)
!40 = distinct !{!40, !41}
!41 = !{!"llvm.loop.unroll.disable"}
!42 = !DILocation(line: 6, column: 1, scope: !14, atomGroup: 7, atomRank: 1)
!43 = !DILocation(line: 4, column: 23, scope: !34, atomGroup: 4, atomRank: 2)
!44 = !DILocation(line: 4, column: 14, scope: !34, atomGroup: 4, atomRank: 1)
!45 = !DILocation(line: 3, column: 29, scope: !28, atomGroup: 5, atomRank: 2)
!46 = !DILocation(line: 4, column: 23, scope: !34, atomGroup: 22, atomRank: 2)
!47 = !DILocation(line: 4, column: 14, scope: !34, atomGroup: 22, atomRank: 1)
!48 = !DILocation(line: 3, column: 29, scope: !28, atomGroup: 23, atomRank: 2)
!49 = !DILocation(line: 4, column: 23, scope: !34, atomGroup: 26, atomRank: 2)
!50 = !DILocation(line: 4, column: 14, scope: !34, atomGroup: 26, atomRank: 1)
!51 = !DILocation(line: 3, column: 29, scope: !28, atomGroup: 27, atomRank: 2)
!52 = !DILocation(line: 4, column: 23, scope: !34, atomGroup: 30, atomRank: 2)
!53 = !DILocation(line: 4, column: 14, scope: !34, atomGroup: 30, atomRank: 1)
!54 = !DILocation(line: 3, column: 29, scope: !28, atomGroup: 31, atomRank: 2)
!55 = !DILocation(line: 3, column: 5, scope: !24, atomGroup: 33, atomRank: 1)
!56 = distinct !{!56, !31, !57, !58}
!57 = !DILocation(line: 5, column: 5, scope: !24)
!58 = !{!"llvm.loop.mustprogress"}
!59 = distinct !DISubprogram(name: "square", linkageName: "_Z6squarei", scope: !1, file: !1, line: 10, type: !60, scopeLine: 10, flags: DIFlagPrototyped | DIFlagAllCallsDescribed, spFlags: DISPFlagDefinition | DISPFlagOptimized, unit: !0, retainedNodes: !62, keyInstructions: true)
!60 = !DISubroutineType(types: !61)
!61 = !{!18, !18}
!62 = !{!63}
!63 = !DILocalVariable(name: "x", arg: 1, scope: !59, file: !1, line: 10, type: !18)
!64 = !DILocation(line: 0, scope: !59)
!65 = !DILocation(line: 11, column: 14, scope: !59, atomGroup: 1, atomRank: 2)
!66 = !DILocation(line: 11, column: 5, scope: !59, atomGroup: 1, atomRank: 1)
!67 = distinct !DISubprogram(name: "use_square", linkageName: "_Z10use_squarei", scope: !1, file: !1, line: 14, type: !60, scopeLine: 14, flags: DIFlagPrototyped | DIFlagAllCallsDescribed, spFlags: DISPFlagDefinition | DISPFlagOptimized, unit: !0, retainedNodes: !68, keyInstructions: true)
!68 = !{!69}
!69 = !DILocalVariable(name: "x", arg: 1, scope: !67, file: !1, line: 14, type: !18)
!70 = !DILocation(line: 0, scope: !67)
!71 = !DILocation(line: 15, column: 12, scope: !67)
!72 = !DILocation(line: 15, column: 33, scope: !67)
!73 = !DILocation(line: 15, column: 24, scope: !67)
!74 = !DILocation(line: 15, column: 22, scope: !67, atomGroup: 1, atomRank: 2)
!75 = !DILocation(line: 15, column: 5, scope: !67, atomGroup: 1, atomRank: 1)
!76 = distinct !DISubprogram(name: "loop_with_exit", linkageName: "_Z14loop_with_exitPiii", scope: !1, file: !1, line: 19, type: !77, scopeLine: 19, flags: DIFlagPrototyped | DIFlagAllCallsDescribed, spFlags: DISPFlagDefinition | DISPFlagOptimized, unit: !0, retainedNodes: !79, keyInstructions: true)
!77 = !DISubroutineType(types: !78)
!78 = !{!18, !17, !18, !18}
!79 = !{!80, !81, !82, !83, !84}
!80 = !DILocalVariable(name: "a", arg: 1, scope: !76, file: !1, line: 19, type: !17)
!81 = !DILocalVariable(name: "n", arg: 2, scope: !76, file: !1, line: 19, type: !18)
!82 = !DILocalVariable(name: "threshold", arg: 3, scope: !76, file: !1, line: 19, type: !18)
!83 = !DILocalVariable(name: "sum", scope: !76, file: !1, line: 20, type: !18)
!84 = !DILocalVariable(name: "i", scope: !85, file: !1, line: 21, type: !18)
!85 = distinct !DILexicalBlock(scope: !76, file: !1, line: 21, column: 5)
!86 = !DILocation(line: 0, scope: !76)
!87 = !DILocation(line: 0, scope: !85)
!88 = !DILocation(line: 21, column: 23, scope: !89, atomGroup: 14, atomRank: 1)
!89 = distinct !DILexicalBlock(scope: !85, file: !1, line: 21, column: 5)
!90 = !DILocation(line: 21, column: 5, scope: !85, atomGroup: 15, atomRank: 1)
!91 = !DILocation(line: 21, column: 23, scope: !89, atomGroup: 3, atomRank: 1)
!92 = !DILocation(line: 21, column: 5, scope: !85)
!93 = !DILocation(line: 22, column: 13, scope: !94)
!94 = distinct !DILexicalBlock(scope: !95, file: !1, line: 22, column: 13)
!95 = distinct !DILexicalBlock(scope: !89, file: !1, line: 21, column: 33)
!96 = !DILocation(line: 22, column: 18, scope: !94, atomGroup: 5, atomRank: 2)
!97 = !DILocation(line: 22, column: 18, scope: !94, atomGroup: 5, atomRank: 1)
!98 = !DILocation(line: 23, column: 13, scope: !95, atomGroup: 7, atomRank: 2)
!99 = !DILocation(line: 21, column: 29, scope: !89, atomGroup: 8, atomRank: 2)
!100 = !DILocation(line: 21, column: 5, scope: !85, atomGroup: 4, atomRank: 1)
!101 = distinct !{!101, !92, !102, !58}
!102 = !DILocation(line: 24, column: 5, scope: !85)
!103 = !DILocation(line: 25, column: 5, scope: !76, atomGroup: 11, atomRank: 1)
!104 = distinct !DISubprogram(name: "slp_missed", linkageName: "_Z10slp_missedPfS_S_", scope: !1, file: !1, line: 29, type: !105, scopeLine: 29, flags: DIFlagPrototyped | DIFlagAllCallsDescribed, spFlags: DISPFlagDefinition | DISPFlagOptimized, unit: !0, retainedNodes: !109, keyInstructions: true)
!105 = !DISubroutineType(types: !106)
!106 = !{null, !107, !107, !107}
!107 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !108, size: 64)
!108 = !DIBasicType(name: "float", size: 32, encoding: DW_ATE_float)
!109 = !{!110, !111, !112}
!110 = !DILocalVariable(name: "a", arg: 1, scope: !104, file: !1, line: 29, type: !107)
!111 = !DILocalVariable(name: "b", arg: 2, scope: !104, file: !1, line: 29, type: !107)
!112 = !DILocalVariable(name: "c", arg: 3, scope: !104, file: !1, line: 29, type: !107)
!113 = !DILocation(line: 0, scope: !104)
!114 = !DILocation(line: 30, column: 12, scope: !104)
!115 = !{!116, !116, i64 0}
!116 = !{!"float", !12, i64 0}
!117 = !DILocation(line: 30, column: 19, scope: !104)
!118 = !DILocation(line: 30, column: 17, scope: !104, atomGroup: 1, atomRank: 2)
!119 = !DILocation(line: 30, column: 10, scope: !104, atomGroup: 1, atomRank: 1)
!120 = !DILocation(line: 31, column: 12, scope: !104)
!121 = !DILocation(line: 31, column: 19, scope: !104)
!122 = !DILocation(line: 31, column: 17, scope: !104, atomGroup: 2, atomRank: 2)
!123 = !DILocation(line: 31, column: 5, scope: !104)
!124 = !DILocation(line: 31, column: 10, scope: !104, atomGroup: 2, atomRank: 1)
!125 = !DILocation(line: 32, column: 12, scope: !104)
!126 = !DILocation(line: 32, column: 19, scope: !104)
!127 = !DILocation(line: 32, column: 17, scope: !104, atomGroup: 3, atomRank: 2)
!128 = !DILocation(line: 32, column: 5, scope: !104)
!129 = !DILocation(line: 32, column: 10, scope: !104, atomGroup: 3, atomRank: 1)
!130 = !DILocation(line: 33, column: 1, scope: !104, atomGroup: 4, atomRank: 1)
!131 = distinct !DISubprogram(name: "sroa_fail", linkageName: "_Z9sroa_failv", scope: !1, file: !1, line: 37, type: !132, scopeLine: 37, flags: DIFlagPrototyped | DIFlagAllCallsDescribed, spFlags: DISPFlagDefinition | DISPFlagOptimized, unit: !0, retainedNodes: !134, keyInstructions: true)
!132 = !DISubroutineType(types: !133)
!133 = !{null}
!134 = !{!135}
!135 = !DILocalVariable(name: "x", scope: !131, file: !1, line: 38, type: !18)
!136 = distinct !DIAssignID()
!137 = !DILocation(line: 0, scope: !131)
!138 = !DILocation(line: 38, column: 5, scope: !131)
!139 = !DILocation(line: 38, column: 9, scope: !131, atomGroup: 1, atomRank: 1)
!140 = distinct !DIAssignID()
!141 = !DILocation(line: 39, column: 5, scope: !131)
!142 = !DILocation(line: 40, column: 1, scope: !131)
!143 = !DILocation(line: 40, column: 1, scope: !131, atomGroup: 2, atomRank: 1)
!144 = !DISubprogram(name: "escape", linkageName: "_Z6escapePi", scope: !1, file: !1, line: 36, type: !145, flags: DIFlagPrototyped, spFlags: DISPFlagOptimized)
!145 = !DISubroutineType(types: !146)
!146 = !{null, !17}
!147 = distinct !DISubprogram(name: "licm_fail", linkageName: "_Z9licm_failPiS_S_", scope: !1, file: !1, line: 43, type: !148, scopeLine: 43, flags: DIFlagPrototyped | DIFlagAllCallsDescribed, spFlags: DISPFlagDefinition | DISPFlagOptimized, unit: !0, retainedNodes: !150, keyInstructions: true)
!148 = !DISubroutineType(types: !149)
!149 = !{null, !17, !17, !17}
!150 = !{!151, !152, !153, !154}
!151 = !DILocalVariable(name: "a", arg: 1, scope: !147, file: !1, line: 43, type: !17)
!152 = !DILocalVariable(name: "b", arg: 2, scope: !147, file: !1, line: 43, type: !17)
!153 = !DILocalVariable(name: "n", arg: 3, scope: !147, file: !1, line: 43, type: !17)
!154 = !DILocalVariable(name: "i", scope: !155, file: !1, line: 44, type: !18)
!155 = distinct !DILexicalBlock(scope: !147, file: !1, line: 44, column: 5)
!156 = !DILocation(line: 0, scope: !147)
!157 = !DILocation(line: 0, scope: !155)
!158 = !DILocation(line: 44, column: 25, scope: !159)
!159 = distinct !DILexicalBlock(scope: !155, file: !1, line: 44, column: 5)
!160 = !DILocation(line: 44, column: 23, scope: !159, atomGroup: 16, atomRank: 1)
!161 = !DILocation(line: 44, column: 5, scope: !155, atomGroup: 17, atomRank: 1)
!162 = !DILocation(line: 47, column: 1, scope: !147, atomGroup: 7, atomRank: 1)
!163 = !DILocation(line: 45, column: 16, scope: !164)
!164 = distinct !DILexicalBlock(scope: !159, file: !1, line: 44, column: 34)
!165 = !DILocation(line: 45, column: 19, scope: !164, atomGroup: 4, atomRank: 2)
!166 = !DILocation(line: 45, column: 9, scope: !164)
!167 = !DILocation(line: 45, column: 14, scope: !164, atomGroup: 4, atomRank: 1)
!168 = !DILocation(line: 44, column: 30, scope: !159, atomGroup: 5, atomRank: 2)
!169 = !DILocation(line: 44, column: 23, scope: !159, atomGroup: 2, atomRank: 1)
!170 = !DILocation(line: 44, column: 5, scope: !155, atomGroup: 3, atomRank: 1)
!171 = distinct !{!171, !172, !173, !58}
!172 = !DILocation(line: 44, column: 5, scope: !155)
!173 = !DILocation(line: 46, column: 5, scope: !155)
