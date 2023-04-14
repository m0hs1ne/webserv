from http import cookies
import os, sys

def show_cookie(c):
    for a, morsel in c.items():
        line = a + ':'  + morsel.value + '<br>'
        print("\r")
        print(line + "\r")

cookie = cookies.BaseCookie()
print("Content-type: text/html\r")
if 'HTTP_COOKIE' in os.environ: 
    cookie.load(os.environ["HTTP_COOKIE"])
    show_cookie(cookie)
else:
    print("\r")
    print("No Cookies Set Yet!")