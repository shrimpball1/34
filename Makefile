# googletest local path
GTEST_DIR = /mnt/f/googletest
GTEST_INC = $(GTEST_DIR)/googletest/include
GMOCK_INC = $(GTEST_DIR)/googlemock/include
GTEST_LIB = $(GTEST_DIR)/build/lib

GTEST_LDFLAGS = -L$(GTEST_LIB) -lgtest -lgmock -lgtest_main -pthread
EXPAT_LDFLAGS = -lexpat

CXX = g++
CXXFLAGS = -std=c++17 -Wall -Iinclude -I$(GTEST_INC) -I$(GMOCK_INC)

INCDIR = include
SRCDIR = src
TESTDIR = testsrc
OBJDIR = obj
BINDIR = bin

APP_SRCS = $(SRCDIR)/kmlout.cpp $(SRCDIR)/speedtest.cpp $(SRCDIR)/transplanner.cpp

ALL_SRCS = $(wildcard $(SRCDIR)/*.cpp)
LIB_SRCS = $(filter-out $(APP_SRCS), $(ALL_SRCS))
LIB_OBJS = $(patsubst $(SRCDIR)/%.cpp, $(OBJDIR)/%.o, $(LIB_SRCS))

APP_OBJS = $(patsubst $(SRCDIR)/%.cpp, $(OBJDIR)/%.o, $(APP_SRCS))

TEST_SRCS = $(wildcard $(TESTDIR)/*.cpp)
TEST_TARGETS = $(patsubst $(TESTDIR)/%.cpp, $(BINDIR)/%, $(TEST_SRCS))
TEST_OBJS = $(patsubst $(TESTDIR)/%.cpp, $(OBJDIR)/%.o, $(TEST_SRCS))

TRANSPLANNER = $(BINDIR)/transplanner
SPEEDTEST = $(BINDIR)/speedtest
KMLOUT      = $(BINDIR)/kmlout

.PHONY: all tests transplanner speedtest kmlout clean

all: tests transplanner speedtest kmlout

$(BINDIR)/%: $(OBJDIR)/%.o $(LIB_OBJS) | $(BINDIR)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(GTEST_LDFLAGS) $(EXPAT_LDFLAGS)

tests: $(TEST_TARGETS)
	@echo "all tests built"

$(TRANSPLANNER): $(OBJDIR)/transplanner.o $(LIB_OBJS) | $(BINDIR)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(EXPAT_LDFLAGS)

$(SPEEDTEST): $(OBJDIR)/speedtest.o $(LIB_OBJS) | $(BINDIR)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(EXPAT_LDFLAGS)

$(KMLOUT): $(OBJDIR)/kmlout.o $(LIB_OBJS) | $(BINDIR)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(EXPAT_LDFLAGS)

# compile lib source files
$(OBJDIR)/%.o: $(SRCDIR)/%.cpp
	@if echo "$<" | grep -q -E "(kmlout.cpp|speedtest.cpp|transplanner.cpp)"; then \
		echo "Skipping app source: $<"; \
	else \
		$(CXX) $(CXXFLAGS) -c $< -o $@; \
	fi

# compile app source files
$(OBJDIR)/transplanner.o: $(SRCDIR)/transplanner.cpp | $(OBJDIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(OBJDIR)/speedtest.o: $(SRCDIR)/speedtest.cpp | $(OBJDIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(OBJDIR)/kmlout.o: $(SRCDIR)/kmlout.cpp | $(OBJDIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(OBJDIR)/%.o: $(TESTDIR)/%.cpp | $(OBJDIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(OBJDIR):
	mkdir -p $(OBJDIR)

$(BINDIR):
	mkdir -p $(BINDIR)

clean:
	rm -rf $(OBJDIR) $(BINDIR)

#ref: https://devhints.io/makefile
