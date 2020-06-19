 CFLAGS   = -Wall -std=gnu99
INCLUDES = -I .
OBJDIR   = obj

SERVER_SRCS =  message.c utils.c shared_memory.c semaphore.c fifo.c ack_table.c board.c position_file.c device.c  server.c
SERVER_OBJS = $(addprefix $(OBJDIR)/, $(SERVER_SRCS:.c=.o))

CLIENT_SRCS = ack_table.c message.c utils.c shared_memory.c semaphore.c fifo.c client.c 
CLIENT_OBJS = $(addprefix $(OBJDIR)/, $(CLIENT_SRCS:.c=.o))

all: $(OBJDIR) server client

server: $(SERVER_OBJS)
	@echo "Making executable: "$@
	@$(CC) $^ -o $@  -lm

client: $(CLIENT_OBJS)
	@echo "Making executable: "$@
	@$(CC) $^ -o $@  -lm

$(OBJDIR):
	@mkdir -p $(OBJDIR)

$(OBJDIR)/%.o: %.c
	$(CC) $(CFLAGS) $(CPPFLAGS) -o $@ -c $<

run: clean server
	@./server 100 input/file_posizioni.txt

clean:
	@rm -vf ${SERVER_OBJS}
	@rm -vf ${CLIENT_OBJS}
	@rm -vf server
	@rm -vf client
	@rm -vf /tmp/dev_fifo.*
	@rm -vf ./output/msg_id_*.txt
	@rm -vf ./output/server.run
	@ipcrm -a
	@echo "Removed object files and executables..."

.PHONY: run clean
