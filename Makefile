# =========================
# Compiler settings
# =========================
NAME        = webserv
CXX         = c++
CXXFLAGS    = -g  -Wall -Wextra -Werror -std=c++98
SRC_DIR     = src
OBJ_DIR     = obj

# =========================
# Main webserv sources
# =========================
SRCS_FILE = \
	FdHandler  SignalHandler CgiHandler   ListenHandler  ConnectionHandler \
	Token  ConfigParser CgiParser ConfigBuilder main   \
	ServerLoop Utils  PortManager\
	HttpProtocol HttpResponse  HttpRequest  HttpParser\
	BodyReader  ChunkBodyReader StartLineReader HeaderFieldReader MimeTypes \
	DeleteMethod PostMethod  GetMethod Session

SRCS = $(addprefix $(SRC_DIR)/, $(addsuffix .cpp, $(SRCS_FILE)))
OBJ  = $(addprefix $(OBJ_DIR)/, $(addsuffix .o, $(SRCS_FILE)))

# =========================
# Config test
# =========================
CONF_NAME = conf_test
CONF_FILE = Utils ConfigParser Token ConfigBuilder ConfigTestMain

CONF_SRCS = $(addprefix $(SRC_DIR)/, $(addsuffix .cpp, $(CONF_FILE)))
CONF_OBJ  = $(addprefix $(OBJ_DIR)/, $(addsuffix .o, $(CONF_FILE)))

# =========================
# Rules
# =========================
all: $(NAME)

$(NAME): $(OBJ)
	$(CXX) $(CXXFLAGS) $^ -o $@
	@echo "=== Build Complete: $(NAME) ==="

$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp | $(OBJ_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

# =========================
# Config test
# =========================
conf: $(CONF_NAME)

$(CONF_NAME): $(CONF_OBJ)
	$(CXX) $(CXXFLAGS) $^ -o $@
	@echo "=== Build Complete: $(CONF_NAME) ==="

# =========================
# Clean
# =========================
clean:
	rm -rf $(OBJ_DIR)

fclean: clean
	rm -f $(NAME) $(ECHO_NAME) $(CONF_NAME) $(CGI_NAME)

re: fclean all

.PHONY: all clean fclean re echo conf cgi
