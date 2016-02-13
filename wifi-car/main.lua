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
"html,body,form{display:flex;justify-content:center;align-items:center;align-content:center;flex-wrap:wrap;height:100%;margin:0;padding:0}\n"..
"form{position:relative}\n"..
"button{display:inline-block;box-sizing:border-box;flex-basis:30%;height:30%;padding:1.25vw 2vw;margin:1%;border:solid .1vw #aaa;border-radius:1vw;background-clip:padding-box;background-color:#eee;font-family:sans-serif;font-size:10vw;font-weight:700;color:#333;text-shadow:.1vw .1vw rgba(153,153,153,0.75);cursor:pointer}\n"..
"button:nth-of-type(3n+1){margin-left:0}\n"..
"button:nth-last-of-type(3n+1){margin-right:0}\n"..
"button:hover{background-color:#6af;color:#000}\n"..
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
"<script type='text/javascript'>"..
"(function iife(w){"..
"var f=w.document.forms[0],"..
"buttons=w.document.querySelectorAll('button'),"..
"minMsBetweenPosts=500,"..
"isPosting=0,"..
"postingTimer=null,"..
"tryXhrPost=function(val){"..
"try{"..
"isPosting=1;"..
"var xhr=new w.XMLHttpRequest;"..
"xhr.open(f.method,f.action,true);"..
"xhr.addEventListener('load',function xhrLoad(xev){var response=this;if(w.console)w.console.info('XHR Load:',response,xev);isPosting=0});"..
"xhr.addEventListener('error',function xhrError(xer){if(w.console)w.console.info('XHR Error:',xer);isPosting=0});"..
"xhr.addEventListener('abort',function xhrAbort(){if(w.console)w.console.log('XHR Aborted.');isPosting=0});"..
"if(w.console)w.console.log('sending cmd='+val);"..
"xhr.send('cmd='+val)"..
"}catch(err){if(w.console)w.console.info('Error caught:',err);f.submit()}},"..
"postTimer=function(val){var delay=minMsBetweenPosts;if(isPosting!==1)tryXhrPost(val);else delay=delay/2;postingTimer=w.setTimeout(function timer(){postTimer(val)},delay)},"..
"holdButton=function(ev){var button=this,val=button.value;ev.preventDefault();w.clearTimeout(postingTimer);postTimer(val)},"..
"releaseButton=function(ev){ev.preventDefault();ev.stopPropagation();isPosting=0;w.clearTimeout(postingTimer);postingTimer=null},"..
"clickButton=function(ev){ev.preventDefault();ev.stopPropagation()},"..
"l=buttons.length;while(l--){"..
"buttons[l].addEventListener('mousedown',holdButton,false);"..
"buttons[l].addEventListener('mouseup',releaseButton,false);"..
"buttons[l].addEventListener('click',clickButton,false)"..
"}})(this);"..
"</script>\n"..
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
