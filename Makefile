NAME = minishell
CC = cc
CFLAGS = -Wall -Wextra -Werror -lreadline
SRCS = src/main.c ...
OBJS = $(SRC:.c=.o)

all: $(NAME)
 $(NAME): $(OBJ)

%.o: 	%.c philo.h
	$(CC) $(CFLAGS) -c $< -o $@
	
clean: 
	rm -f $(OBJ)

fclean: clean 
	rm -f $(NAME)

re: fclean  all

.PHONY:  all clean fclean