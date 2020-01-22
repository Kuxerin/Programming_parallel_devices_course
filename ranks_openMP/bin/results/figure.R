library('ggplot2')
library('reshape')
library('Cairo')
library("grid")

plottitle <- "Ranksort"
plotlabels <- c("1 szál", "2 szál","4 szál","6 szál", "8 szál", "12 szál")
xlabel <- "Mintaszám (*100)"
ylabel <- "Ido (s)"

data1 <- read.table('1thread.dat')
data2 <- read.table('2thread.dat')
data4 <- read.table('4thread.dat')
data6 <- read.table('6thread.dat')
data8 <- read.table('8thread.dat')
data12 <- read.table('12thread.dat')
d <- data.frame(data1$V2, data1$V1, data2$V1, data4$V1, data6$V1, data8$V1, data12$V1)
colnames(d) <- c('sample', 'thread1', 'thread2', 'thread4', 'thread6', 'thread8', 'thread12')
d <- melt(d, id='sample', variable_name='threads')

cairo_ps("ranksort.eps", width=20, height=8)  
p<-ggplot(d, 
  aes_string(x=names(d)[1], y=names(d)[3], colour=names(d)[2]), 
  labeller=label_parsed) + 
  geom_point(size=2) + 
  geom_line(size=1.5) + 
  labs(title=plottitle) + 
  xlab(xlabel) + 
  ylab(ylabel) + 
  scale_colour_manual(values=c("black", "blue", "red", "green", "purple", "yellow"), name="", labels=plotlabels, guide=guide_legend(keyheight=unit(2, "line"), keywidth=unit(5, "line"))) +
  theme_gray(24) +
  scale_x_continuous(breaks=round(seq(0.0, 1000.0, by=100), 1)) +
  scale_y_continuous(breaks=sort(c(round(seq(0, max(d$value)+1, by=mean(d$value)), 1)))) +
  theme(legend.position="bottom")

print(p)
dev.off()
