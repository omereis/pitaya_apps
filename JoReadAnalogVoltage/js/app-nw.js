(function(APP, $, undefined) {
    
    // App configuration
    APP.config = {};
    APP.config.app_id = 'ReadAnalogVoltage';
    APP.config.app_url = '/bazaar?start=' + APP.config.app_id + '?' + location.search.substr(1);
    APP.config.socket_url = 'ws://' + window.location.hostname + ':9002';

    // WebSocket
    APP.ws = null;

    //LED state
    APP.led_state = false;



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
            });
    };




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
                var data = new Uint8Array(ev.data);
				var inflate = pako.inflate(data);
				var text = String.fromCharCode.apply(null, new Uint8Array(inflate));
				var receive = JSON.parse(text);

				if (receive.signals) {
				APP.processSignals(receive.signals);
				}
			console.log('Message recieved');
            };
        }
    };

}(window.APP = window.APP || {}, jQuery));


	APP.processSignals = function(receive.signals)
	{
		var voltage;

		for (sig_name in new_signals) {

		if (new_signals[sig_name].size == 0) continue;

		voltage = new_signals[sig_name].value[new_signals[sig_name].size - 1];

		$('#value').text(parseFloat(voltage).toFixed(2) + "V");
		}
	}

    //Read value
	APP.readValue = function() {
	var local = {};
	local['READ_VALUE'] = { value: true };
	APP.ws.send(JSON.stringify({ parameters: local }));
	}


// Page onload event handler
$(function() {
    
      // Button click analog read
	  
     $("#read_button").click(function() {

         APP.readValue(); 
     });
	
 
 // Click led 
 
   $('#led_state').click(function() {

        
		if (APP.led_state == true)
        {
            $('#led_on').hide();
            $('#led_off').show();
            APP.led_state = false;
        }
        else
        {
            $('#led_off').hide();
            $('#led_on').show();
            APP.led_state = true;
        }
        var local = {};
        local['LED_STATE'] = { value: APP.led_state };
        APP.ws.send(JSON.stringify({ parameters: local }));
    });


    // Start application
    APP.startApp();
});
