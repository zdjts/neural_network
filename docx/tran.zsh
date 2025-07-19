#!/bin/zsh

for file in *.doc; do libreoffice --headless --convert-to docx "$file"; done

for file in *.docx; do pandoc "$file" -o "${file%.docx}.tex"; done
