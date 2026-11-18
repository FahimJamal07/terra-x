#include "dashboard.h"

#include <Arduino.h>

#include "../../communications/web/web_server.h"

namespace {

const char DASHBOARD_HTML[] PROGMEM = R"HTML(<!doctype html>
<html lang="en">
<head>
<meta charset="utf-8">
<meta name="viewport" content="width=device-width,initial-scale=1,maximum-scale=1,user-scalable=no">
<title>TERRA-X Ground Control</title>
<style>
:root{color-scheme:dark;--bg:#0b1014;--panel:#121a20;--line:#293943;--text:#e6eef2;--muted:#91a2ab;--cyan:#52d6e8;--green:#79d6a1;--red:#ff6d72}
*{box-sizing:border-box}body{margin:0;background:var(--bg);color:var(--text);font-family:system-ui,sans-serif}.app{min-height:100vh;display:grid;grid-template-rows:auto 1fr auto;gap:10px;padding:12px}.topbar,.footer{display:flex;justify-content:space-between;align-items:center;gap:10px;border-bottom:1px solid var(--line);padding-bottom:8px}.footer{border-bottom:0;border-top:1px solid var(--line);padding:8px 0 0;color:var(--muted);font-size:11px}.brand h1{margin:0;color:var(--cyan);letter-spacing:2px}.brand .subtle{color:var(--muted);font-size:11px}.indicators{display:flex;gap:6px}.indicator{border:1px solid var(--line);border-radius:4px;padding:7px;font-size:11px}.dot{display:inline-block;width:7px;height:7px;border-radius:50%;margin-right:5px;background:var(--muted)}.dot.ok{background:var(--green)}.dot.bad{background:var(--red)}button{color:var(--text);touch-action:none}.emergency{border:1px solid #73363b;background:#321b20;color:#ff9a9e;font-weight:800;padding:8px}.main{display:grid;grid-template-columns:minmax(220px,1fr) minmax(320px,1.5fr) minmax(220px,1fr);gap:10px}.panel{border:1px solid var(--line);background:var(--panel);min-height:320px}.panel-head{display:flex;justify-content:space-between;border-bottom:1px solid var(--line);padding:10px;color:var(--cyan);font-size:12px}.panel-note{color:var(--muted)}.control-body,.actuator-body{height:calc(100% - 40px);display:flex;flex-direction:column;align-items:center;justify-content:center;gap:14px;padding:12px}.dpad{display:grid;grid-template-columns:repeat(3,70px);grid-template-rows:repeat(3,55px);gap:6px}.control-btn{border:1px solid #3b505b;background:#1d2a32;font-weight:800}.control-btn.active{border-color:var(--cyan)}.control-btn.stop{background:#40272b;color:#ffadb0}.forward{grid-column:2}.left{grid-column:1;grid-row:2}.stop{grid-column:2;grid-row:2}.right{grid-column:3;grid-row:2}.reverse{grid-column:2;grid-row:3}.speed-box{width:220px}.speed-box input{width:100%}.telemetry-body{padding:10px}.data-card{border:1px solid var(--line);padding:10px;margin-bottom:8px}.data-card h3{margin:0 0 8px;color:var(--muted);font-size:11px}.data-row{display:flex;justify-content:space-between;padding:5px 0;border-bottom:1px solid #ffffff0b}.soil-grid{display:grid;grid-template-columns:repeat(4,1fr);gap:6px}.soil-value{font-size:17px;font-weight:800}.soil-name{color:var(--muted);font-size:9px}.actuator-buttons{display:grid;grid-template-columns:repeat(3,1fr);gap:6px;width:100%}.actuator-buttons button{min-height:54px;border:1px solid #3b505b;background:#1d2a32}.actuator-buttons .stop{color:#ffadb0;background:#40272b}.actuator-state{font-size:28px;color:var(--green)}.safety-note{color:var(--muted);text-align:center;font-size:10px}
@media(max-width:700px){.main{grid-template-columns:1fr}.footer{flex-wrap:wrap}.topbar{align-items:flex-start}.indicators{flex-wrap:wrap}}
</style>
</head>
<body>
<div class="app">
<header class="topbar"><div class="brand"><h1>TERRA-X</h1><div class="subtle">ROVER GROUND CONTROL</div></div><div class="indicators"><span class="indicator"><i class="dot ok" id="connectionDot"></i><span id="connectionText">LINK</span></span><span class="indicator"><i class="dot" id="gpsDot"></i>GPS</span><span class="indicator"><i class="dot" id="soilDot"></i>SOIL</span><button class="emergency" id="emergency">E-STOP</button></div></header>
<main class="main">
<section class="panel"><div class="panel-head"><span>Rover control</span><span class="panel-note" id="driveState">STOP</span></div><div class="control-body"><div class="dpad"><button class="control-btn forward" data-drive="forward">FORWARD</button><button class="control-btn left" data-drive="left">LEFT</button><button class="control-btn stop" id="driveStop">STOP</button><button class="control-btn right" data-drive="right">RIGHT</button><button class="control-btn reverse" data-drive="reverse">REVERSE</button></div><div class="speed-box"><div>Speed: <strong id="speedValue">200</strong></div><input id="speed" type="range" min="171" max="255" value="200"></div></div></section>
<section class="panel"><div class="panel-head"><span>Live telemetry</span><span class="panel-note" id="telemetryAge">WAITING</span></div><div class="telemetry-body"><div class="data-card"><h3>NEO-6M GPS position</h3><div class="data-row"><span>Latitude</span><strong id="latitude">--</strong></div><div class="data-row"><span>Longitude</span><strong id="longitude">--</strong></div><div class="data-row"><span>Fix</span><strong id="fix">NO FIX</strong></div><div class="data-row"><span>Satellites</span><strong id="satellites">--</strong></div><div class="data-row"><span>Speed</span><strong id="speedGps">-- km/h</strong></div><div class="data-row"><span>Altitude</span><strong id="altitude">-- m</strong></div></div><div class="data-card"><h3>Soil</h3><div class="soil-grid"><div><div class="soil-value" id="moisture">--</div><div class="soil-name">Moisture</div></div><div><div class="soil-value" id="temperature">--</div><div class="soil-name">Temp °C</div></div><div><div class="soil-value" id="conductivity">--</div><div class="soil-name">EC</div></div><div><div class="soil-value" id="ph">--</div><div class="soil-name">pH</div></div></div></div></div></section>
<section class="panel"><div class="panel-head"><span>Actuator</span><span class="panel-note">TB6612</span></div><div class="actuator-body"><div class="actuator-state" id="actuatorState">STOPPED</div><div class="actuator-buttons"><button data-actuator="up">UP</button><button class="stop" id="actuatorStop">STOP</button><button data-actuator="down">DOWN</button></div><div class="safety-note">Press and hold to operate.</div></div></section>
</main>
<footer class="footer"><span>System <strong id="systemState">MANUAL / STOP</strong></span><span>Wi-Fi <strong>AP LINK</strong></span><span>Health <strong id="healthState">CHECKING</strong></span><span id="emergencyState">E-STOP CLEAR</span></footer>
</div>
<script>
const $=id=>document.getElementById(id);let speed=200,emergencyActive=false;
const post=(path,body)=>fetch(path,{method:'POST',headers:{'Content-Type':'application/json'},body:JSON.stringify(body||{})}).catch(()=>{$('connectionDot').className='dot bad'});
function stopDrive(){post('/api/stop');document.querySelectorAll('[data-drive]').forEach(b=>b.classList.remove('active'))}
function stopActuator(){post('/api/actuator',{action:'stop'});document.querySelectorAll('[data-actuator]').forEach(b=>b.classList.remove('active'))}
function bindHold(button,action,kind){const start=e=>{e.preventDefault();button.classList.add('active');post(kind==='drive'?'/api/drive':'/api/actuator',kind==='drive'?{direction:action,speed}:{action})};const end=e=>{if(e)e.preventDefault();button.classList.remove('active');kind==='drive'?stopDrive():stopActuator()};button.addEventListener('pointerdown',start);['pointerup','pointercancel','pointerleave','lostpointercapture'].forEach(n=>button.addEventListener(n,end))}
document.querySelectorAll('[data-drive]').forEach(b=>bindHold(b,b.dataset.drive,'drive'));document.querySelectorAll('[data-actuator]').forEach(b=>bindHold(b,b.dataset.actuator,'actuator'));$('driveStop').onclick=stopDrive;$('actuatorStop').onclick=stopActuator;
$('speed').oninput=e=>{speed=Number(e.target.value);$('speedValue').textContent=speed;clearTimeout(window.speedTimer);window.speedTimer=setTimeout(()=>post('/api/speed',{speed}),120)};
$('emergency').onclick=()=>post(emergencyActive?'/api/emergency-clear':'/api/emergency-stop');
async function poll(){try{const r=await fetch('/api/telemetry',{cache:'no-store'});if(!r.ok)throw Error();const t=await r.json();emergencyActive=Boolean(t.system.emergency);$('connectionDot').className='dot ok';$('gpsDot').className='dot '+(t.system.sensorHealth.gps?'ok':'bad');$('soilDot').className='dot '+(t.system.sensorHealth.soil?'ok':'bad');$('latitude').textContent=Number(t.gps.latitude).toFixed(6);$('longitude').textContent=Number(t.gps.longitude).toFixed(6);$('fix').textContent=t.gps.fix?'VALID':'NO FIX';$('satellites').textContent=t.gps.satellites;$('speedGps').textContent=Number(t.gps.speedKmph).toFixed(2)+' km/h';$('altitude').textContent=Number(t.gps.altitudeMeters).toFixed(1)+' m';$('moisture').textContent=t.soil.valid?Number(t.soil.moisture).toFixed(1):'--';$('temperature').textContent=t.soil.valid?Number(t.soil.temperature).toFixed(1):'--';$('conductivity').textContent=t.soil.valid?Number(t.soil.conductivity).toFixed(1):'--';$('ph').textContent=t.soil.valid?Number(t.soil.pH).toFixed(1):'--';$('driveState').textContent=(t.system.roverState||'stop').toUpperCase();$('actuatorState').textContent=(t.system.actuatorState||'stop').toUpperCase();$('systemState').textContent=(t.system.operatingMode||'manual').toUpperCase()+' / '+(t.system.roverState||'stop').toUpperCase();$('emergencyState').textContent=t.system.emergency?'E-STOP ACTIVE':'E-STOP CLEAR';$('healthState').textContent=t.system.sensorHealth.gps&&t.system.sensorHealth.soil?'NOMINAL':'DEGRADED';$('telemetryAge').textContent='UPDATED '+new Date().toLocaleTimeString()}catch(e){$('connectionDot').className='dot bad'}}
poll();setInterval(poll,1000);
</script>
</body>
</html>)HTML";

} // namespace

void Dashboard::begin(WebService& webService) {
    webService.server().on("/", HTTP_GET, [&webService]() {
        webService.logRequest();
        webService.server().send_P(200, "text/html", DASHBOARD_HTML);
    });
}

void Dashboard::update() {
}
