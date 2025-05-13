CXX = g++
# CXXFLAGS = -std=c++17 -Wall -Wextra -O2 -fsanitize=address
CXXFLAGS = -std=c++17 -Wall -Wextra -O2
SRC_DIR = .
SIMULATOR_DIR = $(SRC_DIR)/simulator
WORKLOADS_DIR = $(SRC_DIR)/workloads
TESTS_DIR = $(SRC_DIR)/tests
BUILD_DIR = build
BIN_DIR = $(BUILD_DIR)/bin

ifeq ($(OS),Windows_NT)
    MKDIR = -md
    RM = -rd /s /q
else
    MKDIR = mkdir -p
    RM = rm -rf
endif

# Source files
MAIN_SRC = $(SRC_DIR)/main.cpp
SIMULATOR_SRC = $(wildcard $(SIMULATOR_DIR)/*.cpp)
WORKLOADS_SRC = $(wildcard $(WORKLOADS_DIR)/*.cpp)
TESTS_SRC = $(wildcard $(TESTS_DIR)/*.cpp)

# Object files
MAIN_OBJ = $(BUILD_DIR)/main.o
SIMULATOR_OBJS = $(patsubst $(SIMULATOR_DIR)/%.cpp,$(BUILD_DIR)/%.o,$(SIMULATOR_SRC))
WORKLOADS_OBJS = $(patsubst $(WORKLOADS_DIR)/%.cpp,$(BUILD_DIR)/%.o,$(WORKLOADS_SRC))
TESTS_OBJS = $(patsubst $(TESTS_DIR)/%.cpp,$(BUILD_DIR)/tests_%.o,$(TESTS_SRC))

# Final executables
TARGET = $(BIN_DIR)/cache_simulator
TESTS_TARGET = $(BIN_DIR)/unit_tests

all: $(TARGET) $(TESTS_TARGET)

# Note: Windows seems to need double quotes
$(BUILD_DIR):
	$(MKDIR) "$(BUILD_DIR)"

$(BIN_DIR): | $(BUILD_DIR)
	$(MKDIR) "$(BIN_DIR)"

$(TARGET): $(MAIN_OBJ) $(SIMULATOR_OBJS) $(WORKLOADS_OBJS) | $(BIN_DIR)
	$(CXX) $(CXXFLAGS) -o $@ $^

$(TESTS_TARGET): $(TESTS_OBJS) $(SIMULATOR_OBJS) | $(BIN_DIR)
	$(CXX) $(CXXFLAGS) -o $@ $^

$(MAIN_OBJ): $(MAIN_SRC) | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) -c -o $@ $<

$(BUILD_DIR)/%.o: $(SIMULATOR_DIR)/%.cpp | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) -c -o $@ $<

$(BUILD_DIR)/%.o: $(WORKLOADS_DIR)/%.cpp | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) -c -o $@ $<

$(BUILD_DIR)/tests_%.o: $(TESTS_DIR)/%.cpp | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) -c -o $@ $<

clean:
	$(RM) $(BUILD_DIR)
ifeq ($(OS),Windows_NT)
	-del *_trace.txt
else
	$(RM) *_trace.txt
endif

# Run the simulator
run: $(TARGET)
	$(TARGET)

# Run the unit tests
test: $(TESTS_TARGET)
	$(TESTS_TARGET)

.PHONY: all clean run test
