# Copyright 2025 dah4k
# SPDX-License-Identifier: EPL-2.0

PROGRAM     := log2json
_ANSI_NORM  := \033[0m
_ANSI_CYAN  := \033[36m

.PHONY: help usage
help usage:
	@grep -hE '^[0-9a-zA-Z_-]+:.*?## .*$$' $(MAKEFILE_LIST) \
		| awk 'BEGIN {FS = ":.*?##"}; {printf "$(_ANSI_CYAN)%-20s$(_ANSI_NORM) %s\n", $$1, $$2}'

.PHONY: all
all: $(PROGRAM) ## Build program

%: %.c
	$(CC) -o $@ $< -Wall -W -pedantic -O2 -std=c99

.PHONY: test
test: $(PROGRAM) ## Test run program
	./$< testdata/quoted_values.input | jq .

.PHONY: memtest
memtest: $(PROGRAM) ## Memory test program with Valgrind
	valgrind --tool=memcheck --log-file=$(PROGRAM).vg.log ./$(PROGRAM) testdata/quoted_values.input

.PHONY: clean
clean: ## Delete program
	rm -f $(PROGRAM)
	rm -f *.log

