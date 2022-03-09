OBJS := out/tree_view.o

SRC := $(wildcard src/*.c)

app.out : $(OBJS) 
	gcc -o $@ $^

out/%.o : src/%.c
	mkdir -p out
	gcc -c $< -o $@

.PHONY : clean 
clean:
	$(RM) out/* *.out *.o