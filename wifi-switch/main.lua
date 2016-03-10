-- main.lua

HTTP_HEADERS = "Server: ESP8266 Webserver\n"..
"Connection: close\n"..
"Access-Control-Allow-Origin: *\n"
PORT = 80
-- This is the pin for the WeMos relay module
RELAY_PIN = 1
TIMEOUT_SECONDS = 10
CHUNK_SIZE = 256

www = net.createServer(net.TCP, TIMEOUT_SECONDS)
isOn = 0

-- turn pin off
gpio.mode(RELAY_PIN, gpio.OUTPUT)
gpio.write(RELAY_PIN, gpio.LOW)

index = 0
filename = nil

www:listen(PORT, function(sock)
  sock:on("receive", function(conn, request)
    print(request)
    index = 0
    filename = nil
    
    --TODO: use match instead of substring
    local method = string.sub(request, 1, 4)
    
    -- get debugging info?
    
    -- determine selected ON/OFF
    local offSelected = ""
    local onSelected = ""
    if (isOn == 0) then
    	offSelected = "checked"
    elseif (isOn == 1) then
    	onSelected = "checked"
    end
    
    if method == "GET " then
      conn:send("HTTP/1.1 200 OK\n"..
        HTTP_HEADERS..
        "Content-Type: text/html\n"..
        "\n")
      filename = "index.html"
	  elseif method == "POST" then
      -- read toggle from data
      local postparse = {string.find(request, "toggle=")}
      --TODO: use match instead of substring
      local toggle = string.sub(request, postparse[2] + 1, #request)
      
      -- set pin value based on toggle
      if toggle == "on" then
        isOn = 1
        -- turn pin on
        gpio.write(RELAY_PIN, gpio.HIGH);
      elseif toggle == "off" then
        isOn = 0
        -- turn pin off
        gpio.write(RELAY_PIN, gpio.LOW);
      end
      
      -- return JSON
      conn:send("HTTP/1.1 200 OK\n"..
        HTTP_HEADERS..
        "Content-Type: application/json\n"..
        "\n"..
        '{"OK":true,"toggle":"'..toggle..'"}\n')
    else
      -- error
      conn:send("HTTP/1.1 501 Not Implemented\n"..
        HTTP_HEADERS)
    end
    collectgarbage()
  end)
  
  sock:on("sent", function (conn)
    if filename and file.open(filename, "r") then
      file.seek("set", index)
      local chunk = file.read(CHUNK_SIZE)
      file.close()

      if chunk then
        index = index + string.len(chunk)
        conn:send(chunk)
      else
        conn:close()
      end
    end
    collectgarbage()
  end)
end)
