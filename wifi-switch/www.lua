-- www.lua
www = net.createServer(net.TCP)

www:listen(80, function(sock)
  sock:on("receive", function(client, request)
    print(request)
    
    -- if GET:
    -- debugging info?
    -- determine selected ON/OFF
    
    sock:send("HTTP/1.1 200 OK\n"..
"Server: ESP8266 Webserver\n"..
"Connection: close\n"..
"Access-Control-Allow-Origin: *\n"..
"Content-Type: text/html\n"..
"\n")

    sock:send("<!doctype html>\n"..
"<html lang='en'>\n"..
"<head>\n"..
"<meta charset='utf-8' />\n"..
"<meta name='viewport' content='width=device-width' />\n"..
"<link rel='icon' href='data:image/x-icon;,' type='image/x-icon' />\n"..
"<title>esp8266 light switch</title>\n"..
"<!-- \n"..
""..
"-->\n"..
"<style type='text/css'>html,body,form{display:flex;justify-content:center;align-items:center;align-content:center;height:100%}\n"..
"label{display:inline-block;position:relative;border:solid .1vw #aaa;border-right:none;border-radius:0;padding:0;overflow:hidden;background-clip:padding-box;cursor:pointer;font:700 2.5vw sans-serif}\n"..
"label input[type='radio']{position:absolute;top:50%;margin-top:-.5vw;width:1vw;height:1vw;left:0;margin-left:.5vw;z-index:10;cursor:pointer}\n"..
"label input[type='radio'] + span{position:relative;display:block;text-align:center;padding:1.25vw 2vw;z-index:20;background-color:#eee;color:#333;text-shadow:.1vw .1vw rgba(153,153,153,0.75)}\n"..
"label:hover input[type='radio'] + span{background-color:#6af;color:#000}\n"..
"label input[type='radio']:checked{cursor:default}\n"..
"label input[type='radio']:checked + span{cursor:default;background-color:#37c;color:#fff;background-clip:padding-box}\n"..
"label:first-of-type{border-radius:1vw 0 0 1vw}\n"..
"label:last-of-type{border-radius:0 1vw 1vw 0;border-right:solid .1vw #aaa}</style>\n"..
"</head>\n"..
"<body>\n"..
"  <form action='/' method='post'>\n"..
"		<label>\n"..
"			<input type='radio' name='toggle' value='on' autocomplete='off' "..
			""..
			"/>\n"..
"			<span>On</span>\n"..
"		</label>\n"..
"		<label>\n"..
"		  <input type='radio' name='toggle' value='off' autocomplete='off' "..
		  ""..
		  "/>\n"..
"		  <span>Off</span>\n"..
"		</label>\n"..
"		<noscript><input type='submit' value='Switch' /></noscript>\n"..
"	</form>\n"..
"	<script type='text/javascript'>for(var f=document.forms[0],radios=document.querySelectorAll("input[type=radio]"),submitForm=function(a){var b=this;a.preventDefault();try{var c=new XMLHttpRequest;c.open(f.method,f.action,!0),c.onload=function(b){var c=this;window.console&&window.console.info(c,a,b),200==c.status},c.send(b.value)}catch(d){window.console&&window.console.info(a,d),f.submit()}},l=radios.length;l--;)radios[l].addEventListener("change",submitForm,!1);</script>\n"..
"</body>\n"..
"</html>\n"..
"")
    
    client:close()
    collectgarbage()
  end)
end)
