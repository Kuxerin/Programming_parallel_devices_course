library('ggplot2')
library('reshape')
library('Cairo')
library("grid")

plottitle <- "MatrixMN"
plotlabels <- c("1 szál", "2 szál","4 szál", "8 szál")
xlabel <- "Mátrix mérete [N]"
ylabel <- "Ido (s)"

data0 <- read.table('1thread.dat')
data1 <- read.table('2thread.dat')
data2 <- read.table('4thread.dat')
data3 <- read.table('8thread.dat')
d <- data.frame(data0$V2, data0$V1, data1$V1, data2$V1, data3$V1)
colnames(d) <- c('alpha', 'runtime1', 'runtime2', 'runtime3', 'runtime4')
d <- melt(d, id='alpha', variable_name='series')

cairo_ps("matrixmn.eps", width=20, height=8)  
p<-ggplot(d, 
  aes_string(x=names(d)[1], y=names(d)[3], colour=names(d)[2]), 
  labeller=label_parsed) + 
  geom_point(size=4) + 
  geom_line(size=1.5) + 
  labs(title=plottitle) + 
  xlab(xlabel) + 
  ylab(ylabel) + 
  scale_colour_manual(values=c("black", "blue", "red", "green", "purple"), name="", labels=plotlabels, guide=guide_legend(keyheight=unit(2, "line"), keywidth=unit(5, "line"))) +
  theme_gray(24) +
  scale_x_continuous(breaks=round(seq(5000.0, 100000.0, by=5000), 1)) +
  scale_y_continuous(breaks=sort(c(round(seq(0, max(d$value)+1, by=5), 1)))) +
  theme(legend.position="bottom")

print(p)
dev.off()
