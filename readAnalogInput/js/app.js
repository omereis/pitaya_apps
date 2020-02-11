(function(APP, $, undefined) {
    
    // App configuration
    APP.config = {};
    APP.config.app_id = 'readAnalogInput';
    APP.config.app_url = '/bazaar?start=' + APP.config.app_id + '?' + location.search.substr(1);
    APP.config.socket_url = 'ws://' + window.location.hostname + ':9002';

    // WebSocket
    APP.ws = null;

    // Parameters
    APP.processing = false;
//-----------------------------------------------------------------------------
    // Starts template application on server
    APP.startApp = function() {

        $.get(APP.config.app_url)
            .done(function(dresult) {
                if (dresult.status == 'OK') {
                    APP.connectWebSocket();
                } else if (dresult.status == 'ERROR') {
                    console.log(dresult.reason ? dresult.reason : 'Could not start the application (ERR1)');
                    APP.startApp();
                } else {
                    console.log('Could not start the application (ERR2)');
                    APP.startApp();
                }
            })
            .fail(function() {
                console.log('Could not start the application (ERR3)');
                APP.startApp();
            }
        );
    };
//-----------------------------------------------------------------------------
    APP.connectWebSocket = function() {

        //Create WebSocket
        if (window.WebSocket) {
            APP.ws = new WebSocket(APP.config.socket_url);
            APP.ws.binaryType = "arraybuffer";
        } else if (window.MozWebSocket) {
            APP.ws = new MozWebSocket(APP.config.socket_url);
            APP.ws.binaryType = "arraybuffer";
        } else {
            console.log('Browser does not support WebSocket');
        }
//-----------------------------------------------------------------------------
        // Define WebSocket event listeners
        if (APP.ws) {

            APP.ws.onopen = function() {
                $('#hello_message').text("Hello, Red Pitaya!");
                console.log('Socket opened');
            };

            APP.ws.onclose = function() {
                console.log('Socket closed');
            };

            APP.ws.onerror = function(ev) {
                $('#hello_message').text("Connection error");
                console.log('Websocket error: ', ev);         
            };

            APP.ws.onmessage = function(ev) {
                console.log('Message recieved');


                //Capture signals
                if (APP.processing) {
                    return;
                }
                APP.processing = true;

                try {
                    var data = new Uint8Array(ev.data);
                    var inflate = pako.inflate(data);
                    var text = String.fromCharCode.apply(null, new Uint8Array(inflate));
                    var receive = JSON.parse(text);

                    if (receive.parameters) {
                        
                    }

                    if (receive.signals) {
                        APP.processSignals(receive.signals);
                    }
                    APP.processing = false;
                } catch (e) {
                    APP.processing = false;
                    console.log(e);
                } finally {
                    APP.processing = false;
                }



            };
        }
    };
//-----------------------------------------------------------------------------
    //Read value
    APP.readValue = function() {

        var local = {};
        local['READ_VALUE'] = { value: true };
        APP.ws.send(JSON.stringify({ parameters: local }));
    };
//-----------------------------------------------------------------------------
    // Processes newly received data for signals
    APP.processSignals = function(new_signals) {
        var voltage;
        // Draw signals
        for (sig_name in new_signals) {
            // Ignore empty signals
            if (new_signals[sig_name].size == 0) continue;
            // Read signal
            voltage = new_signals[sig_name].value[0];
            //Update value
            $('#value').text(parseFloat(voltage).toFixed(2) + "V");
        }
    };
}(window.APP = window.APP || {}, jQuery));
//-----------------------------------------------------------------------------
// Page onload event handler
$(function() {
    // Button click func
    $("#read_button").click(function() {
        APP.readValue(); 
    });
    // Start application
    APP.startApp();
});
//-----------------------------------------------------------------------------
function toggleStartStop() {
    var btn = document.getElementById('btnStartStop');
    if (btn) {
        if (btn.innerText.toLowerCase() == "start") {
			readAnalogInput();
			//APP.readValue();
            btn.innerText = "Stop";
		}
        else
            btn.innerText = "Start";
    }
}
//-----------------------------------------------------------------------------
function readChannels() {
}
//-----------------------------------------------------------------------------
function readAnalogInput(input) {
	var local = {};
	var t=document.getElementById('txtChannels');

//	console.log(t.value);
	if (typeof(input) == 'undefined')
		input = 0;
	local['READ_VALUE'] = { value: true };
	local['INPUT'] = { value: 17 };
	local['STR_INPUT'] = { value: t.value };

//	alert (JSON.stringify({ parameters: local }));
	var txt = JSON.stringify({ parameters: local });
//	APP.ws.send(JSON.stringify({ parameters: local }));
	APP.ws.send(txt);
    var d=document.getElementById('divLog');
    d.innerHTML += txt + '<br>';
}
//-----------------------------------------------------------------------------
