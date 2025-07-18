#!/usr/bin/env python3
import os
import datetime

print("Content-Type: text/html\n")
print("<!DOCTYPE html><html><head><title>Python CGI</title></head><body>")
print("<h1>Hello from Python CGI!</h1>")
print(f"<p>Date: {datetime.datetime.now()}</p>")
print(f"<p>Method: {os.environ.get('REQUEST_METHOD', 'Unknown')}</p>")
print(f"<p>Query: {os.environ.get('QUERY_STRING', 'None')}</p>")
print("</body></html>")