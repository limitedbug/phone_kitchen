
// let refreshButton = document.getElementById('refreshButton');
// let disconnectButton = document.getElementById('disconnectButton');
// let deviceList = document.getElementById('deviceList');
// let resultDiv = document.getElementById('resultDiv');
// let statusDiv = document.getElementById('statusDiv');
// let mainPage = document.getElementById('mainPage');
// let detailPage = document.getElementById('detailPage');


// (c) 2013-2015 Don Coleman
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.


/* global cordova, bluetoothSerial  */
/* jshint browser: true , devel: true*/
let pause = false;
document.addEventListener('deviceready', onDeviceReady, false);
document.addEventListener('pause', function(){
    pause = true;
    cordova.plugins.notification.local.schedule({
        title: 'Pause',
        text: 'Se ha detectado mucho humo dentro del edificio',
        foreground: true,
        vibrate: true
    });
}, false);
document.addEventListener('resume', function(){
    pause = false;
}, false);

// navigator.notification.alert(
//     'La aplicación inicio correctamente.',  // message
//     function callback(){

//     },         // callback
//     'Kitchen Master',            // title
//     'Ok!'                  // buttonName
// );
let intervalo;
let intevalo2;
var foundDevices = [];
function onDeviceReady(){
    showMainPage();
    cordova.plugins.notification.local.schedule({
        title: 'Advertencia',
        text: 'Se ha detectado mucho humo dentro del edificio',
        foreground: true,
        vibrate: true
    });
    intevalo2 = setInterval(() => {
        bluetoothSerial.isEnabled(
            function () {
                document.getElementById("start-scan").disabled = false;
                setStatus("Bluetooth activo.");
            },
            function () {
                document.getElementById("start-scan").disabled = true;
                setStatus("Porfavor activa el Bluetooth.");
            }
        );
    }, 2000);
}
document.getElementById('start-scan').addEventListener('click', function(){
    document.getElementById("devices").innerHTML = "";
    startScan();
})
function startScan(){
    clearInterval(intevalo2); 
    bluetoothSerial.list(createDeviceList, handleError);
}
function createDeviceList(devices){
    devices.forEach(function(device) {
        addDevice(device.name, device.address);
    });
    if (devices.length === 0) {
        setStatus("No se encontraron dispositivos.");
    } else {
        setStatus("Se encontro: " + devices.length + " dispositivo" + (devices.length === 1 ? "." : "s."));
    }
}
function setStatus(message){
    document.getElementById('status').innerHTML = message;
}
function addDevice(name, address) {

    var button = document.createElement("a");
    button.innerHTML = name + "<br>" + address;

    button.addEventListener("click", function () {
        connect(address);
    });

    document.getElementById("devices").appendChild(button)+'<br>';
}


function connect(address) {
    new Promise(function (resolve, reject) {
        waitingView.create();
        waitingView.show();
        bluetoothSerial.connect(address, resolve, reject);
    }).then(connectSuccess, connectFailure);
}
function connectFailure(error) {
    waitingView.hide();
    alert(JSON.stringify(error));
}
function connectSuccess() {
    bluetoothSerial.subscribe('\n', getData, handleError);
    // document.getElementById('status').innerHTML = result.address;
    waitingView.hide();
    showDetailPage();
    sendData('x');
    intervalo = setInterval(() => {
        sendData('x');
    }, 5000);
    document.getElementById('disconnectButton').addEventListener('click', function(){
        bluetoothSerial.disconnect(disconnectSuccess, handleError);
    });
}
function disconnectSuccess(){
    showMainPage();
    clearInterval(intervalo);
    document.getElementById('status').innerHTML = "Disconnected from device.";
}
function getData(data){
    let dataArray = data.split(',');
    switch (dataArray[0]) {
        case 'data':
            gettingGeneralData(dataArray);
            break;
        case 'error':
            gettingError(dataArray);
            break;
        case 'alert':
            generateAlert(dataArray);
            break;
        default:
            break;
    }
}
function sendData(data){
    var success = function() {
        console.log('Datos enviados');
    };
    bluetoothSerial.write(data, success, handleError);
}
function handleError(error) {
    alert(JSON.stringify(error));
}
function showMainPage() {
    document.getElementById('mainPage').style.display = "block";
    document.getElementById('detailPage').style.display = "none";
    document.getElementById('start-scan').style.display = "block";
    document.getElementById('disconnectButton').style.display = "none";
}
function showDetailPage() {
    document.getElementById('mainPage').style.display = "none";
    document.getElementById('detailPage').style.display = "block";
    document.getElementById('start-scan').style.display = "none";
    document.getElementById('disconnectButton').style.display = "block";
}

function gettingGeneralData(dataArray){
    let humedad = dataArray[1];
    let temperatura = dataArray[2];
    let humo = dataArray[3];
    let gas = dataArray[4];
    document.getElementById('humidity').innerHTML = humedad;
    document.getElementById('temperature').innerHTML = temperatura;
    document.getElementById('smoke').innerHTML = humo;
    document.getElementById('gas').innerHTML = gas;
    let venta = dataArray[6] == '1' ? true : false;
    let ilum = dataArray[5] == '1' ? true : false;
    document.getElementById('venta').checked = venta;
    document.getElementById('ilum').checked = ilum
}

function ventanal(checkbox){
    if(!confirm('¿Estas Seguro?')){
        if(checkbox.checked){
            checkbox.checked = false;
        }else{
            checkbox.checked = true;
        }
        return;
    }
    if(checkbox.checked){
        sendData('V');
    }else{
        sendData('v');
    }
    
}

function iluminacion(checkbox){
    if(!confirm('¿Estas Seguro?')){
        if(checkbox.checked){
            checkbox.checked = false;
        }else{
            checkbox.checked = true;
        }
        return;
    }
    if(checkbox.checked){
        sendData('J');
    }else{
        sendData('j');
    }
    
}
function gettingError(array){

}
function generateAlert(array){
    let titulo = array[pos];
    let texto = array[pos];
    cordova.plugins.notification.local.schedule({
        title: 'Advertencia',
        text: 'Se ha detectado mucho humo dentro del edificio',
        foreground: true,
        vibrate: true
    });
}
