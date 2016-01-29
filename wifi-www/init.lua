print("\n")
print("ESP8266 Started.")

majorVer, minorVer, devVer, chipid, flashid, flashsize, flashmode, flashspeed = node.info();
print("NodeMCU "..majorVer.."."..minorVer.."."..devVer)
print("Chip "..chipid)
print("Flash "..flashid.." size:"..flashsize.." mode:"..flashmode.." speed:"..flashspeed)

print("Configuring wifi...")
cfg = {
    ssid = "WifiName",
    password = "wifipassword"
}

wifi.setmode(wifi.STATION)
wifi.sta.config(cfg.ssid, cfg.password)
ip = wifi.sta.getip()

print(ip)

print("Compiling...")
--TODO: test for www.lc file first?
node.compile("www.lua")

print("Running...")
dofile("www.lc")

print("Open this URL in a browser: ")
print("http://"..ip.."/")

--TODO: blink LED