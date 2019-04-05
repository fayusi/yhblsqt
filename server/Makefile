SRCS:=$(wildcard *.c)
OBJS:=$(SRCS)
ELF:=Server
$(ELF):$(OBJS)
	gcc $^ -o $@ -pthread -I /usr/include/mysql/ -lmysqlclient -lcrypt
clean:
	rm -rf $(OBJS) $(ELF)
