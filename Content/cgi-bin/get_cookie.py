#! /usr/bin/python3

import os
import sys
from http import cookies
# Import modules for CGI handling 
import cgi, cgitb 
import base64

# Create instance of FieldStorage 
form = cgi.FieldStorage() 

# Get data from fields
key = form.getvalue('key')
cookie = cookies.SimpleCookie()
if 'HTTP_COOKIE' in os.environ:
    cookie.load(os.environ["HTTP_COOKIE"])
if key in cookie:
    line = "The Value of Cookie " + key + " is " + cookie[key].value + "\r"
    length = len(line)
    print("Content-Type: text/plain\r")
    print("\r")
    print(line)
else:
    print("Content-Type: text/plain\r")
    print("\r")
    print("Cookie was not found !")