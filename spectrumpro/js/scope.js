/*
 * Red Pitaya Spectrum Analizator client
 *
 * Author: Dakus <info@eskala.eu>
 *
 * (c) Red Pitaya  http://www.redpitaya.com
 *
 */

(function() {
    var originalAddClassMethod = jQuery.fn.addClass;
    var originalRemoveClassMethod = jQuery.fn.removeClass;
    $.fn.addClass = function(clss) {
        var result = originalAddClassMethod.apply(this, arguments);
        $(this).trigger('activeChanged', 'add');
        return result;
    };
    $.fn.removeClass = function(clss) {
        var result = originalRemoveClassMethod.apply(this, arguments);
        $(this).trigger('activeChanged', 'remove');
        return result;
    }
})();


(function() {

    if ("performance" in window == false) {
        window.performance = {};
    }

    Date.now = (Date.now || function() { // thanks IE8
        return new Date().getTime();
    });

    if ("now" in window.performance == false) {
        var nowOffset = Date.now();
        if (performance.timing && performance.timing.navigationStart) {
            nowOffset = performance.timing.navigationStart
        }
        window.performance.now = function now() {
            return Date.now() - nowOffset;
        }
    }

})();

(function(SPEC, $, undefined) {

    SPEC.ch1_max = false;
    SPEC.ch1_min = false;
    SPEC.ch2_max = false;
    SPEC.ch2_min = false;

    SPEC.startTime = 0;

    // App configuration
    SPEC.param_callbacks = {};
    SPEC.config = {};
    SPEC.config.app_id = 'spectrumpro';
    SPEC.config.server_ip = ''; // Leave empty on production, it is used for testing only
    SPEC.config.start_app_url = window.location.origin + '/bazaar?start=' + SPEC.config.app_id;
    SPEC.config.stop_app_url = window.location.origin + '/bazaar?stop=' + SPEC.config.app_id;
    SPEC.config.socket_url = 'ws://' + window.location.host + '/wss';

    SPEC.config.socket_reconnect_timeout = 1000; // Milliseconds
    SPEC.config.graph_colors = {
        'ch1': '#f3ec1a',
        'ch2': '#31b44b',

        'ch3': '#22EDC7',
        'ch4': '#1AD6FD',
        'ch5': '#C644FC',
        'ch6': '#52EDC7',

    };
    SPEC.config.waterf_img_path = "/tmp/ram/"
    SPEC.freq_unit = ['Hz', 'kHz', 'MHz'];
    SPEC.latest_signal = {};
	SPEC.clear = false;

    SPEC.config.xmin = 0;
    SPEC.config.xmax = 63;
    SPEC.config.unit = 2;
    SPEC.config.gen_enable = undefined;
    SPEC.time_steps = [
        // Hz
        1 / 10, 2 / 10, 5 / 10, 1, 2, 5, 10, 20, 50, 100, 200, 500
    ];

    // Voltage scale steps in volts
    SPEC.voltage_steps = [
        // dBm
        1 / 10, 2 / 10, 5 / 10, 1, 2, 5, 10, 20, 50, 100
    ];

    SPEC.xmin = -1000000;
    SPEC.xmax = 1000000;
    SPEC.ymax = 20.0;
    SPEC.ymin = -120.0;

    SPEC.compressed_data = 0;
    SPEC.decompressed_data = 0;
    SPEC.refresh_times = [];

    SPEC.parameterStack = [];
    SPEC.signalStack = [];
    SPEC.compressed_data = 0;
    SPEC.decompressed_data = 0;

    SPEC.points_per_px = 5; // How many points per pixel should be drawn. Set null for unlimited (will disable client side decimation).
    SPEC.scale_points_size = 10;
    // App state
    SPEC.state = {
        socket_opened: false,
        processing: false,
        editing: false,
        resized: false,
        cursor_dragging: false,
        sel_sig_name: null,
        fine: false,
        demo_label_visible: false
    };

    // Params cache
    SPEC.params = {
        orig: {},
        local: {}
    };
    // Other global variables
    SPEC.ws = null;
    SPEC.graphs = {};
    SPEC.touch = {};
    SPEC.waterfalls = [];

    SPEC.datasets = [];
    SPEC.unexpectedClose = false;

    var g_PacketsRecv = 0;
    var g_CpuLoad = 100.0;
    var g_TotalMemory = 256.0;
    var g_FreeMemory = 256.0;

	SPEC.value1 = [];
	SPEC.value2 = [];
	for (let i = 0; i < 1024; ++i) {
		SPEC.value1[i] = -999999;
		SPEC.value2[i] = 999999;
	}
	SPEC.ch1sig_max = JSON.parse(JSON.stringify({value: SPEC.value1}));
	SPEC.ch1sig_min = JSON.parse(JSON.stringify({value: SPEC.value2}));
	SPEC.ch2sig_max = JSON.parse(JSON.stringify({value: SPEC.value1}));
	SPEC.ch2sig_min = JSON.parse(JSON.stringify({value: SPEC.value2}));
	SPEC.ch1sig_max.size = SPEC.ch1sig_min.size = SPEC.ch2sig_max.size = SPEC.ch2sig_min.size = 1024;

    // Starts the spectrum application on server
    SPEC.startApp = function() {
        $.get(
                SPEC.config.start_app_url
            )
            .done(function(dresult) {
                if (dresult.status == 'OK') {
                    try {
                        SPEC.connectWebSocket();
                    } catch (e) {
                        SPEC.startApp();
                    }
                } else if (dresult.status == 'ERROR') {
                    console.log(dresult.reason ? dresult.reason : 'Could not start the application (ERR1)');
                    SPEC.startApp();
                } else {
                    console.log('Could not start the application (ERR2)');
                    SPEC.startApp();
                }
            })
            .fail(function() {
                console.log('Could not start the application (ERR3)');
                SPEC.startApp();
            })
    };

	var makeMinMaxSignals = function(signals) {
		for (signame in signals) {
			if (SPEC.params.orig["CH1_SHOW"].value && signame == 'ch1' && signals[signame].size > 1) {
				for (let i = 0; i < signals[signame].size; ++i) {
					if (SPEC.ch1_max && signals[signame].value[i] > SPEC.ch1sig_max.value[i])
						SPEC.ch1sig_max.value[i] = signals[signame].value[i];
					if (SPEC.ch1_min && signals[signame].value[i] < SPEC.ch1sig_min.value[i])
						SPEC.ch1sig_min.value[i] = signals[signame].value[i];
				}
				SPEC.ch1sig_max.size = SPEC.ch1sig_min.size = signals[signame].size;
				signals['ch3'] = JSON.parse(JSON.stringify(SPEC.ch1sig_max));
				signals['ch4'] = JSON.parse(JSON.stringify(SPEC.ch1sig_min));
			}
			if (SPEC.params.orig["CH2_SHOW"].value && signame == 'ch2' && signals[signame].size > 1) {
				for (let i = 0; i < signals[signame].size; ++i) {
					if (SPEC.ch2_max && signals[signame].value[i] > SPEC.ch2sig_max.value[i])
						SPEC.ch2sig_max.value[i] = signals[signame].value[i];
					if (SPEC.ch2_min && signals[signame].value[i] < SPEC.ch2sig_min.value[i])
						SPEC.ch2sig_min.value[i] = signals[signame].value[i];
				}
				SPEC.ch2sig_max.size = SPEC.ch2sig_min.size = signals[signame].size;
				signals['ch5'] = JSON.parse(JSON.stringify(SPEC.ch2sig_max));
				signals['ch6'] = JSON.parse(JSON.stringify(SPEC.ch2sig_min));
			}
		}
	}

	var isVisibleSignal = function(signame) {
		if (signame == 'ch1') {
			return SPEC.params.orig['CH1_SHOW'].value;
		} else if (signame == 'ch2') {
			return SPEC.params.orig['CH2_SHOW'].value;
		} else if (signame == 'ch3') {
			return SPEC.ch1_max;
		} else if (signame == 'ch4') {
			return SPEC.ch1_min;
		} else if (signame == 'ch5') {
			return SPEC.ch2_max;
		} else if (signame == 'ch6') {
			return SPEC.ch2_min;
		}
	}

    var guiHandler = function() {
        if (SPEC.signalStack.length > 0) {
            let p = performance.now();

			let signals = JSON.parse(JSON.stringify(SPEC.signalStack[0]));

			if (SPEC.clear) {
				SPEC.ch1sig_max = JSON.parse(JSON.stringify({value: SPEC.value1}));
				SPEC.ch1sig_min = JSON.parse(JSON.stringify({value: SPEC.value2}));
				SPEC.ch2sig_max = JSON.parse(JSON.stringify({value: SPEC.value1}));
				SPEC.ch2sig_min = JSON.parse(JSON.stringify({value: SPEC.value2}));
				signals = [];				
				SPEC.clear = false;
			}

			// create ch3, ch4, ch5, ch6 signals for min/max values
			makeMinMaxSignals(signals)

            SPEC.processSignals(signals);
            SPEC.signalStack.splice(0, 1);
        }
        if (SPEC.signalStack.length > 2)
            SPEC.signalStack.length = [];
    }

    var parametersHandler = function() {
        if (SPEC.parameterStack.length > 0) {
            SPEC.processParameters(SPEC.parameterStack[0]);
            if (SPEC.params.orig['is_demo']) {
                if (!SPEC.state.demo_label_visible) {
                    SPEC.state.demo_label_visible = true;
                    var htmlText = "<p id='browser_detect_text'>You are working in demo mode.</p>";
                    PopupStack.add(htmlText);
                    $('#demo_label').hide();
                }
            } else {
                if (SPEC.state.demo_label_visible) {
                    SPEC.state.demo_label_visible = false;
                    $('#demo_label').hide();
                }
            }
            SPEC.parameterStack.splice(0, 2);
        }
    }

    var performanceHandler = function() {
        $('#throughput_view2').text((SPEC.compressed_data / 1024).toFixed(2) + "kB/s");
        $('#connection_icon').attr('src', '../assets/images/good_net.png');
        $('#connection_meter').attr('title', 'It seems like your connection is ok');
        g_PacketsRecv = 0;

        SPEC.compressed_data = 0;
        SPEC.decompressed_data = 0;
    }

    setInterval(performanceHandler, 1000);
    setInterval(guiHandler, 40);
    setInterval(parametersHandler, 1);

    // Creates a WebSocket connection with the web server
    SPEC.connectWebSocket = function() {

        if (window.WebSocket) {
            SPEC.ws = new WebSocket(SPEC.config.socket_url);
            SPEC.ws.binaryType = "arraybuffer";
        } else if (window.MozWebSocket) {
            SPEC.ws = new MozWebSocket(SPEC.config.socket_url);
            SPEC.ws.binaryType = "arraybuffer";
        } else {
            console.log('Browser does not support WebSocket');
        }

        // Define WebSocket event listeners
        if (SPEC.ws) {

            SPEC.ws.onopen = function() {
                SPEC.state.socket_opened = true;
                console.log('Socket opened');
                setTimeout(function() {
                    if (SPEC.state.demo_label_visible)
                        $('#get_lic').modal('show');
                }, 2500);
                SPEC.unexpectedClose = true;
                SPEC.startTime = performance.now();
                $('body').addClass('loaded');
            };

            SPEC.ws.onclose = function() {
                SPEC.state.socket_opened = false;
                $('#graphs .plot').hide(); // Hide all graphs
                SPEC.hideCursors();
                SPEC.hideInfo();
                console.log('Socket closed. Trying to reopen in ' + SPEC.config.socket_reconnect_timeout / 1000 + ' sec...');
                if (SPEC.unexpectedClose == true) {
                    var currentTime = performance.now();
                    var timeDiff = SPEC.startTime = performance.now();
                    if (timeDiff < 10000)
                        location.reload();
                    else
                        $('#feedback_error').modal('show');
                }
            };

            $('#send_report_btn').on('click', function() {
                var mail = "support@redpitaya.com";
                var subject = "Crash report Red Pitaya OS";
                var body = "%0D%0A%0D%0A------------------------------------%0D%0A" + "DEBUG INFO, DO NOT EDIT!%0D%0A" + "------------------------------------%0D%0A%0D%0A";
                body += "Parameters:" + "%0D%0A" + JSON.stringify({ parameters: SPEC.params }) + "%0D%0A";
                body += "Browser:" + "%0D%0A" + JSON.stringify({ parameters: $.browser }) + "%0D%0A";

                var url = 'info/info.json';
                $.ajax({
                    method: "GET",
                    url: url
                }).done(function(msg) {
                    body += " info.json: " + "%0D%0A" + msg.responseText;
                }).fail(function(msg) {
                    var info_json = msg.responseText
                    var ver = '';
                    try {
                        var obj = JSON.parse(msg.responseText);
                        ver = " " + obj['version'];
                    } catch (e) {};

                    body += " info.json: " + "%0D%0A" + msg.responseText;
                    document.location.href = "mailto:" + mail + "?subject=" + subject + ver + "&body=" + body;
                });
            });

            $('#restart_app_btn').on('click', function() {
                location.reload();
            });

            SPEC.ws.onerror = function(ev) {
                console.log('Websocket error: ', ev);
                if (!SPEC.state.socket_opened)
                    SPEC.startApp();
            };

            var g_count = 0;
            var g_time = 0;
            var g_iter = 10;
            var g_delay = 200;

            SPEC.ws.onmessage = function(ev) {
                ++g_count;
                var start_time = +new Date();
                if (SPEC.state.processing) {
                    return;
                }
                SPEC.state.processing = true;

                try {
                    var data = new Uint8Array(ev.data);
                    SPEC.compressed_data += data.length;
                    var inflate = pako.inflate(data);
                    var text = SPEC.handleCodePoints(inflate);

                    SPEC.decompressed_data += text.length;
                    var receive = JSON.parse(text);

                    if (receive.parameters) {
                        SPEC.parameterStack.push(receive.parameters);
                        if ((Object.keys(SPEC.params.orig).length == 0) && (Object.keys(receive.parameters).length == 0)) {
                            SPEC.params.local['in_command'] = { value: 'send_all_params' };
                            SPEC.ws.send(JSON.stringify({ parameters: SPEC.params.local }));
                            SPEC.params.local = {};
                        }
                    }

                    if (receive.signals) {
                        for (var k in receive.signals) {
                            SPEC.latest_signal[k] = JSON.parse(JSON.stringify(receive.signals[k]));
                        }
                        g_PacketsRecv++;

                        SPEC.signalStack.push(SPEC.latest_signal);
                    }
                    SPEC.state.processing = false;
                } catch (e) {
                    SPEC.state.processing = false;
                    console.log(e);
                } finally {
                    SPEC.state.processing = false;
                }
            };
        }
    };

    // For firefox

    function fireEvent(obj, evt) {
        var fireOnThis = obj;
        if (document.createEvent) {
            var evObj = document.createEvent('MouseEvents');
            evObj.initEvent(evt, true, false);
            fireOnThis.dispatchEvent(evObj);

        } else if (document.createEventObject) {
            var evObj = document.createEventObject();
            fireOnThis.fireEvent('on' + evt, evObj);
        }
    }

    SPEC.SaveGraphsPNG = function() {
        html2canvas($('body'), {
            background: '#343433', // Like background of BODY
            onrendered: function(canvas) {
                var a = document.createElement('a');
                a.href = canvas.toDataURL("image/jpeg").replace("image/jpeg", "image/octet-stream");
                a.download = 'graphs.jpg';
                fireEvent(a, 'click');
            }
        });
    }

    SPEC.processRun = function(new_params) {
        if (new_params['SPEC_RUN'].value === true) {
            $('#SPEC_RUN').hide();
            $('#SPEC_STOP').css('display', 'block');
        } else {
            $('#SPEC_STOP').hide();
            $('#SPEC_RUN').show();
        }
    }

    SPEC.processIsDemo = function(new_params) {
        if (SPEC.config.gen_enable == undefined)
            SPEC.config.gen_enable = new_params['is_demo'].value;
        SPEC.params.orig['is_demo'] = new_params['is_demo'].value;
        if (new_params['is_demo'].value)
            $('#demo-info').show();
        else
            $('#demo-info').hide();
    }

    SPEC.xMax = function(new_params) {
        if (new_params['xmax'].value > 5) {
            if (SPEC.config.gen_enable === true && SPEC.params.orig['freq_unit'] && SPEC.params.orig['freq_unit'].value == 2) {
                $('#xmax').val(5);
                new_params['xmax'].value = 5;
                SPEC.params.local['xmax'] = { value: 5 };
                SPEC.sendParams();
            }
        }
    }

    SPEC.peak1Unit = function(new_params) {
        // 0 - Hz, 1 - kHz, 2 - MHz
        var freq_unit1 = 'Hz';
        var unit = new_params['peak1_unit'];
        if (unit)
            freq_unit1 = (unit.value == 1 ? 'k' : (unit.value == 2 ? 'M' : '')) + 'Hz';

        if (new_params['peak1_power'] && new_params['peak1_freq'] && !$('#CH1_FREEZE').hasClass('active')) {
            var result = '';
            var multiplier = (freq_unit1.charAt(0) == 'k') ? 1000 : (freq_unit1.charAt(0) == 'M') ? 1000000 : 1;
            if (new_params['peak1_power'].value < -120 || new_params['peak1_power'].value > 20)
                result = 'OVER RANGE';
            else if (((new_params['peak1_freq'].value * multiplier) < 0) || ((new_params['peak1_freq'].value * multiplier) > 50e6))
                result = 'OVER RANGE';
            else
                result = SPEC.floatToLocalString(new_params['peak1_power'].value.toFixed(3)) + ' dBm @ ' + SPEC.floatToLocalString(new_params['peak1_freq'].value.toFixed(2)) + ' ' + freq_unit1;
            $('#peak_ch1').val(result);
        }
    }

    SPEC.peak2Unit = function(new_params) {
        // 0 - Hz, 1 - kHz, 2 - MHz
        var freq_unit2 = 'Hz';
        var unit = new_params['peak2_unit'];
        if (unit)
            freq_unit2 = (unit.value == 1 ? 'k' : (unit.value == 2 ? 'M' : '')) + 'Hz';

        if (new_params['peak2_power'] && new_params['peak2_freq'] && !$('#CH2_FREEZE').hasClass('active')) {
            var multiplier = (freq_unit2.charAt(0) == 'k') ? 1000 : (freq_unit2.charAt(0) == 'M') ? 1000000 : 1;
            if (new_params['peak2_power'].value < -120 || new_params['peak2_power'].value > 20)
                result = 'OVER RANGE';
            else if (((new_params['peak2_freq'].value * multiplier) < 0) || ((new_params['peak2_freq'].value * multiplier) > 50e6))
                result = 'OVER RANGE';
            else
                result = SPEC.floatToLocalString(new_params['peak2_power'].value.toFixed(3)) + ' dBm @ ' + SPEC.floatToLocalString(new_params['peak2_freq'].value.toFixed(2)) + ' ' + freq_unit2;
            $('#peak_ch2').val(result);
        }
    }

    SPEC.cursorY1 = function(new_params) {
        if (!SPEC.state.cursor_dragging) {
            var y = 'y1';

            if (new_params['SPEC_CURSOR_Y1'].value) {

                var plot = SPEC.getPlot();
                if (SPEC.isVisibleChannels() && plot) {

                    var axes = plot.getAxes();
                    var min_y = axes.yaxis.min;
                    var max_y = axes.yaxis.max;

                    var new_value = new_params[y == 'y1' ? 'SPEC_CUR1_V' : 'SPEC_CUR2_V'].value;
                    var correct_val = Math.max(min_y, new_value);
                    new_value = Math.min(correct_val, max_y);

                    var volt_per_px = 1 / axes.yaxis.scale;
                    var top = (axes.yaxis.max - new_value) / volt_per_px;

                    $('#cur_' + y + '_arrow, #cur_' + y + ', #cur_' + y + '_info').css('top', top).show();
                    $('#cur_' + y + '_info').html((new_value.toFixed(Math.abs(new_value) >= 0.1 ? 2 : 3)) + 'dBm').css('margin-top', (top < 16 ? 3 : ''));
                }
            } else {
                $('#cur_' + y + '_arrow, #cur_' + y + ', #cur_' + y + '_info').hide();
            }
        }
    }

    SPEC.cursorY2 = function(new_params) {
        if (!SPEC.state.cursor_dragging) {
            var y = 'y2';

            if (new_params['SPEC_CURSOR_Y2'].value) {

                var plot = SPEC.getPlot();
                if (SPEC.isVisibleChannels() && plot) {

                    var axes = plot.getAxes();
                    var min_y = axes.yaxis.min;
                    var max_y = axes.yaxis.max;

                    var new_value = new_params[y == 'y1' ? 'SPEC_CUR1_V' : 'SPEC_CUR2_V'].value;
                    var correct_val = Math.max(min_y, new_value);
                    new_value = Math.min(correct_val, max_y);

                    var volt_per_px = 1 / axes.yaxis.scale;
                    var top = (axes.yaxis.max - new_value) / volt_per_px;

                    $('#cur_' + y + '_arrow, #cur_' + y + ', #cur_' + y + '_info').css('top', top).show();
                    $('#cur_' + y + '_info').html((new_value.toFixed(Math.abs(new_value) >= 0.1 ? 2 : 3)) + 'dBm').css('margin-top', (top < 16 ? 3 : ''));
                }
            } else {
                $('#cur_' + y + '_arrow, #cur_' + y + ', #cur_' + y + '_info').hide();
            }
        }
    }

    SPEC.cursorX1 = function(new_params) {
        if (!SPEC.state.cursor_dragging) {
            var x = 'x1';

            if (new_params['SPEC_CURSOR_X1'].value) {

                var plot = SPEC.getPlot();
                if (SPEC.isVisibleChannels() && plot) {

                    var axes = plot.getAxes();
                    var min_x = Math.max(0, axes.xaxis.min);
                    var max_x = axes.xaxis.max;
                    var new_value = -1 * new_params[x == 'x1' ? 'SPEC_CUR1_T' : 'SPEC_CUR2_T'].value;
                    var correct_val = Math.max(min_x, new_value);
                    new_value = -1 * Math.min(correct_val, max_x);
                    var graph_width = $('#graph_grid').width();
                    var ms_per_px = 1 / axes.xaxis.scale;
                    var msg_width = $('#cur_' + x + '_info').outerWidth();
                    var left = (-axes.xaxis.min - new_value) / ms_per_px;
                    var unit = SPEC.freq_unit[new_params['freq_unit'].value];
                    $('#cur_' + x + '_arrow, #cur_' + x + ', #cur_' + x + '_info').css('left', left).show();
                    $('#cur_' + x + '_info')
                        .html(-(new_value.toFixed(Math.abs(new_value) >= 0.1 ? 2 : Math.abs(new_value) >= 0.001 ? 4 : 6)) + unit)
                        .css('margin-left', (left + msg_width > graph_width - 2 ? -msg_width - 1 : ''));
                }
            } else {
                $('#cur_' + x + '_arrow, #cur_' + x + ', #cur_' + x + '_info').hide();
            }
        }
    }

    SPEC.cursorX2 = function(new_params) {
        if (!SPEC.state.cursor_dragging) {
            var x = 'x2';

            if (new_params['SPEC_CURSOR_X2'].value) {

                var plot = SPEC.getPlot();
                if (SPEC.isVisibleChannels() && plot) {

                    var axes = plot.getAxes();
                    var min_x = Math.max(0, axes.xaxis.min);
                    var max_x = axes.xaxis.max;
                    var new_value = -1 * new_params[x == 'x1' ? 'SPEC_CUR1_T' : 'SPEC_CUR2_T'].value;
                    var correct_val = Math.max(min_x, new_value);
                    new_value = -1 * Math.min(correct_val, max_x);
                    var graph_width = $('#graph_grid').width();
                    var ms_per_px = 1 / axes.xaxis.scale;
                    var msg_width = $('#cur_' + x + '_info').outerWidth();
                    var left = (-axes.xaxis.min - new_value) / ms_per_px;
                    var unit = SPEC.freq_unit[new_params['freq_unit'].value];
                    $('#cur_' + x + '_arrow, #cur_' + x + ', #cur_' + x + '_info').css('left', left).show();
                    $('#cur_' + x + '_info')
                        .html(-(new_value.toFixed(Math.abs(new_value) >= 0.1 ? 2 : Math.abs(new_value) >= 0.001 ? 4 : 6)) + unit)
                        .css('margin-left', (left + msg_width > graph_width - 2 ? -msg_width - 1 : ''));
                }
            } else {
                $('#cur_' + x + '_arrow, #cur_' + x + ', #cur_' + x + '_info').hide();
            }
        }
    }

    SPEC.param_callbacks["SPEC_RUN"] = SPEC.processRun;
    SPEC.param_callbacks["is_demo"] = SPEC.processIsDemo;
    SPEC.param_callbacks["xmax"] = SPEC.xMax;
    SPEC.param_callbacks["peak1_unit"] = SPEC.peak1Unit;
    SPEC.param_callbacks["peak2_unit"] = SPEC.peak2Unit;
    SPEC.param_callbacks["SPEC_CURSOR_Y1"] = SPEC.cursorY1;
    SPEC.param_callbacks["SPEC_CURSOR_Y2"] = SPEC.cursorY2;
    SPEC.param_callbacks["SPEC_CURSOR_X1"] = SPEC.cursorX1;
    SPEC.param_callbacks["SPEC_CURSOR_X2"] = SPEC.cursorX2;

    // Processes newly received values for parameters
    SPEC.processParameters = function(new_params) {
        var old_params = $.extend(true, {}, SPEC.params.orig);

        for (var param_name in new_params) {
            // Save new parameter value
            SPEC.params.orig[param_name] = new_params[param_name];

            var field = $('#' + param_name);

            if (SPEC.param_callbacks[param_name] !== undefined)
                SPEC.param_callbacks[param_name](new_params);

            // Do not change fields from dialogs when user is editing something
            if ((old_params[param_name] === undefined || old_params[param_name].value !== new_params[param_name].value)) {
                if (field.is('select') || field.is('input:text')) {
                    field.val(new_params[param_name].value);

                    if (param_name == 'xmin' || param_name == 'xmax' || param_name == 'freq_unit') {
                        SPEC.updateZoom();

                        if (new_params['freq_unit'])
                            $('#freq').html('Frequency [' + SPEC.freq_unit[new_params['freq_unit'].value] + ']');
                        $('.freeze.active').removeClass('active');
                    }
                } else if (field.is('button')) {
                    field[new_params[param_name].value === true ? 'addClass' : 'removeClass']('active');
                } else if (field.is('input:radio')) {
                    var radios = $('input[name="' + param_name + '"]');

                    radios.closest('.btn-group').children('.btn.active').removeClass('active');
                    radios.eq([+new_params[param_name].value]).prop('checked', true).parent().addClass('active');
                } else if (field.is('span')) {

                    field.html(new_params[param_name].value);
                    if (param_name == 'SPEC_TIME_SCALE') {
                        var unit = SPEC.freq_unit[new_params["freq_unit"].value];
                        $('#freq_scale_unit').html(unit);
                        $('#SPEC_TIME_SCALE').html(new_params[param_name].value.toFixed(3));
                    }
                }
            }
        }

        // Resize double-headed arrows showing the difference between cursors
        SPEC.updateYCursorDiff();
        SPEC.updateXCursorDiff();
    };

    // Processes newly received data for signals
    SPEC.processSignals = function(new_signals) {
        var visible_btns = [];
        var visible_plots = [];
        var visible_info = '';
        var start = +new Date();

        // Do nothing if no parameters received yet
		if ($.isEmptyObject(SPEC.params.orig)) {
            return;
        }

        var frozen_dsets = [
            null,
            null,
            null,
            null,
            null,
            null
        ];

        if ($('#CH1_FREEZE').hasClass('active')) {
            frozen_dsets[0] = SPEC.datasets[0];
            frozen_dsets[2] = SPEC.datasets[2];
            frozen_dsets[3] = SPEC.datasets[3];
        }

        if ($('#CH2_FREEZE').hasClass('active')) {
            frozen_dsets[1] = SPEC.datasets[1];
            frozen_dsets[4] = SPEC.datasets[4];
            frozen_dsets[5] = SPEC.datasets[5];
        }


        SPEC.datasets = [ // For logic
            null,
            null,
            null,
            null,
            null,
            null
        ];

        var bufferDataset = []; // For output

        var sig_count = 0;
        // (Re)Draw every signal
        if (SPEC.params.orig["CH1_SHOW"].value)
        {
            $('#info .left-info .info-title .ch1, #info .left-info .info-value .ch1').hide();
            $('#waterfall-holder_ch1').show();
            $('#right_menu .menu-btn.ch1').prop('disabled', false);
        }
        else
        {
            $('#info .left-info .info-title .ch1, #info .left-info .info-value .ch1').show();
            $('#waterfall-holder_ch1').hide();
            $('#right_menu .menu-btn.ch1').prop('disabled', true);
        }
        if (SPEC.params.orig["CH2_SHOW"].value)
        {
            $('#info .left-info .info-title .ch2, #info .left-info .info-value .ch2').hide();
            $('#waterfall-holder_ch2').show();
            $('#right_menu .menu-btn.ch2').prop('disabled', false);
        }
        else
        {
            $('#info .left-info .info-title .ch2, #info .left-info .info-value .ch2').show();
            $('#waterfall-holder_ch2').hide();
            $('#right_menu .menu-btn.ch2').prop('disabled', true);
        }

        for (sig_name in new_signals) {
            // Ignore empty signals
            if (new_signals[sig_name].size == 0 || !isVisibleSignal(sig_name)) {
                continue;
            }

            sig_count++;

            var points = [];
            var color = SPEC.config.graph_colors[sig_name];

            var idx = sig_name[sig_name.length - 1] - '0' - 1;
            if (frozen_dsets[idx]) {
                points = frozen_dsets[idx].data;
            } else if (SPEC.params.orig['xmax'] && SPEC.params.orig['xmin']) {
                for (var i = 0; i < new_signals[sig_name].size; i++) {
                    var d = (SPEC.params.orig['xmax'].value) / (new_signals[sig_name].size - 1);
                    var p = d * i;
                    points.push([p, new_signals[sig_name].value[i]]);
                }
            }
            SPEC.datasets[idx] = ({ color: color, data: points });
            bufferDataset.push({ color: color, data: points });

            // Update watefalls
            if (sig_name == 'ch1') {
                SPEC.waterfalls[0].setSize($('#graphs').width() - 30, 60);
                if(!$('#CH1_FREEZE').hasClass('active') && $('#SPEC_STOP').is(':visible')){
                	var buf = [];
                	for (var i = 0; i < new_signals[sig_name].size; i++){
                		if (points[i][0] > SPEC.params.orig['xmin'].value)
                			buf.push(points[i][1]);
                	}
					SPEC.waterfalls[0].addData(buf);
                }
                SPEC.waterfalls[0].draw();
            } 
            else if (sig_name == 'ch2') {
                SPEC.waterfalls[1].setSize($('#graphs').width() - 30, 60);
                if(!$('#CH2_FREEZE').hasClass('active') && $('#SPEC_STOP').is(':visible')){
                	var buf = [];
                	for (var i = 0; i < new_signals[sig_name].size; i++){
                		if (points[i][0] > SPEC.params.orig['xmin'].value)
                			buf.push(points[i][1]);
                	}
					SPEC.waterfalls[1].addData(buf);
				}
                SPEC.waterfalls[1].draw();
            }

            var sig_btn = $('#right_menu .menu-btn.' + sig_name);
            visible_btns.push(sig_btn[0]);
            visible_info += (visible_info.length ? ',' : '') + '.' + sig_name;
        }

        if (SPEC.isVisibleChannels()) {

            if (SPEC.graphs && SPEC.graphs.elem) {

                SPEC.graphs.elem.show();

                if (SPEC.state.resized) {
                    SPEC.graphs.plot.resize();
                    SPEC.graphs.plot.setupGrid();
                }
                var filtered_data = SPEC.filterData(bufferDataset, SPEC.graphs.plot.width());
                SPEC.graphs.plot.setData(filtered_data);
                SPEC.graphs.plot.draw();
                SPEC.updateCursors();
                //to change position while resizing automatically
                $('.harrow').css('left', 'inherit');
                $('.varrow').css('top', 'inherit');
            } else {
                SPEC.graphs.elem = $('<div class="plot" />').css($('#graph_grid').css(['height', 'width'])).appendTo('#graphs');
                // Local optimization
                var filtered_data = SPEC.filterData(bufferDataset, SPEC.graphs.elem.width());

                SPEC.graphs.plot = $.plot(SPEC.graphs.elem, filtered_data, {
                    colors: [SPEC.config.graph_colors['ch1'], SPEC.config.graph_colors['ch2']], // channel1, channel2
                    series: {
                        shadowSize: 0 // Drawing is faster without shadows
                    },
                    yaxis: {
                        autSPECaleMargin: 1,
                        min: -120, // (sig_name == 'ch1' || sig_name == 'ch2' ? SPEC.params.orig['SPEC_' + sig_name.toUpperCase() + '_SCALE'].value * -5 : null),
                        max: 20, // (sig_name == 'ch1' || sig_name == 'ch2' ? SPEC.params.orig['SPEC_' + sig_name.toUpperCase() + '_SCALE'].value * 5 : null)
                    },
                    xaxis: {
                        min: 0,
                    },
                    yaxes: [
                        {font: { color: "#888888" }}
                    ],
                    xaxes: [
                        {font: { color: "#888888" }}
                    ],
                    grid: {
                        show: true,
                        borderColor: '#888888',
                        tickColor: '#888888',
                    }
                });
                SPEC.updateZoom();
                //update power div
                var scale = SPEC.graphs.plot.getAxes().yaxis.tickSize;
                $('#SPEC_CH1_SCALE').html(scale);
                $('#SPEC_CH2_SCALE').html(scale);
                SPEC.params.local['SPEC_CH1_SCALE'] = { value: scale };
                SPEC.params.local['SPEC_CH2_SCALE'] = { value: scale };
                SPEC.sendParams();

                var offset = SPEC.graphs.plot.getPlotOffset();
                var margins = {};
                margins.marginLeft = offset.left + 'px';
                margins.marginRight = offset.right + 'px';
                $('.waterfall-holder').css(margins);
            }

            visible_plots.push(SPEC.graphs.elem[0]);



            // Disable buttons related to inactive signals
            $('#right_menu .menu-btn').not(visible_btns).prop('disabled', true);

            $('.pull-right').show();
            // Reset resize flag
            SPEC.state.resized = false;
        }
    };

    // Exits from editing mode
    SPEC.exitEditing = function(noclose) {
        if (!SPEC.isVisibleChannels()) {
            $('#graphs .plot').hide();
            var sig_btn = $('#right_menu .menu-btn.ch2');
            sig_btn.prop('disabled', true);
            var sig_btn = $('#right_menu .menu-btn.ch1');
            sig_btn.prop('disabled', true);
        }

        var t_min = SPEC.config.xmin;
        var t_max = SPEC.config.xmax;
        for (var key in SPEC.params.orig) {
            var field = $('#' + key);
            var value = undefined;

            if (key == 'SPEC_RUN') {
                value = (field.is(':visible') ? 0 : 1);
            } else if (field.is('select') || field.is('input:text')) {
                value = field.val();
            } else if (field.is('button')) {
                value = (field.hasClass('active') ? 1 : 0);
            } else if (field.is('input:radio')) {
                value = $('input[name="' + key + '"]:checked').val();
            }

            if (SPEC.params.orig['CH1_SHOW'] && SPEC.params.orig['CH1_SHOW'].value)
                $('#waterfall-holder_ch1').show();
            else
                $('#waterfall-holder_ch1').hide();

            if (value !== undefined && value != SPEC.params.orig[key].value) {
                SPEC.params.local[key] = { value: ($.type(SPEC.params.orig[key].value) == 'boolean' ? !!value : value) };
            }
            if (key == 'xmin')
                SPEC.config.xmin = value;
            if (key == 'xmax')
                SPEC.config.xmax = value;
            if (key == 'freq_unit')
                SPEC.config.unit = value;
        }

        if (SPEC.config.xmin >= SPEC.config.xmax) {
            SPEC.params.local['xmin'] = { value: t_min };
            SPEC.params.local['xmax'] = { value: t_max };
            SPEC.params.orig['xmin'] = { value: t_min };
            SPEC.config.xmin = t_min;
            SPEC.params.orig['xmax'] = { value: t_max };
            SPEC.config.xmax = t_max;
        }

        // Check changes in measurement list
        var mi_count = 0;
        $('#info-meas').empty();
        $('#meas_list .meas-item').each(function(index, elem) {
            var $elem = $(elem);
            var item_val = $elem.data('value');

            if (item_val !== null) {
                SPEC.params.local['SPEC_MEAS_SEL' + (++mi_count)] = { value: item_val };
                $('#info-meas').append(
                    '<div>' + $elem.data('operator') + '(<span class="' + $elem.data('signal').toLowerCase() + '">' + $elem.data('signal') + '</span>) <span id="SPEC_MEAS_VAL' + mi_count + '">-</span></div>'
                );
            }
        });

        // Send params then reset editing state and hide dialog
        SPEC.sendParams();
        SPEC.state.editing = false;
        if (noclose) return;
        $('.dialog:visible').hide();
        $('#right_menu').show();
    };

    // Sends to server modified parameters
    SPEC.sendParams = function() {
        if ($.isEmptyObject(SPEC.params.local)) {
            return false;
        }

        if (!SPEC.state.socket_opened) {
            console.log('ERROR: Cannot save changes, socket not opened');
            return false;
        }

        SPEC.setDefCursorVals();
        SPEC.params.local['in_command'] = { value: 'send_all_params' };
        SPEC.ws.send(JSON.stringify({ parameters: SPEC.params.local }));
        SPEC.params.local = {};
        return true;
    };

    // Draws the grid on the lowest canvas layer
    SPEC.drawGraphGrid = function() {
        var canvas_width = $('#graphs').width() - 2;
        var canvas_height = Math.round(canvas_width / 2.3);

        var center_x = canvas_width / 2;
        var center_y = canvas_height / 2;

        $('.waterfall-holder').css('width', canvas_width - 30);

        var ctx = $('#graph_grid')[0].getContext('2d');

        var x_offset = 0;
        var y_offset = 0;

        // Set canvas size
        ctx.canvas.width = canvas_width;
        ctx.canvas.height = canvas_height;

        // Set draw options
        ctx.beginPath();
        ctx.lineWidth = 1;
        ctx.strokeStyle = '#343433';

        // Draw ticks
        for (var i = 1; i < 50; i++) {
            x_offset = x_offset + (canvas_width / 50);
            y_offset = y_offset + (canvas_height / 50);

            if (i == 25) {
                continue;
            }

            ctx.moveTo(x_offset, canvas_height - 3);
            ctx.lineTo(x_offset, canvas_height);

            ctx.moveTo(0, y_offset);
            ctx.lineTo(3, y_offset);
        }

        // Draw lines
        x_offset = 0;
        y_offset = 0;

        for (var i = 1; i < 10; i++) {
            x_offset = x_offset + (canvas_height / 10);
            y_offset = y_offset + (canvas_width / 10);

            if (i == 5) {
                continue;
            }

            ctx.moveTo(y_offset, 0);
            ctx.lineTo(y_offset, canvas_height);

            ctx.moveTo(0, x_offset);
            ctx.lineTo(canvas_width, x_offset);
        }

        ctx.stroke();

        // Draw central cross
        ctx.beginPath();
        ctx.lineWidth = 1;
        ctx.strokeStyle = '#343433';

        ctx.moveTo(center_x, 0);
        ctx.lineTo(center_x, canvas_height);

        ctx.moveTo(0, center_y);
        ctx.lineTo(canvas_width, center_y);

        ctx.stroke();
    };

    // Changes Y zoom/scale for the selected signal
    SPEC.changeYZoom = function(direction, curr_scale, send_changes) {
        if (!(SPEC.graphs && SPEC.graphs.elem))
            return null;

        var plot_elem = SPEC.graphs.elem;

        if (!SPEC.isVisibleChannels())
            return null;

        var options = SPEC.graphs.plot.getOptions();
        var axes = SPEC.graphs.plot.getAxes();
        var curr_scale = axes.yaxis.tickSize;

        if ((curr_scale >= SPEC.voltage_steps[SPEC.voltage_steps.length - 1] && direction == '-') ||
            (curr_scale <= SPEC.voltage_steps[0] && direction == '+'))
            return null;


        var range = axes.yaxis.max - axes.yaxis.min;
        var delta = direction == '+' ? 1 : -1
        options.yaxes[0].min = axes.yaxis.min + delta * range * 0.1;
        options.yaxes[0].max = axes.yaxis.max - delta * range * 0.1;

        SPEC.graphs.plot.setupGrid();
        SPEC.graphs.plot.draw();
        axes = SPEC.graphs.plot.getAxes();
        var new_scale = axes.yaxis.tickSize;

        if (send_changes !== false) {
            SPEC.params.local['SPEC_CH1_SCALE'] = { value: new_scale };
            SPEC.params.local['SPEC_CH2_SCALE'] = { value: new_scale };
            SPEC.sendParams();
        }
        SPEC.updateWaterfallWidth();

        return new_scale;

    };

    // Changes X zoom/scale for all signals
    SPEC.changeXZoom = function(direction, curr_scale, send_changes) {
        if (!(SPEC.graphs && SPEC.graphs.elem))
            return null;
        if (!SPEC.isVisibleChannels())
            return null;
        var options = SPEC.graphs.plot.getOptions();
        var axes = SPEC.graphs.plot.getAxes();
        var curr_scale = axes.xaxis.tickSize;

        if ((curr_scale >= SPEC.time_steps[SPEC.time_steps.length - 1] && direction == '-') || (curr_scale <= SPEC.time_steps[0] && direction == '+')) {
            return null;
        }

        var range = axes.xaxis.max - axes.xaxis.min;
        var delta = direction == '+' ? 1 : -1

        options.xaxes[0].min = Math.max(SPEC.config.xmin, axes.xaxis.min + delta * range * 0.1);
        options.xaxes[0].max = Math.min(SPEC.config.xmax, axes.xaxis.max - delta * range * 0.1);

        SPEC.params.local['xmin'] = { value: options.xaxes[0].min };
        SPEC.params.local['xmax'] = { value: options.xaxes[0].max };

        SPEC.graphs.plot.setupGrid();
        SPEC.graphs.plot.draw();
        axes = SPEC.graphs.plot.getAxes();
        var new_scale = axes.xaxis.tickSize;

        if (send_changes !== false) {
            SPEC.params.local['SPEC_TIME_SCALE'] = { value: new_scale };
            SPEC.sendParams();
        }
        SPEC.updateWaterfallWidth();

        return new_scale;
    };

    SPEC.resetZoom = function() {
        if (!(SPEC.graphs && SPEC.graphs.elem))
            return;
        if (!SPEC.isVisibleChannels())
            return;

        var plot = SPEC.graphs.plot;
        var curr_options = plot.getOptions();
        curr_options.xaxes[0].min = SPEC.params.orig['xmin'].value;
        curr_options.xaxes[0].max = SPEC.params.orig['xmax'].value;
        curr_options.yaxes[0].min = SPEC.ymin;
        curr_options.yaxes[0].max = SPEC.ymax;

        SPEC.params.local['xmin'] = { value: SPEC.config.xmin };
        SPEC.params.local['xmax'] = { value: SPEC.config.xmax };
        SPEC.sendParams();

        plot.setupGrid();
        plot.draw();
        var axes = plot.getAxes();
        var cur_x_scale = axes.xaxis.tickSize;
        var cur_y_scale = axes.yaxis.tickSize;
        SPEC.params.local['SPEC_TIME_SCALE'] = { value: cur_x_scale };
        SPEC.params.local['SPEC_CH1_SCALE'] = { value: cur_y_scale };

        SPEC.sendParams();
        SPEC.updateWaterfallWidth();
        SPEC.updateCursors();

    };

    SPEC.isVisibleChannels = function() {
        return ((SPEC.params.orig.CH1_SHOW && SPEC.params.orig.CH1_SHOW.value == true) || (SPEC.params.orig['CH2_SHOW'] && SPEC.params.orig['CH2_SHOW'].value == true));
    };

    SPEC.getPlot = function() {

        if (SPEC.graphs && SPEC.graphs.elem) {
            var plot = SPEC.graphs.plot;
            return plot;
        }
        return null;
    };

    SPEC.updateWaterfallWidth = function() {
        var plot = SPEC.getPlot();
        if (!(SPEC.isVisibleChannels() && plot)) {
            return;
        }
        var offset = plot.getPlotOffset();
        var margins = {};
        margins.marginLeft = offset.left + 'px';
        margins.marginRight = offset.right + 'px';

        $('.waterfall-holder').css(margins);
    };

    SPEC.updateCursors = function() {
        var plot = SPEC.getPlot();
        if (!(SPEC.isVisibleChannels() && plot)) {
            return;
        }
        var offset = plot.getPlotOffset();
        var left = offset.left + 1 + 'px';
        var right = offset.right + 1 + 'px';
        var top = offset.top + 1 + 'px';
        var bottom = offset.bottom + 9 + 'px';

        //update lines length
        $('.hline').css('left', left);
        $('.hline').css('right', right);
        $('.vline').css('top', top);
        $('.vline').css('bottom', bottom);

        //update arrows positions
        var diff_left = offset.left + 2 + 'px';
        var diff_top = offset.top - 2 + 'px';
        var margin_left = offset.left - 7 - 2 + 'px';
        var margin_top = -7 + offset.top - 2 + 'px';
        var margin_bottom = -2 + offset.bottom + 'px';
        var line_margin_left = offset.left - 2 + 'px';
        var line_margin_top = offset.top - 2 + 'px';
        var line_margin_bottom = offset.bottom - 2 + 'px';

        $('.varrow').css('margin-left', margin_left);
        $('.harrow').css('margin-top', margin_top);
        $('.harrow').css('margin-bottom', margin_bottom);
        $('.vline').css('margin-left', line_margin_left);
        $('.hline').css('margin-top', line_margin_top);
        $('.hline').css('margin-bottom', line_margin_bottom);

        $('#cur_x_diff').css('margin-left', diff_left);
        $('#cur_y_diff').css('margin-top', diff_top);
        $('#cur_x_diff_info').css('margin-left', diff_left);
        $('#cur_y_diff_info').css('margin-top', diff_top);

        SPEC.params.local['in_command'] = { value: 'send_all_params' };
        SPEC.ws.send(JSON.stringify({ parameters: SPEC.params.local }));
    };

    SPEC.hideCursors = function() {
        $('.hline, .vline, .harrow, .varrow, .cur_info').hide();
        $('#cur_y_diff').hide();
        $('#cur_x_diff').hide();
    };

    SPEC.hideInfo = function() {
        $('.pull-right').hide();
        // Disable buttons related to inactive signals
        $('#right_menu .menu-btn').prop('disabled', true);
        $('#info').hide();
        $('.waterfall-holder').hide();
    };

    SPEC.updateZoom = function() {

        if (SPEC.graphs && SPEC.graphs.elem) {
            var plot_elem = SPEC.graphs.elem;
            if (SPEC.isVisibleChannels()) {

                var plot = SPEC.graphs.plot;
                SPEC.params.local['xmin'] = { value: SPEC.params.orig['xmin'].value };
                SPEC.params.local['xmax'] = { value: SPEC.params.orig['xmax'].value };

                var axes = plot.getAxes();
                var options = plot.getOptions();

                options.xaxes[0].min = SPEC.params.local['xmin'].value;
                options.xaxes[0].max = SPEC.params.local['xmax'].value;
                options.yaxes[0].min = axes.yaxis.min;
                options.yaxes[0].max = axes.yaxis.max;

                plot.setupGrid();
                plot.draw();
                axes = plot.getAxes();

                $('#SPEC_TIME_SCALE').html(axes.xaxis.tickSize.toFixed(3));
                //$('#SPEC_TIME_SCALE').html(1);
            }
        }
    };

    SPEC.handleCodePoints = function(array) {
        var CHUNK_SIZE = 0x8000; // arbitrary number here, not too small, not too big
        var index = 0;
        var length = array.length;
        var result = '';
        var slice;
        while (index < length) {
            slice = array.slice(index, Math.min(index + CHUNK_SIZE, length)); // `Math.min` is not really necessary here I think
            result += String.fromCharCode.apply(null, slice);
            index += CHUNK_SIZE;
        }
        return result;
    }

    // Use only data for selected channels and do downsamling (data decimation), which is required for
    // better performance. On the canvas cannot be shown too much graph points.
    SPEC.filterData = function(dsets, points) {

        var filtered = [];
        var num_of_channels = 6;

        for (var l = 0; l < num_of_channels; l++) {
            i = Math.min(l, dsets.length - 1);
            if (i < 0)
                return [];
            filtered.push({ color: dsets[i].color, data: [] });

            if (SPEC.points_per_px === null || dsets[i].data.length > points * SPEC.points_per_px) {
                var step = Math.ceil(dsets[i].data.length / (points * SPEC.points_per_px));
                var k = 0;
                for (var j = 0; j < dsets[i].data.length; j++) {
                    if (k > 0 && ++k < step) {
                        continue;
                    }
                    filtered[filtered.length - 1].data.push(dsets[i].data[j]);
                    k = 1;
                }
            } else {
                filtered[filtered.length - 1].data = dsets[i].data.slice(0);
            }
        }

        return filtered;
    };

    SPEC.getLocalDecimalSeparator = function() {
        var n = 1.1;
        return n.toLocaleString().substring(1, 2);
    };

    SPEC.floatToLocalString = function(num) {
        // Workaround for a bug in Safari 6 (reference: https://github.com/mleibman/SlickGrid/pull/472)
        return (num + '').replace('.', SPEC.getLocalDecimalSeparator());
    };

    // Sets default values for cursors, if values not yet defined
    SPEC.setDefCursorVals = function() {

        var plot = SPEC.getPlot();
        if (!(SPEC.isVisibleChannels() && plot)) {
            return;
        }
        var offset = plot.getPlotOffset();
        var graph_height = $('#graph_grid').height() - offset.top - offset.bottom;
        var graph_width = $('#graph_grid').width() - offset.left - offset.right;

        var axes = plot.getAxes();

        var volt_per_px = 1 / axes.yaxis.scale;

        SPEC.updateCursors();

        // Default value for Y1 cursor is 1/4 from graph height
        if (SPEC.params.local['SPEC_CURSOR_Y1'] && SPEC.params.local['SPEC_CURSOR_Y1'].value && SPEC.params.local['SPEC_CUR1_V'] === undefined && $('#cur_y1').data('init') === undefined) {
            var cur_arrow = $('#cur_y1_arrow');
            var top = (graph_height + 7) * 0.25;

            SPEC.params.local['SPEC_CUR1_V'] = { value: axes.yaxis.max - top * volt_per_px };
            $('#cur_y1_arrow, #cur_y1').css('top', top).show();
            $('#cur_y1').data('init', true);
        }

        // Default value for Y2 cursor is 1/3 from graph height
        if (SPEC.params.local['SPEC_CURSOR_Y2'] && SPEC.params.local['SPEC_CURSOR_Y2'].value && SPEC.params.local['SPEC_CUR2_V'] === undefined && $('#cur_y2').data('init') === undefined) {
            var cur_arrow = $('#cur_y2_arrow');
            var top = (graph_height + 7) * 0.33;

            SPEC.params.local['SPEC_CUR2_V'] = { value: axes.yaxis.max - top * volt_per_px };

            $('#cur_y2_arrow, #cur_y2').css('top', top).show();
            $('#cur_y2').data('init', true);
        }

        var ms_per_px = 1 / axes.xaxis.scale;
        // Default value for X1 cursor is 1/4 from graph width
        if (SPEC.params.local['SPEC_CURSOR_X1'] && SPEC.params.local['SPEC_CURSOR_X1'].value && SPEC.params.local['SPEC_CUR1_T'] === undefined && $('#cur_x1').data('init') === undefined) {
            var cur_arrow = $('#cur_x1_arrow');
            var left = graph_width * 0.25;

            SPEC.params.local['SPEC_CUR1_T'] = { value: -axes.xaxis.min - left * ms_per_px };
            $('#cur_x1_arrow, #cur_x1').css('left', left).show();
            $('#cur_x1').data('init', true);
        }

        // Default value for X2 cursor is 1/3 from graph width
        if (SPEC.params.local['SPEC_CURSOR_X2'] && SPEC.params.local['SPEC_CURSOR_X2'].value && SPEC.params.local['SPEC_CUR2_T'] === undefined && $('#cur_x2').data('init') === undefined) {
            var cur_arrow = $('#cur_x2_arrow');
            var left = graph_width * 0.33;

            SPEC.params.local['SPEC_CUR2_T'] = { value: -axes.xaxis.min - left * ms_per_px };

            $('#cur_x2_arrow, #cur_x2').css('left', left).show();
            $('#cur_x2').data('init', true);
        }
    };

    // Updates all elements related to a Y cursor
    SPEC.updateYCursorElems = function(ui, save) {
        var y = (ui.helper[0].id == 'cur_y1_arrow' ? 'y1' : 'y2');

        var plot = SPEC.getPlot();
        if (!(SPEC.isVisibleChannels() && plot)) {
            return;
        }
        var axes = plot.getAxes();

        var volt_per_px = 1 / axes.yaxis.scale;
        var new_value = axes.yaxis.max - ui.position.top * volt_per_px;
        $('#cur_' + y + ', #cur_' + y + '_info').css('top', ui.position.top);
        $('#cur_' + y + '_info').html((new_value.toFixed(Math.abs(new_value) >= 0.1 ? 2 : 3)) + 'dBm').css('margin-top', (ui.position.top < 16 ? 3 : ''));

        SPEC.updateYCursorDiff();

        if (save) {
            SPEC.params.local[y == 'y1' ? 'SPEC_CUR1_V' : 'SPEC_CUR2_V'] = { value: new_value };
            SPEC.sendParams();
        }
    };

    // Updates all elements related to a X cursor
    SPEC.updateXCursorElems = function(ui, save) {
        var x = (ui.helper[0].id == 'cur_x1_arrow' ? 'x1' : 'x2');
        var plot = SPEC.getPlot();
        if (!(SPEC.isVisibleChannels() && plot)) {
            return;
        }
        var axes = plot.getAxes();
        var offset = plot.getPlotOffset();

        var graph_width = $('#graph_grid').width() - offset.left - offset.right;
        var ms_per_px = 1 / axes.xaxis.scale;
        var msg_width = $('#cur_' + x + '_info').outerWidth();
        var new_value = -axes.xaxis.min - ui.position.left * ms_per_px;

        $('#cur_' + x + ', #cur_' + x + '_info').css('left', ui.position.left);

        var unit = SPEC.freq_unit[SPEC.params.orig['freq_unit'].value];
        $('#cur_' + x + '_info')
            .html(-(new_value.toFixed(Math.abs(new_value) >= 0.1 ? 2 : Math.abs(new_value) >= 0.001 ? 4 : 6)) + unit)
            .css('margin-left', (ui.position.left + msg_width > graph_width - 2 ? -msg_width - 1 : ''));

        SPEC.updateXCursorDiff();

        if (save) {
            SPEC.params.local[x == 'x1' ? 'SPEC_CUR1_T' : 'SPEC_CUR2_T'] = { value: new_value };
            SPEC.sendParams();
        }
    };

    // Resizes double-headed arrow showing the difference between Y cursors
    SPEC.updateYCursorDiff = function() {
        var y1 = $('#cur_y1');
        var y2 = $('#cur_y2');
        var y1_top = parseInt(y1.css('top'));
        var y2_top = parseInt(y2.css('top'));
        var diff_px = Math.abs(y1_top - y2_top) - 6;

        if (y1.is(':visible') && y2.is(':visible') && diff_px > 12) {
            var top = Math.min(y1_top, y2_top);
            var value = parseFloat($('#cur_y1_info').html()) - parseFloat($('#cur_y2_info').html());

            $('#cur_y_diff')
                .css('top', top + 5)
                .height(diff_px)
                .show();
            $('#cur_y_diff_info')
                .html(Math.abs(+(value.toFixed(Math.abs(value) >= 0.1 ? 2 : 3))) + 'dB')
                .css('top', top + diff_px / 2 - 2)
                .show();
        } else {
            $('#cur_y_diff, #cur_y_diff_info').hide();
        }
    };

    // Resizes double-headed arrow showing the difference between X cursors
    SPEC.updateXCursorDiff = function() {
        var x1 = $('#cur_x1');
        var x2 = $('#cur_x2');
        var x1_left = parseInt(x1.css('left'));
        var x2_left = parseInt(x2.css('left'));
        var diff_px = Math.abs(x1_left - x2_left) - 9;

        if (x1.is(':visible') && x2.is(':visible') && diff_px > 30) {
            var left = Math.min(x1_left, x2_left);
            var value = parseFloat($('#cur_x1_info').html()) - parseFloat($('#cur_x2_info').html());
            var unit = SPEC.freq_unit[SPEC.params.orig['freq_unit'].value];
            $('#cur_x_diff')
                .css('left', left + 1)
                .width(diff_px)
                .show();
            $('#cur_x_diff_info')
                .html(Math.abs(+(value.toFixed(Math.abs(value) >= 0.1 ? 2 : Math.abs(value) >= 0.001 ? 4 : 6))) + unit)
                .show()
                .css('left', left + diff_px / 2 - $('#cur_x_diff_info').width() / 2 + 3);
        } else {
            $('#cur_x_diff, #cur_x_diff_info').hide();
        }
    };

    SPEC.downloadDataAsCSV = function(filename) {
        var strings = "";

        var ch1 = $('#CH1_SHOW').hasClass('active');
        var ch2 = $('#CH2_SHOW').hasClass('active');
        var ch1_max = SPEC.ch1_max;
        var ch1_min = SPEC.ch1_min;
        var ch2_max = SPEC.ch2_max;
        var ch2_min = SPEC.ch2_min;
        var col_delim = ", ";
        var row_delim = "\n";

        if (ch1)
            strings += "IN1";
        if (ch1_max && ch1)
            strings += ", MAX_IN1";
        if (ch1_min && ch1)
            strings += ", MIN_IN1";
        if (ch2 && ch1)
            strings += ", IN2";
        if (ch2 && !ch1)
            strings += "IN2";
        if (ch2_max && ch2)
            strings += ", MAX_IN2";
        if (ch2_min && ch2)
            strings += ", MIN_IN2";

        strings += row_delim;

        for (var i = 0; i < 1024; i++) {
            if (ch1)
                strings += SPEC.latest_signal.ch1.value[i];
            if (ch1_max && ch1)
                strings += col_delim + SPEC.latest_signal.ch3.value[i];
            if (ch1_min && ch1)
                strings += col_delim + SPEC.latest_signal.ch4.value[i];
            if (ch2 && ch1)
                strings += col_delim + SPEC.latest_signal.ch2.value[i];
            if (ch2 && !ch1)
                strings += SPEC.latest_signal.ch2.value[i];
            if (ch2_max && ch2)
                strings += col_delim + SPEC.latest_signal.ch5.value[i];
            if (ch2_min && ch2)
                strings += col_delim + SPEC.latest_signal.ch6.value[i];

            strings += row_delim;
        };

        var link = document.createElement('a');
        link.setAttribute('download', filename);
        link.setAttribute('href', 'data:' + 'text/html' + ';charset=utf-8,' + encodeURIComponent(strings));
        link.click();
    }

}(window.SPEC = window.SPEC || {}, jQuery));

// Page onload event handler
$(function() {
    var reloaded = $.cookie("spectrum_forced_reload");
    if (reloaded == undefined || reloaded == "false") {
        $.cookie("spectrum_forced_reload", "true");
        window.location.reload(true);
    }

    $('button').bind('activeChanged', function() {
        if ($(this).attr('id') == "CH1_MAX_SHOW") {
            SPEC.ch1_max = $(this).hasClass('active');
    		SPEC.ch1sig_max = JSON.parse(JSON.stringify({value: SPEC.value1}));
        } else if ($(this).attr('id') == "CH1_MIN_SHOW") {
            SPEC.ch1_min = $(this).hasClass('active');
    		SPEC.ch1sig_min = JSON.parse(JSON.stringify({value: SPEC.value2}));
        } else if ($(this).attr('id') == "CH2_MAX_SHOW") {
            SPEC.ch2_max = $(this).hasClass('active');
    		SPEC.ch2sig_max = JSON.parse(JSON.stringify({value: SPEC.value1}));
        } else if ($(this).attr('id') == "CH2_MIN_SHOW") {
            SPEC.ch2_min = $(this).hasClass('active');
    		SPEC.ch2sig_min = JSON.parse(JSON.stringify({value: SPEC.value2}));
        }

        SPEC.exitEditing(true);
    });

    $('#downl_graph').on('click', function() {
        if ($('#CH1_SHOW').hasClass('active') || $('#CH2_SHOW').hasClass('active'))
            setTimeout(SPEC.SaveGraphsPNG, 30);
    });

    $('#downl_csv').on('click', function() {
        SPEC.downloadDataAsCSV("spectrumData.csv");
    });

    $('select, input').on('change', function() {
        SPEC.exitEditing(true);
        setTimeout('SPEC.exitEditing(true)', 1000);
    });

    // Initialize FastClick to remove the 300ms delay between a physical tap and the firing of a click event on mobile browsers
    new FastClick(document.body);

    // Process clicks on top menu buttons
    $('#SPEC_RUN').on('click', function() {
        //ev.preventDefault();
        $('#SPEC_RUN').hide();
        $('#SPEC_STOP').css('display', 'block');
        SPEC.params.local['SPEC_RUN'] = { value: true };
        SPEC.sendParams();
    });

    $('#SPEC_STOP').on('click', function() {
        //ev.preventDefault();
        $('#SPEC_STOP').hide();
        $('#SPEC_RUN').show();
        SPEC.params.local['SPEC_RUN'] = { value: false };
        SPEC.sendParams();
    });

    $('#SPEC_SINGLE').on('click', function() {
        ev.preventDefault();
        SPEC.params.local['SPEC_SINGLE'] = { value: true };
        SPEC.sendParams();
    });

    $('#SPEC_AUTSPECALE').on('click', function() {
        ev.preventDefault();
        SPEC.params.local['SPEC_AUTSPECALE'] = { value: true };
        SPEC.sendParams();
    });

    $('#demo_toggle').on('click', function() {
        SPEC.config.gen_enable = !SPEC.config.gen_enable;
        if (SPEC.config.gen_enable) {
            $('#demo_toggle').children().html('Disable Demo signals');
            $('#demo_img').show();
        } else {
            $('#demo_toggle').children().html('Enable Demo signals');
            $('#demo_img').hide();
        }

        SPEC.params.local['SPEC_GEN_ENABLE'] = { value: SPEC.config.gen_enable };
        SPEC.sendParams();
    });

    // Opening a dialog for changing parameters
    $('.edit-mode').on('click', function() {
        SPEC.state.editing = true;
        $('#right_menu').hide();
        $('#' + $(this).attr('id') + '_dialog').show();
    });

    // Close parameters dialog after Enter key is pressed
    $('input').keyup(function(event) {
        if (event.keyCode == 13) {
            SPEC.exitEditing(true);
        }
    });

    // Close parameters dialog on close button click
    $('.close-dialog').on('click', function() {
        SPEC.exitEditing();
    });

    // Measurement dialog
    $('#meas_done').on('click', function() {
        var meas_signal = $('#meas_dialog input[name="meas_signal"]:checked');

        if (meas_signal.length) {
            var operator_name = $('#meas_operator option:selected').html();
            var operator_val = parseInt($('#meas_operator').val());
            var signal_name = meas_signal.val();
            var item_id = 'meas_' + operator_name + '_' + signal_name;

            // Check if the item already exists
            if ($('#' + item_id).length > 0) {
                return;
            }

            // Add new item
            $('<div id="' + item_id + '" class="meas-item">' + operator_name + ' (' + signal_name + ')</div>').data({
                value: (signal_name == 'CH1' ? operator_val : (signal_name == 'CH2' ? operator_val + 1 : null)), // Temporarily set null for MATH signal, because no param yet defined fot that signal
                operator: operator_name,
                signal: signal_name
            }).prependTo('#meas_list');
        }
    });

    $(document).on('click', '.meas-item', function() {
        $(this).remove();
    });

    // Joystick events
    $('#jtk_up').on('mousedown touchstart', function() { $('#jtk_btns').attr('src', 'img/node_up.png'); });
    $('#jtk_left').on('mousedown touchstart', function() { $('#jtk_btns').attr('src', 'img/node_left.png'); });
    $('#jtk_right').on('mousedown touchstart', function() { $('#jtk_btns').attr('src', 'img/node_right.png'); });
    $('#jtk_down').on('mousedown touchstart', function() { $('#jtk_btns').attr('src', 'img/node_down.png'); });
    $('#jtk_fine').on('mousedown touchstart', function() { $('#jtk_fine').attr('src', 'img/reset_active.png'); });


    $(document).on('mouseup touchend', function() {
        $('#jtk_btns').attr('src', 'img/node_fine.png');
        $('#jtk_fine').attr('src', 'img/reset.png');
    });

    $('#jtk_fine').on('click', function(ev) {
        SPEC.resetZoom();
    });

    $('#jtk_up, #jtk_down').on('click', function(ev) {
        SPEC.changeYZoom(ev.target.id == 'jtk_down' ? '-' : '+');
    });

    $('#jtk_left, #jtk_right').on('click', function(ev) {
        SPEC.changeXZoom(ev.target.id == 'jtk_left' ? '-' : '+');
    });

    // Y cursor arrows dragging
    $('#cur_y1_arrow, #cur_y2_arrow').draggable({
        axis: 'y',
        containment: 'parent',
        start: function(ev, ui) {
            SPEC.state.cursor_dragging = true;
        },
        drag: function(ev, ui) {
            SPEC.updateYCursorElems(ui, false);
        },
        stop: function(ev, ui) {
            SPEC.updateYCursorElems(ui, true);
            SPEC.state.cursor_dragging = false;
        }
    });

    // X cursor arrows dragging
    $('#cur_x1_arrow, #cur_x2_arrow').draggable({
        axis: 'x',
        containment: 'parent',
        start: function(ev, ui) {
            SPEC.state.cursor_dragging = true;
        },
        drag: function(ev, ui) {
            SPEC.updateXCursorElems(ui, false);
        },
        stop: function(ev, ui) {
            SPEC.updateXCursorElems(ui, true);
            SPEC.state.cursor_dragging = false;
        }
    });

    // Touch events
    $(document).on('touchstart', '.plot', function(ev) {
        ev.preventDefault();

        if (!SPEC.touch.start && ev.originalEvent.touches.length > 1) {
            SPEC.touch.zoom_axis = null;
            SPEC.touch.start = [
                { clientX: ev.originalEvent.touches[0].clientX, clientY: ev.originalEvent.touches[0].clientY },
                { clientX: ev.originalEvent.touches[1].clientX, clientY: ev.originalEvent.touches[1].clientY }
            ];
        }
    });

    $(window).on('focus', function() {
        SPEC.drawGraphGrid();
    });

    $(document).on('touchmove', '.plot', function(ev) {
        ev.preventDefault();

        if (ev.originalEvent.touches.length < 2) {
            return;
        }

        SPEC.touch.curr = [
            { clientX: ev.originalEvent.touches[0].clientX, clientY: ev.originalEvent.touches[0].clientY },
            { clientX: ev.originalEvent.touches[1].clientX, clientY: ev.originalEvent.touches[1].clientY }
        ];

        // Find zoom axis
        if (!SPEC.touch.zoom_axis) {
            var delta_x = Math.abs(SPEC.touch.curr[0].clientX - SPEC.touch.curr[1].clientX);
            var delta_y = Math.abs(SPEC.touch.curr[0].clientY - SPEC.touch.curr[1].clientY);

            if (Math.abs(delta_x - delta_y) > 10) {
                if (delta_x > delta_y) {
                    SPEC.touch.zoom_axis = 'x';
                } else if (delta_y > delta_x) {
                    SPEC.touch.zoom_axis = 'y';
                }
            }
        }

        // Skip first touch event
        if (SPEC.touch.prev) {

            // Time zoom
            if (SPEC.touch.zoom_axis == 'x') {
                var prev_delta_x = Math.abs(SPEC.touch.prev[0].clientX - SPEC.touch.prev[1].clientX);
                var curr_delta_x = Math.abs(SPEC.touch.curr[0].clientX - SPEC.touch.curr[1].clientX);

                if (SPEC.state.fine || Math.abs(curr_delta_x - prev_delta_x) > $(this).width() * 0.9 / SPEC.time_steps.length) {
                    var new_scale = SPEC.changeXZoom((curr_delta_x < prev_delta_x ? '-' : '+'), SPEC.touch.new_scale_x, false);

                    if (new_scale !== null) {
                        SPEC.touch.new_scale_x = new_scale;
                        $('#new_scale').html('X scale: ' + new_scale);
                    }

                    SPEC.touch.prev = SPEC.touch.curr;
                }
            }
            // Voltage zoom
            else if (SPEC.touch.zoom_axis == 'y') {
                var prev_delta_y = Math.abs(SPEC.touch.prev[0].clientY - SPEC.touch.prev[1].clientY);
                var curr_delta_y = Math.abs(SPEC.touch.curr[0].clientY - SPEC.touch.curr[1].clientY);

                if (SPEC.state.fine || Math.abs(curr_delta_y - prev_delta_y) > $(this).height() * 0.9 / SPEC.voltage_steps.length) {
                    var new_scale = SPEC.changeYZoom((curr_delta_y < prev_delta_y ? '-' : '+'), SPEC.touch.new_scale_y, false);

                    if (new_scale !== null) {
                        SPEC.touch.new_scale_y = new_scale;
                        $('#new_scale').html('Y scale: ' + new_scale);
                    }

                    SPEC.touch.prev = SPEC.touch.curr;
                }
            }
        } else if (SPEC.touch.prev === undefined) {
            SPEC.touch.prev = SPEC.touch.curr;
        }
    });

    $(document).on('touchend', '.plot', function(ev) {
        ev.preventDefault();

        // Send new scale
        if (SPEC.touch.new_scale_y !== undefined) {
            SPEC.params.local['SPEC_' + SPEC.state.sel_sig_name.toUpperCase() + '_SCALE'] = { value: SPEC.touch.new_scale_y };
            SPEC.sendParams();
        } else if (SPEC.touch.new_scale_x !== undefined) {
            SPEC.params.local['SPEC_TIME_SCALE'] = { value: SPEC.touch.new_scale_x };
            SPEC.sendParams();
        }

        // Reset touch information
        SPEC.touch = {};
        $('#new_scale').empty();
    });

    // Prevent native touch activity like scrolling
    $('html, body').on('touchstart touchmove', function(ev) {
        ev.preventDefault();
    });

	$('#SPEC_CLEAR').click(function() {
		SPEC.clear = true;
	});

    // Preload images which are not visible at the beginning
    $.preloadImages = function() {
        for (var i = 0; i < arguments.length; i++) {
            $('<img />').attr('src', 'img/' + arguments[i]);
        }
    }
    $.preloadImages('node_up.png', 'node_left.png', 'node_right.png', 'node_down.png');
    SPEC.drawGraphGrid();
    // Bind to the window resize event to redraw the graph; trigger that event to do the first drawing
    $(window).resize(function() {

        var window_width = window.innerWidth;
        var window_height = window.innerHeight;
        if (window_width > 768 && window_height > 580) {
            var global_width = window_width - 30,
                global_height = global_width / 1.77885;
            if (window_height < global_height) {
                global_height = window_height - 70 * 1.77885;
                global_width = global_height * 1.77885;
            }
            $('#global_container').css('width', global_width);
            $('#global_container').css('height', global_height);


            SPEC.drawGraphGrid();
            var main_width = $('#main').outerWidth(true);
            var main_height = $('#main').outerHeight(true);
            $('#global_container').css('width', main_width);
            $('#global_container').css('height', main_height);

            SPEC.drawGraphGrid();
            main_width = $('#main').outerWidth(true);
            main_height = $('#main').outerHeight(true);
            window_width = window.innerWidth;
            window_height = window.innerHeight;
            console.log("window_width = " + window_width);
            console.log("window_height = " + window_height);
            if (main_height > (window_height - 80)) {
                $('#global_container').css('height', window_height - 80);
                $('#global_container').css('width', 1.82 * (window_height - 80));
                SPEC.drawGraphGrid();
                $('#global_container').css('width', $('#main').outerWidth(true) - 2);
                $('#global_container').css('height', $('#main').outerHeight(true) - 2);
                SPEC.drawGraphGrid();
            }
        }

        $('#global_container').offset({ left: (window_width - $('#global_container').width()) / 2 });
        // Resize the graph holders
        $('.plot').css($('#graph_grid').css(['height', 'width']));

        // Hide all graphs, they will be shown next time signal data is received
        $('#graphs .plot').hide();

        // Hide all offset arrows
        $('.y-offset-arrow, #time_offset_arrow').hide();

        SPEC.updateCursors();

        // Set the resized flag
        SPEC.state.resized = true;

    }).resize();


    SPEC.waterfalls[0] = $.createWaterfall($("#waterfall_ch1"), $('#waterfall-holder_ch1').width(), 60);
    SPEC.waterfalls[1] = $.createWaterfall($("#waterfall_ch2"), $('#waterfall-holder_ch2').width(), 60);


    // Stop the application when page is unloaded
    window.onbeforeunload = function() {
        SPEC.ws.onclose = function() {}; // disable onclose handler first
        SPEC.ws.close();
        $.ajax({
            url: SPEC.config.stop_app_url,
            async: false
        });
        SPEC.unexpectedClose = false;
    };

    // Init help
    Help.init(helpListSpectrum);
    Help.setState("idle");

    // Everything prepared, start application
    SPEC.startApp();

});
