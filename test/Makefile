ROOT=..
is64=true

%: %.mod
ifeq ($(is64),true)
	$(ROOT)/snuplc/snuplc 64 --ast --tac $<
	cp $<.s $<_64.s
	cp $<.tac $<_64.tac
	gcc -static -o $@ $<.s $(ROOT)/rte/AMD64/IO.s $(ROOT)/rte/AMD64/ARRAY.s
else
	$(ROOT)/snuplc/snuplc 32 --ast --tac $<
	cp $<.s $<_32.s
	cp $<.tac $<_32.tac
	gcc -static -m32 -o $@ $<.s $(ROOT)/rte/IA32/IO.s $(ROOT)/rte/IA32/ARRAY.s
endif

all:
	@for i in *.mod; do \
		make $${i%%.mod}; \
	done

clean:
	@for i in *.mod; do \
		rm -f $${i%%.mod} $${i}.s $${i}.tac $${i}.ast $${i}.*.dot $${i}.*.pdf $${i}_64.s $${i}_32.s $${i}_64.tac; \
	done
