
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
    
}, false);

document.addEventListener('resume', function(){
    pause = false;
}, false);

let intervalo;
let intervalo2;
let intervalo3;
var foundDevices = [];
function onDeviceReady(){
    showMainPage();
    bluetoothSerial.isEnabled(
        function () {
            document.getElementById("start-scan").disabled = false;
            setStatus("Bluetooth activo.");
            document.getElementById('start-scan').addEventListener('click', function(){
                document.getElementById("devices").innerHTML = "";
                startScan();
            })
        },
        function () {
            document.getElementById("start-scan").disabled = true;
            setStatus("Porfavor activa el Bluetooth.");
        }
    );
    intervalo2 = setInterval(() => {
        bluetoothSerial.isEnabled(
            function () {
                document.getElementById("start-scan").disabled = false;
                setStatus("Bluetooth activo.");
                document.getElementById('start-scan').addEventListener('click', function(){
                    document.getElementById("devices").innerHTML = "";
                    startScan();
                })
            },
            function () {
                document.getElementById("start-scan").disabled = true;
                setStatus("Porfavor activa el Bluetooth.");
            }
        );
    }, 2000);
    
}

function startScan(){
    clearInterval(intervalo2); 
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
    intervalo3 = setInterval(() => {
        bluetoothSerial.isConnected(function () {
            
        }, disconnectSuccess);
    }, 5000);
    document.getElementById('disconnectButton').addEventListener('click', function(){
        bluetoothSerial.disconnect(disconnectSuccess, handleError);
    });
}
function disconnectSuccess(){
    showMainPage();
    clearInterval(intervalo);
    clearInterval(intervalo3);
    document.getElementById('status').innerHTML = "Disconnected from device.";
}

async function getData(data){
    let dataArray = data.split(',');
    console.log(dataArray);
    switch (dataArray[0]) {
        case 'data':
            gettingGeneralData(dataArray);
            break;
        case 'error':
            gettingError(dataArray);
            break;
        case 'alert':
            if(!cooldown_activo){
                generateAlert(dataArray);
            }
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
    let ilum = dataArray[5] == '1' ? true : false;
    let venta = dataArray[6] == '1' ? true : false;
    let horno = parseInt(dataArray[7]) < 5 ? false : parseInt(dataArray[7]);
    let disi = dataArray[8] == '1' ? true : false;
    let extra = dataArray[9] == '1' ? true : false;
    if(disi == true || extra == true){
        document.getElementById('venti').checked = true;
        // Poner si es extractor o disipador
    }else {
        document.getElementById('venti').checked = false;
    }
    if(!horno){
        document.getElementById('horno').checked = false;
    }else{
        document.getElementById('horno').checked = true;
        //poner a que temperatura
    }
    document.getElementById('venta').checked = venta;
    document.getElementById('ilum').checked = ilum
}

function ventanal(checkbox){
    var titulo;
    if(checkbox.checked){
        titulo = "Se prenderan";
    }else{
        titulo = "Se apagaran";
    }
    Swal.fire({
        title: titulo,
        showCancelButton: true,
        confirmButtonText: 'Si!',
      }).then((result) => {
        /* Read more about isConfirmed, isDenied below */
        if (result.isConfirmed) {
            Swal.fire('Saved!', '', 'success')
            if(checkbox.checked){
                sendData('V');
            }else{
                sendData('v');
            }
        } else if (result.isDismissed) {
            if(checkbox.checked){
                checkbox.checked = false;
            }else{
                checkbox.checked = true;
            }
        }
      })  
    
}

function iluminacion(checkbox){
    var titulo;
    if(checkbox.checked){
        titulo = "Se prenderan";
    }else{
        titulo = "Se apagaran";
    }
    Swal.fire({
        title: titulo,
        showCancelButton: true,
        confirmButtonText: 'Si!',
      }).then((result) => {
        /* Read more about isConfirmed, isDenied below */
        if (result.isConfirmed) {
            Swal.fire('Saved!', '', 'success')
            if(checkbox.checked){
                sendData('J');
            }else{
                sendData('j');
            }
        } else if (result.isDismissed) {
            if(checkbox.checked){
                checkbox.checked = false;
            }else{
                checkbox.checked = true;
            }
        }
      })   
    
}

async function ventilacion(checkbox){
    var titulo;
    if(checkbox.checked){
        titulo = "Se prenderan";
        const inputOptions = new Promise((resolve) => {
            setTimeout(() => {
            resolve({
                'K': 'Disipador',
                'H': 'Extractor'
            })
            }, 1000)
        })
        
        const { value: tipo } = await Swal.fire({
            title: titulo,
            input: 'radio',
            inputOptions: inputOptions,
            inputValidator: (value) => {
            if (!value) {
                return 'Necesitas escojer uno!'
            }
            }
        })
        
        if (tipo) {
            if(tipo == "K"){
                Swal.fire({
                    title: titulo,
                    showCancelButton: true,
                    confirmButtonText: 'Si!',
                  }).then((result) => {
                    /* Read more about isConfirmed, isDenied below */
                    if (result.isConfirmed) {
                        Swal.fire('Saved!', '', 'success')
                        if(checkbox.checked){
                            sendData('K');
                        }
                    } else if (result.isDismissed) {
                        if(checkbox.checked){
                            checkbox.checked = false;
                        }else{
                            checkbox.checked = true;
                        }
                    }
                  })   
            }else if(tipo == "H"){
                Swal.fire({
                    title: titulo,
                    showCancelButton: true,
                    confirmButtonText: 'Si!',
                }).then((result) => {
                /* Read more about isConfirmed, isDenied below */
                if (result.isConfirmed) {
                    Swal.fire('Saved!', '', 'success')
                    if(checkbox.checked){
                        sendData('H');
                    }
                } else if (result.isDismissed) {
                    if(checkbox.checked){
                        checkbox.checked = false;
                    }else{
                        checkbox.checked = true;
                    }
                }
                })   
            }
        }
    }else{
        titulo = "Se apagara el ventilador";
        Swal.fire({
            title: titulo,
            showCancelButton: true,
            confirmButtonText: 'Si!',
        }).then((result) => {
        /* Read more about isConfirmed, isDenied below */
        if (result.isConfirmed) {
            Swal.fire('Saved!', '', 'success')
            if(!checkbox.checked){
                sendData('k');
            }
        } else if (result.isDismissed) {
            if(checkbox.checked){
                checkbox.checked = false;
            }else{
                checkbox.checked = true;
            }
        }
        })  
    }
}
function hornos(checkbox){
    var titulo;
    var mensaje;
    if(checkbox.checked){
        titulo = "Se prenderan";
        mensaje = Swal.fire({
            title: titulo,
            icon: 'question',
            input: 'range',
            inputLabel: 'Intensidad de llama',
            inputAttributes: {
              min: 0,
              max: 100,
              step: 25
            },
            inputValue: 25
          });
    }else{
        titulo = "Se apagaran";
        mensaje = Swal.fire({
            title: titulo,
            icon: 'question',
            input: 'range',
            inputLabel: 'Intensidad de llama',
            inputAttributes: {
              min: 0,
              max: 0
            },
            inputValue: 0
          });
    }
    mensaje.then((result) => {
        if (result.isConfirmed) {
            Swal.fire('Saved!', '', 'success')
            if(checkbox.checked){
                if(result.value > 0 && result.value <= 33){
                    sendData('h');//amarillo
                }else if(result.value > 33 && result.value <= 66){
                    sendData('w');//rojo
                }else if(result.value > 66){
                    sendData('z');//azul
                }
            }else{
                sendData('Z');//apagar
            }
        } else if (result.isDismissed) {
            if(checkbox.checked){
                checkbox.checked = false;
            }else{
                checkbox.checked = true;
            }
        }
      })
    // Swal.fire({
    //     title: titulo,
    //     showCancelButton: true,
    //     confirmButtonText: 'Si!',
    //   }).then((result) => {
    //     /* Read more about isConfirmed, isDenied below */
    //     if (result.isConfirmed) {
    //         Swal.fire('Saved!', '', 'success')
    //         if(checkbox.checked){
    //             sendData('J');
    //         }else{
    //             sendData('j');
    //         }
    //     } else if (result.isDismissed) {
    //         if(checkbox.checked){
    //             checkbox.checked = false;
    //         }else{
    //             checkbox.checked = true;
    //         }
    //     }
    //   })   
    
}
var contador = 0;
function generateAlert(array){
    let titulo = array[1] || "efe";
    let texto = array[2] || "efe";
    contador +=1;
    document.getElementById('datagetter').innerHTML += contador;
    cooldown_activo = true;
    setTimeout(() => {
        console.log("efe");
        cooldown_activo = false;
    }, 6000);
    // alert("hola");
    if(pause){
        cordova.plugins.notification.local.schedule({
            title: titulo,
            text: texto,
            foreground: true,
            vibrate: true
        });
    }else{
        Swal.fire({
            icon: 'warning',
            title: titulo,
            text: texto
          })
    }
    
}