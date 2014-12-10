
device    <- "default"
fontScale <- 4.0


plotBenchmarkSummary("stack",
                     units          = c ("simstack",
                                         "lockfreestack"),
                     execId         = "s4_t1-14_n32000_i200000_r0_ia10-1000__1",
                     plotTitles     = c("Stack buffer benchmark"),
                     axisXLabel     = "threads",
                     ops            = c("Add", "RemoveAny"),
                     featureName    = "opsPerSec",
                     groupAttribute = "numElements",
                     plotColSpan    = 2,
                     showLegend     = TRUE,
                     legendTitle    = "Preallocation %",
                     showTitleY     = TRUE,
                     plotDevice     = device,
                     fontScale      = fontScale,
                     commonYScale   = FALSE)


if (F) {
plotBenchmarkLatencyData("pool",
                         units         = c("michaelscott-ap",
                                           "koganpetrank-pl"),
                         scenario      = 4,
                         execNrs       = c(0,1),
                         selectedOps   = c("Buffer"),
                         plotTitle     = "Queue buffer benchmark: 1:1 and 2:2 producer:consumer threads",
                         facetsTitles  = c("2 threads", 
                                           "4 threads"),
                         plotType      = "violin", 
                         plotDevice    = device, 
                         fontScale     = fontScale,
                         maxLabelHeadroom = 1.8)
}
if (F) {
plotBenchmarkLatencyData("pool",
                         units         = c("treepool",
                                           "compartmentpool",
                                           "arraypool"),
                         scenario      = 0,
                         execNrs       = c(18,19,20),
                         selectedOps   = c("RemoveAny"),
                         plotTitle     = "Pool benchmark scenario ED-P: Enqueue/dequeue with increasing preallocation",
                         facetsTitles  = c("prealloc: 0", 
                                           "prealloc: 2500", 
                                           "prealloc: 5000"),
                         plotType      = "violin", 
                         plotDevice    = device, 
                         fontScale     = fontScale,
                         maxLabelHeadroom = 1.8)
}
if (F) {
plotBenchmarkSummary("pool",
                     units          = c ("treepool",
                                         "arraypool",
                                         "compartmentpool"),
                     execId         = "s2_t2-32_n200000-1e06_i1_r0_iaMax",
                     plotTitles     = c("Pool benchmark Fill-Up: Increasing capacity"),
                     axisXLabel     = "threads",
                     axisYLabel     = "operations / s",
                     ops            = c("Add", "RemoveAny"),
                     featureName    = "opsPerSec",
                     groupAttribute = "numElements",
                     plotColSpan    = 2,
                     showLegend     = TRUE,
                     legendTitle    = "Preallocation %",
                     showTitleY     = TRUE,
                     plotDevice     = device,
                     fontScale      = fontScale,
                     commonYScale   = FALSE)
}
if (F) {
#
plotBenchmarkSummary("pool",
                     units          = c ("treepool",
                                         "arraypool",
                                         "compartmentpool"),
                     execId         = "s2_t2-32_n100000-500000_i1_r0_iaMax",
                     plotTitles     = c("Pool benchmark Fill-Up: Increasing capacity"),
                     axisXLabel     = "threads",
                     ops            = c("Add", "RemoveAny"),
                     featureName    = "opsPerSec",
                     groupAttribute = "numElements",
                     plotColSpan    = 2,
                     showLegend     = TRUE,
                     legendTitle    = "Preallocation %",
                     showTitleY     = TRUE,
                     plotDevice     = device,
                     fontScale      = fontScale,
                     commonYScale   = FALSE)
}
if (F) {
plotBenchmarkSummary("pool",
                     units          = c ("treepool",
                                         "arraypool",
                                         "compartmentpool"),
                     execId         = "s0_t2-32_n100000_i1e06_r0-50_ia0",
                     plotTitles     = c("Pool benchmark ED-P: Preallocation from 0% to 75%"),
                     axisXLabel     = "threads",
                     ops            = c("Add", "RemoveAny"),
                     featureName    = "opsPerSec",
                     groupAttribute = "rPrealloc",
                     plotColSpan    = 2,
                     showLegend     = TRUE,
                     legendTitle    = "Preallocation %",
                     showTitleY     = TRUE,
                     plotDevice     = device,
                     fontScale      = fontScale,
                     commonYScale   = TRUE)
}
if (F) {
plotBenchmarkSummary("pool",
                     units          = c ("treepool",
                                         "compartmentpool",
                                         "arraypool"),
                     execId         = "s0_t1-16_n100000_i1e06_r0-75_ia0",
                     plotTitles     = c("Pool benchmark ED-P: Preallocation from 0% to 75%"),
                     axisXLabel     = "threads",
                     ops            = c("Add", "RemoveAny"),
                     featureName    = "opsPerSec",
                     groupAttribute = "rPrealloc",
                     plotColSpan    = 2,
                     showLegend     = TRUE,
                     legendTitle    = "Preallocation %",
                     showTitleY     = TRUE,
                     plotDevice     = device,
                     fontScale      = fontScale,
                     commonYScale   = TRUE)
}
if (F) {
#
plotBenchmarkSummary("pool",
                     units          = c ("treepool",
                                         "compartmentpool",
                                         "arraypool"),
                     execId         = "s3_t2-32_n320000_i1_r50_ia5000",
                     plotTitles     = c("Pool benchmark ADR: Increasing number of producers"),
                     axisXLabel     = "threads",
                     ops            = c("Add", "RemoveAny"),
                     featureName    = "opsPerSec",
                     groupAttribute = "numIterations",
                     plotColSpan    = 2,
                     showLegend     = TRUE,
                     legendTitle    = NA,
                     showTitleY     = TRUE,
                     plotDevice     = device,
                     fontScale      = fontScale,
                     commonYScale   = TRUE)
}
if (F) {
plotBenchmarkLatencyData("pool",
                         units         = c("treepool",
                                           "compartmentpool",
                                           "arraypool"),
                         scenario      = 0,
                         execNrs       = c(0,1,2),
                         selectedOps   = c("Add", "RemoveAny"),
                         plotTitle     = "Pool benchmark scenario ED-P: Enqueue/dequeue with increasing preallocation",
                         facetsTitles  = c("prealloc: 0", 
                                           "prealloc: 2500", 
                                           "prealloc: 5000"),
                         plotType      = "violin", 
                         plotDevice    = device, 
                         fontScale     = fontScale)
}
if (F) {
plotBenchmarkSummary("queue",
                     units          = c ("koganpetrank-pl", 
                                       "michaelscott"),
                     execId         = "s4.0_t2-20_n30000_i200000_r0-75_ia1",
                     plotTitles     = c("Queue buffer benchmark, preallocation from 0% to 75%"),
                     axisXLabel     = "threads",
                     ops            = c("Add", "RemoveAny"),
                     featureName    = "opsPerSec",
                     groupAttribute = "rPrealloc",
                     plotColSpan    = 2,
                     showLegend     = TRUE,
                     legendTitle    = "Preallocation %",
                     showTitleY     = TRUE,
                     plotDevice     = device,
                     fontScale      = fontScale,
                     commonYScale   = TRUE)
}
if (F) {
plotBenchmarkSummary("queue",
                     units          = c ("koganpetrank-pl", 
                                         "michaelscott"),
                     execId         = "s3.0_t2-20_n30000_i100000-300000_r0_ia1",
                     plotTitles     = c("Queue buffer benchmark, preallocation from 0% to 75%"),
                     axisXLabel     = "threads",
                     ops            = c("Add", "RemoveAny"),
                     featureName    = "opsPerSec",
                     groupAttribute = "numIterations",
                     plotColSpan    = 2,
                     showLegend     = TRUE,
                     legendTitle    = "Preallocation %",
                     showTitleY     = TRUE,
                     plotDevice     = device,
                     fontScale      = fontScale,
                     commonYScale   = TRUE)
}

