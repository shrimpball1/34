CXX        = g++
CXXFLAGS   = -std=c++17 -Iinclude

GTEST_DIR  = /mnt/f/googletest/build/lib
GTEST_INC  = /mnt/f/googletest/googletest/include

INC_DIRS   = -I$(GTEST_INC) -Iinclude

LDLIBS     = -L$(GTEST_DIR) -lgtest -lgtest_main -lpthread

TARGET     = bin/StringUtilsTest

SRCS       = src/StringUtils.cpp \
             testsrc/StringUtilsTest.cpp

OBJ_DIR    = obj
BIN_DIR    = bin

OBJS       = $(patsubst src/%.cpp, $(OBJ_DIR)/%.o, $(filter src/%, $(SRCS))) \
             $(patsubst testsrc/%.cpp, $(OBJ_DIR)/%.o, $(filter testsrc/%, $(SRCS)))

all: $(BIN_DIR) $(OBJ_DIR) $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) $^ $(LDLIBS) -o $@
$(OBJ_DIR)/%.o: src/%.cpp | $(OBJ_DIR)
	$(CXX) $(CXXFLAGS) $(INC_DIRS) -c $< -o $@
$(OBJ_DIR)/%.o: testsrc/%.cpp | $(OBJ_DIR)
	$(CXX) $(CXXFLAGS) $(INC_DIRS) -c $< -o $@

$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)

$(BIN_DIR):
	mkdir -p $(BIN_DIR)

clean:
	rm -rf $(BIN_DIR) $(OBJ_DIR)
