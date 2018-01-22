IMAGEMKNAME		:= imagemk
IMAGEMKBIN		:= $(IMAGEMKDIR)/build/$(IMAGEMKNAME)

VPATH			+= src

dir-imagemk:
	@mkdir -p $(IMAGEMKDIR)/obj
	@mkdir -p $(IMAGEMKDIR)/build

build-imagemk: dir-imagemk $(IMAGEMKBIN)

clean-imagemk:
	@rm -f $(IMAGEMKBIN)
	@rm -f $(IMAGEMKDIR)/obj/imagemk.o

$(IMAGEMKBIN): $(IMAGEMKDIR)/obj/imagemk.o
	@g++ -o $(IMAGEMKBIN) $(IMAGEMKDIR)/obj/imagemk.o -std=c++14

$(IMAGEMKDIR)/obj/imagemk.o: $(IMAGEMKDIR)/src/imagemk.cc
	@g++ -c $^ -o $@ -std=c++14
