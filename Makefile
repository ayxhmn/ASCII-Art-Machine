# Compilers & Toolchain
CLANG     = clang
GCC       = gcc
BUILD_DIR = bin

# Base standard + modern safety warnings (Strict Day-1 configuration)
CFLAGS  = -std=c23 -Wall -Wextra -Wpedantic -Wshadow -Wpointer-arith \
          -Wstrict-prototypes -Wwrite-strings -Wconversion
LDFLAGS = -lm

# High-performance optimizations for Zen 3 and mold linker
OPT_FLAGS = -O3 -march=znver3 -fuse-ld=mold

# Runtime memory and undefined-behavior sanitizer flags (Incompatible with Valgrind)
SANITIZE_FLAGS = -O0 -g3 -fsanitize=address,undefined -fno-omit-frame-pointer

# Dedicated Valgrind flags
VALGRIND_FLAGS = -O0 -g3 -fno-inline -fno-omit-frame-pointer

# Standard Valgrind analysis execution command
VALGRIND_RUN   = valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes

# Handles tab-completion stripping (.c) and sets output binary name cleanly
SRC_BASE = $(basename $(SOURCE))
OUT      = $(if $(OUTPUT),$(OUTPUT),$(notdir $(SRC_BASE)))

.PHONY: all clean clang gcc clang_optimized gcc_optimized sanitize valgrind asm

# Helper macro to enforce that SOURCE variable is passed
define check_source
	@if [ -z "$(SOURCE)" ]; then \
		echo "❌ Error: SOURCE variable is missing!"; \
		echo "Usage: make $@ SOURCE=path/to/file.c [OUTPUT=outname]"; \
		exit 1; \
	fi
	@mkdir -p $(BUILD_DIR)
endef

# Standard build with Clang + safety flags (fast compilation)
clang:
	$(check_source)
	$(CLANG) $(CFLAGS) $(SRC_BASE).c -o $(BUILD_DIR)/$(OUT) $(LDFLAGS)
	@echo "🚀 Compilation successful! Executing... :)"
	@./$(BUILD_DIR)/$(OUT)

# Standard build with GCC + safety flags (fast compilation)
gcc:
	$(check_source)
	$(GCC) $(CFLAGS) $(SRC_BASE).c -o $(BUILD_DIR)/$(OUT) $(LDFLAGS)
	@echo "🚀 Compilation successful! Executing... :)"
	@./$(BUILD_DIR)/$(OUT)

# Optimized build with Clang (-O3, znver3 AVX2/AVX512 instructions, mold linker)
clang_optimized:
	$(check_source)
	$(CLANG) $(CFLAGS) $(OPT_FLAGS) $(SRC_BASE).c -o $(BUILD_DIR)/$(OUT) $(LDFLAGS)
	@echo "🔥 Compiled with Clang (Optimized: -O3, znver3, mold). Executing..."
	@./$(BUILD_DIR)/$(OUT)

# Optimized build with GCC (-O3, znver3 AVX2/AVX512 instructions, mold linker)
gcc_optimized:
	$(check_source)
	$(GCC) $(CFLAGS) $(OPT_FLAGS) $(SRC_BASE).c -o $(BUILD_DIR)/$(OUT) $(LDFLAGS)
	@echo "🔥 Compiled with GCC (Optimized: -O3, znver3, mold). Executing..."
	@./$(BUILD_DIR)/$(OUT)

# Memory bounds check, use-after-free, and undefined behavior trap (Do NOT use Valgrind here)
sanitize:
	$(check_source)
	$(CLANG) $(CFLAGS) $(SANITIZE_FLAGS) $(SRC_BASE).c -o $(BUILD_DIR)/$(OUT) $(LDFLAGS)
	@echo "🛡️ Running with AddressSanitizer & UndefinedBehaviorSanitizer..."
	@./$(BUILD_DIR)/$(OUT)

# Dedicated target for Valgrind (Compiles cleanly without sanitizers to avoid collision)
valgrind:
	$(check_source)
	$(CLANG) $(CFLAGS) $(VALGRIND_FLAGS) $(SRC_BASE).c -o $(BUILD_DIR)/$(OUT) $(LDFLAGS)
	@echo "🔍 Running deep memory analysis under Valgrind..."
	@$(VALGRIND_RUN) ./$(BUILD_DIR)/$(OUT)

# Emits Intel-syntax annotated assembly for low-level inspection
asm:
	$(check_source)
	$(CLANG) $(CFLAGS) $(OPT_FLAGS) -S -masm=intel -fverbose-asm $(SRC_BASE).c -o $(BUILD_DIR)/$(OUT).s
	@echo "📁 Assembly generated at: $(BUILD_DIR)/$(OUT).s"

clean:
	@rm -rf $(BUILD_DIR)
	@echo "🧹 Deleted builds directory!"

