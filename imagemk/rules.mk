VPATH		+= src

build-imagemk: $(IMAGEMKDIR)/imagemk

clean-imagemk:
	@rm -f $(IMAGEMKDIR)/imagemk
	@rm -f $(IMAGEMKDIR)/obj/imagemk.o

$(IMAGEMKDIR)/imagemk: $(IMAGEMKDIR)/obj/imagemk.o
	@g++ -o $(IMAGEMKDIR)/imagemk $(IMAGEMKDIR)/obj/imagemk.o

$(IMAGEMKDIR)/obj/imagemk.o: $(IMAGEMKDIR)/src/imagemk.cc
	@g++ -c $^ -o $@