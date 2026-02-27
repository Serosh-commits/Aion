; ModuleID = 'test/cases.cpp'
source_filename = "test/cases.cpp"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-i128:128-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux-gnu"

; Function Attrs: mustprogress nofree norecurse nosync nounwind sspstrong memory(argmem: readwrite) uwtable
define dso_local void @_Z15loop_dependencyPiS_i(ptr noundef captures(none) %0, ptr noundef readonly captures(none) %1, i32 noundef %2) local_unnamed_addr #0 !dbg !10 {
    #dbg_value(ptr %0, !16, !DIExpression(), !21)
    #dbg_value(ptr %1, !17, !DIExpression(), !21)
    #dbg_value(i32 %2, !18, !DIExpression(), !21)
    #dbg_value(i32 1, !19, !DIExpression(), !22)
  %4 = icmp sgt i32 %2, 1, !dbg !23
  br i1 %4, label %5, label %29, !dbg !25

5:                                                ; preds = %3
  %6 = zext nneg i32 %2 to i64, !dbg !23
  %7 = load i32, ptr %0, align 4
  %8 = add nsw i64 %6, -1, !dbg !25
  %9 = and i64 %8, 3, !dbg !25
  %10 = add nsw i32 %2, -2, !dbg !25
  %11 = icmp ult i32 %10, 3, !dbg !25
  br i1 %11, label %14, label %12, !dbg !25

12:                                               ; preds = %5
  %13 = and i64 %8, -4, !dbg !25
  br label %30, !dbg !25

14:                                               ; preds = %30, %5
  %15 = phi i32 [ %7, %5 ], [ %52, %30 ]
  %16 = phi i64 [ 1, %5 ], [ %53, %30 ]
  %17 = icmp eq i64 %9, 0, !dbg !25
  br i1 %17, label %29, label %18, !dbg !25

18:                                               ; preds = %14, %18
  %19 = phi i32 [ %25, %18 ], [ %15, %14 ], !dbg !26
  %20 = phi i64 [ %26, %18 ], [ %16, %14 ]
  %21 = phi i64 [ %27, %18 ], [ 0, %14 ]
    #dbg_value(i64 %20, !19, !DIExpression(), !22)
  %22 = getelementptr i32, ptr %0, i64 %20, !dbg !26
  %23 = getelementptr inbounds nuw i32, ptr %1, i64 %20, !dbg !28
  %24 = load i32, ptr %23, align 4, !dbg !28, !tbaa !29
  %25 = add nsw i32 %24, %19, !dbg !33
  store i32 %25, ptr %22, align 4, !dbg !34, !tbaa !29
  %26 = add nuw nsw i64 %20, 1, !dbg !35
    #dbg_value(i64 %26, !19, !DIExpression(), !22)
  %27 = add i64 %21, 1, !dbg !25
  %28 = icmp eq i64 %27, %9, !dbg !25
  br i1 %28, label %29, label %18, !dbg !25, !llvm.loop !36

29:                                               ; preds = %14, %18, %3
  ret void, !dbg !38

30:                                               ; preds = %30, %12
  %31 = phi i32 [ %7, %12 ], [ %52, %30 ], !dbg !26
  %32 = phi i64 [ 1, %12 ], [ %53, %30 ]
  %33 = phi i64 [ 0, %12 ], [ %54, %30 ]
    #dbg_value(i64 %32, !19, !DIExpression(), !22)
  %34 = getelementptr i32, ptr %0, i64 %32, !dbg !26
  %35 = getelementptr inbounds nuw i32, ptr %1, i64 %32, !dbg !28
  %36 = load i32, ptr %35, align 4, !dbg !28, !tbaa !29
  %37 = add nsw i32 %36, %31, !dbg !33
  store i32 %37, ptr %34, align 4, !dbg !34, !tbaa !29
  %38 = add nuw nsw i64 %32, 1, !dbg !35
    #dbg_value(i64 %38, !19, !DIExpression(), !22)
  %39 = getelementptr i32, ptr %0, i64 %38, !dbg !26
  %40 = getelementptr inbounds nuw i32, ptr %1, i64 %38, !dbg !28
  %41 = load i32, ptr %40, align 4, !dbg !28, !tbaa !29
  %42 = add nsw i32 %41, %37, !dbg !33
  store i32 %42, ptr %39, align 4, !dbg !34, !tbaa !29
  %43 = add nuw nsw i64 %32, 2, !dbg !35
    #dbg_value(i64 %43, !19, !DIExpression(), !22)
  %44 = getelementptr i32, ptr %0, i64 %43, !dbg !26
  %45 = getelementptr inbounds nuw i32, ptr %1, i64 %43, !dbg !28
  %46 = load i32, ptr %45, align 4, !dbg !28, !tbaa !29
  %47 = add nsw i32 %46, %42, !dbg !33
  store i32 %47, ptr %44, align 4, !dbg !34, !tbaa !29
  %48 = add nuw nsw i64 %32, 3, !dbg !35
    #dbg_value(i64 %48, !19, !DIExpression(), !22)
  %49 = getelementptr i32, ptr %0, i64 %48, !dbg !26
  %50 = getelementptr inbounds nuw i32, ptr %1, i64 %48, !dbg !28
  %51 = load i32, ptr %50, align 4, !dbg !28, !tbaa !29
  %52 = add nsw i32 %51, %47, !dbg !33
  store i32 %52, ptr %49, align 4, !dbg !34, !tbaa !29
  %53 = add nuw nsw i64 %32, 4, !dbg !35
    #dbg_value(i64 %53, !19, !DIExpression(), !22)
  %54 = add i64 %33, 4, !dbg !25
  %55 = icmp eq i64 %54, %13, !dbg !25
  br i1 %55, label %14, label %30, !dbg !25, !llvm.loop !39
}

; Function Attrs: mustprogress nocallback nofree nosync nounwind willreturn memory(argmem: readwrite)
declare void @llvm.lifetime.start.p0(i64 immarg, ptr captures(none)) #1

; Function Attrs: mustprogress nocallback nofree nosync nounwind willreturn memory(argmem: readwrite)
declare void @llvm.lifetime.end.p0(i64 immarg, ptr captures(none)) #1

; Function Attrs: mustprogress nofree noinline norecurse nosync nounwind sspstrong willreturn memory(none) uwtable
define dso_local noundef i32 @_Z6squarei(i32 noundef %0) local_unnamed_addr #2 !dbg !42 {
    #dbg_value(i32 %0, !46, !DIExpression(), !47)
  %2 = mul nsw i32 %0, %0, !dbg !48
  ret i32 %2, !dbg !49
}

; Function Attrs: mustprogress nofree norecurse nosync nounwind sspstrong willreturn memory(none) uwtable
define dso_local noundef i32 @_Z10use_squarei(i32 noundef %0) local_unnamed_addr #3 !dbg !50 {
    #dbg_value(i32 %0, !52, !DIExpression(), !53)
  %2 = tail call noundef i32 @_Z6squarei(i32 noundef %0), !dbg !54
  %3 = add nsw i32 %0, 1, !dbg !55
  %4 = tail call noundef i32 @_Z6squarei(i32 noundef %3), !dbg !56
  %5 = add nsw i32 %4, %2, !dbg !57
  ret i32 %5, !dbg !58
}

; Function Attrs: mustprogress nofree norecurse nosync nounwind sspstrong willreturn memory(argmem: read) uwtable
define dso_local noundef i32 @_Z14loop_with_exitPiii(ptr noundef readonly captures(none) %0, i32 noundef %1, i32 noundef %2) local_unnamed_addr #4 !dbg !59 {
    #dbg_value(ptr %0, !63, !DIExpression(), !69)
    #dbg_value(i32 %1, !64, !DIExpression(), !69)
    #dbg_value(i32 %2, !65, !DIExpression(), !69)
    #dbg_value(i32 0, !66, !DIExpression(), !69)
    #dbg_value(i32 0, !67, !DIExpression(), !70)
  %4 = icmp sgt i32 %1, 0, !dbg !71
  br i1 %4, label %5, label %17, !dbg !73

5:                                                ; preds = %3
  %6 = zext nneg i32 %1 to i64, !dbg !71
  br label %7, !dbg !73

7:                                                ; preds = %5, %13
  %8 = phi i64 [ 0, %5 ], [ %15, %13 ]
  %9 = phi i32 [ 0, %5 ], [ %14, %13 ]
    #dbg_value(i64 %8, !67, !DIExpression(), !70)
    #dbg_value(i32 %9, !66, !DIExpression(), !69)
  %10 = getelementptr inbounds nuw i32, ptr %0, i64 %8, !dbg !74
  %11 = load i32, ptr %10, align 4, !dbg !74, !tbaa !29
  %12 = icmp sgt i32 %11, %2, !dbg !77
  br i1 %12, label %17, label %13, !dbg !77

13:                                               ; preds = %7
  %14 = add nsw i32 %11, %9, !dbg !78
    #dbg_value(i32 %14, !66, !DIExpression(), !69)
  %15 = add nuw nsw i64 %8, 1, !dbg !79
    #dbg_value(i64 %15, !67, !DIExpression(), !70)
  %16 = icmp eq i64 %15, %6, !dbg !71
  br i1 %16, label %17, label %7, !dbg !73, !llvm.loop !80

17:                                               ; preds = %13, %7, %3
  %18 = phi i32 [ 0, %3 ], [ %9, %7 ], [ %14, %13 ], !dbg !69
  ret i32 %18, !dbg !82
}

; Function Attrs: mustprogress nofree norecurse nosync nounwind sspstrong willreturn memory(argmem: readwrite) uwtable
define dso_local void @_Z10slp_missedPfS_S_(ptr noundef writeonly captures(none) initializes((0, 4), (8, 12), (16, 20)) %0, ptr noundef readonly captures(none) %1, ptr noundef readonly captures(none) %2) local_unnamed_addr #5 !dbg !83 {
    #dbg_value(ptr %0, !89, !DIExpression(), !92)
    #dbg_value(ptr %1, !90, !DIExpression(), !92)
    #dbg_value(ptr %2, !91, !DIExpression(), !92)
  %4 = load float, ptr %1, align 4, !dbg !93, !tbaa !94
  %5 = load float, ptr %2, align 4, !dbg !96, !tbaa !94
  %6 = fadd float %4, %5, !dbg !97
  store float %6, ptr %0, align 4, !dbg !98, !tbaa !94
  %7 = getelementptr inbounds nuw i8, ptr %1, i64 4, !dbg !99
  %8 = load float, ptr %7, align 4, !dbg !99, !tbaa !94
  %9 = getelementptr inbounds nuw i8, ptr %2, i64 4, !dbg !100
  %10 = load float, ptr %9, align 4, !dbg !100, !tbaa !94
  %11 = fadd float %8, %10, !dbg !101
  %12 = getelementptr inbounds nuw i8, ptr %0, i64 8, !dbg !102
  store float %11, ptr %12, align 4, !dbg !103, !tbaa !94
  %13 = getelementptr inbounds nuw i8, ptr %1, i64 8, !dbg !104
  %14 = load float, ptr %13, align 4, !dbg !104, !tbaa !94
  %15 = getelementptr inbounds nuw i8, ptr %2, i64 8, !dbg !105
  %16 = load float, ptr %15, align 4, !dbg !105, !tbaa !94
  %17 = fadd float %14, %16, !dbg !106
  %18 = getelementptr inbounds nuw i8, ptr %0, i64 16, !dbg !107
  store float %17, ptr %18, align 4, !dbg !108, !tbaa !94
  ret void, !dbg !109
}

; Function Attrs: mustprogress sspstrong uwtable
define dso_local void @_Z9sroa_failv() local_unnamed_addr #6 !dbg !110 {
  %1 = alloca i32, align 4, !DIAssignID !115
    #dbg_assign(i1 poison, !114, !DIExpression(), !115, ptr %1, !DIExpression(), !116)
  call void @llvm.lifetime.start.p0(i64 4, ptr nonnull %1) #8, !dbg !117
  store i32 42, ptr %1, align 4, !dbg !118, !tbaa !29, !DIAssignID !119
    #dbg_assign(i32 42, !114, !DIExpression(), !119, ptr %1, !DIExpression(), !116)
  call void @_Z6escapePi(ptr noundef nonnull %1), !dbg !120
  call void @llvm.lifetime.end.p0(i64 4, ptr nonnull %1) #8, !dbg !121
  ret void, !dbg !121
}

declare !dbg !122 void @_Z6escapePi(ptr noundef) local_unnamed_addr #7

attributes #0 = { mustprogress nofree norecurse nosync nounwind sspstrong memory(argmem: readwrite) uwtable "min-legal-vector-width"="0" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #1 = { mustprogress nocallback nofree nosync nounwind willreturn memory(argmem: readwrite) }
attributes #2 = { mustprogress nofree noinline norecurse nosync nounwind sspstrong willreturn memory(none) uwtable "min-legal-vector-width"="0" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #3 = { mustprogress nofree norecurse nosync nounwind sspstrong willreturn memory(none) uwtable "min-legal-vector-width"="0" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #4 = { mustprogress nofree norecurse nosync nounwind sspstrong willreturn memory(argmem: read) uwtable "min-legal-vector-width"="0" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #5 = { mustprogress nofree norecurse nosync nounwind sspstrong willreturn memory(argmem: readwrite) uwtable "min-legal-vector-width"="0" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #6 = { mustprogress sspstrong uwtable "min-legal-vector-width"="0" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #7 = { "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #8 = { nounwind }

!llvm.dbg.cu = !{!0}
!llvm.module.flags = !{!2, !3, !4, !5, !6, !7, !8}
!llvm.ident = !{!9}

!0 = distinct !DICompileUnit(language: DW_LANG_C_plus_plus_14, file: !1, producer: "clang version 21.1.6", isOptimized: true, runtimeVersion: 0, emissionKind: FullDebug, splitDebugInlining: false, nameTableKind: None)
!1 = !DIFile(filename: "test/cases.cpp", directory: "/home/serosh/scratch/aion", checksumkind: CSK_MD5, checksum: "ce27b29466640b146bac85bf8a26c74c")
!2 = !{i32 7, !"Dwarf Version", i32 5}
!3 = !{i32 2, !"Debug Info Version", i32 3}
!4 = !{i32 1, !"wchar_size", i32 4}
!5 = !{i32 8, !"PIC Level", i32 2}
!6 = !{i32 7, !"PIE Level", i32 2}
!7 = !{i32 7, !"uwtable", i32 2}
!8 = !{i32 7, !"debug-info-assignment-tracking", i1 true}
!9 = !{!"clang version 21.1.6"}
!10 = distinct !DISubprogram(name: "loop_dependency", linkageName: "_Z15loop_dependencyPiS_i", scope: !1, file: !1, line: 2, type: !11, scopeLine: 2, flags: DIFlagPrototyped | DIFlagAllCallsDescribed, spFlags: DISPFlagDefinition | DISPFlagOptimized, unit: !0, retainedNodes: !15)
!11 = !DISubroutineType(types: !12)
!12 = !{null, !13, !13, !14}
!13 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !14, size: 64)
!14 = !DIBasicType(name: "int", size: 32, encoding: DW_ATE_signed)
!15 = !{!16, !17, !18, !19}
!16 = !DILocalVariable(name: "a", arg: 1, scope: !10, file: !1, line: 2, type: !13)
!17 = !DILocalVariable(name: "b", arg: 2, scope: !10, file: !1, line: 2, type: !13)
!18 = !DILocalVariable(name: "n", arg: 3, scope: !10, file: !1, line: 2, type: !14)
!19 = !DILocalVariable(name: "i", scope: !20, file: !1, line: 3, type: !14)
!20 = distinct !DILexicalBlock(scope: !10, file: !1, line: 3, column: 5)
!21 = !DILocation(line: 0, scope: !10)
!22 = !DILocation(line: 0, scope: !20)
!23 = !DILocation(line: 3, column: 23, scope: !24)
!24 = distinct !DILexicalBlock(scope: !20, file: !1, line: 3, column: 5)
!25 = !DILocation(line: 3, column: 5, scope: !20)
!26 = !DILocation(line: 4, column: 16, scope: !27)
!27 = distinct !DILexicalBlock(scope: !24, file: !1, line: 3, column: 33)
!28 = !DILocation(line: 4, column: 25, scope: !27)
!29 = !{!30, !30, i64 0}
!30 = !{!"int", !31, i64 0}
!31 = !{!"omnipotent char", !32, i64 0}
!32 = !{!"Simple C++ TBAA"}
!33 = !DILocation(line: 4, column: 23, scope: !27)
!34 = !DILocation(line: 4, column: 14, scope: !27)
!35 = !DILocation(line: 3, column: 29, scope: !24)
!36 = distinct !{!36, !37}
!37 = !{!"llvm.loop.unroll.disable"}
!38 = !DILocation(line: 6, column: 1, scope: !10)
!39 = distinct !{!39, !25, !40, !41}
!40 = !DILocation(line: 5, column: 5, scope: !20)
!41 = !{!"llvm.loop.mustprogress"}
!42 = distinct !DISubprogram(name: "square", linkageName: "_Z6squarei", scope: !1, file: !1, line: 10, type: !43, scopeLine: 10, flags: DIFlagPrototyped | DIFlagAllCallsDescribed, spFlags: DISPFlagDefinition | DISPFlagOptimized, unit: !0, retainedNodes: !45)
!43 = !DISubroutineType(types: !44)
!44 = !{!14, !14}
!45 = !{!46}
!46 = !DILocalVariable(name: "x", arg: 1, scope: !42, file: !1, line: 10, type: !14)
!47 = !DILocation(line: 0, scope: !42)
!48 = !DILocation(line: 11, column: 14, scope: !42)
!49 = !DILocation(line: 11, column: 5, scope: !42)
!50 = distinct !DISubprogram(name: "use_square", linkageName: "_Z10use_squarei", scope: !1, file: !1, line: 14, type: !43, scopeLine: 14, flags: DIFlagPrototyped | DIFlagAllCallsDescribed, spFlags: DISPFlagDefinition | DISPFlagOptimized, unit: !0, retainedNodes: !51)
!51 = !{!52}
!52 = !DILocalVariable(name: "x", arg: 1, scope: !50, file: !1, line: 14, type: !14)
!53 = !DILocation(line: 0, scope: !50)
!54 = !DILocation(line: 15, column: 12, scope: !50)
!55 = !DILocation(line: 15, column: 33, scope: !50)
!56 = !DILocation(line: 15, column: 24, scope: !50)
!57 = !DILocation(line: 15, column: 22, scope: !50)
!58 = !DILocation(line: 15, column: 5, scope: !50)
!59 = distinct !DISubprogram(name: "loop_with_exit", linkageName: "_Z14loop_with_exitPiii", scope: !1, file: !1, line: 19, type: !60, scopeLine: 19, flags: DIFlagPrototyped | DIFlagAllCallsDescribed, spFlags: DISPFlagDefinition | DISPFlagOptimized, unit: !0, retainedNodes: !62)
!60 = !DISubroutineType(types: !61)
!61 = !{!14, !13, !14, !14}
!62 = !{!63, !64, !65, !66, !67}
!63 = !DILocalVariable(name: "a", arg: 1, scope: !59, file: !1, line: 19, type: !13)
!64 = !DILocalVariable(name: "n", arg: 2, scope: !59, file: !1, line: 19, type: !14)
!65 = !DILocalVariable(name: "threshold", arg: 3, scope: !59, file: !1, line: 19, type: !14)
!66 = !DILocalVariable(name: "sum", scope: !59, file: !1, line: 20, type: !14)
!67 = !DILocalVariable(name: "i", scope: !68, file: !1, line: 21, type: !14)
!68 = distinct !DILexicalBlock(scope: !59, file: !1, line: 21, column: 5)
!69 = !DILocation(line: 0, scope: !59)
!70 = !DILocation(line: 0, scope: !68)
!71 = !DILocation(line: 21, column: 23, scope: !72)
!72 = distinct !DILexicalBlock(scope: !68, file: !1, line: 21, column: 5)
!73 = !DILocation(line: 21, column: 5, scope: !68)
!74 = !DILocation(line: 22, column: 13, scope: !75)
!75 = distinct !DILexicalBlock(scope: !76, file: !1, line: 22, column: 13)
!76 = distinct !DILexicalBlock(scope: !72, file: !1, line: 21, column: 33)
!77 = !DILocation(line: 22, column: 18, scope: !75)
!78 = !DILocation(line: 23, column: 13, scope: !76)
!79 = !DILocation(line: 21, column: 29, scope: !72)
!80 = distinct !{!80, !73, !81, !41}
!81 = !DILocation(line: 24, column: 5, scope: !68)
!82 = !DILocation(line: 25, column: 5, scope: !59)
!83 = distinct !DISubprogram(name: "slp_missed", linkageName: "_Z10slp_missedPfS_S_", scope: !1, file: !1, line: 29, type: !84, scopeLine: 29, flags: DIFlagPrototyped | DIFlagAllCallsDescribed, spFlags: DISPFlagDefinition | DISPFlagOptimized, unit: !0, retainedNodes: !88)
!84 = !DISubroutineType(types: !85)
!85 = !{null, !86, !86, !86}
!86 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !87, size: 64)
!87 = !DIBasicType(name: "float", size: 32, encoding: DW_ATE_float)
!88 = !{!89, !90, !91}
!89 = !DILocalVariable(name: "a", arg: 1, scope: !83, file: !1, line: 29, type: !86)
!90 = !DILocalVariable(name: "b", arg: 2, scope: !83, file: !1, line: 29, type: !86)
!91 = !DILocalVariable(name: "c", arg: 3, scope: !83, file: !1, line: 29, type: !86)
!92 = !DILocation(line: 0, scope: !83)
!93 = !DILocation(line: 30, column: 12, scope: !83)
!94 = !{!95, !95, i64 0}
!95 = !{!"float", !31, i64 0}
!96 = !DILocation(line: 30, column: 19, scope: !83)
!97 = !DILocation(line: 30, column: 17, scope: !83)
!98 = !DILocation(line: 30, column: 10, scope: !83)
!99 = !DILocation(line: 31, column: 12, scope: !83)
!100 = !DILocation(line: 31, column: 19, scope: !83)
!101 = !DILocation(line: 31, column: 17, scope: !83)
!102 = !DILocation(line: 31, column: 5, scope: !83)
!103 = !DILocation(line: 31, column: 10, scope: !83)
!104 = !DILocation(line: 32, column: 12, scope: !83)
!105 = !DILocation(line: 32, column: 19, scope: !83)
!106 = !DILocation(line: 32, column: 17, scope: !83)
!107 = !DILocation(line: 32, column: 5, scope: !83)
!108 = !DILocation(line: 32, column: 10, scope: !83)
!109 = !DILocation(line: 33, column: 1, scope: !83)
!110 = distinct !DISubprogram(name: "sroa_fail", linkageName: "_Z9sroa_failv", scope: !1, file: !1, line: 37, type: !111, scopeLine: 37, flags: DIFlagPrototyped | DIFlagAllCallsDescribed, spFlags: DISPFlagDefinition | DISPFlagOptimized, unit: !0, retainedNodes: !113)
!111 = !DISubroutineType(types: !112)
!112 = !{null}
!113 = !{!114}
!114 = !DILocalVariable(name: "x", scope: !110, file: !1, line: 38, type: !14)
!115 = distinct !DIAssignID()
!116 = !DILocation(line: 0, scope: !110)
!117 = !DILocation(line: 38, column: 5, scope: !110)
!118 = !DILocation(line: 38, column: 9, scope: !110)
!119 = distinct !DIAssignID()
!120 = !DILocation(line: 39, column: 5, scope: !110)
!121 = !DILocation(line: 40, column: 1, scope: !110)
!122 = !DISubprogram(name: "escape", linkageName: "_Z6escapePi", scope: !1, file: !1, line: 36, type: !123, flags: DIFlagPrototyped, spFlags: DISPFlagOptimized)
!123 = !DISubroutineType(types: !124)
!124 = !{null, !13}
