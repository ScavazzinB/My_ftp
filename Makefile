##
## EPITECH PROJECT, 2025
## B-NWP-400-NCE-4-1-myftp-baptiste.scavazzin
## File description:
## Makefile
##

CXX = g++
CXXFLAGS = -Wall -Wextra -std=c++11

TARGET = myftp
SRCS = $(wildcard *.cpp)
OBJS = $(SRCS:.cpp=.o)

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(OBJS)

clean:
	rm -f $(OBJS) $(TARGET)

fclean: clean
	rm -f $(TARGET)

re: fclean all

phony: all clean fclean re
