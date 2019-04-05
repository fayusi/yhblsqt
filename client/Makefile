SRCS:=$(wildcard *.c)
OBJS:=$(SRCS:%.c=%.o)
ELF:=Client
$(ELF):$(OBJS)
	gcc  $^ -o $@ -pthread -lcrypt
clean:
	rm -rf $(OBJS) $(ELF)
