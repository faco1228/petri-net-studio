# Makefile — top level
# Autori: xfacka00, xlogin02

LOGIN1 = xfacka00
LOGIN2 = xlogin02
ARCHIVE = $(LOGIN1)-$(LOGIN2).zip

.PHONY: all run doxygen clean pack

all:
	$(MAKE) -C src

run: all
	./src/icp_petri

doxygen:
	doxygen Doxyfile

clean:
	$(MAKE) -C src clean
	rm -rf doc/html doc/latex
	rm -rf generated/*.cpp generated/interpreter_*
	rm -f $(ARCHIVE)

pack: clean
	zip -r $(ARCHIVE) \
	    Makefile \
	    README.txt \
	    Doxyfile \
	    src/ \
	    examples/ \
	    design.pdf
	@echo "Archív vytvorený: $(ARCHIVE)"
	@echo "Veľkosť: $$(du -sh $(ARCHIVE))"
