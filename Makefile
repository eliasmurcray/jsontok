TARGET = build/main

all: $(TARGET)

$(TARGET): $(wildcard src/*.c)
	@mkdir -p $(dir $@)
	gcc -std=c89 -Ofast -Wall -Wextra -Iinclude/ -o $@ $^

clean:
	rm -rf $(dir $(TARGET))

.PHONY: all clean

