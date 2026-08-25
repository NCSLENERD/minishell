NAME = minishell
CC = cc
CFLAGS = -Wall -Wextra -Werror
CLIBS =  -lreadline
SRCS = main.c lexer.c parser.c src/minilibft/ft_substr.c src/minilibft/ft_itoa.c src/minilibft/ft_strchr.c\
src/minilibft/ft_putstr_fd.c src/minilibft/ft_strjoin.c src/utils/utils_env.c src/utils/utils_command.c\
src/utils/utils_lexer.c src/utils/utils_lexer2.c src/utils/utils_redirect.c\
src/utils/utils_piece.c src/utils/utils_token.c src/utils/utils_parser.c\
src/utils/utils_parser2.c src/utils/utils_envp.c debug.c src/utils/utils_expansion.c
OBJS = $(SRCS:.c=.o)

all: $(NAME)
$(NAME): $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o $(NAME) $(CLIBS)

%.o: 	%.c minishell.h
	$(CC) $(CFLAGS) -c $< -o $@
clean: 
	rm -f $(OBJS)

fclean: clean 
	rm -f $(NAME)

re: fclean  all

.PHONY:  all clean fclean re