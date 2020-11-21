TARGET = qnx4opts.a
SRCDIR = src
SRCS = re.cpp optparam.cpp optparser.cpp

BINDIR = bin
OBJDIR = obj
OBJS = $(SRCS:.cpp=.o)
CC = cc
INCLUDEDIRS +=  ./inc

CFLAGS = -5r -ms -Wc,-s -WC,-xss -Oentx \
						$(addprefix -I, $(INCLUDEDIRS))

.PHONY:     			all clean

all:					$(BINDIR)/$(TARGET)

clean:
						-rm -f $(OBJDIR)/*.o $(BINDIR)/$(TARGET) $(BINDIR)/*.map *.err

$(OBJDIR)/%.o : $(SRCDIR)/%.cpp
						$(CC) $(CFLAGS) -c -o $@ $<

$(BINDIR)/$(TARGET): 	$(addprefix $(OBJDIR)/, $(OBJS))
						$(AR) $(ARFLAGS) $@ $?
