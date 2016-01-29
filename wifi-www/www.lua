-- www.lua
www = net.createServer(net.TCP)

www:listen(80, function(sock)
  sock:on("receive", function(client, request)
    print(request)
    
    local f = file.open("index.html", "r")
    if f ~= nil then
    conn:send([[
HTTP/1.1 200 OK
Content-Type:text/html
Connection: close

]])
      client:send(file.read())
      file.close()
    else
      -- send 404
       conn:send([[
HTTP/1.1 404 File not found
Content-Type:text/html
Connection: close

]])
      client:send("<!doctype html>")
      client:send("<html lang='en'><head><title>404</title></head><body><h1>404</h1>")
      client:send("<p><a href='/'>/</a></p></body></html>")
    end
    
    client:close()
    collectgarbage()
    f = nil
    url = nil
        
  end)
end)