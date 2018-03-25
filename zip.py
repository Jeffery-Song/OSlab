#!/usr/bin/python3
import zipfile
formatlist=set(['.c','.s','.ld','.h','.html','.png'])
import os
files=os.listdir('.')
number={
    '1':'一',
    '2':'二',
    '3':'三',
    '4':'四',
    '5':'五',
    '6':'六',
    '7':'七',
    '8':'八'
}
myOS=os.listdir('myOS')
userApp=os.listdir('userApp')
zipfilename='实验报告'+number[os.getcwd()[-1]]+'_宋小牛_PB15000301.zip'
try:
    fp=zipfile.ZipFile(zipfilename,'x',zipfile.ZIP_DEFLATED)
except FileExistsError:
    os.remove(zipfilename)
    fp=zipfile.ZipFile(zipfilename,'x',zipfile.ZIP_DEFLATED)
for file in files:
    if os.path.splitext(file)[1] in formatlist:#os.path.splitext(file) split file into name and format
        fp.write(file)
for file in myOS:
    fp.write('myOS/'+file)
for file in userApp:
    fp.write('userApp/'+file)
#fp.write('myOS/start32.s')
#fp.write('userApp')
fp.write('Makefile')
fp.close()
