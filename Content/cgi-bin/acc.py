#! /usr/bin/python3
from http import cookies
import os
import cgi
import time
import hashlib
import pickle
import sys

class Session:
    def __init__(self, name):
        self.name = name
        self.sid = hashlib.sha1(str(time.time()).encode("utf-8")).hexdigest()
        with open('Content/cgi-bin/sessions/session_' + self.sid, 'wb') as f:
            pickle.dump(self, f)
    def getSid(self):
        return self.sid

""" Stores Users and thier data  """
class UserDataBase:
    def __init__(self):
        self.user_pass = {}
        self.user_firstname = {}
    def addUser(self, username, password, firstname):
        self.user_pass[username] = password
        self.user_firstname[username] = firstname
        with open('Content/cgi-bin/user_database', 'wb') as f:
            pickle.dump(self, f)


def printAccPage(session):
    print("Content-type: text/html\r")
    print("\r")
    print("<html>")
    print("<head>")
    print("<title>Account Page</title>")
    print("</head>")
    print("<body>")
    print("<h1>Welcome Again", session.name, "!</h1>")
    print("<p>Your Session ID is: ", session.getSid(), "</p>")
    print("</body>")
    print("<button id=\"logout\"> Click here to go back to login page </button>")
    print("<script src=\"/assets/js/logout.js\"></script>")
    print("</html>")

def printUserMsg(msg):
    print("Content-type: text/html\r")
    print("\r")
    print("<html>")
    print("<head>")
    print("<title>USER MSG</title>")
    print("</head>")
    print("<body>")
    print("<h1>", msg ,"</h1>")
    print("</body>")
    print("<a href=\"/cgi-bin/acc.py\"> Click here to go back to login page </a>")
    print("</html>")

def printLogin():
    print("Content-Type: text/html\r")
    print("\r")
    print("<html> ")
    print("<head>")
    print("<meta charset=\"UTF-8\" name=\"viewport\" content=\"width=device-width, initial-scale=1\">")
    print("<link rel=\"stylesheet\" href=\"/assets/css/accstyle.css\">")
    print("<title> Login Page </title>")
    print("</head>")
    print("<body>  ")
    print("<center> <h1> Webserv Login Form </h1> </center> ")
    print("<form action = \"/cgi-bin/acc.py\" method = \"get\">")
    print("<div class=\"container\"> ")
    print("<label>Username : </label> ")
    print("<input type=\"text\" placeholder=\"Enter Username\" name=\"username\" required>")
    print("<label>Password : </label> ")
    print("<input type=\"password\" placeholder=\"Enter Password\" name=\"password\" required>")
    print("<button type=\"submit\">Login</button> ")
    print("No Account?<a href=\"/register.html\"> Register Here </a> ")
    print("</div> ")
    print("</form>   ")
    print("</body>   ")
    print("</html>")




def authUser(name, password):
    if os.path.exists('Content/cgi-bin/user_database'):
        with open('Content/cgi-bin/user_database', 'rb') as f:
            database = pickle.load(f)
            if name in database.user_pass and database.user_pass[name] == password:
                session = Session(database.user_firstname[name])
                return session
            else:
                return None
    else:
        return None

def handleLogin(cookie):
    username = form.getvalue('username')
    password = form.getvalue('password')
    firstname = form.getvalue('firstname')
    if username == None:
        printLogin()
    elif firstname == None:
        session = authUser(form.getvalue('username'), form.getvalue('password'))
        if(session == None):
            printUserMsg("Failed To Login, Username or Passowrd is wrong!")
        else:
            cookie.clear()
            cookie["SID"] = session.getSid()
            cookie["SID"]["SameSite"] = "None"
            cookie["SID"]["expires"] = 120 # Session Expires after 2 mins
            print(cookie.output() + "\r")
            print("location: /cgi-bin/acc.py\r")
            printAccPage(session)
    else :
        if os.path.exists('Content/cgi-bin/user_database'):
            with open('Content/cgi-bin/user_database', 'rb') as f:
                database = pickle.load(f)
                if username in database.user_pass:
                    printUserMsg("Username is already Registerd !")
                else:
                    database.addUser(username, password, firstname)
                    printUserMsg("Account Registerd Successfully!")
        else:
            database = UserDataBase()
            if username in database.user_pass:
                printUserMsg("Username is already Registerd !")
            else:
                database.addUser(username, password, firstname)
                printUserMsg("Account Registerd Successfully!")

form = cgi.FieldStorage()
cookie = cookies.SimpleCookie()
if 'HTTP_COOKIE' in os.environ: 
    cookie.load(os.environ["HTTP_COOKIE"])
    if "SID" in cookie:
        print("Your Session ID is", cookie["SID"].value,file=sys.stderr)
        with open('Content/cgi-bin/sessions/session_'+ cookie["SID"].value, 'rb') as f:
            sess = pickle.load(f)
        printAccPage(sess)
    else:
        handleLogin(cookie)
else:
    handleLogin(cookie)
