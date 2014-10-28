require(tikzDevice)
library(extrafont)

# install.packages('tikzDevice', repos = "http://www.rforge.net/", type = "source")
# mtxrun --script fonts --reload --simple
# luaotfload-tool.exe --update

options(tikzMetricsDictionary  = "C:/Users/fuchsto/AppData/Local/Temp/tikz" )
# options(tikzMetricPackages     = c("\\usepackage[utf8]{inputenc}",
#                                   "\\usepackage[T1]{fontenc}",
#                                   "\\usetikzlibrary{calc}" ))

repositoryPath <- "C:/Users/fuchsto/Workspaces/Thesis.git/"
texBinPath     <- "C:/MiKTeX-2.9/miktex/bin/"
# texBinPath <- "C:/Users/Z003C9SP/Applications/miktex-2.9.4503/miktex/bin/"
# options(tikzPdftexWarnUTF     = TRUE)
options(tikzXelatex           = paste(texBinPath, "xelatex.exe",  sep = ''))
options(tikzLualatex          = paste(texBinPath, "lualatex.exe", sep = ''))
options(tikzLatex             = paste(texBinPath, "pdflatex.exe", sep = ''))
# options(tikzLatex             = paste(texBinPath, "pdflatex.exe", sep = ''))
options(tikzDefaultEngine     = "pdftex" )

# Needed only on Windows - run once per R session
# Adjust the path to match your installation of Ghostscript
Sys.setenv(R_GSCMD = paste(texBinPath, "mgs.exe", sep = ''))
# font_import(prompt = F)
# loadfonts()

print(tikzCompilerInfo())

Sys.setenv("BENCHMARK_DATAFILES_BASEPATH" =
             paste("C:/Users/fuchsto/Workspaces/benchmark-data", sep = ''))
Sys.setenv("BENCHMARK_PLOTFILES_BASEPATH" =
             paste(repositoryPath, "Latex/plots", sep = ''))

Sys.setenv("PLOTS_COLUMNWIDTH_MM" = 147.54)
Sys.setenv("PLOTS_COLUMNWIDTH_IN" = 5.809)
