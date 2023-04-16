#!/usr/bin/python3

import cgi, os, sys

form = cgi.FieldStorage()

# Get filename here
fileitem = form['filename']

print("fileitem.filename: " + fileitem.filename, file=sys.stderr)
# Test if the file was uploaded
if fileitem.filename:
   open(os.getcwd() + '/Content/cgi-bin/tmp/' + os.path.basename(fileitem.filename), 'wb').write(fileitem.file.read())
   message = 'The file "' + os.path.basename(fileitem.filename) + '" was uploaded to ' + os.getcwd() + '/cgi-bin/tmp'
else:
   message = 'Uploading Failed'

print("Content-Type: text/html;charset=utf-8\r")
print("Content-type:text/html\r")
print("\r")
print("<H1> " + message + " </H1>")