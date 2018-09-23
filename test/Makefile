ROOT=..

%: %.mod
	$(ROOT)/snuplc/snuplc --ast --tac $<
	gcc -static -m32 -o $@ $<.s $(ROOT)/rte/IA32/IO.s $(ROOT)/rte/IA32/ARRAY.s

all:
	@for i in *.mod; do \
		make $${i%%.mod}; \
	done

clean:
	@for i in *.mod; do \
		rm -f $${i%%.mod} $${i}.s $${i}.tac $${i}.ast $${i}.*.dot $${i}.*.pdf; \
	done
