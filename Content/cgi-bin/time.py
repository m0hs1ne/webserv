#!/usr/bin/python3

import datetime
import cgi

print("Content-type: text/html\r")
print("\r")
print("<html>")
print("<head>")
print(datetime.datetime.strftime(datetime.datetime.now(), "<h1>  %H:%M:%S </h1>"))