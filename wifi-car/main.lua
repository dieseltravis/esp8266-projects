-- main.lua

HTTP_HEADERS = "Server: ESP8266 Webserver\n"..
"Connection: close\n"..
"Access-Control-Allow-Origin: *\n"
PORT = 80
RELAY_PIN = 2
TIMEOUT_SECONDS = 60

www = net.createServer(net.TCP, TIMEOUT_SECONDS)

www:listen(PORT, function(sock)
  sock:on("receive", function(client, request)
    print(request)
    
    --TODO: use match instead of substring
    local method = string.sub(request, 1, 4)
    
    -- get debugging info?
    
    if method == "GET " then
      sock:send("HTTP/1.1 200 OK\n"..
        HTTP_HEADERS..
        "Content-Type: text/html\n"..
        "\n")

      sock:send("<!doctype html>\n"..
"<html lang='en'>\n"..
"<head>\n"..
"<meta charset='utf-8' />\n"..
"<meta name='viewport' content='width=device-width' />\n"..
"<link rel='icon' href='data:image/png;base64,"..
"iVBORw0KGgoAAAANSUhEUgAAABAAAAAQCAYAAAAf8/9hAAAAS0lEQVR42s2SMQ4AIAjE+P+ncSYdasgNXMJgcyIIlVKPIKdvioAXyWBeJmVpqRZKWtj9QWAKZyWll50b8IcL9JUeQF50n28ckyb0ADG8RLwp05YBAAAAAElFTkSuQmCC"..
"' type='image/x-png' />\n"..
"<title>esp8266 car</title>\n"..
"<style type='text/css'>\n"..
"/* todo  */\n"..
"</style>\n"..
"</head>\n"..
"<body>\n"..
"  <form method='post'>\n"..
"		<button name='cmd' value='FL' title='forward to the left'>&#8598;</button>\n"..
"		<button name='cmd' value='F' title='forward'>&#8679;</button>\n"..
"		<button name='cmd' value='FR' title='forward to the right'>&#8599;</button>\n"..

"		<button name='cmd' value='L' title='rotate counter-clockwise'>&#8693;</button>\n"..
"		<button name='cmd' value='STOP' title='stop'>.</button>\n"..
"		<button name='cmd' value='R' title='rotate clockwise'>&#8645;</button>\n"..

"		<button name='cmd' value='BL' title='backward to the left'>&#8601;</button>\n"..
"		<button name='cmd' value='B' title='backward'>&#8681;</button>\n"..
"		<button name='cmd' value='BR' title='backward to the right'>&#8600;</button>\n"..
"	</form>\n"..
"	<script type='text/javascript'>\n"..
"	/* todo  */\n"..
"	</script>\n"..
"</body>\n"..
"</html>\n")

	  elseif method == "POST" then
      -- read cmd from data
      local postparse = {string.find(request, "cmd=")}
      --TODO: use match instead of substring
      local cmd = string.sub(request, postparse[2] + 1, #request)
      
      -- todo: cmd
      
      
      -- return JSON
      sock:send("HTTP/1.1 200 OK\n"..
        HTTP_HEADERS..
        "Content-Type: application/json\n"..
        "\n"..
        '{"OK":true,"cmd":"'..cmd..'"}\n')
    else
      -- error
      sock:send("HTTP/1.1 501 Not Implemented\n"..
        HTTP_HEADERS)
    end
  
    client:on("sent", function (conn) conn:close() end)
    collectgarbage()
  end)
end)
