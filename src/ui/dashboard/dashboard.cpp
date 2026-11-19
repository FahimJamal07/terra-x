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
:root{color-scheme:dark;--bg:#091016;--panel:#111b22;--panel2:#17242c;--line:#2a3c46;--text:#edf5f7;--muted:#9aadb6;--cyan:#58d6e8;--green:#7ed7a5;--red:#ff747b}
*{box-sizing:border-box}body{margin:0;background:radial-gradient(circle at top,#13242d 0,var(--bg) 55%);color:var(--text);font-family:system-ui,-apple-system,sans-serif}.app{min-height:100vh;display:grid;grid-template-rows:auto 1fr auto;gap:10px;padding:12px}.topbar,.footer{display:flex;align-items:center;justify-content:space-between;gap:10px;border:1px solid var(--line);background:#0d171c;min-height:58px;padding:8px 12px}.brand h1{margin:0;color:var(--cyan);font-size:20px;letter-spacing:3px}.brand div{margin-top:2px;color:var(--muted);font-size:10px;letter-spacing:1px}.indicators{display:flex;align-items:center;gap:6px}.indicator{border:1px solid var(--line);border-radius:3px;padding:8px;font-size:11px}.dot{display:inline-block;width:8px;height:8px;border-radius:50%;margin-right:6px;background:#60717a}.dot.ok{background:var(--green)}.dot.bad{background:var(--red)}button{color:var(--text);touch-action:none}.emergency{min-height:38px;border:1px solid #8a4147;background:#3a2024;color:#ffadb1;font-weight:900}.emergency.active{background:#b9343d;color:white}.main{display:grid;grid-template-columns:minmax(220px,0.9fr) minmax(330px,1.35fr) minmax(220px,0.9fr);gap:10px}.panel{border:1px solid var(--line);background:rgba(17,27,34,.96);min-height:320px}.panel-head{height:42px;display:flex;align-items:center;justify-content:space-between;padding:0 12px;border-bottom:1px solid var(--line);color:var(--cyan);font-size:12px;font-weight:800;text-transform:uppercase;letter-spacing:.8px}.panel-note{color:var(--muted);font-size:10px}.control-body,.actuator-body{height:calc(100% - 42px);display:flex;flex-direction:column;align-items:center;justify-content:center;gap:18px;padding:14px}.dpad{display:grid;grid-template-columns:repeat(3,68px);grid-template-rows:repeat(3,54px);gap:7px}.control-btn{border:1px solid #405866;background:var(--panel2);font-weight:900;font-size:11px}.control-btn.active{border-color:var(--cyan);background:#20414a;box-shadow:0 0 12px #58d6e840}.control-btn.stop{background:#40272b;color:#ffadb0}.forward{grid-column:2}.left{grid-column:1;grid-row:2}.stop{grid-column:2;grid-row:2}.right{grid-column:3;grid-row:2}.reverse{grid-column:2;grid-row:3}.speed-box{width:min(220px,100%)}.speed-box input{width:100%;accent-color:var(--cyan)}.telemetry-body{display:grid;gap:8px;padding:10px}.data-card{border:1px solid var(--line);background:#0e181d;padding:10px}.data-card h3{margin:0 0 7px;color:var(--muted);font-size:11px;text-transform:uppercase}.data-row{display:flex;justify-content:space-between;gap:12px;padding:6px 0;border-bottom:1px solid #ffffff0b;font-size:12px}.data-row:last-child{border-bottom:0}.data-row strong{color:#dff8fb}.soil-grid{display:grid;grid-template-columns:repeat(4,1fr);gap:6px}.soil-value{font-size:18px;font-weight:900;color:var(--green)}.soil-name{margin-top:3px;color:var(--muted);font-size:9px}.actuator-state{font-size:30px;font-weight:900;color:var(--green)}.actuator-buttons{display:grid;grid-template-columns:repeat(3,1fr);gap:7px;width:100%}.actuator-buttons button{min-height:58px;border:1px solid #405866;background:var(--panel2);font-weight:900}.actuator-buttons button.active{border-color:var(--cyan);background:#20414a}.actuator-buttons .stop{color:#ffadb0;background:#40272b}.safety-note{color:var(--muted);text-align:center;font-size:10px}.footer{min-height:38px;border-top:1px solid var(--line);font-size:11px;color:var(--muted)}
@media(max-width:700px){.app{padding:7px}.topbar{align-items:flex-start}.brand h1{font-size:17px}.indicators{justify-content:flex-end;flex-wrap:wrap}.main{grid-template-columns:1fr}.panel{min-height:280px}.footer{flex-wrap:wrap}.dpad{grid-template-columns:repeat(3,74px)}}
</style>
</head>
<body>
<div class="app">
<header class="topbar">
  <div class="brand"><h1>TERRA-X</h1><div>ROVER GROUND CONTROL</div></div>
  <div class="indicators">
    <span class="indicator"><i class="dot ok" id="connectionDot"></i><span id="connectionText">LINK</span></span>
    <span class="indicator"><i class="dot" id="gpsDot"></i>GPS</span>
    <span class="indicator"><i class="dot" id="soilDot"></i>SOIL</span>
    <button class="emergency" id="emergency">E-STOP</button>
  </div>
</header>
<main class="main">
<section class="panel">
  <div class="panel-head"><span>Rover control</span><span class="panel-note" id="driveState">STOP</span></div>
  <div class="control-body">
    <div class="dpad">
      <button class="control-btn forward" data-drive="forward">FORWARD</button>
      <button class="control-btn left" data-drive="left">LEFT</button>
      <button class="control-btn stop" id="driveStop">STOP</button>
      <button class="control-btn right" data-drive="right">RIGHT</button>
      <button class="control-btn reverse" data-drive="reverse">REVERSE</button>
    </div>
    <div class="speed-box"><div>Speed: <strong id="speedValue">200</strong></div><input id="speed" type="range" min="171" max="255" value="200"></div>
  </div>
</section>
<section class="panel">
  <div class="panel-head"><span>Live telemetry</span><span class="panel-note" id="telemetryAge">WAITING</span></div>
  <div class="telemetry-body">
    <div class="data-card"><h3>NEO-6M GPS position</h3>
      <div class="data-row"><span>Latitude</span><strong id="latitude">--</strong></div>
      <div class="data-row"><span>Longitude</span><strong id="longitude">--</strong></div>
      <div class="data-row"><span>Fix</span><strong id="fix">NO FIX</strong></div>
      <div class="data-row"><span>Satellites</span><strong id="satellites">--</strong></div>
      <div class="data-row"><span>Speed</span><strong id="speedGps">-- km/h</strong></div>
      <div class="data-row"><span>Altitude</span><strong id="altitude">-- m</strong></div>
    </div>
    <div class="data-card"><h3>Soil probe</h3>
      <div class="soil-grid">
        <div><div class="soil-value" id="moisture">--</div><div class="soil-name">Moisture</div></div>
        <div><div class="soil-value" id="temperature">--</div><div class="soil-name">Temp °C</div></div>
        <div><div class="soil-value" id="conductivity">--</div><div class="soil-name">EC</div></div>
        <div><div class="soil-value" id="ph">--</div><div class="soil-name">pH</div></div>
      </div>
      <div class="safety-note" id="soilStatus">WAITING FOR SENSOR</div>
    </div>
  </div>
</section>
<section class="panel">
  <div class="panel-head"><span>Actuator</span><span class="panel-note">L298N</span></div>
  <div class="actuator-body">
    <div class="actuator-state" id="actuatorState">STOPPED</div>
    <div class="actuator-buttons">
      <button data-actuator="up">UP</button><button class="stop" id="actuatorStop">STOP</button><button data-actuator="down">DOWN</button>
    </div>
    <div class="speed-box"><div>Actuator PWM: <strong id="actuatorSpeedValue">50</strong></div><input id="actuatorSpeed" type="range" min="0" max="255" value="50"></div>
    <div class="safety-note">Press and hold to operate.<br>DOWN activates the soil probe.</div>
  </div>
</section>
</main>
<footer class="footer"><span>System <strong id="systemState">MANUAL / STOP</strong></span><span>Wi-Fi <strong>AP LINK</strong></span><span>Health <strong id="healthState">CHECKING</strong></span><span id="emergencyState">E-STOP CLEAR</span></footer>
</div>
<script>
const $=id=>document.getElementById(id);let speed=200,actuatorSpeed=50,emergencyActive=false;
const post=(path,body)=>fetch(path,{method:'POST',headers:{'Content-Type':'application/json'},body:JSON.stringify(body||{})}).catch(()=>{$('connectionDot').className='dot bad'});
function stopDrive(){post('/api/stop');document.querySelectorAll('[data-drive]').forEach(b=>b.classList.remove('active'))}
function stopActuator(){post('/api/actuator',{action:'stop'});document.querySelectorAll('[data-actuator]').forEach(b=>b.classList.remove('active'))}
function bindHold(button,action,kind){
 let timer=null;
 const send=()=>post(kind==='drive'?'/api/drive':'/api/actuator',kind==='drive'?{direction:action,speed}:{action,speed:actuatorSpeed});
 const start=e=>{e.preventDefault();button.classList.add('active');send();timer=setInterval(send,300)};
 const end=e=>{if(e)e.preventDefault();if(timer){clearInterval(timer);timer=null}button.classList.remove('active');kind==='drive'?stopDrive():stopActuator()};
 button.addEventListener('pointerdown',start);button.addEventListener('pointerup',end);button.addEventListener('pointercancel',end);
}
document.querySelectorAll('[data-drive]').forEach(b=>bindHold(b,b.dataset.drive,'drive'));document.querySelectorAll('[data-actuator]').forEach(b=>bindHold(b,b.dataset.actuator,'actuator'));
$('driveStop').onclick=stopDrive;$('actuatorStop').onclick=stopActuator;
$('speed').oninput=e=>{speed=Number(e.target.value);$('speedValue').textContent=speed;clearTimeout(window.speedTimer);window.speedTimer=setTimeout(()=>post('/api/speed',{speed}),120)};
$('actuatorSpeed').oninput=e=>{actuatorSpeed=Number(e.target.value);$('actuatorSpeedValue').textContent=actuatorSpeed;clearTimeout(window.actuatorSpeedTimer);window.actuatorSpeedTimer=setTimeout(()=>post('/api/actuator',{action:'stop',speed:actuatorSpeed}),120)};
$('emergency').onclick=()=>post(emergencyActive?'/api/emergency-clear':'/api/emergency-stop');
async function poll(){try{const r=await fetch('/api/telemetry',{cache:'no-store'});if(!r.ok)throw Error();const t=await r.json();emergencyActive=Boolean(t.system.emergency);$('connectionDot').className='dot ok';$('gpsDot').className='dot '+(t.system.sensorHealth.gps?'ok':'bad');$('soilDot').className='dot '+(t.system.sensorHealth.soil?'ok':'bad');$('latitude').textContent=Number(t.gps.latitude).toFixed(6);$('longitude').textContent=Number(t.gps.longitude).toFixed(6);$('fix').textContent=t.gps.fix?'VALID':'NO FIX';$('satellites').textContent=t.gps.satellites;$('speedGps').textContent=Number(t.gps.speedKmph).toFixed(2)+' km/h';$('altitude').textContent=Number(t.gps.altitudeMeters).toFixed(1)+' m';$('moisture').textContent=t.soil.valid?Number(t.soil.moisture).toFixed(1):'--';$('temperature').textContent=t.soil.valid?Number(t.soil.temperature).toFixed(1):'--';$('conductivity').textContent=t.soil.valid?Number(t.soil.conductivity).toFixed(1):'--';$('ph').textContent=t.soil.valid?Number(t.soil.pH).toFixed(1):'--';$('soilStatus').textContent=t.soil.status||'UNKNOWN';$('driveState').textContent=(t.system.roverState||'stop').toUpperCase();$('actuatorState').textContent=(t.system.actuatorState||'stop').toUpperCase();$('systemState').textContent=(t.system.operatingMode||'manual').toUpperCase()+' / '+(t.system.roverState||'stop').toUpperCase();$('emergencyState').textContent=t.system.emergency?'E-STOP ACTIVE':'E-STOP CLEAR';$('emergency').className=t.system.emergency?'emergency active':'emergency';$('healthState').textContent=t.system.sensorHealth.gps&&t.system.sensorHealth.soil?'NOMINAL':'DEGRADED';$('telemetryAge').textContent='UPDATED '+new Date().toLocaleTimeString()}catch(e){$('connectionDot').className='dot bad'}}
poll();setInterval(poll,1000);
</script>
</body>
</html>)HTML";

}

void Dashboard::begin(WebService& webService) {
    webService.server().on("/", HTTP_GET, [&webService]() {
        webService.logRequest();
        webService.server().send_P(200, "text/html", DASHBOARD_HTML);
    });
}

void Dashboard::update() {
}
