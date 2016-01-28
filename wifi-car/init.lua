-- init...

print ("\n");
print ("ESP8266 Starting...");

cfg={
	wifi= "Hardiman",
	wifipass="wifipassword", 
	--device_id='DOIT-SN-851A-73814',
	--device_key='e14b22b5b29bceba4e8389f2a1182763',	
	--server_addr='s.doit.am',
	--server_port=8810,
	--apssid='Doit_ESP_'..node.chipid(),
};

local function pwmInit()
	print('pwmInit');

	gpio.mode(1, gpio.OUTPUT);
	gpio.write(1, gpio.LOW);
	gpio.mode(2, gpio.OUTPUT);
	gpio.write(2, gpio.LOW);
				 						   
	gpio.mode(3, gpio.OUTPUT);
	gpio.write(3, gpio.HIGH);
	gpio.mode(4, gpio.OUTPUT);
	gpio.write(4, gpio.HIGH);

	pwm.setup(1, 1000, 1023);--PWM 1KHz, Duty 1023
	pwm.start(1);
	pwm.setduty(1, 0);

	pwm.setup(2, 1000, 1023);
	pwm.start(2);
	pwm.setduty(2, 0);
end


function initWifi ()
	wifi.setmode(wifi.STATION);
	wifi.sta.config(cfg.wifi, cfgwifipass);
	ip = wifi.sta.getip();
end

function webServer()
	sckTcpSrv = net.createServer(net.TCP, 60);

	sckTcpSrv:listen(80, function (conn) 
		conn:on("receive", function (conn, request)
			print("TCPSrv:"..request);
			
			if request == nil then 
				return;
			end
			
			if string.find(request, "GET / HTTP/1.1") then
				conn:send([[
HTTP/1.1 200 OK
Content-Type:text/html
Connection: close

]]);
				fOffset = 0;

				-- send index file:
				conn:on("sent", function(conn)
					if fOffset == 65534 then
						conn:close();
						node.restart();
					elseif fOffset == 65535 then
						conn:close();
						conn = nil;
						collectgarbage();
					else
						if file.open("index.html", "r") then
							file.seek("set", fOffset);
							local chunk = file.read(1024);
							file.close();
							if chunk then
								conn:send(chunk);
							end
							chunk = nil;
							fOffset = fOffset + 1024;
							if fOffset > fz then
								fOffset = 65535;
							end
						end
					end
					collectgarbage();
				end);

			elseif string.find(request, "POST /submit") then
				-- TODO: process command
				--local commandStart, commandEnd = string.find(request, "command=");
				--local command = string.sub(request, commandEnd);
				--print(command);

				local _, _, command2 = string.find(request, "command=(%a+)");
				print(command2);

				conn:send([[
					{
						"OK": "OK"
					}
				]]);

				carCommand(command2, 250);
			else
				conn:send([[
HTTP/1.1 404 Not Found
Content-Type: text/html
Content-Length: 9
Connection: close

¯\_(ツ)_/¯
]]);
			end
			collectgarbage();
		end);--end c:on receive
	end);
end



function carCommand (command, time)
	print('command:'..command);

	time = time or 500;

	tmr.alarm(1, time, 0, function() 
		if command == 'stop' then
			pwm.setduty(1, 0);
			pwm.setduty(2, 0);
		elseif command == 'forward' then
			gpio.write(3, gpio.HIGH);
			gpio.write(4, gpio.HIGH);
		elseif command == 'backward' then
			gpio.write(3, gpio.LOW);
			gpio.write(4, gpio.LOW);
		elseif command == 'left' then
			gpio.write(3, gpio.LOW);
			gpio.write(4, gpio.HIGH);
		elseif command == 'right' then
			gpio.write(3, gpio.HIGH);
			gpio.write(4, gpio.LOW);
		else 
			print("Invalid Command:"..command);
			pwm.setduty(1, 0);
			pwm.setduty(2, 0);
			tmr.stop(0); 
		end
	end );

	return;
end

function blinker()
	local x = 1;
	local counter = 0;
	tmr.alarm(0, (1000 / 30), 1, function() 
		if x == 1 then
			gpio.write(0,gpio.LOW);
			x = x + 1;
		else
			gpio.write(0,gpio.HIGH);
			x = x - 1;
		end
		if counter == 100 then
			gpio.write(0,gpio.HIGH);
			tmr.stop(0) 
		end
		counter = counter + 1;
	end );
end
