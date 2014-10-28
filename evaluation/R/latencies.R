
# 'C:/Users//fuchsto//Workspaces//thesis.git//Benchmarks/data/armQuad/embb-benchmarks/'

library(ggplot2)
library(gtable)
library(gridExtra)
library(grid)
library(tikzDevice)
library(reshape2)
library(plyr)
library(scales)
library(dplyr)

source('colors.R')
source('util.R')

# l.dframe <- collectLatencyDataFrame(
#    dataBasePath = 'C:/Users/fuchsto/Workspaces/thesis.git/Benchmarks/data/armQuad/',
#    units       = c("treepool",
#                    "compartmentpool",
#                    "arraypool"),
#    scenario    = 0,
#    execNrs     = c(35),
#    execId      = '',
#    selectedOps = c("Add", "RemoveAny"))


## Usage:
##
##   plotBenchmarkLatencyData("pool",
##                            units       = c("treepool",
##                                            "compartmentpool",
##                                            "arraypool"),
##                            scenario    = 0,
##                            execNrs     = seq(30,31),
##                            selectedOps = c("Add", "RemoveAny"),
##                            plotType    = "jitter" | "histogram" | "boxplot",
##                            previewPlot = TRUE)
##
plotBenchmarkLatencyData <- function(datatype,
                                     units,
                                     scenario,
                                     execNrs,
                                     selectedOps,
                                     execId       = '',
                                     plotType     = "violin",
                                     plotDevice   = "eps",
                                     ...)
{
  baseDataFilePath <- Sys.getenv(c("BENCHMARK_DATAFILES_BASEPATH"))
  basePlotFilePath <- Sys.getenv(c("BENCHMARK_PLOTFILES_BASEPATH"))

  baseDataFilePath <- paste(baseDataFilePath, "armQuad", sep = '/')

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

  dframe <- collectLatencyDataFrame(
    baseDataFilePath,
    units       = units,
    scenario    = scenario,
    execNrs     = execNrs,
    execId      = execId,
    selectedOps = selectedOps)

  cout("Latency data records: ", dim(dframe))

  execNrsF <- sprintf("%02d", execNrs)
  execNrsF <- ifelse(length(execNrsF) == 0,
                     execNrsF,
                     conc(execNrsF[[1]], "-",
                          execNrsF[[length(execNrsF)]]))

  plotBasePath <- paste(basePlotFilePath, datatype, sep = '/')
  plotFileName <- conc("latency-",
                       paste(c(units, conc("s", scenario), "dataset", execNrsF, selectedOps),
                             collapse = "-"))
  plotFilePath <- paste(plotBasePath, plotFileName, sep = '/')

  fontScale = 1
  if (plotDevice == "win") {
    fontScale = 1.3
  }

  if (plotType == "jitter") {
    gp <- gOperationLatencyJitterPlot(
      dframe, selectedOps = selectedOps, fontScale = fontScale, ...)
  } else if(plotType == "violin") {
    gp <- gOperationLatencyViolinPlot(
      dframe, selectedOps = selectedOps, fontScale = fontScale, ...)
  } else if(plotType == "histogram") {
    gp <- gOperationLatencyHistogram(
      dframe, selectedOps = selectedOps, fontScale = fontScale, ...)
  } else if(plotType == "heat") {
    gp <- gOperationLatencyHeatPlot(
      dframe, selectedOps = selectedOps, fontScale = fontScale, ...)
  } else if(plotType == "boxplot") {
    gp <- gOperationLatencyBoxPlot(
      dframe, selectedOps = selectedOps, fontScale = fontScale, ...)
  }

  # Shrink height of each additional row progressively:
  plotBaseHeight  <- 3.1
  nPlotFacetRows  <- length(execNrs)
  plotTotalHeight <- plotBaseHeight +
                       (0.3 * (nPlotFacetRows-1) * plotBaseHeight)
  colWidthInches  <- as.double(Sys.getenv(c("PLOTS_COLUMNWIDTH_IN")))

  if (plotDevice == "tex") {
    cout("Writing to file ", plotFilePath, ".tex")
    tikz(conc(plotFilePath, ".tex"),
         engine     = "pdftex",
         standAlone = FALSE,
         pointsize  = 8,
         width      = colWidthInches,
         height     = plotTotalHeight)
  }
  else if (plotDevice == "eps") {
    cout("Writing to file ", plotFilePath, ".eps")
    ggsave(filename = conc(plotFilePath, ".eps"),
           plot     = gp,
           width    = colWidthInches,
           height   = plotTotalHeight,
           scale    = 1.0,
           units    = "in")
  }
  else if (plotDevice == "win") {
    cout("Output file path: ", plotFilePath, ".eps")
    windows(width     = colWidthInches,
            height    = plotTotalHeight,
            antialias = "cleartype")
  }

  print(gp)

  if (plotDevice != "win") {
    dev.off()
  }

  return(TRUE)
}

readLatData <- function(filename) {
  if (!file.exists(filename)) {
    cout("Data file does not exist: ", filename)
  }

  baseDataFilePath <- Sys.getenv(c("BENCHMARK_DATAFILES_BASEPATH"))
  shortFilePath    <- gsub(filename,
                           pattern     = baseDataFilePath,
                           replacement = '')
  cout("Reading ", shortFilePath)
  sample <- read.csv(filename, sep = ",",
                     comment.char  = '#',
                     skipNul       = T,
                     header        = TRUE,
                     fill          = TRUE,
                     nrows         = 1)
  nCols <- dim(sample)[[2]]
  df <- read.csv(filename, sep = ",",
                 comment.char  = '#',
                 skipNul       = T,
                 header        = TRUE,
                 fill          = TRUE,
                 skip          = 1,
                 colClasses    = rep("numeric", nCols))
  cout("Data dimensions: ", dim(df))
  # Delete superfluous column:
  if (nCols %% 2 > 0) {
    df[[nCols]] <- NULL
    nCols <- nCols-1
  }
  nThreads <- nCols / 2

  cols <- c(sapply(seq(0, nThreads-1),
                   function(x) {
                     c(paste("t", x, "s", sep = ''),
                       paste("t", x, "e", sep = '')) }))
  names(df) <- cols
  return(as.data.frame(df))
}

##    dframe <- collectLatencyDataFrame(
##      'C:/Users/fuchsto/Workspaces/thesis.git/Benchmarks/data/armQuad/',
##      units = c("treepool", "compartmentpool"), scenario = 0,
##      execNrs = seq(33,35), selectedOps = c("RemoveAny", "Add"))
collectLatencyDataFrame <- function(dataBasePath,
                                    units,
                                    scenario,
                                    execNrs,
                                    execId      = '',
                                    selectedOps = c(":all:"))
{
  env <- environment()
  cout("collectLatencyDataFrame(\n",
       "    '", dataBasePath, "', \n",
       "    units = c(", paste(conc("'", units, "'"), sep = '', collapse = ','), "), \n",
       "    ", scenario, ", \n",
       "    execNrs = c(", paste(execNrs, sep = '', collapse = ','), "), \n",
       "    execId = '", execId, "', \n",
       "    selectedOps = c(", conc(paste("'", selectedOps, "'", sep = '', collapse = ',')), "))")

  execNrsF  <- sprintf("%02d", execNrs)
  dataFiles <- sapply(
    units,
    function(u) {
      unitDataPath <- conc(dataBasePath, '/', u, "/")
      unitPattern  <- paste("data_", execNrsF, "__(.+)?", execId,
                            sep = "", collapse = "|")
      cout("Searching for ", conc(unitDataPath, unitPattern))
      list(
        list.files(path      = unitDataPath,
                   recursive = TRUE,
                   pattern   = unitPattern))
    })
  if (length(dataFiles) < 1) {
    cout("No data files found")
    return(FALSE)
  }
  dataFiles <- lapply(
    dataFiles,
    function(l) {
      l[grep(l, pattern = conc("_s", scenario, "_"))]
    })
  if (length(dataFiles) < 1) {
    cout("Malformed data file name")
    return(FALSE)
  }
  dFilesVec <- gsub(
    unlist(dataFiles, use.names = FALSE),
    pattern = "^.+/",
    replacement = "")
  opLatenciesThreads <- list()
  # names(opLatenciesThreads) <- c("latency", "thread", "op")
  maxMeasurements <- 0
  for (unit in names(dataFiles)) {
    for (dFile in dataFiles[[unit]]) {
      op <- strsplit(dFile, split = "\\.")[[1]][[2]]
      if (!any(selectedOps == ":all:")) {
        if (!any(selectedOps == op)) {
          next
        }
      }
      opLatenciesRaw <- readLatData(conc(dataBasePath, "/", unit, "/", dFile))
      if (maxMeasurements < nrow(opLatenciesRaw)) {
        maxMeasurements <- nrow(opLatenciesRaw)
      }
    }
  }
  cout("Measurements per thread and op: ", maxMeasurements)
  for (unit in names(dataFiles)) {
    for (dFile in dataFiles[[unit]]) {
      op <- strsplit(dFile, split = "\\.")[[1]][[2]]
      if (!any(selectedOps == ":all:")) {
        if (!any(selectedOps == op)) {
          next
        }
      }
      opLatenciesRaw <- readLatData(conc(dataBasePath, "/", unit, "/", dFile))
      execNr <- strsplit(
        strsplit(
          tail(strsplit(dFile, '/')[[1]], n = 1),
          '__')[[1]][1],
        '_')[[1]][2]
      nThreads <- ncol(opLatenciesRaw) / 2
      for (tId in seq(0, nThreads-1)) {
        colEnd      <- (tId + 1 ) * 2
        colStart    <- colEnd - 1
        latValues   <- opLatenciesRaw[[colEnd]] - opLatenciesRaw[[colStart]]
        # Filter non-zero columns
        if (opLatenciesRaw[[colEnd]][[1]] <= 0) {
          cout("No measurements from thread ", tId)
          next
        }
        # Pad measurements to same length:
        numPadSamples    <- maxMeasurements - length(latValues)
        if (numPadSamples > 0) {
          cout("Padding: ", numPadSamples)
        }
        latValues        <- c(latValues, rep(0, numPadSamples))
        opLatenciesStart <- c(opLatenciesRaw[[colStart]],
                              rep(0, numPadSamples))
        opLatenciesEnd   <- c(opLatenciesRaw[[colEnd]],
                              rep(0, numPadSamples))
        opLatenciesThreads <- list(
          "thread"  = c(opLatenciesThreads$thread,  tId),
          "start"   = c(opLatenciesThreads$start,   opLatenciesStart),
          "end"     = c(opLatenciesThreads$end,     opLatenciesEnd),
          "latency" = c(opLatenciesThreads$latency, latValues),
          "op"      = c(opLatenciesThreads$op,      rep(op,     maxMeasurements)),
          "unit"    = c(opLatenciesThreads$unit,    rep(unit,   maxMeasurements)),
          "exec"    = c(opLatenciesThreads$exec,    rep(execNr, maxMeasurements))
        )
      }
    }
  }
  latenciesDf <- data.frame(opLatenciesThreads)
  return(latenciesDf)
}

gOperationLatencyBoxPlot <- function(dframe,
                                     plotTitle   = "Latency benchmark (ARM Cortex A9)",
                                     selectedOps = c(),
                                     fontScale   = 1)
{
  scaleXBreaks <- sort(c(unique(dframe$latency)))

  limits <- aes(ymin = min(latency), ymax = max(latency))

  gp <- ggplot(dframe,
         aes(
           x      = unit,
           y      = latency,
           fill   = unit)) +
    geom_boxplot() +
  #  geom_line(aes(x      = unit,
  #                y      = max(latency),
  #                colour = op)) +
    scale_y_sqrt() +
    guides(fill = guide_legend(keywidth = 2,
                               title    = "Operation")) +
    theme(legend.position  = "top",
          legend.direction = "horizontal")

  return(gp)
}

gOperationLatencyJitterPlot <- function(dframe,
                                        plotTitle        = "Latency benchmark (ARM Cortex A9)",
                                        selectedOps      = c(),
                                        facetsTitles     = NA,
                                        maxClusterPoints = 300,
                                        minClusterPoints = 10,
                                        fontScale        = 1)
{
  pointScale     <- 0.15

  opNames     <- selectedOps
  execNrs     <- unique(dframe$exec)

  dframe.agg  <- aggregateLatencyData(dframe)
  dframe.sum  <- summarizeLatencyData(dframe)

  dfOutliers  <- dplyr::filter(dframe.agg, outlier == T)
  dfInliers   <- dplyr::filter(dframe.agg, outlier == F)
  # Splitting data set into outliers and inliers so only
  # inlier data is consolidated. Consolidation drops data
  # points from dense regions, this ensures no outliers
  # are being dropped.
  dfInliers.c <- consolidateLatencyData(dfInliers,
                                        maxClusterPoints,
                                        minClusterPoints)

  dframe.c    <- rbind(dfInliers.c, dfOutliers)

  # Every aesthetic in the main ggplot call is expected in every
  # subsequent geom, so repeat attribte name "latency":
  dframeSummary <- ddply(dframe,
                         c("op", "exec", "unit"), summarise,
                         latencyMin = min(latency),
                         latencyMax = max(latency))
  dframeMaxLabs <- ddply(dframe,
                         c("op", "exec", "unit"), summarise,
                         latencyMax = max(latency),
                         labelMax   = paste("max:",
                                            max(round(latency / 1000,
                                                      digits = 3)),
                                            " ms"))
  dframeSummary$grp <- paste(dframeSummary$op,
                             dframeSummary$exec,
                             sep = '.')
  dframeMaxLabs$grp <- paste(dframeMaxLabs$op,
                             dframeMaxLabs$exec,
                             sep = '.')

  cout("Inliers:  ", nrow(dfInliers))
  cout("Outliers: ", nrow(dfOutliers))
  cout("Data points after consolidation: ",
       nrow(dfOutliers) + nrow(dfInliers.c))

  labelY <- "Observed latency (ms)"
  labelX <- "Implementation"

  gp <- ggplot(dframe.c,
               aes(
                 x      = unit,
                 y      = latency,
                 colour = unit,
                 fill   = unit),
               show_guide  = FALSE,
               environment = environment()) +
    geom_bar(data  = dframeSummary,
             aes(
               y     = latencyMax,
               group = interaction(dframeSummary$exec,
                                   dframeSummary$op),
             ),
             color    = coreglitchColors$brightOutline,
             fill     = "white",  # 75% white = bright background
             alpha    = 0.75,
             stat     = "identity",
             position = position_dodge(width = 0.9)) +
    # Underplot fill:
    geom_boxplot(data  = dframe.c,
              #  group = factor(dframe$exec),
                 aes(
                   y        = latency,
                 ),
                 fill           = "#cfcfcf",
                 show_guide     = FALSE,
                 alpha          = 0.3,
                 outlier.colour = NA,
                 colour         = NA,
                 width          = 1.2,
                 position       = position_dodge(width = 0.9)) +
    geom_point(data = dfInliers.c,
               aes(
                 size  = (pointScale / 2) + (2 * pointScale * prob),
             #   alpha = 0.4 + (prob * 2)
               ),
               group    = unit,
               size     = 2,
               position = position_jitterdodge(
                 dodge.width  = 0.9,
                 jitter.width = 0.9),
               show_guide = TRUE) +
    geom_point(data     = dfOutliers,
               group    = unit,
               position = position_jitterdodge(
                 dodge.width  = 0.9,
                 jitter.width = 0.6
               ),
               show_guide = FALSE,
               size       = 0.5 + 4 * pointScale,
               shape      = 17,
            #  alpha      = 0.5,
               colour     = "#50506c") +
    # Overplot outlines:
    geom_boxplot(data  = dframe.c,
                 fill           = NA,
                 alpha          = 0.8,
                 outlier.colour = NA,
                 colour         = "black",
                 width          = 1.2,
                 show_guide     = FALSE,
                 position       = position_dodge(width = 0.9)) +
    geom_errorbar(data  = dframeSummary,
                  color = coreglitchColors$marker,
                  show_guide = FALSE,
                  linestyle  = "solid",
                  aes(
                    y    = dframeSummary$latencyMax,
                    ymax = latencyMax,
                    ymin = latencyMax,
                  ),
                  width    = 0.9,
                  size     = 1,
                  position = position_dodge(width = 0.9)) +
    geom_text(data = dframeMaxLabs,
              aes(
                y     = latencyMax - 5,
                label = labelMax,
                group = interaction(dframeMaxLabs$exec,
                                    dframeMaxLabs$op)
              ),
              vjust    = -0.5,
              size     = 2.0,
              color    = "black",
              position = position_dodge(width = 0.9)) +

    scale_y_continuous(labelY,
                       trans  = sqrt_trans(),
                       breaks = trans_breaks("sqrt", function(x) x^2),
                       labels = trans_format("sqrt", math_format(.x^2))) +
    scale_x_discrete(labelX) +
    labs(title = plotTitle) +

    coreglitchScaleColorDiscrete() +

    guides(linetype = guide_legend(keywidth = 2,
                                   title    = "Operation"),
           size     = guide_legend(keywidth = 2,
                                   title    = "Operation"),
           fill     = guide_legend(keywidth = 2,
                                   title    = "Operation"),
           colour   = guide_legend(keywidth = 2,
                                   title    = "Operation"))

  facetLabeller <- function(var, value) {
    v <- droplevels(value)
    # Match index of value and label:
    if (var == "exec") {
      return(facetsTitles[which(execNrs == v)])
    }
    if (var == "op") {
      opLevels <- unique(dframe.agg$op)
      return(opNames[which(opLevels == v)])
    }
  }

  if (length(execNrs) > 1) {
    gp <- gp + facet_grid(exec ~ op, labeller = facetLabeller)
  } else {
    gp <- gp + facet_grid(. ~ op)
  }

  gp <- gp +
    coreglitchTheme()

  return(gp)
}

gOperationLatencyViolinPlot <- function(dframe,
                                        plotTitle        = "Latency benchmark (ARM Cortex A9)",
                                        selectedOps      = c(),
                                        facetsTitles     = NA,
                                        maxClusterPoints = 300,
                                        minClusterPoints = 10,
                                        scale            = 1000.0,
                                        fontScale        = 1)
{
  opNames     <- selectedOps
  execNrs     <- unique(dframe$exec)

  dframe.agg  <- aggregateLatencyData(dframe)
  dframe.sum  <- summarizeLatencyData(dframe)

  dfOutliers  <- dplyr::filter(dframe.agg, outlier == T)
  dfInliers   <- dplyr::filter(dframe.agg, outlier == F)
  dfInliers.c <- consolidateLatencyData(dfInliers,
                                        maxClusterPoints,
                                        minClusterPoints)

# dframe.c    <- rbind(dfInliers.c, dfOutliers)
  dframe.c    <- dfInliers.c

  # Every aesthetic in the main ggplot call is expected in every
  # subsequent geom, so repeat attribte name "latency":
  dframeSummary <- ddply(dframe.c,
                         c("op", "exec", "unit"), summarise,
                         latencyMedian = median(latency),
                         latencyMin = min(latency),
                         latencyMax = max(latency))
  dframeMaxLabs <- ddply(dframe.c,
                         c("op", "exec", "unit"), summarise,
                         latencyMax = max(latency),
                         labelMax   = paste("max:",
                                            max(round(latency / 1000,
                                                      digits = 3)),
                                            " ms"))
  dframeSummary$grp <- paste(dframeSummary$op,
                             dframeSummary$exec,
                             sep = '.')
  dframeMaxLabs$grp <- paste(dframeMaxLabs$op,
                             dframeMaxLabs$exec,
                             sep = '.')

  cout("Inliers:  ", nrow(dfInliers))
  cout("Outliers: ", nrow(dfOutliers))
  cout("Data points after consolidation: ",
       nrow(dfOutliers) + nrow(dfInliers.c))

  labelY <- "Observed latency (ms)"
  labelX <- "Implementation"

  yBreaks <- seq(
    from       = min(dframe.c$latency),
    to         = max(dframe.c$latency),
    length.out = 5
  )

  yMinValue <- min(dframe.c$latency)
  yMaxValue <- max(dframe.c$latency)
  # yLabels <- formatC(yBreaks, format = "g", digits = 2)
  yLabels <- sapply(
    yBreaks,
    function(x) { round(x / 1000, digits = 0) }
  )

  gp <- ggplot(
    dframe.c,
    aes(
      x      = unit,
      y      = latency,
      colour = unit,
      fill   = unit
    ),
    show_guide  = FALSE,
    environment = environment()
  ) +
    geom_bar(
      data = dframeSummary,
      aes(
        y     = latencyMax,
        group = interaction(dframeSummary$exec,
                            dframeSummary$op),
      ),
      color    = coreglitchColors$brightLine,
      fill     = NA,
      alpha    = 0.75,
      stat     = "identity",
      width    = 0.9
    ) +
    stat_summary(
      data     = dframe.c,
      geom     = "crossbar",
      position = "identity",
      fun.y    = median,
      fun.ymin = function(x) mean(x) - sd(x),
      fun.ymax = function(x) mean(x) + sd(x),
      size     = 0.5,
      color    = NA,
      fill     = coreglitchColors$darkerBackground
    ) +
    # Clear background behind violin:
    stat_summary(
      data     = dframe.c,
      geom     = "crossbar",
      position = "identity",
      fun.y    = median,
      fun.ymin = function(x) mean(x) - sd(x),
      fun.ymax = function(x) mean(x) + sd(x),
      size     = 0.5,
      color    = NA,
      width    = 0.75,
      fill     = "white"
    ) +
    # Median indicator:
    geom_errorbar(
      data       = dframeSummary,
      color      = coreglitchColors$brightOutline,
      show_guide = FALSE,
      linestyle  = "solid",
      aes(
        y    = latencyMedian,
        ymax = latencyMedian,
        ymin = latencyMedian,
      ),
      width = 0.9,
      size  = 0.5
    ) +
    geom_violin(
      data  = dframe.c,
      scale = "area",
      aes(
        group  = unit,        ,
        x      = as.numeric(unit) + 0
      ),
      fill       = "white",
      color      = "white",
      size       = 1.9,
      alpha      = 0.92,
      adjust     = 1,
      show_guide = FALSE,
      width      = 0.9
    ) +
    geom_violin(
      data  = dframe.c,
      scale = "area",
      aes(
        group  = unit,
        colour = unit,
        fill   = unit,
        x      = as.numeric(unit) + 0
      ),
      alpha      = 0.85,
      adjust     = 1,
      show_guide = FALSE,
      width      = 0.9
    ) +
    # Maximum indicator:
    geom_errorbar(
      data  = dframeSummary,
      color = coreglitchColors$darkOutline,
      show_guide = FALSE,
      linestyle  = "solid",
      aes(
        y    = latencyMax,
        ymax = latencyMax,
        ymin = latencyMax,
      ),
      hjust = 1,
      width = 0.9,
      size  = 0.2
    ) +
    geom_text(
      data = dframeMaxLabs,
      aes(
        x     = as.numeric(unit) + 0.0,
        y     = latencyMax - 5,
        label = labelMax,
        group = interaction(dframeMaxLabs$exec,
                            dframeMaxLabs$op)
      ),
      vjust    = -0.5,
      size     = 2.3,
      color    = "black",
    ) +
    labs(
      title = plotTitle
    ) +
    scale_y_sqrt(
      labelY,
      breaks = yBreaks,
      labels = yLabels
    ) +
    expand_limits(
      # More headroom for annotation of max latency:
      y = c(0, max(pretty(c(dframe.c$y, yMaxValue * (1.1)))))
    ) +
    scale_x_discrete(
      labelX
    ) +

    coreglitchScaleFillDiscrete(type = "seq") +
    coreglitchScaleColorDiscrete(type = "seq") +

    guides(
      linetype = guide_legend(keywidth = 2,
                              title    = "Operation"),
      size     = guide_legend(keywidth = 2,
                              title    = "Operation"),
      fill     = guide_legend(keywidth = 2,
                              title    = "Operation"),
      colour   = guide_legend(keywidth = 2,
                              title    = "Operation")
    )

  facetLabeller <- function(var, value) {
    v <- droplevels(value)
    # Match index of value and label:
    if (var == "exec") {
      return(facetsTitles[which(execNrs == v)])
    }
    if (var == "op") {
      opLevels <- unique(dframe.agg$op)
      return(opNames[which(opLevels == v)])
    }
  }

  if (length(execNrs) > 1) {
    gp <- gp + facet_grid(exec ~ op,
                          labeller = facetLabeller,
                          scales   = "free")
  } else {
    gp <- gp + facet_grid(. ~ op)
  }

  gp <- gp + coreglitchTheme(
    hasLegend   = FALSE,
    reducedGrid = TRUE,
    fontScale   = fontScale
  )

  return(gp)
}

gOperationLatencyHistogram <- function(dframe,
                                       plotTitle = "Latency benchmark (ARM Cortex A9)",
                                       opNames   = c())
{
  env <- environment()

  scaleXBreaks <- sort(c(unique(dframe$latency)))
  cout("Latency factors: ", length(scaleXBreaks), ": ",
       min(scaleXBreaks), " - ", max(scaleXBreaks))

  latencyMedian <- mean(dframe$latency)
  cout("Latency median: ", latencyMedian)

  #  dframeSummary <- ddply(dframe,
  #                         c("op", "unit"), summarise,
  #                         latency.min = min(latency),
  #                         latency.max = max(latency))
  #  dframeHist    <- hist(dframe$latency, plot=FALSE)$counts

  dframe.c <- dframe%.% dplyr::group_by(unit, op, latency) %.%
                        dplyr::summarise(n = length(op)) %.%
                        dplyr::mutate(total = sum(n)) %.%
                        dplyr::mutate(prop = n/total)

  gp <- ggplot(dframe,
               environment = env,
               aes(x       = latency,
                   group   = unit,
                   colour  = op,
                   fill    = unit)) +
      #   geom_histogram(
      #      aes(y        = ..density..,
      #          colour   = op,),
      #      binwidth = length(scaleXBreaks) * 30,
      #                  stat     = "bin",
      #                  alpha    = 0.7,
      #                  position = "dodge"
      #  ) +
        geom_density(linestyle = unit, alpha = 0.5) +
        geom_vline(xintercept = latencyMedian, colour = "red") +

        scale_y_sqrt() +
        guides(fill = guide_legend(keywidth = 2,
                                   title    = "Implementation")) +
        coreglitchScaleColorDiscrete() +
        coreglitchScaleFillDiscrete() +

        theme_bw() +
        theme(legend.position  = "top",
              legend.direction = "horizontal")
  return(gp)
}

aggregateLatencyData <- function(dframe, outlierMaxProb = 0.000009)
{
  dframe.agg <- dframe %.%
    # total = amount of measurements for a specific
    #         combination of (unit, op):
    dplyr::group_by(unit, exec, op) %.%
    dplyr::mutate(total   = sum(op)) %.%
    dplyr::group_by(unit, exec, op, latency) %.%
    # n = amount of measurements for (unit,op) with
    #     this latency value:
    dplyr::mutate(n       = length(op)) %.%
    dplyr::mutate(prob    = n/total) %.%
    dplyr::mutate(outlier = prob < outlierMaxProb)
  return(dframe.agg)
}

summarizeLatencyData <- function(dframe, outlierMaxProb = 0.000009)
{
  dframe.sum <- dframe %.%
    dplyr::group_by(unit, exec, op, latency) %.%
    dplyr::summarise(n = length(op)) %.%
    dplyr::mutate(latencyMin = min(latency)) %.%
    dplyr::mutate(latencyMax = max(latency)) %.%
    dplyr::mutate(total      = sum(n)) %.%
    dplyr::mutate(prob       = n/total) %.%
    dplyr::mutate(outlier    = prob < outlierMaxProb)
  return(dframe.sum)
}

consolidateLatencyData <- function(dframe.agg, minClusterPoints, maxClusterPoints)
{
  # There are thousands and millions of data points in a latency
  # plot, so overplotting would render vector output useless.
  # Make data points more sparse in dense areas, and use larger
  # points in the plot for data with high probability.
  df.sparse <- dframe.agg %.%
    dplyr::filter(outlier == F) %.%
    dplyr::group_by(unit, exec, op, latency) %.%
    dplyr::arrange(prob) %.%
    dplyr::do({
      nClusterSize <- nrow(.)
      nClusterData <- as.integer(
        min(nClusterSize,
            max(minClusterPoints,
                maxClusterPoints * (.$prob[1]))))
      data.frame(.)[1:nClusterData,]
    })
  return(df.sparse)
}