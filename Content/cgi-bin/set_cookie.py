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
value  = form.getvalue('value')
cookie = cookies.SimpleCookie()
cookie[key] = value
print("Content-Type: text/plain\r")
print(cookie.output() + "\r")
print("\r")
print("Cookie was set !")