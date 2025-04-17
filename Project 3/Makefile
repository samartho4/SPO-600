# Makefile for SPO600 Project Stage III
# This Makefile helps build and test the enhanced GCC pass

# Default compiler flags
CFLAGS = -Wall -g

# Paths - Update these with your actual paths on the servers
GCC_SRC_DIR = ~/gcc-src
GCC_BUILD_DIR = ~/gcc-build
PASS_DIR = $(GCC_SRC_DIR)/gcc/tree-pass/passes
GCC_INSTALL_DIR = $(GCC_BUILD_DIR)/gcc

# Test programs
TEST_PROGRAMS = test-multi-clone

# Default target - show help message
all:
	@echo "SPO600 Project Stage III Makefile"
	@echo ""
	@echo "Available targets:"
	@echo "  install-pass    - Install the enhanced cloneprune pass into GCC source tree"
	@echo "  build-gcc      - Build GCC with the installed pass"
	@echo "  build-tests    - Build the test programs with the custom GCC"
	@echo "  run-tests      - Run the compiled test programs and show dump output"
	@echo "  clean          - Remove generated files"
	@echo ""
	@echo "Full workflow: make install-pass build-gcc build-tests run-tests"

# Install the pass to the GCC source tree
install-pass:
	@echo "Installing cloneprune pass to GCC source tree..."
	@if [ ! -d $(PASS_DIR) ]; then \
		mkdir -p $(PASS_DIR); \
	fi
	cp tree-cloneprune.cc $(GCC_SRC_DIR)/gcc
	
	@echo "Updating GCC configuration files..."
	@if ! grep -q "cloneprune" $(GCC_SRC_DIR)/gcc/tree-pass.h; then \
		echo "extern gimple_opt_pass *make_pass_cloneprune (gcc::context *ctxt);" >> $(GCC_SRC_DIR)/gcc/tree-pass.h; \
	fi
	@if ! grep -q "cloneprune" $(GCC_SRC_DIR)/gcc/passes.def; then \
		echo "NEXT_PASS (pass_cloneprune, 1);" >> $(GCC_SRC_DIR)/gcc/passes.def; \
	fi
	@echo "Checking if Makefile.in needs updating..."
	@if ! grep -q "tree-cloneprune" $(GCC_SRC_DIR)/gcc/Makefile.in; then \
		sed -i 's/tree-ssa-loop-ivcanon.o/tree-ssa-loop-ivcanon.o tree-cloneprune.o/g' $(GCC_SRC_DIR)/gcc/Makefile.in; \
	fi
	@echo "Pass installation complete."

# Build GCC with our custom pass
build-gcc:
	@echo "Building GCC with custom pass..."
	cd $(GCC_BUILD_DIR) && make -j$(nproc)
	@echo "GCC build complete."

# Build the test programs using our custom GCC
build-tests:
	@echo "Building test programs..."
	for prog in $(TEST_PROGRAMS); do \
		$(GCC_INSTALL_DIR)/xgcc -B$(GCC_INSTALL_DIR) -fdump-tree-cloneprune $$prog.c -o $$prog; \
	done
	@echo "Test builds complete."

# Run the compiled test programs
run-tests:
	@echo "Running test programs and showing dump output..."
	for prog in $(TEST_PROGRAMS); do \
		echo "===== Running $$prog ====="; \
		./$$prog; \
		echo ""; \
		echo "===== Dump file for $$prog ====="; \
		cat $$prog.c.*.cloneprune; \
		echo ""; \
	done

# Clean up generated files
clean:
	rm -f $(TEST_PROGRAMS)
	rm -f *.o
	rm -f *.cloneprune

.PHONY: all install-pass build-gcc build-tests run-tests clean
