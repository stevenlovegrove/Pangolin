#!/bin/bash

FONT_FILENAME=$1

if ! command -v fc-query &> /dev/null
then
    echo "First install fontconfig for fc-query tool"
    exit
fi

fc-query --format='%{charset}\n' "$FONT_FILENAME" > "${FONT_FILENAME}_fcquery.txt"
sed 's/\([0-9a-fA-F]*\)/0x\1/g' "${FONT_FILENAME}_fcquery.txt" | sed 's/\(0x[0-9a-fA-F]*\)-\(0x[0-9a-fA-F]*\)/\[\1, \2\]/g' > "${FONT_FILENAME}_charset.txt"
./bin/msdf-atlas-gen -font "$FONT_FILENAME" -charset "${FONT_FILENAME}_charset.txt" -size 32 -yorigin top -imageout "${FONT_FILENAME}_map.png" -kerning -json "${FONT_FILENAME}_map.json"
