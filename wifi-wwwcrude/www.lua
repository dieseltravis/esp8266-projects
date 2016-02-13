-- www.lua
www = net.createServer(net.TCP)

www:listen(80, function(sock)
  sock:on("receive", function(client, request)
    print(request)
    
    sock:send("HTTP/1.1 200 OK\n"..
"Server: ESP8266 Webserver\n"..
"Connection: close\n"..
"Content-Type: text/html\n"..
"\n")

    sock:send("<!doctype html>\n"..
"<html lang='en'>"..
"    <head>"..
"        <meta charset='utf-8' />"..
"        <title>ESP8266</title>"..
"        <meta name='viewport' content='width=device-width, initial-scale=1' />"..
"        <link rel='icon' href='data:image/png;base64,"..
"iVBORw0KGgoAAAANSUhEUgAAABAAAAAQCAYAAAAf8/9hAAAAS0lEQVR42s2SMQ4AIAjE+P+ncSYdasgNXMJgcyIIlVKPIKdvioAXyWBeJmVpqRZKWtj9QWAKZyWll50b8IcL9JUeQF50n28ckyb0ADG8RLwp05YBAAAAAElFTkSuQmCC"..
"' type='image/x-png' />".. 
"    </head>"..
"    <body>"..
"        <h1>Hello world!</h1>"..
"        <p>node.heap() == "..node.heap().."</p>"..
"        <p>node.chipid() == "..node.chipid().."</p>"..
"        <ul>"..
"            <li><a href='http://www.lua.org/manual/5.3/'>Lua Manual</a></li>"..
"            <li><a href='http://www.nodemcu.com/docs/index/'>NodeMCU Docs</a></li>"..
"        </ul>"..
"    </body>"..
"</html>\n"..
"")
    
    client:close()
    collectgarbage()
        
  end)
end)
