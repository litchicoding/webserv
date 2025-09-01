
- launch the installation of siege with homebrew
- Explain the basics of an HTTP server.
- Ask which function they used for I/O Multiplexing.
- Ask to get an explanation of how select (or equivalent) is working.
- Ask if they use only one select (or equivalent) and how they've managed the server accept and the client
read/write.
- **the select (or equivalent) should be in the main loop and should check fd for read and write AT THE SAME TIME**, if not please give a 0 and stop the evaluation.
- **There should be only one read or one write per client per select** (or equivalent). Ask to show you the code that goes from the select (or equivalent) to the read and write of a client.
- Search for all read/recv/write/send on a socket and check that if **an error returned the client is removed**
- Search for all read/recv/write/send and check if the returned value is well checked. (checking only -1 or 0 is not good you should check both)
- **If a check of errno is done after read/recv/write/send. Please stop the evaluation and put a mark to 0**
- **Writing or reading ANY file descriptor** without going through the select (or equivalent) is strictly FORBIDDEN
- The project must compile without any re-link issue if not use the compile flag.
- If any point is unclear or is not correct use the flag for incomplete work

In the configuration file check if you can do the following and test the result:
- look for the HTTP response status codes list on internet and during this evaluation
if any status codes is wrong don't give related points.
- setup multiple servers with different port
- setup multiple servers with different hostname (use something like: curl --resolve example.com:80:127.0.0.1
http://example.com/)
- setup default error page (try to change the error 404)
- limit the client body (use curl -X POST -H "Content-Type: plain/text" --data "BODY IS HERE write something shorter or longer than body limit")
- setup routes in a server to different directories
- setup a default file to search for if you ask for a directory
- setup a list of method accepted for a certain route (ex: try to delete something with and without permission)
- TEST PARSING


Using telnet, curl, prepared files demonstrates that the following features work properly:
- GET requests -> should work
- POST requests -> should work
- DELETE requests -> should work
- UNKNOWN requests -> should not produce any crash
- For every test the status code must be good
- upload some file to the server and **get it back**
- Use the reference browser of the team, open the network part of it and try to connect to the server with it
- Look at the request header and response header
- It should be compatible to serve a fully static website
- Try a wrong URL on the server
- Try to list a directory
- Try a redirected URL
- Try things

- In the configuration file setup multiple ports and use different websites, use the browser to check that the
configuration is working as expected, and show the right website.
- In the configuration try to setup the same port multiple times. It should not work.
- Launch multiple servers at the same time with different configurations but with common ports. Is it working? If it is
working, ask why the server should work if one of the configurations isn't working. keep going

- Use Siege to run some stress tests.
- Availability should be above 99.5% for a simple get on an empty page with a siege -b on that page
- Check if there is no memory leak (monitor the process memory usage it should not go up indefinitely)
- Check if there is no hanging connection
- You should be able to use siege indefinitely without restarting the server (look at siege -b)