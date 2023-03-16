

SPHINXOPTS    ?= 
SPHINXBUILD   ?= sphinx-build
SOURCEDIR     = ./docs
BUILDDIR      = ./build/$(1)
PYTHON 		= python3

# use shell pattern expansion to remove 'Tool' from R2DTool-Documentation
PUBLDIR = $(shell v="$(SIMDOC_APP)"; echo "../$${v%Tool}-Documentation/docs/")

# Directories to remove when cleaning
CLEANDIR      = _sources _static _images common

# This environment variable should specify a directory
# that contains both the SimCenterDocumentation/ and
# application source repositories.
export SIMCENTER_DEV = $(shell pwd | xargs dirname)

#-Help-----------------------------------------------------
help:
	@echo 'usage: make <target>'
	@echo '   or: make <all|update>'
	@printf '\n'
	@echo 'and <target> is one of:'
	@echo '    web    Run html target with build directory'
	@echo '           set to app publishing repository.'
	@echo '    html   Run html target in dev build directory.'
	@echo '    spell  Run spell checker.'
	@echo '    latex  Run latex target in dev build directory.'
	@printf "\nRunning 'make all' will run 'make <app> html'\n"
	@printf "for all <app> options listed above.\n\n"
#----------------------------------------------------------

.PHONY: help Makefile OpenSRA html pdf latexpdf latex

# Export target-specific environment vars
export SIMDOC_APP=OpenSRA

export SIMDOC_APP


# LaTeX path variables
export TEXINPUTS:=${SIMCENTER_DEV}/SimCenterDocumentation/latex//:./build/${SIMDOC_APP}/latex//:${TEXINPUTS}
#export TEXINPUTS:=/usr/share/texmf-dist//:${TEXINPUTS}
export BSTINPUTS:=../texmf//:${BSTINPUTS}


web:
	find . -type f -name "*.rst" -exec touch {} +
	@echo cleaning directories: $(addprefix $(PUBLDIR),$(CLEANDIR))
	rm -fr $(addprefix $(PUBLDIR),$(CLEANDIR))
	$(SPHINXBUILD) -b html "$(SOURCEDIR)" "$(PUBLDIR)" $(O)
	@$(SPHINXBUILD) -b html "$(SOURCEDIR)" "$(PUBLDIR)" $(O)
	@$(SPHINXBUILD) -b html "$(SOURCEDIR)" "$(PUBLDIR)" $(O)

spell:
	@$(SPHINXBUILD) -b spelling "$(SOURCEDIR)" "$(call BUILDDIR,$(SIMDOC_APP))/html" $(O)

html:
	@$(SPHINXBUILD) -b html "$(SOURCEDIR)" "$(call BUILDDIR,$(SIMDOC_APP))/html" $(O)


latex:
	@$(SPHINXBUILD) -b latex "$(SOURCEDIR)" "$(call BUILDDIR,$(SIMDOC_APP))/latex" $(O)


pdf:
	mkdir -p $(call BUILDDIR,$(SIMDOC_APP))/pdf/
	$(PDFLATEX) \
	-output-directory="$(call BUILDDIR,$(SIMDOC_APP))/pdf/" \
	$(join $(call BUILDDIR,$(SIMDOC_APP)),/latex/*.tex)

latexpdf:
	make latex
	make pdf

update:
	pip install -U -r requirements.txt

#examples:
#	make build/$(SIMDOC_APP)_Examples.json
#
#build/%.json: examples.yaml
#	$(PYTHON) scripts/index_examples.py $(SIMDOC_APP) \
#    | aurore -D- -B ../$(SIMDOC_APP)/Examples/ -C scripts/config.yml get \
#    > $(call BUILDDIR,$(SIMDOC_APP))_Examples.json
#
#
#$(CSVDIR)/%.csv: $(JSONDIR)/%.json ./scripts/json2csv.py
#	python3 ./scripts/json2csv.py \
#		-Eqfem $(SIMCENTER_DEV)/quoFEM/Examples/qfem*/src/input.json \
#		-Eeeuq $(SIMCENTER_DEV)/EE-UQ/Examples/eeuq-*/src/input.json \
#		-Eweuq -  \
#		-Epbdl $(SIMCENTER_DEV)/PBE/Examples/pbdl-*/src/input.json \
#		-Er2dt $(SIMCENTER_DEV)/R2DTool/Examples/E*/input.json \
#		-Ehydr - \
#		< '$<' > '$@'
#
#csv-debug: FORCE
#	for i in $(JSONDIR)/*.json; do \
#	    json_file="$${i##*/}"; \
#        echo $$json_file; \
#        python3 ./scripts/json2csv.py -v \
#            -Eqfem $(SIMCENTER_DEV)/quoFEM/Examples/qfem*/src/input.json \
#            -Eeeuq $(SIMCENTER_DEV)/EE-UQ/Examples/eeuq-*/src/input.json \
#            -Eweuq -  \
#            -Epbdl $(SIMCENTER_DEV)/PBE/Examples/pbdl-*/src/input.json \
#            -Er2dt $(SIMCENTER_DEV)/R2DTool/Examples/E*/input.json \
#            -Ehydr - \
#            < "$(JSONDIR)/$$json_file"; \
	done

FORCE:
.PHONY: csv-debug

