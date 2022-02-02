CXX			= clang++

CXXFLAGS	= -Wall -Wextra -Werror -std=c++98 -pedantic -g -fsanitize=address

SRCS_PATH	= srcs

SRCS		= Internet_socket.cpp Server.cpp utils.cpp Rules.cpp webserv.cpp parsing.cpp Virtual_server.cpp post_requests.cpp Request.cpp \
				Location.cpp get_requests.cpp delete_requests.cpp cgi_handling.cpp file_uploads.cpp

SRCS_NAME	= $(addprefix $(SRCS_PATH)/, $(SRCS))

OBJS		= $(SRCS_NAME:.cpp=.o)

NAME		= webserv

RM			= rm -rf



all		:	$(NAME)

$(NAME)	:	$(OBJS)
				$(CXX) $(CXXFLAGS) -o $(NAME) $(OBJS)


run		:	$(NAME)
				./$(NAME)

clean	:	
				$(RM) $(OBJS)

fclean	:	clean
				$(RM) $(NAME)

re		:	fclean all

.PHONY	:	all fclean re
