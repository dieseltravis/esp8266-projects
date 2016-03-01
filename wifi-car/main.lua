-- main.lua

--1,2EN 	D1 GPIO5
--3,4EN 	D2 GPIO4
--1A  ~2A   D3 GPIO0
--3A  ~4A   D4 GPIO2

-- this pin config doesn't work
L1 = 1
R1 = 2
L2 = 3
R2 = 4
A1 = 1
A2 = 2

stopFlag = true
spdTargetA = 1023
spdCurrentA = 0
spdTargetB = 1023
spdCurrentB = 0

function pwmInit()
  print('gpio & pwm init...')
  
  --LED Light on
  --gpio.mode(0,gpio.OUTPUT)
  --gpio.write(0,gpio.LOW)

  gpio.mode(L1, gpio.OUTPUT)
  gpio.write(L1, gpio.LOW)
  gpio.mode(R1, gpio.OUTPUT)
  gpio.write(R1, gpio.LOW)

  gpio.mode(L2, gpio.OUTPUT)
  gpio.write(L2, gpio.HIGH)
  gpio.mode(R2, gpio.OUTPUT)
  gpio.write(R2, gpio.HIGH)

  --PWM 1KHz, Duty 1023
  pwm.setup(A1, 1000, 1023)
  pwm.start(A1)
  pwm.setduty(A1, 0)

  pwm.setup(A2, 1000, 1023)
  pwm.start(A2)
  pwm.setduty(A2, 0)

  tmr.alarm(1, 200, 1, function()
    if stopFlag == false then
      spdCurrentA = spdTargetA
      spdCurrentB = spdTargetB
      pwm.setduty(A1, spdCurrentA)
      pwm.setduty(A2, spdCurrentB)
    else
      pwm.setduty(A1, 0)
      pwm.setduty(A2, 0)
    end
  end)
end

function carCommand (command)
  print('command:'..command)

  if command == 'STOP' then
    pwm.setduty(A1, 0)
    pwm.setduty(A2, 0)
    stopFlag = true
  elseif command == 'F' then
    gpio.write(L2, gpio.HIGH)
    gpio.write(R2, gpio.HIGH)
    stopFlag = false
  elseif command == 'B' then
    gpio.write(L2, gpio.LOW)
    gpio.write(R2, gpio.LOW)
    stopFlag = false
  elseif command == 'L' then
    gpio.write(L2, gpio.LOW)
    gpio.write(R2, gpio.HIGH)
    stopFlag = false
  elseif command == 'R' then
    gpio.write(L2, gpio.HIGH)
    gpio.write(R2, gpio.LOW)
    stopFlag = false
  elseif command == 'FL' then
    gpio.write(L2, gpio.HIGH)
    gpio.write(R2, gpio.HIGH)
    stopFlag = false
  elseif command == 'FR' then
    gpio.write(L2, gpio.HIGH)
    gpio.write(R2, gpio.HIGH)
    stopFlag = false
  elseif command == 'BL' then
    gpio.write(L2, gpio.LOW)
    gpio.write(R2, gpio.LOW)
    stopFlag = false
  elseif command == 'BR' then
    gpio.write(L2, gpio.LOW)
    gpio.write(R2, gpio.LOW)
    stopFlag = false
  else
    print("Invalid Command:"..command)
    pwm.setduty(A1, 0)
    pwm.setduty(A2, 0)
    stopFlag = true
  end
end


function initWifi(callback)
  -- set timer to wait for an IP address
  tmr.alarm(0, 1000, 1, function()
    print("wifi status: "..wifi.sta.status())
    cfg.ip = wifi.sta.getip()
    if cfg.ip == nil then
      print("connecting...")
    else
      -- IP isn't nil, stop timer
      tmr.stop(0)
      print('ip: ', cfg.ip)
      callback()
    end

  end)
end


function webServer()
  HTTP_HEADERS = "Server: ESP8266 Webserver\n"..
    "Connection: close\n"..
    "Access-Control-Allow-Origin: *\n"
  PORT = 80
  TIMEOUT_SECONDS = 10
  CHUNK_SIZE = 256
  
  index = 0
  filename = nil


  if www then www:close() www = nil end
  www = net.createServer(net.TCP, TIMEOUT_SECONDS)

  www:listen(PORT, function(sock)
    sock:on("receive", function(client, request)
      print(request)

      if request == nil then
        return
      end

      --TODO: use match instead of substring
      local method = string.sub(request, 1, 6)
      filename = nil

      if method == "GET / " then
        client:send("HTTP/1.1 200 OK\n"..
          HTTP_HEADERS..
          "Content-Type: text/html\n"..
          "\n")

        filename = "index.html"
        index = 0
      elseif method == "POST /" then
        local _, _, cmd = string.find(request, "cmd=(%a+)")
        print("cmd:"..cmd)

        carCommand(cmd)

        -- return JSON
        client:send("HTTP/1.1 200 OK\n"..
          HTTP_HEADERS..
          "Content-Type: application/json\n\n"..
          '{"OK":true,"cmd":"'..cmd..'"}\n')
        client:close()
      else
        -- error
        client:send("HTTP/1.1 501 Not Implemented\n"..
          HTTP_HEADERS)
        client:close()
      end

      collectgarbage()
    end)

    sock:on("sent", function (conn)
      if filename and file.open(filename, "r") then

        file.seek("set", index)
        local chunk=file.read(CHUNK_SIZE)
        file.close()

        if chunk then
          index = index + string.len(chunk)
          conn:send(chunk)
        else
          conn:close();
        end

        collectgarbage()
      end
    end)
  end)
end



--TODO: use this somewhere
--function blinker(blink_count)
--  local x = 1
--  local counter = 0
--  blink_count = blink_count or 100
--  tmr.alarm(2, (1000 / 30), 1, function()
--    if x == 1 then
--      gpio.write(0,gpio.LOW)
--      x = x + 1
--    else
--      gpio.write(0,gpio.HIGH)
--      x = x - 1
--    end
--    if counter == blink_count then
--      gpio.write(0,gpio.HIGH)
--      tmr.stop(2)
--    end
--    counter = counter + 1
--  end)
--end
