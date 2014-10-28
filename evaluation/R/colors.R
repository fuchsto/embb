
require('scales')

colorBlindPaletteDark   <- c("#999999", "#E69F00", "#56B4E9", "#009E73",
                             "#F0E442", "#0072B2", "#D55E00", "#CC79A7")
colorBlindPaletteBright <- c("#000000", "#E69F00", "#56B4E9", "#009E73",
                             "#F0E442", "#0072B2", "#D55E00", "#CC79A7")

coreglitchColors <- list (
  "marker"            = "#569730",

  "mediumText"        = "#454545",

  "groupOutline"      = "#c9c9c9",
  "brightOutline"     = "#969696",
  "mediumOutline"     = "#7c7c7c",
  "darkOutline"       = "#414141",
  "inverseOutline"    = "#f7f7f7",

  "brightLine"        = "#e0e0e0",

  "brightBackground"  = "#fcfcfc", # 75% white
  "mediumBackground"  = "#f2f2f2",
  "neutralBackground" = "#f5f5f5", # neutral = 12% black + 75% white = 4% black
  "darkerBackground"  = "#d5d5d5",
  "darkBackground"    = "#d7d7d7",  # 12% black

  "inverseBackground" = "#989898",

  "highlightColor"    = "#ff6600"
)

# RColorBrewer::display.brewer.all()

coreglitchPalettes <- list(
  "a"  = c("#a60005", "#66a9a6", "#0cb460", "#554646",
            "#0fc7f1","#2fa68c", "#d58d13", "#7a0026"),
  "b"  = c("#5e3c99", "#fdb863", "#e66101", "#b2abd2"),
  "c"  = c("#f1b6da", "#b8e186", "#d01c8b", "#4dac26"),
  "d"  = c("#d01c8b", "#4dac26", "#0072bc", "#d01c8b", "#66a9a6", "#5e3c99"),
  "e"  = c("#1fffad", "#ff80ad", "#7aff45", "#ffa424"),
  "seqGreen"  = c("#b6fe60", "#9fdf4a", "#41bbad", "#69a632", "#569730", "#003023"),
  "seqPurple" = c("#c994c7", "#df65b0", "#e7298a", "#ce1256", "#980043", "#67001f",
                  "#e7e1ef", "#d4b9da"),
  "seqRedGreen" = c("#ff5732", "#ef7f00", "#be9700", "#758e00", "#3d5800", "#263f00"),
  "ylgn" = c("#addd8e", "#78c679", "#41ab5d", "#238443", "#006837", "#004529")
)

coreglitchScaleColorDiscrete <- function(nColors = 6, guide = F, type = "qual") {
  if (type == "qual") {
#    scale_color_brewer(type = "qual", palette = "Set1")
   scale_color_manual(values = coreglitchPalettes$ylgn)
#    colorFun <- colorRampPalette(c("#00d653", "#ff0000"), space = "rgb", interpolate = "linear")
#    scale_color_manual(values = colorFun(nColors))
  } else if (type == "seq") {
  # scales::seq_gradient_pal(low = "#132B43", high = "#56B1F7", space = "Lab")
    scale_color_manual(values = coreglitchPalettes$d)
  # scale_color_brewer(type = "seq", palette = "Spectral")
  }
#  scale_color_discrete(h.start = 190,
#                       h = range(10,260),
#                       l = range(40,60),
#                       c = 190)
}

coreglitchScaleFillDiscrete <- function(nColors = 6, guide = F, type = "qual") {
  if (type == "qual") {
#    scale_fill_brewer(type = "qual", palette = "Set1")
    scale_fill_manual(values = coreglitchPalettes$ylgn)
#    colorFun <- colorRampPalette(c("#00d653", "#ff0000"), space = "rgb", interpolate = "linear")
#    scale_fill_manual(values = colorFun(nColors))
  } else if (type == "seq") {
    scale_fill_manual(values = coreglitchPalettes$d)
  }
}

coreglitchScaleFillContinuous <- function(nColors = 8, guide = F, type = "qual") {
  colorFun <- colorRampPalette(c("#efefef", "#ff0000"), space = "rgb", interpolate = "linear")
  scale_fill_manual(values = colorFun(nColors))
# scale_fill_manual(values = coreglitchPalettes$d)
# scale_fill_brewer(type = "qual", palette = "Set1")
# cols <- brewer_pal("div")(nColors)
# show_col(gradient_n_pal(cols)(seq(0, 1, length = 30)))
}

coreglitchScaleColorContinuous <- function(nColors = 8, guide = F, type = "qual") {
   scale_color_manual(values = coreglitchPalettes$d)
#  scale_color_brewer(type = "qual", palette = "Set1")
#  cols <- brewer_pal("div")(nColors)
#  show_col(gradient_n_pal(cols)(seq(0, 1, length = 30)))
}

#  scale_color_brewer(
#    palette = "Set1", type = "div") +
#  scale_fill_brewer(
#    palette = "Set1", type = "div") +
#  scale_fill_manual(values = colorBlindPaletteBright) +
#  scale_color_manual(values = colorBlindPaletteDark) +

coreglitchTheme <- function(hasLegend = T, hasAxisTitleY = T, reducedGrid = F, fontScale = 1, ...)
{
  if (hasLegend) {
    legendPosition <- c(0, -0.4)
  } else {
    legendPosition <- "none"
  }
  if (hasAxisTitleY) {
    axisTitleY <- element_text(size   = 8,
                               hjust  = 0.5,
                               vjust  = 1.5,
                               angle  = 90)
  } else {
    axisTitleY <- element_blank()
  }

  theme(
    legend.title         = element_text(color  = "black",
                                        size   = fontScale * 8,
                                        face   = "plain"),
    legend.title.align   = 0,
    legend.text          = element_text(color  = "black",
                                        size   = fontScale * 8,
                                        face   = "plain"),
    legend.text.align    = 0,
    legend.position      = legendPosition,
    legend.justification = c(0, 0.5),
    legend.background    = element_rect(colour = NA,
                                        fill   = NA),
    legend.margin        = unit(2, "points"),
    legend.key.size      = unit(2, "points"),
    legend.key.width     = unit(1, "points"),
    legend.key           = element_rect(fill   = FALSE,
                                        colour = FALSE),
    strip.background     = element_rect(colour = coreglitchColors$mediumOutline,
                                        fill   = NA),
    strip.text.x         = element_text(size   = fontScale * 8,
                                         color  = "black"),
    strip.text.y         = element_text(size   = fontScale * 6,
                                        color  = "black"),

    axis.text            = element_text(size   = fontScale * 6,
                                        color  = coreglitchColors$mediumText),
    axis.title.x         = element_text(size   = fontScale * 8,
                                        hjust  = 0.5,
                                        vjust  = -0.5),
    axis.title.y         = axisTitleY,
    axis.ticks.x         = element_line(colour = coreglitchColors$mediumOutline,
                                        size   = fontScale * 0.1),
    axis.ticks.y         = element_line(colour = coreglitchColors$mediumOutline,
                                        size   = fontScale * 0.1),
    axis.ticks.length    = unit(1.5, "points"),

    panel.background     = element_rect(colour = coreglitchColors$mediumOutline,
                                        fill   = "white"),
    panel.border         = element_rect(colour = coreglitchColors$mediumOutline,
                                        fill   = NA),
    panel.margin.y       = unit(0, "points"),
    panel.margin.x       = unit(0, "points"),
    panel.grid.minor.x   = element_blank(),
    panel.grid.major.x   = element_blank(),
    panel.grid.minor.y   = element_line(colour   = NA,
                                        linetype = ifelse(reducedGrid,
                                                          "dotted", "solid")),
    panel.grid.major.y   = element_line(colour   = coreglitchColors$brightLine,
                                        linetype = ifelse(reducedGrid,
                                                          "dotted", "solid")),
    plot.title           = element_text(size  = fontScale * 8, face = "bold",
                                        hjust = 0, vjust = 1.3),
    plot.margin          = unit(c(8,   # top
                                  0.5, # right
                                  # bottom
                                  ifelse(hasLegend,24, 8),
                                  # left
                                  ifelse(hasAxisTitleY, 4, -2)
                                ) * fontScale,
                                "points"),
    plot.background      = element_rect(colour = FALSE,
                                        fill   = FALSE),
    ...
  )
}


