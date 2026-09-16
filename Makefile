TARGETS_C = src/main.c src/lex.c src/lexSetup.c src/parse.c src/gen-peobj.c src/gen-bin.c
TARGETS_H = inc/main.h inc/lex.h inc/lexSetup.h inc/parse.h inc/gen-peobj.h inc/gen-bin.h

nemai: $(TARGETS_C) $(TARGETS_H)
	clang -Iinc/ $(TARGETS_C) -o nemai

debug: nemai-debug

nemai-debug: $(TARGETS_C) $(TARGETS_H)
	clang -g -Iinc/ $(TARGETS_C) -o nemai-debug
