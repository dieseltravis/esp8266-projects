print("\n")
print("ESP8266 Started.")

-- print out some debugging info:
majorVer, minorVer, devVer, chipid, flashid, flashsize, flashmode, flashspeed = node.info();
print("NodeMCU "..majorVer.."."..minorVer.."."..devVer)
print("Chip "..chipid)
print("Flash "..flashid.." size:"..flashsize.." mode:"..flashmode.." speed:"..flashspeed)

print("Configuring wifi...")
-- set the ssid and password to the values for the network that you are connecting to
cfg = {
    ssid = "WifiName",
    password = "WifiPassword"
}

wifi.setmode(wifi.STATION)
wifi.sta.config(cfg.ssid, cfg.password)
ip = nil

-- set timer to wait for an IP address
tmr.alarm(0, 1000, 1, function()
    print("wifi status: "..wifi.sta.status())
    ip = wifi.sta.getip()
    if ip == nil then
        print("connecting...")
    else
        -- IP isn't nil, stop timer
        tmr.stop(0)

        print('ip: ', ip)

        -- compile and execute www code
        print("Compiling...")
        node.compile("www.lua")

        print("Running...")
        dofile("www.lc")

        print("Open this URL in a browser: ")
        print("http://"..ip.."/")
    end
end)
