
source('colors.R')
source('util.R')

library(ggplot2)
library(gtable)
library(gridExtra)
library(grid)
require(tikzDevice)
library(scales)
library(plyr)

if (F) {
plotLatencyTimeSeries(nBins = 50,
                      unit = "koganpetrank-pl",
                      selectedOps = c("Add", "RemoveAny", "Buffer"),
                      execNr = 0,
                      scenario = 4,
                      zoom = 0)
}
plotLatencyTimeSeries <- function(nBins       = 400,
                                  selectedOps = c('Add', 'RemoveAny'),
                                  scenario,
                                  execNr,
                                  unit,
                                  zoom = 0)
{
  baseDataFilePath <- Sys.getenv(c("BENCHMARK_DATAFILES_BASEPATH"))

  dframe.lats <- collectLatencyDataFrame(
    baseDataFilePath,
    units       = c(unit),
    scenario,
    execNrs     = c(execNr),
    execId      = '',
    selectedOps = selectedOps)

  dframe.lats <- dframe.lats %>% dplyr::arrange(start) %>% dplyr::filter(start > 0)

  minTimestamp <- min(dframe.lats$start[dframe.lats$start > 0])
  dframe.lats$start <- (dframe.lats$start - minTimestamp) / 1000
  dframe.lats$end   <- (dframe.lats$end - minTimestamp) / 1000

  if (length(zoom) > 1) {
    dframe.lats <- dframe.lats %>% dplyr::filter(start > zoom[1], end < zoom[2])
  }

  nThreads <- max(dframe.lats$thread)
  timeRes  <- resolution(dframe.lats$start)
  cout("Time resolution: ", timeRes)

  # Consolidate measurements:
  # Summarize latencies in segments of length timeSeg/nThreads,
  # containing the maximum latency in the respective time span.
  list.lats.c <- list()
  for (threadId in seq(0, nThreads)) {
    for (tOp in factor(selectedOps)) {
      dframe.lats.t.o <- dframe.lats %>%
        dplyr::filter(thread == threadId) %>%
        dplyr::filter(op == tOp) %>%
        dplyr::arrange(start)

      segMinT  <- min(dframe.lats.t.o$start)
      segMaxT  <- max(dframe.lats.t.o$end)
      seqRange <- segMaxT - segMinT
      seqStep  <- seqRange / nBins
      cout(threadId, "-", tOp, ": ",
           "segMinT:  ", segMinT,  ", ",
           "segMaxT:  ", segMaxT,  ", ",
           "seqRange: ", seqRange, ", ",
           "seqStep:  ", seqStep,  ", ")
      for (tseg in c(seq(from = segMinT, to = segMaxT - seqStep, by = seqStep))) {
        dframe.lats.seg <- subset(dframe.lats.t.o,
                                  dframe.lats.t.o$op    == tOp &
                                    dframe.lats.t.o$start >= tseg &
                                    dframe.lats.t.o$end   < tseg + seqStep)
        if (nrow(dframe.lats.seg) < 2) {
          #  cout("Empty time segment")
        }
        else {
          list.lats.c <- list(
            "thread"         = c(list.lats.c$thread,      threadId),
            "op"             = c(list.lats.c$op,          tOp),
            "latency.max"    = c(list.lats.c$latency.max, max(dframe.lats.seg$latency)),
            "start.min"      = c(list.lats.c$start.min,   tseg),
            "end.max"        = c(list.lats.c$end.max,     tseg + seqStep)
          )
        }
      }
    }
  }
  dframe.lats.c <- data.frame(list.lats.c)

  tMin <- min(dframe.lats.c$start.min)
  tMax <- max(dframe.lats.c$end.max)

  # Distinct latency values:
  lats.f.lvls <- sort(unique(dframe.lats.c$latency.max))
  # History of latencies:
  hist.lats.f <- hist(dframe.lats.c$latency.max, breaks = lats.f.lvls, plot = F)

if (F) {
  dframe.lats.c$latency.max <- sapply(
    dframe.lats.c$latency.max,
    function(l) {
      li <- which(hist.lats.f$breaks == l)
      vl <- ifelse(li < 10 | li == length(hist.lats.f$breaks), l, hist.lats.f$breaks[10])
      vl
    }
  )
}
  dframe.lats.c$latency.max <- droplevels(factor(dframe.lats.c$latency.max))

  dframe.lats.c$thread.op   <- paste(dframe.lats.c$thread,
                                     dframe.lats.c$op,
                                     sep = '.')
  latLevels <- unique(c(dframe.lats.c$latency.max))
  latLevels <- 300
  cout("Measurements:   ", nrow(dframe.lats.c))
  cout("Operations:     ", unique(dframe.lats.c$op))
  cout("Latency levels: ", latLevels)

  p.lats <-
    ggplot(
      dframe.lats.c,
      env = environment()
    )
  p.lats <- p.lats +
    geom_tile(
      aes(
        group    = factor(op),
        y        = start.min,
        x        = factor(thread),
        fill     = latency.max,
        color    = factor(op),
      # alpha    = latency.max,
        width    = 0.8,
        height   = (end.max-start.min)
      ),
    # color    = "#adadad",
      position = "dodge"
    )

  p.lats <- p.lats +
    guides(
           fill  = element_blank(),
       #   alpha = guide_legend(position = "none",
       #                        title    = ""),
           color = guide_legend(position = "Operation")
    )

  p.lats <- p.lats +
    coreglitchScaleColorDiscrete(type = "seq") +
    coreglitchScaleFillContinuous(type = "qual", nColors = latLevels)

  p.lats <- p.lats +
    scale_y_continuous() +
    scale_x_discrete()

  p.lats <- p.lats +
    xlab("Thread ID") +
    ylab("Time (ms)")

  if (length(zoom) > 1) {
    p.lats <- p.lats +
      coord_flip(ylim = zoom)
  } else {
    p.lats <- p.lats +
      coord_flip()
  }
  if (F) {
    p.lats <- p.lats +
      coreglitchTheme(
        legend.position = "right"
      )
  }
  else {
    p.lats <- p.lats + theme_classic()
  }
  p.lats
}

plotLatencyTimeSeriesMap <- function(nBins       = 200,
                                     selectedOps = c('Add', 'RemoveAny'),
                                     scenario,
                                     execNr,
                                     unit,
                                     zoom = 0)
{
  baseDataFilePath <- Sys.getenv(c("BENCHMARK_DATAFILES_BASEPATH"))

  dframe.lats <- collectLatencyDataFrame(
    baseDataFilePath,
    units       = c(unit),
    scenario    = scenario,
    execNrs     = c(execNr),
    execId      = '',
    selectedOps = selectedOps)

  dframe.lats <- dframe.lats %>% dplyr::arrange(start)

  nMeasurements <- nrow(dframe.lats)
  nThreads      <- max(dframe.lats$thread)
  nSegSize      <- ceiling(nMeasurements / nBins)
  nBins         <- nMeasurements / nSegSize
  timeRes       <- resolution(dframe.lats$start)
  latRes        <- resolution(dframe.lats$latency)

  dframe.lats.h       <- dframe.lats
  dframe.lats.h$start <- floor(dframe.lats.h$start / timeRes / 1000)
  dframe.lats.h$end   <- floor(dframe.lats.h$end   / timeRes / 1000)
  tMin <- min(dframe.lats.h$start)
  tMax <- max(dframe.lats.h$end)
  lMax <- max(dframe.lats.h$latency)

  # Reduce latency values to resolution:
  dframe.lats.h$latency.f <- dframe.lats.h$latency / latRes
  # Distinct latency values:
  lats.f.lvls <- sort(unique(dframe.lats.h$latency.f))
  # History of latencies:
  hist.lats.f <- hist(dframe.lats.h$latency.f, breaks = lats.f.lvls, plot = F)

  dframe.lats.h$latency.f <- sapply(
    dframe.lats.h$latency.f,
    function(l) {
      li <- which(hist.lats.f$breaks == l)
      vl <- ifelse(li < 5 | li == length(hist.lats.f$breaks), l, hist.lats.f$breaks[5])
      vl
    }
  )

  binResolution <- resolution(dframe.lats.h$start)
  cout("Bin resolution: ", binResolution)

  dframe.lats.h$tMax <- tMax
  dframe.lats.h$tMin <- tMin

  p.lats <-
    ggplot(
      dframe.lats.h,
      aes(
        x = factor(thread)
      ),
      env = environment()
    )
  p.lats <- p.lats +
    geom_bar(
      aes(
        x = factor(thread),
        y = tMax
      ),
      color    = "grey",
      fill     = "ghostwhite",
      stat     = "identity",
      position = "identity",
      width    = 0.6
    )
  p.lats <- p.lats +
    geom_bin2d(
      aes(
        ymin     = factor(tMin),
        y        = start,
        ymax     = factor(tMax),
        x        = factor(thread),
        # alpha    = ..count..,
        fill     = factor(latency.f)
      ),
      na.fill  = "white",
      position = "dodge",
      binwidth = c(0.6, binResolution),
    )
  p.lats <- p.lats +
    coord_flip() +
    scale_x_discrete() +
    scale_y_discrete() +
    coreglitchScaleColorDiscrete() +
    coreglitchScaleFillDiscrete() +
    ylab("Time") +
    xlab("Latency (us) by thread")

  if (F) {
    p.lats <- p.lats +
      coreglitchTheme(
        panel.grid.minor.x = element_blank(),
        panel.grid.major.x = element_blank(),
        panel.grid.minor.y = element_blank(),
        panel.grid.major.y = element_blank()
      )
  }
  else {
    p.lats <- p.lats + theme_classic()
  }

  p.lats
}
