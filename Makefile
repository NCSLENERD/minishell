NAME = minishell
CC = cc
CFLAGS = -Wall -Wextra -Werror
CLIBS =  -lreadline
LIBFT_DIR = libft
LIBFT = $(LIBFT_DIR)/libft.a

SRCS = main.c \
	lexer.c \
	parser.c \
	debug.c \
	src/utils/utils_env.c \
	src/utils/utils_envp.c \
	src/utils/utils_command.c \
	src/utils/utils_lexer.c \
	src/utils/utils_lexer2.c \
	src/utils/utils_redirect.c \
	src/utils/utils_piece.c \
	src/utils/utils_token.c \
	src/utils/utils_parser.c \
	src/utils/utils_parser2.c \
	src/utils/utils_parser3.c \
	src/utils/utils_parser4.c \
	src/utils/utils_expansion.c \
	src/utils/utils_expansion2.c \
	exec/exec.c \
	exec/env_tab.c \
	exec/path.c \
	exec/error_exec.c \
	exec/redirect.c \
	exec/pipeline.c

OBJS = $(SRCS:.c=.o)

all: $(NAME)

$(LIBFT):
	$(MAKE) -C $(LIBFT_DIR)

$(NAME): $(LIBFT) $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o $(NAME) -L$(LIBFT_DIR) -lft $(CLIBS)

%.o: 	%.c minishell.h
	$(CC) $(CFLAGS) -c $< -o $@
clean:
	$(MAKE) -C $(LIBFT_DIR) clean
	rm -f $(OBJS)

fclean: clean
	$(MAKE) -C $(LIBFT_DIR) fclean
	rm -f $(NAME)

re: fclean  all

.PHONY:  all clean fclean re
