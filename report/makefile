all:
	pdflatex --output-directory /tmp report.tex  && \
	cp report.bib /tmp && \
	cd /tmp  && \
	bibtex report  && \
	cd -  && \
	pdflatex --output-directory /tmp report.tex  && \
	pdflatex --output-directory /tmp report.tex  && \
	evince /tmp/report.pdf
