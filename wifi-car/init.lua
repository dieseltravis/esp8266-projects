-- init...

print ("\n")
print ("ESP8266 Starting...")

cfg={
	wifi= "WifiName",
	wifipass="wifipassword", 
	ip="127.0.0.1",
	wwwtimeout=60,  -- seconds
	wwwport=80
}

local function pwmInit()
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

function webServer()
	sckTcpSrv = net.createServer(net.TCP, cfg.wwwtimeout)

	sckTcpSrv:listen(cfg.wwwport, function (conn) 
		conn:on("receive", function (conn, request)
			print("TCPSrv:"..request)
			
			if request == nil then 
				return
			end
			
			if string.find(request, "GET / HTTP/1.") then
				conn:send("HTTP/1.1 200 OK\n"..
          "Content-Type:text/html\n"..
          "Connection: close\n"..
          "\n")
				fOffset = 0

				-- send index file:
				conn:on("sent", function(conn)
					if fOffset == 65534 then
						conn:close()
						node.restart()
					elseif fOffset == 65535 then
						conn:close()
						conn = nil
						collectgarbage()
					else
						if file.open("index.html", "r") then
							file.seek("set", fOffset)
							local chunk = file.read(1024)
							file.close()
							if chunk then
								conn:send(chunk)
							end
							chunk = nil
							fOffset = fOffset + 1024
							if fOffset > fz then
								fOffset = 65535
							end
						end
					end
					collectgarbage()
				end)

			elseif string.find(request, "POST /submit") then
				-- TODO: process command
				--local commandStart, commandEnd = string.find(request, "command=")
				--local command = string.sub(request, commandEnd)
				--print(command)

				local _, _, command2 = string.find(request, "command=(%a+)")
				print(command2)

				conn:send("{\n"..
					'"OK": "OK"\n'..
					"}\n")

				carCommand(command2, 250)
			else
				conn:send("HTTP/1.1 404 Not Found\n"..
          "Content-Type: text/html\n"..
          "Connection: close\n"..
          "\n"..
          "404\n")
			end
			collectgarbage()
		end)  --end c:on receive
	end)
end

function carCommand (command, time)
	print('command:'..command)

	time = time or 500

	tmr.alarm(1, time, 0, function() 
		if command == 'stop' then
			pwm.setduty(1, 0)
			pwm.setduty(2, 0)
		elseif command == 'forward' then
			gpio.write(3, gpio.HIGH)
			gpio.write(4, gpio.HIGH)
		elseif command == 'backward' then
			gpio.write(3, gpio.LOW)
			gpio.write(4, gpio.LOW)
		elseif command == 'left' then
			gpio.write(3, gpio.LOW)
			gpio.write(4, gpio.HIGH)
		elseif command == 'right' then
			gpio.write(3, gpio.HIGH)
			gpio.write(4, gpio.LOW)
		else 
			print("Invalid Command:"..command)
			pwm.setduty(1, 0)
			pwm.setduty(2, 0)
			tmr.stop(0) 
		end
	end)

	return
end

function blinker()
	local x = 1
	local counter = 0
	tmr.alarm(0, (1000 / 30), 1, function() 
		if x == 1 then
			gpio.write(0,gpio.LOW)
			x = x + 1
		else
			gpio.write(0,gpio.HIGH)
			x = x - 1
		end
		if counter == 100 then
			gpio.write(0,gpio.HIGH)
			tmr.stop(0) 
		end
		counter = counter + 1
	end )
end

-- compile and execute main code
--print("Compiling...")
--node.compile("main.lua")

--print("Running...")
--dofile("main.lc")

initWifi(function ()
    print("Open this URL in a browser: ")
    print("http://"..cfg.ip.."/")
    webserver()
  end)
