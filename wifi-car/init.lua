-- init...

print ("\n")
print ("ESP8266 Starting...")

cfg={
	wifi= "wifi",
	wifipass="pass",
	ip="127.0.0.1"  -- this will get set once connected to wifi
}

-- compile and execute main code
print("Compiling main...")
node.compile("main.lua")

print("Running main...")
dofile("main.lc")

print("Connecting to wifi...")
initWifi(function ()
  print("Open this URL in a browser: ")
  print("http://"..cfg.ip.."/")

  pwmInit()
  carCommand('STOP')

  webServer()
end)
