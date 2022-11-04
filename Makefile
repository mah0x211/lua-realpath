MODULE=realpath
TARGET=src/realpath.$(LIB_EXTENSION)
SRCS=$(wildcard src/*.c)
OBJS=$(SRCS:.c=.$(LIB_EXTENSION))
GCDAS=$(OBJS:.$(LIB_EXTENSION)=.gcda)
INSTALL?=install

ifdef REALPATH_COVERAGE
COVFLAGS=--coverage
endif

.PHONY: all install

all: $(OBJS)

%.o: %.c
	$(CC) $(CFLAGS) $(WARNINGS) $(COVFLAGS) $(CPPFLAGS) -o $@ -c $<

%.$(LIB_EXTENSION): %.o
	$(CC) -o $@ $^ $(LDFLAGS) $(LIBS) $(PLATFORM_LDFLAGS) $(COVFLAGS)

install: $(OBJS)
	$(INSTALL) -d $(INST_LIBDIR)/$(MODULE)
	$(INSTALL) $(TARGET) $(INST_LIBDIR)
	$(INSTALL) $(filter-out $(TARGET), $(OBJS)) $(INST_LIBDIR)/$(MODULE)
	rm -f $(OBJS) $(GCDAS)
