CC = g++

CFLAGS = -Wall -Wextra -Werror -std=c++20

LDFLAGS = -L/usr/local/lib -ldpp

SRC = main.cpp src/Bot.cpp src/Commands.cpp

OBJ = $(SRC:.cpp=.o)

NAME = Bot

all: $(NAME)

$(NAME): $(OBJ)
	$(CC) -g -o $(NAME) $(OBJ) $(LDFLAGS)

%.o: %.cpp
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -rf $(OBJ)

fclean: clean
	rm -rf $(NAME)

re: fclean all
