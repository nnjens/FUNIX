targets := libuthread.a
objs	:= queue.o uthread.o context.o semaphore.o preempt.o bitmap.o palloc.o tls.o disk.o fs.o
CC	:= gcc
CFLAGS	:= -Wall
CFLAGS	+= -g
CFLAGS	+= -O0

ifneq ($(V),1)
Q = @
endif

all: $(targets)

deps    := $(patsubst %.o,%.d,$(objs))
-include $(deps)

libuthread.a: $(objs) 
	ar -cr $@ $(objs)

%.o: %.c
	@echo "CC $@"
	$(Q)$(CC) $(CFLAGS) -c -o $@ $< -MMD -MF $(@:.o=.d)

clean: 
	@echo "clean"
	$(Q)rm -f $(targets) $(objs) $(deps)
