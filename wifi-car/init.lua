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

-- compile and execute main code
print("Compiling main...")
node.compile("main.lua")

print("Running main...")
dofile("main.lc")
