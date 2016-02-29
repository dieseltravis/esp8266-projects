-- main.lua

function pwmInit()
  print('pwmInit')

  gpio.mode(1, gpio.OUTPUT)
  gpio.write(1, gpio.LOW)
  gpio.mode(2, gpio.OUTPUT)
  gpio.write(2, gpio.LOW)

  gpio.mode(3, gpio.OUTPUT)
  gpio.write(3, gpio.HIGH)
  gpio.mode(4, gpio.OUTPUT)
  gpio.write(4, gpio.HIGH)

  pwm.setup(1, 1000, 1023)--PWM 1KHz, Duty 1023
  pwm.start(1)
  pwm.setduty(1, 0)

  pwm.setup(2, 1000, 1023)
  pwm.start(2)
  pwm.setduty(2, 0)
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

function carCommand (command, time)
  print('command:'..command)

  time = time or 500

  tmr.alarm(1, time, 0, function()
    if command == 'STOP' then
      pwm.setduty(1, 0)
      pwm.setduty(2, 0)
    elseif command == 'F' then
      gpio.write(3, gpio.HIGH)
      gpio.write(4, gpio.HIGH)
    elseif command == 'B' then
      gpio.write(3, gpio.LOW)
      gpio.write(4, gpio.LOW)
    elseif command == 'L' then
      gpio.write(3, gpio.LOW)
      gpio.write(4, gpio.HIGH)
    elseif command == 'R' then
      gpio.write(3, gpio.HIGH)
      gpio.write(4, gpio.LOW)
    else
      print("Invalid Command:"..command)
      pwm.setduty(1, 0)
      pwm.setduty(2, 0)
      tmr.stop(1)
    end
  end)

  return
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

        carCommand(cmd, 250)

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
