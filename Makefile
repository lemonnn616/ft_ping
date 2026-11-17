NAME = ft_ping

CC = cc

CFLAGS = -Wall -Wextra -Werror -g3

CPPFLAGS = -Iinclude

SRC = src/main.c \
      src/options/parse_options.c \
      src/options/print_help.c \
      src/network/resolve.c \
      src/network/socket.c \
      src/icmp/checksum.c \
      src/icmp/request.c

OBJ = $(SRC:src/%.c=build/%.o)

DEP = $(OBJ:.o=.d)

all: $(NAME)

$(NAME): $(OBJ)
	$(CC) $(CFLAGS) $(OBJ) -o $(NAME)

build/%.o: src/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CPPFLAGS) $(CFLAGS) -MMD -MP -c $< -o $@

clean:
	rm -rf build

fclean: clean
	rm -f $(NAME)

re: fclean all

-include $(DEP)

.PHONY: all clean fclean re