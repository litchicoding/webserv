#!/usr/bin/env python3
import datetime

now = datetime.datetime.now()

print("Content-Type: text/html\n")
print("<!DOCTYPE html>")
print("<html lang='fr'>")
print("<head><meta charset='UTF-8'><title>Python Test CGI</title></head>")
print("<body>")
print("<h1>Test Python</h1>")
print(f"<p>Date : {now.strftime('%Y-%m-%d')}</p>")
print(f"<p>Heure : {now.strftime('%H:%M')}</p>")
print("</body></html>")
