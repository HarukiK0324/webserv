# =========================
# Compiler settings
# =========================
NAME        = webserv
CONF_NAME   = conf_test
CXX         = c++
CXXFLAGS    = -g -Wall -Wextra -Werror -std=c++98
SRC_DIR     = src
OBJ_DIR     = obj
INC_DIR     = include
CPPFLAGS    = -I$(INC_DIR) -I$(INC_DIR)/core -I$(INC_DIR)/http -I$(INC_DIR)/runtime

# =========================
# Sources
# =========================
SRCS        = \
	$(SRC_DIR)/main.cpp \
	$(SRC_DIR)/core/MimeTypes.cpp \
	$(SRC_DIR)/core/PortManager.cpp \
	$(SRC_DIR)/core/Session.cpp \
	$(SRC_DIR)/core/Token.cpp \
	$(SRC_DIR)/core/Utils.cpp \
	$(SRC_DIR)/http/BodyReader.cpp \
	$(SRC_DIR)/http/CgiHandler.cpp \
	$(SRC_DIR)/http/CgiParser.cpp \
	$(SRC_DIR)/http/DeleteMethod.cpp \
	$(SRC_DIR)/http/GetMethod.cpp \
	$(SRC_DIR)/http/HeaderFieldReader.cpp \
	$(SRC_DIR)/http/HttpParser.cpp \
	$(SRC_DIR)/http/HttpProtocol.cpp \
	$(SRC_DIR)/http/HttpRequest.cpp \
	$(SRC_DIR)/http/HttpResponse.cpp \
	$(SRC_DIR)/http/Method.cpp \
	$(SRC_DIR)/http/PostMethod.cpp \
	$(SRC_DIR)/http/StartLineReader.cpp \
	$(SRC_DIR)/runtime/AFdHandler.cpp \
	$(SRC_DIR)/runtime/ChunkBodyReader.cpp \
	$(SRC_DIR)/runtime/ConfigBuilder.cpp \
	$(SRC_DIR)/runtime/ConfigParser.cpp \
	$(SRC_DIR)/runtime/ConnectionHandler.cpp \
	$(SRC_DIR)/runtime/ListenHandler.cpp \
	$(SRC_DIR)/runtime/ServerLoop.cpp
OBJ         = $(patsubst $(SRC_DIR)/%.cpp,$(OBJ_DIR)/%.o,$(SRCS))

# conf_test: config parser/builder test binary only
CONF_SRCS   = \
	$(SRC_DIR)/core/Utils.cpp \
	$(SRC_DIR)/core/Token.cpp \
	$(SRC_DIR)/runtime/ConfigParser.cpp \
	$(SRC_DIR)/runtime/ConfigBuilder.cpp \
	$(SRC_DIR)/ConfigTestMain.cpp
CONF_OBJ    = $(patsubst $(SRC_DIR)/%.cpp,$(OBJ_DIR)/%.o,$(CONF_SRCS))

# =========================
# Rules
# =========================
all: $(NAME)

$(NAME): $(OBJ)
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) $^ -o $@
	@echo "=== Build Complete: $(NAME) ==="

conf: $(CONF_NAME)

$(CONF_NAME): $(CONF_OBJ)
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) $^ -o $@
	@echo "=== Build Complete: $(CONF_NAME) ==="

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) -c $< -o $@

clean:
	rm -rf $(OBJ_DIR)

fclean: clean
	rm -f $(NAME) $(CONF_NAME)

re: fclean all

.PHONY: all conf clean fclean re
