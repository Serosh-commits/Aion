; ModuleID = 'test/sroa/sroa_address_escape.c'
source_filename = "test/sroa/sroa_address_escape.c"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-i128:128-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux-gnu"

%struct.S = type { i32, i32 }

; Function Attrs: nounwind sspstrong uwtable
define dso_local i32 @test(i32 noundef %0) local_unnamed_addr #0 !dbg !8 {
  %2 = alloca %struct.S, align 4
  call void @llvm.lifetime.start.p0(i64 8, ptr nonnull %2) #3, !dbg !11
  store i32 %0, ptr %2, align 4, !dbg !12, !tbaa !13
  %3 = getelementptr inbounds nuw i8, ptr %2, i64 4, !dbg !12
  store i32 0, ptr %3, align 4, !dbg !12, !tbaa !18
  call void @escape(ptr noundef nonnull %2) #3, !dbg !19
  %4 = load i32, ptr %3, align 4, !dbg !20, !tbaa !18
  call void @llvm.lifetime.end.p0(i64 8, ptr nonnull %2) #3, !dbg !21
  ret i32 %4, !dbg !22
}

; Function Attrs: mustprogress nocallback nofree nosync nounwind willreturn memory(argmem: readwrite)
declare void @llvm.lifetime.start.p0(i64 immarg, ptr captures(none)) #1

declare void @escape(ptr noundef) local_unnamed_addr #2

; Function Attrs: mustprogress nocallback nofree nosync nounwind willreturn memory(argmem: readwrite)
declare void @llvm.lifetime.end.p0(i64 immarg, ptr captures(none)) #1

attributes #0 = { nounwind sspstrong uwtable "min-legal-vector-width"="0" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #1 = { mustprogress nocallback nofree nosync nounwind willreturn memory(argmem: readwrite) }
attributes #2 = { "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #3 = { nounwind }

!llvm.dbg.cu = !{!0}
!llvm.module.flags = !{!2, !3, !4, !5, !6}
!llvm.ident = !{!7}

!0 = distinct !DICompileUnit(language: DW_LANG_C11, file: !1, producer: "clang version 21.1.8", isOptimized: true, runtimeVersion: 0, emissionKind: LineTablesOnly, splitDebugInlining: false, nameTableKind: None)
!1 = !DIFile(filename: "test/sroa/sroa_address_escape.c", directory: "/home/serosh/scratch/aion")
!2 = !{i32 2, !"Debug Info Version", i32 3}
!3 = !{i32 1, !"wchar_size", i32 4}
!4 = !{i32 8, !"PIC Level", i32 2}
!5 = !{i32 7, !"PIE Level", i32 2}
!6 = !{i32 7, !"uwtable", i32 2}
!7 = !{!"clang version 21.1.8"}
!8 = distinct !DISubprogram(name: "test", scope: !1, file: !1, line: 3, type: !9, scopeLine: 3, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition | DISPFlagOptimized, unit: !0)
!9 = !DISubroutineType(types: !10)
!10 = !{}
!11 = !DILocation(line: 3, column: 19, scope: !8)
!12 = !DILocation(line: 3, column: 32, scope: !8)
!13 = !{!14, !15, i64 0}
!14 = !{!"S", !15, i64 0, !15, i64 4}
!15 = !{!"int", !16, i64 0}
!16 = !{!"omnipotent char", !17, i64 0}
!17 = !{!"Simple C/C++ TBAA"}
!18 = !{!14, !15, i64 4}
!19 = !DILocation(line: 3, column: 40, scope: !8)
!20 = !DILocation(line: 3, column: 63, scope: !8)
!21 = !DILocation(line: 3, column: 66, scope: !8)
!22 = !DILocation(line: 3, column: 54, scope: !8)
