
confidenceInterval = function(dataSeries, prob = 0.975)
{
  error <- qt(prob, df = length(dataSeries) - 1) *
           sd(dataSeries) / 
           sqrt(length(dataSeries))
  # subtract / add error from mean for left / right
  # interval limits: 
  return(c(mean(dataSeries)-error, 
           mean(dataSeries)+error))
}

conc = function(...) paste(... , sep='')
cout = function(...) write(conc(...), stdout())

# improved list of objects
.ls.objects <- function (pos = 1, 
                         pattern, 
                         order.by,
                         decreasing = FALSE, 
                         head = FALSE, 
                         n = 5) 
{
  napply <- function(names, fn) sapply(names, function(x)
    fn(get(x, pos = pos)))
  names <- ls(pos = pos, pattern = pattern)
  obj.class <- napply(names, function(x) as.character(class(x))[1])
  obj.mode <- napply(names, mode)
  obj.type <- ifelse(is.na(obj.class), obj.mode, obj.class)
  obj.prettysize <- napply(names, function(x) {
    capture.output(format(utils::object.size(x), units = "auto")) })
  obj.size <- napply(names, object.size)
  obj.dim <- t(napply(names, function(x)
    as.numeric(dim(x))[1:2]))
  vec <- is.na(obj.dim)[, 1] & (obj.type != "function")
  obj.dim[vec, 1] <- napply(names, length)[vec]
  out <- data.frame(obj.type, obj.size, obj.prettysize, obj.dim)
  names(out) <- c("Type", "Size", "PrettySize", "Rows", "Columns")
  if (!missing(order.by))
    out <- out[order(out[[order.by]], decreasing=decreasing), ]
  if (head)
    out <- head(out, n)
  out
}

# shorthand
lsos <- function(..., n=10) {
  .ls.objects(..., order.by="Size", decreasing=TRUE, head=TRUE, n=n)
}

setupFonts = function(load=FALSE, reset=FALSE) {
  # https://github.com/wch/extrafont/blob/master/README.md
  # Prevent accidential load of font cache: 
  if (load) {
    library(extrafont)
    font_import()
    loadfonts()
  }
  if (reset) {
    install.packages("extrafontdb")
  }
}

f2si = function (number, 
                 rounding = TRUE, 
                 digits   = ifelse(rounding, NA, 2), 
                 space    = TRUE) 
{
  if (is.na(number)) {
    number = 0
  }
  
  lut <- c(1e-24, 1e-21, 1e-18, 1e-15, 1e-12, 1e-09, 1e-06, 0.001, 
           1,     1000,  1e+06, 1e+09, 1e+12, 1e+15, 1e+18, 1e+21, 
           1e+24, 1e+27)
  pre <- c("y", "z", "a", "f", "p", "n", "u", "m", "", "k", 
           "M", "G", "T", "P", "E", "Z", "Y", NA)
  
  ix <- findInterval(number, lut)
  
  sepSpace <- ifelse(space, " ", '')
  
  if (ix>0 && ix<length(lut) && lut[ix]!=1) 
  {
    if (rounding==T && !is.numeric(digits))  {
      sistring <- paste(round(number/lut[ix]), pre[ix], sep = sepSpace)
    }
    else if (rounding == T || is.numeric(digits)) {
      sistring <- paste(signif(number/lut[ix], digits), pre[ix], sep = sepSpace)
    }
    else {
      sistring <- paste(number/lut[ix], pre[ix], sep = sepSpace)
    } 
  }
  else 
  {
    sistring <- as.character(number)
  }
  
  return(sistring)
}

read.tcsv = function(file, 
                     colClasses       = c('data.frame.vector', 'data.frame.difftime'), 
                     header           = TRUE, 
                     sep              = ";", 
                     quote            = "'", 
                     dec              = ".", 
                     stringsAsFactors = FALSE) {
  
  n = max(count.fields(file, sep), na.rm=TRUE)
  x = readLines(file)
  
  .splitvar = function(x, sep, n) {
    var = unlist(strsplit(x, split=sep))
    length(var) = n
    return(var)
  }
  
  x = do.call(cbind, lapply(x, .splitvar, sep=sep, n=n))
  x = apply(x, 1, paste, collapse=sep) 
  out = read.table(text=x, sep=sep, header=header, quote=quote, dec=dec, stringsAsFactors=stringsAsFactors, fill=TRUE, blank.lines.skip=TRUE, strip.white=TRUE)
  omitNa = out[complete.cases(out),]
  return(omitNa)
}

unitNames <- function(typeId) {
  if (typeId == 2) {
    return(c("MS-queue (AP)",
             "MS-queue (array pool)",
             "Michael-Scott queue (AP)", 
             "Michael-Scott queue (array pool)", 
             "MICHAEL_SCOTT_QUEUE_AP"))
  }
  if (typeId == 3) {
    return(c("MS-queue (TP)",
             "MS-queue (tree pool)",
             "Michael-Scott queue (TP)", 
             "Michael-Scott queue (tree pool)", 
             "MICHAEL_SCOTT_QUEUE_TP"))
  }
  if (typeId == 4) {
    return(c("KP-queue (PC)",
             "KP-queue (phase count)",
             "Kogan-Petrank queue (PC)", 
             "Kogan-Petrank queue (phase count)", 
             "KOGAN_PETRANK_QUEUE"))
  }
  if (typeId == 5) {
    return(c("KP-queue (NP)",
             "KP-queue (no phase)",
             "Kogan-Petrank queue (NP)", 
             "Kogan-Petrank queue (no phase)", 
             "KOGAN_PETRANK_QUEUE_PL"))
  }
  if (typeId == 6) {
    return(c("Tree pool",
             "Tree-based pool",
             "Lock-free tree pool", 
             "Lock-free tree-based pool", 
             "LOCKFREE_TREE_POOL"))
  }
  if (typeId == 7) {
    return(c("Array pool",
             "Array-based pool",
             "Wait-free array pool",
             "Wait-free array-based pool",
             "WAITFREE_ARRAY_POOL"))
  }
  if (typeId == 8) {
    return(c("Compartment pool",
             "Compartment pool",
             "Array-based compartment pool",
             "Wait-free array-based compartment pool",
             "WAITFREE_COMPARTMENT_POOL"))
  }
  if (typeId == 9) {
    return(c("WF stack",
             "Wait-free stack",
             "Wait-free stack", 
             "Wait-free stack with elimination", 
             "WAIT_FREE_SIM_STACK"))
  }
  if (typeId == 10) {
    return(c("LF stack",
             "Lock-free stack",
             "Lock-free Treiber stack",
             "Lock-free Treiber stack",
             "LOCK_FREE_STACK"))
  }
  if (typeId == 12) {
    return(c("KP-queue (NP, TP)",
             "KP-queue (tree pool)",
             "Kogan-Petrank queue (NP, TP)", 
             "Kogan-Petrank queue (tree pool)", 
             "KOGAN_PETRANK_QUEUE_PL_TP"))
  }
  return(rep("Undefined type", 5))
}
