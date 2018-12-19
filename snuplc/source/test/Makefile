ROOT=..

%: %.mod
	$(ROOT)/snuplc/snuplc --ast --tac $<
	gcc -static -o $@ $<.s $(ROOT)/rte/AMD64/IO.s $(ROOT)/rte/AMD64/ARRAY.s

all:
	@for i in *.mod; do \
		make $${i%%.mod}; \
	done

clean:
	@for i in *.mod; do \
		rm -f $${i%%.mod} $${i}.s $${i}.tac $${i}.ast $${i}.*.dot $${i}.*.pdf; \
	done
