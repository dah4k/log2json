# Copyright 2025 dah4k
# SPDX-License-Identifier: EPL-2.0

PROGRAM     := log2json
_ANSI_NORM  := \033[0m
_ANSI_CYAN  := \033[36m

.PHONY: all
all: $(PROGRAM) ## Build program

%: %.c
	$(CC) -o $@ $< -Wall -W -pedantic -O2 -std=c99

.PHONY: test
test: $(PROGRAM) ## Test run program
	./$< testdata/quoted_values.input testdata/quoted_values.json

.PHONY: memtest
memtest: $(PROGRAM) ## Memory test program with Valgrind
	valgrind --tool=memcheck --log-file=$(PROGRAM).vg.log ./$(PROGRAM) testdata/quoted_values.input testdata/quoted_values.json
	cat $(PROGRAM).vg.log

.PHONY: clean
clean: ## Delete program
	rm -f $(PROGRAM)
	rm -f *.log
	rm -f testdata/quoted_values.json

.PHONY: help usage
help usage:
	@grep -hE '^[0-9a-zA-Z_-]+:.*?## .*$$' $(MAKEFILE_LIST) \
		| awk 'BEGIN {FS = ":.*?##"}; {printf "$(_ANSI_CYAN)%-20s$(_ANSI_NORM) %s\n", $$1, $$2}'

