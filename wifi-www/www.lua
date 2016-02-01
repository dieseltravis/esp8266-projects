-- www.lua
www = net.createServer(net.TCP)

www:listen(80, function(sock)
  sock:on("receive", function(client, request)
    print(request)

	-- find requested file:
    local startPos = string.find(request, "GET /") + 5
    --TODO: see if this works:
    local _, startPos2 = string.find(request, "GET /")
    print(startPos2)
    local endPos = string.find(request, "HTTP/") - 2
    -- maybe use pattern matching instead: http://www.lua.org/pil/20.2.html
    -- example: https://github.com/EvandroLG/pegasus.lua/blob/master/src/pegasus/request.lua#L21-L25

    local url = string.sub(request, startPos, endPos)

    -- default file:
    if url == "" then
      url = "index.html"
    end

    local f = file.open(url, "r")
    if f ~= nil then
      sock:send("HTTP/1.1 200 OK\n"..
"Server: ESP8266 Webserver\n"..
"Connection: close\n"..
"Content-Type: text/html\n"..
"\n")

	  -- send whole file at once:
      sock:send(file.read())

	  -- TODO: send in chunks

	  -- TODO: send line by line

      file.close()
    else
      -- send 404
	  sock:send("HTTP/1.1 404 File not found\n"..
"Server: ESP8266 Webserver\n"..
"Connection: close\n"..
"Content-Type: text/html\n"..
"\n")

      sock:send("<!doctype html>")
      sock:send("<html lang='en'><head><title>404</title><link rel='icon' href='data:image/x-icon;,' type='image/x-icon' /></head><body><h1>404</h1>")
      sock:send("<p><a href='/'>/</a></p></body></html>")
    end

    client:close()
    collectgarbage()
    f = nil
    url = nil

  end)
end)
