// Red Pitaya demo mode functions
// Author: Artem Kokos <a.kokos@integrasources.com>
// (c) Red Pitaya  http://www.redpitaya.com

var UartCreated = false;
var SpiCreated = false;
var I2cCreated = false;
var CreatorID = 0;

var SetAcqSpeed = function() {
    $('#ACQ_SPEED').val(128); // Set 1 MS/s
}

var SetUartDecoder = function() {
    var bus = 'bus1';
    OSC.buses[bus] = {};

    OSC.buses[bus].name = "UART";
    OSC.buses[bus].enabled = true;
    OSC.buses[bus].rx = 1;
    OSC.buses[bus].rxtxstr = 'RX';
    OSC.buses[bus].baudrate = 38400;
    OSC.buses[bus].num_data_bits = 8;
    OSC.buses[bus].num_stop_bits = 1;
    OSC.buses[bus].parity = 0; // No parity check
    OSC.buses[bus].bitOrder = 0; // Least significant first
    OSC.buses[bus].invert_rx = 0; // Non inverted
    OSC.buses[bus].samplerate = 1000000;
    OSC.showInfoArrow(parseInt(OSC.buses[bus].rx) - 1);

    OSC.buses[bus].decoder = 'uart' + OSC.state.decoder_id;
    OSC.state.decoder_id++;

    OSC.params.local['CREATE_DECODER'] = {
        value: 'uart'
    };
    OSC.params.local['DECODER_NAME'] = {
        value: OSC.buses[bus].decoder
    };
    OSC.params.local[OSC.buses[bus].decoder + "_parameters"] = {
        value: OSC.buses[bus]
    };

    OSC.ws.send(JSON.stringify({
        parameters: OSC.params.local
    }));

    OSC.params.local = {};

    $("#BUS1_ENABLED").find('img').show();
    $("#BUS1_NAME").text('UART');
    $("#DATA_BUS0").text('UART');
}

var SetSpiDecoder = function() {
    var bus = 'bus2';
    OSC.buses[bus] = {};

    OSC.buses[bus].name = "SPI";
    OSC.buses[bus].enabled = true;

    OSC.buses[bus].clk = 6;
    OSC.buses[bus].miso = 7;
    OSC.buses[bus].mosi = -1; // No MOSI decoder
    OSC.buses[bus].cs = 5;

    OSC.showInfoArrow(parseInt(OSC.buses[bus].clk) - 1);
    OSC.showInfoArrow(parseInt(OSC.buses[bus].miso) - 1);
    OSC.showInfoArrow(parseInt(OSC.buses[bus].mosi) - 1);
    OSC.showInfoArrow(parseInt(OSC.buses[bus].cs) - 1);

    OSC.buses[bus].bit_order = 0; // Most significant first
    OSC.buses[bus].word_size = 8;
    OSC.buses[bus].cpol = 0;
    OSC.buses[bus].cpha = 1;
    OSC.buses[bus].cs_polarity = 0; // Active Low
    OSC.buses[bus].acq_speed = 0; // Do not care

    OSC.buses[bus].miso_decoder = 'spi' + OSC.state.decoder_id;
    OSC.params.local['CREATE_DECODER'] = {
        value: 'spi'
    };
    OSC.params.local['DECODER_NAME'] = {
        value: 'spi' + OSC.state.decoder_id
    };
    OSC.state.decoder_id++;
    OSC.buses[bus]['data'] = OSC.buses[bus].miso;
    OSC.params.local[OSC.buses[bus].miso_decoder + "_parameters"] = {
        value: OSC.buses[bus]
    };

    OSC.ws.send(JSON.stringify({
        parameters: OSC.params.local
    }));

    OSC.params.local = {};

    $("#BUS2_ENABLED").find('img').show();
    $("#BUS2_NAME").text('SPI');
    $("#DATA_BUS1").text('SPI');
}

var SetI2cDecoder = function() {
    var bus = 'bus3';
    OSC.buses[bus] = {};

    OSC.buses[bus].name = "I2C";
    OSC.buses[bus].enabled = true;
    OSC.buses[bus].scl = 3;
    OSC.buses[bus].sda = 4;
    OSC.buses[bus].address_format = 1; // Unshifted address
    OSC.buses[bus].acq_speed = 0; // Do not care

    OSC.showInfoArrow(parseInt(OSC.buses[bus].scl) - 1);
    OSC.showInfoArrow(parseInt(OSC.buses[bus].sda) - 1);

    OSC.buses[bus].decoder = 'i2c' + OSC.state.decoder_id;
    OSC.state.decoder_id++;

    OSC.params.local['CREATE_DECODER'] = {
        value: 'i2c'
    };
    OSC.params.local['DECODER_NAME'] = {
        value: OSC.buses[bus].decoder
    };
    OSC.params.local[OSC.buses[bus].decoder + "_parameters"] = {
        value: OSC.buses[bus]
    };

    OSC.ws.send(JSON.stringify({
        parameters: OSC.params.local
    }));

    OSC.params.local = {};

    $("#BUS3_ENABLED").find('img').show();
    $("#BUS3_NAME").text('I2C');
    $("#DATA_BUS2").text('I2C');
}

var SetOneOfParameter = function() {
    if (!UartCreated) {
        UartCreated = true;
        SetUartDecoder();
        return;
    }
    if (!SpiCreated) {
        SpiCreated = true;
        SetSpiDecoder();
        return;
    }
    if (!I2cCreated) {
        I2cCreated = true;
        SetI2cDecoder();
        return;
    }

    clearInterval(CreatorID);
}

var SetDemoParameters = function() {
    // Enable channels
    $('#CH1_ENABLED').click();
    $('#CH2_ENABLED').click();
    $('#CH3_ENABLED').click();
    $('#CH4_ENABLED').click();
    $('#CH5_ENABLED').click();
    $('#CH6_ENABLED').click();
    $('#CH7_ENABLED').click();
    $('#CH8_ENABLED').click();

    SetAcqSpeed();
    CreatorID = setInterval(SetOneOfParameter, 3000);
}
