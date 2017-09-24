VPATH		+= src

dir-imagemk:
	@mkdir -p $(IMAGEMKDIR)/obj

build-imagemk: dir-imagemk $(IMAGEMKDIR)/imagemk

clean-imagemk:
	@rm -f $(IMAGEMKDIR)/imagemk
	@rm -f $(IMAGEMKDIR)/obj/imagemk.o

$(IMAGEMKDIR)/imagemk: $(IMAGEMKDIR)/obj/imagemk.o
	@g++ -o $(IMAGEMKDIR)/imagemk $(IMAGEMKDIR)/obj/imagemk.o -std=c++14

$(IMAGEMKDIR)/obj/imagemk.o: $(IMAGEMKDIR)/src/imagemk.cc
	@g++ -c $^ -o $@ -std=c++14
