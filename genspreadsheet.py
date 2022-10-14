#!/usr/bin/python3
import xlsxwriter
from datetime import datetime
import sys
import re

if len(sys.argv)<2:
    print("Usage: genspreadsheet.py <results.txt>\n")
    sys.exit(0)

infilename = sys.argv[1]
results = {}
scanners = set()
with open( infilename, "r" ) as filein:
    headers = filein.readline().split(';')
    headmap = {}
    for index,name in enumerate(headers):
        match = re.match('(.*\s)*\[ms\]',name)
        if match:
            regexname = match.groups(1)[0].strip()
            headmap[regexname] = index
            scanners.add(regexname)
    for line in filein:
        values = line.strip().split(';')
        regex = values[0].strip()
        results[regex] = dict( [ (name,float(values[index])) for (name,index) in headmap.items() ] )

nowstr = datetime.now().strftime( "%Y%m%d-%H%M%S" )
outfilename = "regex-results-%s.xlsx" % (nowstr,)

# Create a workbook and add a worksheet.
workbook = xlsxwriter.Workbook(outfilename)
worksheet = workbook.add_worksheet()
worksheet.hide_gridlines(2)
worksheet.set_column(0,0,30)
worksheet.set_column(1,len(scanners),10)
worksheet.set_row(0,20)

# Add a bold format to use to highlight cells.
headerfmt = workbook.add_format({'bold': True})
headerfmt.set_bg_color('black')
headerfmt.set_font_color('white')
headerfmt.set_rotation(0)
highfmt = workbook.add_format({'bold': True})
highfmt.set_bg_color( 'orange' )
highfmt.set_font_color( 'white' )
lowfmt = workbook.add_format({'bold': True})
lowfmt.set_bg_color( 'blue' )
lowfmt.set_font_color( 'white' )
warnfmt = workbook.add_format({'bold':False})
warnfmt.set_bg_color( 'yellow' )
warnfmt.set_font_color( 'black' )
warnfmt.set_align('center')

# Write headers.
scanners = list(scanners)
row = 0
for col,scanner in enumerate(scanners):
    worksheet.write( row, col+1, scanner, headerfmt )
worksheet.write( row, 0, "Regex", headerfmt )

for regex,stats in results.items():
    values = sorted([ ms for ms in stats.values() ])
    lowcut = values[1]
    highcut = values[-2]
    row += 1
    worksheet.write( row, 0, regex, headerfmt )
    for col,scanner in enumerate(scanners):
        if scanner not in stats:
            worksheet.write( row, col+1, "n/a", warnfmt )
            continue
        ms = stats[scanner]
        if ms>=999999 or ms<=0:
            worksheet.write( row, col+1, "n/a", warnfmt )
            continue
        fmt = None
        if ms <= lowcut:
            fmt = lowfmt
        if ms >= highcut:
            fmt = highfmt
        worksheet.write( row, col+1, ms, fmt)

workbook.close()
print("Wrote spreadsheet",outfilename)
