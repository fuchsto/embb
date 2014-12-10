
source('theme.R')
source('util.R')

library(ggplot2)
library(gtable)
library(gridExtra)
library(grid)
require(tikzDevice)
library(scales)
library(plyr)

## Usage:
##
##   plotBenchmarkSummary(datatype    = "pool",
##                        units       = c("treepool"),
##                        execId      = "s0_t1-16_n100000_i100000_r0-75_ia0",
##                        featureName = "latMedian",
##                        ops         = c("new", "free"),
##                        plotTitles  = c("allocation", "deallocation"))
##
##  plotBenchmarkSummary("pool", c("arraypool", "treepool"), 0,
##                       ops            = c("opsPerSec"),
##                       plotTitles     = c("Throughput array", "Throughput tree"),
##                       featureName    = "opsPerSec",
##                       groupAttribute = "numElements",
##                       execId         = "s0_t1-16_n100000_i100000_r0-75_ia0")
##
plotBenchmarkSummary = function(datatype,
                                units,
                                execId,
                                plotTitles   = c(),
                                axisXLabel   = "threads",
                                axisYLabel   = NA,
                                ops,
                                featureName,
                                groupAttribute,
                                plotColSpan  = 2,
                                showLegend   = FALSE,
                                legendTitle  = "",
                                showTitleY   = TRUE,
                                plotDevice   = "default",
                                commonYScale = TRUE,
                                dframeOnly   = FALSE,
                                ...)
{
  units      <- c(units)
  plotTitles <- c(plotTitles)

  sumData = collectSummaryDataFrame(datatype       = datatype,
                                    units          = units,
                                    ops            = ops,
                                    featureName    = featureName,
                                    groupAttribute = groupAttribute,
                                    execId         = execId)
  if (dframeOnly) {
    return(sumData)
  }

  benchmarkParams <- list(...)

  baseDataFilePath <- Sys.getenv(c("BENCHMARK_DATAFILES_BASEPATH"))
  basePlotFilePath <- Sys.getenv(c("BENCHMARK_PLOTFILES_BASEPATH"))

  if (is.na(file.info(baseDataFilePath)["isdir"])) {
    write("BENCHMARK_DATAFILES_BASEPATH does not exist", stderr())
    write(baseDataFilePath, stderr())
    return ()
  }
  if (is.na(file.info(basePlotFilePath)["isdir"])) {
    write("BENCHMARK_PLOTFILES_BASEPATH does not exist", stderr())
    write(basePlotFilePath, stderr())
    return ()
  }

  benchmarkStrPar <- benchmarkParams
  benchmarkParamI <- -1
  benchmarkParamN <- -1
  benchmarkParamR <- -1
  if (length(benchmarkStrPar$i) > 0) {
    benchmarkStrPar$i <- f2si(benchmarkStrPar$i, space = FALSE)
  }
  if (length(benchmarkStrPar$n) > 0) {
    benchmarkStrPar$n <- f2si(benchmarkStrPar$n, space = FALSE)
  }
  benchmarkConfStr <- paste(names(benchmarkStrPar), unlist(benchmarkStrPar), sep='-', collapse='--')

  featureNames   <- c("opsPerSec",
                      "latMin", "latMax", "latMedian",
                      "op:throughputTime", "op:numOps", "op:opsPerSec",
                      "jitNeg", "jitPos", "jitSum")
  featureColumns <- function(op) {
                     featureToColumn = function(s) {
                       featPre = toupper(substring(s, 1,1))
                       featSuf = substring(s, first = 2)
                       paste(op, featPre, featSuf, sep = "")
                     }
                     c("opsPerSec",
                       sapply(
                         c("latMin", "latMax", "latMedian", "throughputTime", "numOps", "opsPerSec"),
                         featureToColumn),
                       sapply(
                         c("jitNeg", "jitPos", "jitSum", "throughputTime", "numOps", "opsPerSec"),
                         featureToColumn))
                   }
  featureLabels  <- c("operations / sec",
                      "minimum latency ($\\mu$s)", "maximum latency ($\\mu$s)", "median latency ($\\mu$s)",
                      "troughput time", "number of operations", "operations / s",
                      "negative jitter ($\\mu$s)", "positive jitter ($\\mu$s)", "jitter range ($\\mu$s)")

  featureIndex   <- which(featureNames == featureName)

  vfColumns <- sapply(ops, function(op) { featureColumns(op) })
  fColumns  <- sapply(ops, function(op) { featureColumns(op)[featureIndex] })
  fLabel    <- ifelse(is.na(axisYLabel), featureLabels[featureIndex], axisYLabel)

  plotBasePath <- paste(basePlotFilePath, datatype, sep = '/')
  plotFileName <- paste(c(units, fColumns, execId), collapse = "-")
  plotFilePath <- paste(plotBasePath, plotFileName, sep = '/')
  cout("Writing ", plotFilePath, ".", plotDevice)

  ggPlots      <- list()
  nPlotsTotal  <- ifelse(groupAttribute == "op",
                         length(units),
                         ifelse(groupAttribute == "type",
                                length(fColumns),
                                length(units) * length(fColumns)))
# nPlotColumns <- ifelse(nPlotsTotal >= 2,
#                        2, 1)
  nPlotColumns <- 1
  nPlot        <- 1
  unitIds      <- sort(unique(sumData$type))

  # Units grouped in plots
  if (groupAttribute == "type") {
    cout("Plots of units ", paste(unitIds, collapse = ', '))
    # Operations in single plots
    for (fColumn in fColumns) {
      cout("Plot of feature ", fColumn)

      iTitle <- ((nPlot-1) %% nPlotColumns)+1
      pTitle <- ifelse(length(plotTitles) >= iTitle,
                       plotTitles[[iTitle]], "")
      gpRes  <- gLineplotFeaturesVsNumThreads(summaryData    = sumData,
                                              plotTitle      = pTitle,
                                              feature        = fColumn,
                                              axisYLabel     = axisYLabel,
                                              axisXLabel     = axisXLabel,
                                              selectedOps    = ops,
                                              groupAttribute = groupAttribute,
                                              showLegend     = (nPlot == 1),
                                              legendTitle    = legendTitle,
                                              showTitleY     = showTitleY,
                                              commonYScale   = commonYScale,
                                              ...)
      ggPlots[[nPlot]] <- gpRes
      nPlot <- nPlot + 1
    }
  }
  else {
    # Every unit in single plot
    cout("Plots of operations ", groupAttribute)

    if (groupAttribute == "op") {
      # Operations in one plot
      cout("Plot of operations ", paste(fColumns, collapse = ', '))

      iTitle <- ((nPlot-1) %% nPlotColumns)+1
      pTitle <- ifelse(length(plotTitles) >= iTitle,
                       plotTitles[[iTitle]], "")
      gpRes  <- gLineplotFeaturesVsNumThreads(summaryData    = sumData,
                                              plotTitle      = pTitle,
                                              feature        = fColumns,
                                              axisYLabel     = axisYLabel,
                                              axisXLabel     = axisXLabel,
                                              selectedOps    = ops,
                                              groupAttribute = groupAttribute,
                                              showLegend     = (nPlot == 1),
                                              legendTitle    = legendTitle,
                                              showTitleY     = showTitleY,
                                              commonYScale   = commonYScale,
                                              ...)
      ggPlots[[nPlot]] <- gpRes
      nPlot <- nPlot + 1
    }
    else {
      # Operations in single plots
      for (fColumn in fColumns) {
        cout("Plot of feature ", fColumn)

        iTitle <- ((nPlot-1) %% nPlotColumns)+1
        pTitle <- ifelse(length(plotTitles) >= iTitle,
                         plotTitles[[iTitle]], "")
        gpRes  <- gLineplotFeaturesVsNumThreads(summaryData    = sumData,
                                                plotTitle      = pTitle,
                                                feature        = fColumn,
                                                axisYLabel     = axisYLabel,
                                                axisXLabel     = axisXLabel,
                                                selectedOps    = ops,
                                                groupAttribute = groupAttribute,
                                                showLegend     = (nPlot == 1),
                                                legendTitle    = legendTitle,
                                                showTitleY     = showTitleY,
                                                commonYScale   = commonYScale,
                                                ...)
        ggPlots[[nPlot]] <- gpRes
        nPlot <- nPlot + 1
      }
    }
  }

  colWidthInches  <- as.double(Sys.getenv(c("PLOTS_COLUMNWIDTH_IN")))
  plotTotalHeight <- 2.4 * fontScale
  if (plotDevice == "win") {
    windows(width     = colWidthInches,
            height    = plotTotalHeight,
            antialias = "cleartype")
  } else if (plotDevice == "tex") {
    ## Write plot to TEX file:
    tikz(conc(plotFilePath, ".tex"),
         engine     = "pdftex",
         standAlone = FALSE,
         pointsize  = 8,
         width      = colWidthInches,
         height     = plotTotalHeight) # was: 2.6
  } else if (plotDevice == "default") {
    return(gpRes)
  } else {
    cout("Writing to file ", plotFilePath, ".", plotDevice)
    ggsave(filename = conc(plotFilePath, ".", plotDevice),
           plot     = gpRes,
           width    = colWidthInches,
           height   = plotTotalHeight,
           scale    = 1.0,
           units    = "in")
    }

  print(ggPlots[1])

  # y-Label element:
# yLabel <- textGrob(fLabel,
#                    rot   = 90,
#                    gp    = gpar(fontsize = 8, lineheight = 8))
# ggPlots$ncol <- nPlotColumns
# plotGrobs    <- do.call(arrangeGrob, ggPlots)
# grid.arrange(yLabel,
#              plotGrobs,
#              ncol = 2,
#              widths = unit.c(unit(0.7, "lines"),
#                              unit(1, "npc") - unit(0.5, "lines")))
  if (!previewPlot) {
    dev.off()
  }
}

collectSummaryDataFrame <- function(datatype,
                                    units,
                                    execId,
                                    ops,
                                    featureName,
                                    groupAttribute,
                                    ...)
{
  units <- c(units)
  benchmarkParams  <- list(...)

  baseDataFilePath <- Sys.getenv(c("BENCHMARK_DATAFILES_BASEPATH"))
  basePlotFilePath <- Sys.getenv(c("BENCHMARK_PLOTFILES_BASEPATH"))

  if (is.na(file.info(baseDataFilePath)["isdir"])) {
    write("BENCHMARK_DATAFILES_BASEPATH does not exist", stderr())
    write(baseDataFilePath, stderr())
    return ()
  }
  if (is.na(file.info(basePlotFilePath)["isdir"])) {
    write("BENCHMARK_PLOTFILES_BASEPATH does not exist", stderr())
    write(basePlotFilePath, stderr())
    return ()
  }

  benchmarkParamI <- -1
  benchmarkParamN <- -1
  benchmarkParamR <- -1
  if (length(benchmarkParams$i) > 0) {
    benchmarkParamI <- benchmarkParams$i
  }
  if (length(benchmarkParams$n) > 0) {
    benchmarkParamN <- benchmarkParams$n
  }
  if (length(benchmarkParams$r) > 0) {
    benchmarkParamR <- benchmarkParams$r
  }
  summaryBasePath <- paste(baseDataFilePath, datatype, sep = '/')

  cout("Search in ", summaryBasePath)
  summaryFilePaths <- c()
  for (u in units) {
    # filePattern = paste(conc(units, "_", execId), collapse = "|")
    filePattern = conc("*", u, "_", execId, ".csv")
    cout("Searching for files ", filePattern)
    unitSummaryFiles <- unlist(list.files(path       = summaryBasePath,
                                          pattern    = filePattern,
                                          recursive  = TRUE,
                                          full.names = TRUE))
    if (length(unitSummaryFiles) == 0) {
      cout("No files for unit ", u)
    }
    summaryFilePaths <- c(summaryFilePaths,
                          unitSummaryFiles)
  }
  sumData <- ldply(
    lapply(summaryFilePaths,
           function(fp) {
             as.data.frame(read.csv(fp, sep=','))
           }),
    data.frame)

  cout("Reading ", summaryFilePaths)

  subsetSummaryData <- function(dFrame, I, N, R) {
    if (I > 0) {
      if (dim(subset(dFrame, dFrame$numIterations == I))[[1]] == 0) {
        cout("No record found for i=", I)
        cout("Records found for i=", unique(dFrame$numIterations))
        return(FALSE)
      }
      dFrame <- subset(dFrame, dFrame$numIterations == I)
    }
    if (N > 0) {
      if (dim(subset(unitSumData, unitSumData$numElements == N))[[1]] == 0) {
        cout("No record found for n=", mN)
        cout("Records found for n=", unique(dFrame$numElements))
        return(FALSE)
      }
      dFrame <- subset(dFrame, dFrame$numElements == N)
    }
    if (R >= 0) {
      if (dim(subset(unitSumData, unitSumData$rPrealloc == R))[[1]] == 0) {
        cout("No record found for r=", R)
        cout("Records found for r=", unique(dFrame$rPrealloc))
        return(FALSE)
      }
      dFrame <- subset(dFrame, dFrame$rPrealloc == R)
    }
    return(dFrame)
  }
  sumData = subsetSummaryData(sumData,
                              benchmarkParamI, benchmarkParamN, benchmarkParamR)
  return(sumData)
}

gLineplotFeaturesVsNumThreads = function(summaryData,
                                         feature        = "opsPerSec",
                                         featureLabel   = "operations / s",
                                         axisXLabel     = "threads",
                                         axisYLabel     = "operations / s",
                                         facetAttribute = "type",
                                         groupAttribute = "numIterations",
                                         selectedOps    = c(),
                                         legendTitle    = "",
                                         plotTitle      = NA,
                                         showLegend     = TRUE,
                                         showTitleY     = TRUE,
                                         numPlotColumns = 2,
                                         commonYScale   = TRUE, 
                                         fontScale      = 1)
{
  env         <- environment()
  units       <- unique(summaryData$type)
  opNames     <- selectedOps

  write(paste("Grouping data by", groupAttribute, legendTitle), stdout())
  write(paste("Matching records in summary data:", dim(summaryData)[[1]]), stdout())

  # Possibly More than one feature on y-scale:
  yMaxValue <- max(sapply(c(feature), function(f) { summaryData[[f]] }))
  yMinValue <- min(sapply(c(feature), function(f) { summaryData[[f]] }))

  if (length(feature) > 1) {
    # Create artificial grouping and duplicate records in data frame
    # for every feature:
    summaryDataGrouped <- data.frame()
    for (featureName in feature) {
      # Duplicate original data frame:
      summaryDataFeature <- summaryData
      # Add group attribute / value to duplicate;
      summaryDataFeature[[groupAttribute]] <- featureName
      # Add a common column for values of the different features,
      # e.g.  featureValue <- 'addOpsPerSec'
      #       featureValue <- 'removeOpsPerSec'
      summaryDataFeature[['featureValue']] <- summaryData[[featureName]]
      # Aggregate duplicates new data frame that
      # provides grouping by <groupAttribute>:
      summaryDataGrouped <- ldply(
        list(summaryDataGrouped, summaryDataFeature),
        data.frame)
    }
    # Use column name used to aggregate values from
    # different features:
    feature     <- 'featureValue'
    summaryData <- summaryDataGrouped
  }

  yNumBreaks  <- 5
  ySteporder  <- 10^floor(log10(yMaxValue-yMinValue)-1)
  yStepsize   <- round(((yMaxValue-yMinValue) / ySteporder) / yNumBreaks, digits = 0)
  yBreaks     <- seq(from = yMinValue, to = yMaxValue, length.out = 6)
  if (yMinValue < 1000) {
    yLabels <- formatC(yBreaks, format = "g", digits = 2)
  } else {
    yLabels <- sapply(yBreaks, function(x) {
                 f2si(number   = x,
                      rounding = TRUE,
                      digits   = 2,
                      space    = TRUE) })
  }

  groupValues <- unique(summaryData[[groupAttribute]])
  if (groupAttribute == 'type') {
    groupValues <- units
    keyLabels   <- sapply(units, function(x) {
      (unitNames(x))[1] })
  } else {
    if (is.numeric(groupValues)) {
      keyLabels <- sapply(groupValues, function(x) {
                     f2si(number   = x,
                          digits   = 2,
                          rounding = TRUE,
                          space    = TRUE) })
    }
    else {
      keyLabels <- groupValues
    }
  }

  grouping <- factor(summaryData[[groupAttribute]], labels = keyLabels)

  legendPosition <- c(0.0, -0.5)

  # c(2,4,8,12,16,20,24,28,32)
  scaleBreaksX <- unique(summaryData$numProducers)

  a <- ggplot(
    summaryData,
    aes(x          = numProducers,
        y          = summaryData[[feature]],
        group      = grouping),
    environment = env
  ) +
    scale_x_continuous(
      breaks = scaleBreaksX
    ) +
    labs(title = plotTitle,
         x     = axisXLabel,
         y     = axisYLabel) +
    guides(fill     = guide_legend(keywidth = 2, title = legendTitle),
           linetype = guide_legend(keywidth = 2, title = legendTitle),
           shape    = guide_legend(keywidth = 2, title = legendTitle),
           colour   = guide_legend(keywidth = 2, title = legendTitle)) +
    geom_line(
      size       = 0.3 * fontScale,
      show_guide = FALSE,
      aes(linetype = grouping,
          colour   = grouping)) +
    geom_point(
      show_guide = T,
      size       = 1.5,
      aes(colour = grouping,
          shape  = grouping))

  if (commonYScale) {
    a <- a + scale_y_continuous(limits = c(yMinValue, yMaxValue),
                                breaks = yBreaks,
                                labels = yLabels,
                                name   = axisYLabel)
  }
  else {
    a <- a + scale_y_continuous(labels = f2si)
  }

  unitLabels    <- sapply(units, function(x) {
    (unitNames(x))[1] })
  facetLabeller <- function(var, value) {
    v <- value
    # Match index of value and label:
    if (var == "type") {
      return(unitLabels)
    }
    if (var == "op") {
      opLevels <- unique(dframe.agg$op)
      return(opNames[which(opLevels == v)])
    }
  }
  if (groupAttribute == 'type' & length(opNames) > 1) {
    a <- a + facet_grid(. ~ op,
                        labeller = facetLabeller)
  } else if (length(units) > 1) {
    a <- a + facet_grid(. ~ type,
                        labeller = facetLabeller)
  }
if (groupAttribute == "type") {
  a <- a +
    coreglitchScaleColorDiscrete(type = "seq", nColors = length(keyLabels))
}
else {
  a <- a +
    coreglitchScaleColorDiscrete(type = "qual", nColors = length(keyLabels))
}
  a <- a + coreglitchTheme(
      hasLegend        = showLegend,
      reducedGrid      = TRUE,
      hasAxisTitleY    = showTitleY,
      legend.direction = "horizontal",
      legend.box       = "horizontal",
      fontScale        = fontScale
    )

  a
}

gBarplotFeaturesVsNumThreads = function(summaryData,
                                        I            = 1000000,
                                        feature      = "opsPerSec",
                                        featureLabel = "operations / sec")
{
  # Select subset with given iterations:
  summarySubsetIt <- subset(summaryData, summaryData$numIterations==I &
                                         summaryData$numProducers)
  env <- environment()

  print(summarySubsetIt$numProducers)
  print(summarySubsetIt)

  ggplot(summarySubsetIt,
         aes(x = numProducers, y = newLatMedian, fill=factor(numElements)),
         environment = env) +
    labs(x = "threads",
         y = featureLabel) +
    geom_bar(stat="identity", position = "dodge") +
    scale_x_continuous(breaks=c(2,4,8,12,16,20,24,28,32)) +
    #   scale_y_continuous(breaks=c(1500000, 3000000,6000000,9000000,12000000),
    #                      labels=c("1.5m", "3m", "6m", "9m", "12m")) +
    scale_fill_discrete(guide = guide_legend(title = "capacity      "),
                        h.start = 30,
                        h = range(30,120),
                        l = range(10,60),
                        c = 20) +
    theme(legend.position = "top",
          legend.box = "horizontal",
          legend.title = element_text(size=20),
          legend.text = element_text(size = 20),
          axis.text = element_text(size=18),
          axis.title.x = element_text(size=20, face = "bold", vjust = -2),
          axis.title.y = element_text(size=22, face = "bold", vjust = 3),
          panel.grid.minor.x = element_blank(),
          plot.margin=unit(c(.5, .5, .5, .5), "in"),
          legend.key.size = unit(10,"mm"),
          legend.key.width = unit(6, "mm"))
}
