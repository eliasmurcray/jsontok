CFLAGS = -std=c89 -Ofast -Wall -Wextra -Iinclude/
OUT = build

$(OUT):
	@mkdir -p $(OUT)

lib: $(OUT)
	$(CC) $(CFLAGS) -c src/jsontok.c -o $(OUT)/jsontok.o
	$(AR) rcs $(OUT)/libjsontok.a $(OUT)/jsontok.o

test: $(OUT)
	$(CC) $(CFLAGS) src/jsontok.c src/test.c -o $(OUT)/test
	./$(OUT)/test

benchmark: $(OUT)
	$(CC) $(CFLAGS) src/jsontok.c src/benchmark.c -o $(OUT)/benchmark
	./$(OUT)/benchmark

clean:
	rm -rf $(OUT)/

.PHONY: all clean
