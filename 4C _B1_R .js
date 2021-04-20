/* web socket to the web browser */
var app = require('express')();
var http = require('http').Server(app);
var io = require('socket.io')(http);
var path = require('path');
var sock;

// Change to the IP adress of your BeagleBone Black
var BeagleBone = 'ta4y8c9ekw6ayr9k.myfritz.net';

/* stream socket to the deamaon */
var net = require('net');
var client = new net.Socket();

/* start the local server */
app.get('/', function(req, res){
	res.sendFile('index.html', {root: __dirname});
});

app.get('/menue.js', function(req, res){
	res.sendFile('menue.js', {root: __dirname});
});
app.get('/layout.css', function(req, res){
	res.sendFile('layout.css', {root: __dirname});
});
app.get('/menue.css', function(req, res){
	res.sendFile('menue.css', {root: __dirname});
});
app.get('/strobe.css', function(req, res){
	res.sendFile('strobe.css', {root: __dirname});
});
app.get('/news.html', function(req, res){
	res.sendFile('news.html', {root: __dirname});
});
app.get('/register.html', function(req, res){
	res.sendFile('register.html', {root: __dirname});
});
app.get('/register.js', function(req, res){
	res.sendFile('register.js', {root: __dirname});
});
app.get('/socket.js', function(req, res){
	res.sendFile('socket.js', {root: __dirname});
});
app.get('/init.js', function(req, res){
	res.sendFile('init.js', {root: __dirname});
});
app.get('/export-c.js', function(req, res){
	res.sendFile('export-c.js', {root: __dirname});
});



function toArrayBuffer(buf) {
    var ab = new ArrayBuffer(buf.length);
    var view = new Uint8Array(ab);
    for (var i = 0; i < buf.length; ++i) {
        view[i] = buf[i];
    }
    return ab;
}

function toBuffer(ab) {
    var buf = Buffer.alloc(ab.byteLength)
    var view = new Uint8Array(ab);
    for (var i = 0; i < buf.length; ++i) {
        buf[i] = view[i];
    }
    return buf;
}

/* web socket request from the browser */
io.on('connection', function(socket){
	console.log('A user connected');
	sock = socket;
	// open a stream socket to the deamon on BeagleBone Black 
	client.connect(8081/*4711*/, BeagleBone, function() {
		console.log('Connected to' + BeagleBone + ':4711');

	});
		
	socket.on('clientEvent', function(data){
		// console.log(data);
		//client.write(data);
	});
	
	socket.on('RegRead', function(buf){
	
		var buffer = toArrayBuffer(buf);
		
		// var cmd = new Uint16Array(buffer, 0, 1);
		// var adr = new Uint16Array(buffer, 2, 1);
    	//  val = new Uint16Array(buffer, 4, 1);
    	// var byt = new Uint8Array(buffer);
    	    	
    	// console.log ("Read Reg:");
    	// console.log ("cmd: " + cmd[0]);
    	// console.log ("adr: " + adr[0].toString(16));
    	// console.log ("val: " + val[0].toString(16));
    	    	
    	client.write (toBuffer(buffer));
	});
	
	socket.on('RegWrite', function(buf) {
	
		var buffer = toArrayBuffer(buf);
		
		var cmd = new Uint16Array(buffer, 0, 1);
		var adr = new Uint16Array(buffer, 2, 1);
    	var val = new Uint16Array(buffer, 4, 1);
    	var byt = new Uint8Array(buffer);
    	    	
    	// console.log ("Write Reg:");
    	// console.log ("cmd: " + cmd[0]);
    	// console.log ("adr: " + adr[0].toString(16));
    	// console.log ("val: " + val[0].toString(16));
    	
    	client.write (toBuffer(buffer));
	
	});
	
	socket.on('SpiInit', function(buf) {
	
		var buffer = toArrayBuffer(buf);
		
		var cmd = new Uint16Array(buffer, 0, 1);
    	    	
    	// console.log ("Initializing SPI XXXXXXXXXXXXXXX");
    	
    	client.write (toBuffer(buffer));
	
	});
	
	socket.on('CC1200Command', function(buf) {
	
		var buffer = toArrayBuffer(buf);
		
		// var cmd     = new Uint16Array(buffer, 0, 1);
		// var command = new Uint16Array(buffer, 2, 1);
		
		// console.log ("Executing command strobe:" + command[0]);
		client.write (toBuffer(buffer));
	});
	
	socket.on('CC1200State', function(buf) {
	
		var buffer = toArrayBuffer(buf);
		// var cmd     = new Uint16Array(buffer, 0, 1);
		
		//console.log ("Retrieving state");
		client.write (toBuffer(buffer));
	});

	socket.on('SET_MODE', function(buf) {
		var buffer = toArrayBuffer(buf);
		// var cmd = new Uint16Array(buffer, 0, 1);
		// var mode = new Uint16Array(buffer, 2, 1);
		
		client.write (toBuffer(buffer));
	});
	
	socket.on('RSSI_GET', function(buf) {
		var buffer = toArrayBuffer(buf);
		client.write (toBuffer(buffer));
	});

	socket.on('StartRx', function(buf) {
	
		var buffer = toArrayBuffer(buf);
		
		// var cmd        = new Uint16Array(buffer, 0, 1);
		// var seq        = new Uint16Array(buffer, 2, 1);
		// var pkt_format = new Uint16Array(buffer, 4, 1);
		// var pkt_len    = new Uint16Array(buffer, 6, 1);
    	    	
    	// console.log ("Starting RX " + seq + " format:" + pkt_format + " pkt_len:" + pkt_len);
    	
    	client.write (toBuffer(buffer));
	
	});


	socket.on('StopRx', function(buf) {
	
		var buffer = toArrayBuffer(buf);
		
		// var cmd = new Uint16Array(buffer, 0, 1);
    	    	
    	// console.log ("Stopping RX");
    	
    	client.write (toBuffer(buffer));
	
	});

	socket.on('StartTx', function(buf) {
	
		var buffer = toArrayBuffer(buf);
		
		// var cmd        = new Uint16Array(buffer, 0, 1);
		// var count      = new Uint16Array(buffer, 2, 1);
		// var seq        = new Uint16Array(buffer, 4, 1);
		// var pkt_format = new Uint16Array(buffer, 6, 1);
		// var pkt_len    = new Uint16Array(buffer, 8, 1);
		// var pkt        = new Uint8Array(buffer, 10, pkt_len);
    	    	
    	// console.log ("Starting TX " + seq + " format:" + pkt_format + " pkt_len:" + pkt_len + " bufferlen:" + buf.length);
    	// console.log ("Starting TX ");
    	
    	client.write (toBuffer(buffer));
	});
	
	socket.on('StopTx', function(buf) {
	
		var buffer = toArrayBuffer(buf);
		
		// var cmd = new Uint16Array(buffer, 0, 1);
    	    	
    	// console.log ("Stopping TX");
    	
    	client.write (toBuffer(buffer));
	
	});

	socket.on('clientNumber', function(buf){
	
		var buffer = toArrayBuffer(buf);
		var Uint32View =  new Uint16Array(buffer);
		var data = new Uint16Array(buffer, 4, 1);
		var zahl1;
		var zahl2;
		var order = new Uint8Array(buffer);
		var order2 = new Uint8Array(buffer);
		var sorted = 0;
		
		var tmp=0

		tmp = parseInt(13882);
		
		zahl2 = data[4];
		// console.log ("2: " + zahl2);
		
		// console.log('Buffersize: ' + buffer.byteLength);
		// console.log('hex cont ist:' + tmp);
		// console.log('Zahl ist:' + Uint32View[4]);

		// console.log('Zahl2 ist:' + data[4]);
		
		// console.log ("Uint32View (HEX): " + Uint32View[0].toString(16));
		// console.log ("Byte0: " + order[0]);
    	// console.log ("Byte1: " + order[1]);
    	// console.log ("Byte2: " + order[2]);
    	// console.log ("Byte3: " + order[3]);
    	
    	// big endian
    	sorted = 0x1000000 * order[3] + 0x10000 * order[2] + 0x100 * order[1] +  order[0];
		// little endian
//    	sorted = order[3] + 0x100 * order[2] + 0x10000 * order[1] + 0x1000000 * order[0];


    	// console.log('HEX: ' + sorted.toString(16));
       	Uint32View[0] = sorted;
       	// console.log('HEX: ' + Uint32View.toString(16));


		// console.log ("Uint32View (HEX): " + Uint32View[0].toString(16));
       	
    	
		// console.log ("Byte0: " + order2[0]);
    	// console.log ("Byte1: " + order2[1]);
    	// console.log ("Byte2: " + order2[2]);
    	// console.log ("Byte3: " + order2[3]);

		
	
		client.write (toBuffer(buffer));

	});
	
	socket.on('disconnect', function (){
		console.log('A user disconnected');
		client.end();
	});
	
});

client.on('close', function() {
	console.log('Connection closed');
	client.end();
	});
	
client.on('data', function(data) {
	var buffer = toArrayBuffer(data);
	var current_length = 0;
	var current_idx    = 0;	
	var current_size   = 1;	

	current_length = data.length;

//	console.log ("Buffer length: " + data.length);
	
	while (current_length > 0) {
		var cmd = new Uint16Array(buffer, current_idx*current_size, 1);
//		console.log ("Command is " + cmd[0]);
//		console.log ("Current length is " + current_length);
//		console.log ("Current idx is " + current_idx);
		switch (cmd[0]) {
			case 1:
				break;
			case 2:
				var reg = new ArrayBuffer(6);
				var reg_cmd = new Uint16Array(reg, 0, 1);
				var reg_adr = new Uint16Array(reg, 2, 1);
    			var reg_val = new Uint16Array(reg, 4, 1);

				var adr = new Uint16Array(buffer, 2+current_idx*6, 1);
    			var val = new Uint16Array(buffer, 4+current_idx*6, 1);

				reg_cmd[0] = cmd[0];
				reg_adr[0] = adr[0];
				reg_val[0] = val[0];
				
    			sock.emit('RegRead', reg);
    			current_idx++;
    			current_length-=6;
    			current_size = 6;
    			break;
    		case 5:
    			var state = new ArrayBuffer(4);
    			var state_cmd = new Uint16Array(state, 0, 1);
    			var state_val = new Uint16Array(state, 2, 1);
    			
    			var val = new Uint16Array(buffer, 2+current_idx*4, 1);
    			
    			state_cmd[0] = cmd[0];
    			state_val[0] = val[0];
    			sock.emit('CC1200State', state);
    			
    			current_idx++;
    			current_length-=4;
    			current_size = 4;
    			break;
   			case 10:
				var length = new Uint16Array(buffer, 2+current_idx*4, 1);
				var seq    = new Uint16Array(buffer, 4+current_idx*4, 1);
				var rssi   = new Uint16Array(buffer, 6+current_idx*4, 1);
				var crc    = new Uint16Array(buffer, 8+current_idx*4, 1);
				var data   = new Uint8Array(buffer, 10+current_idx*4, length[0]);

    			var fifo_data        = new ArrayBuffer(length[0]+10);
				var fifo_data_cmd    = new Uint16Array(fifo_data, 0, 1);
				var fifo_data_length = new Uint16Array(fifo_data, 2, 1);
				var fifo_data_seq    = new Uint16Array(fifo_data, 4, 1);
				var fifo_data_rssi   = new Uint16Array(fifo_data, 6, 1);
				var fifo_data_crc    = new Uint16Array(fifo_data, 8, 1);
				var fifo_data_data   = new Uint8Array(fifo_data, 10, length[0]);

				
				var enc = new TextDecoder();

				fifo_data_cmd[0] = cmd[0];
				fifo_data_length[0] = length[0];
				fifo_data_seq[0] = seq[0];
				fifo_data_rssi[0] = rssi[0];
				fifo_data_crc[0] = crc[0];

				var cnt;
				for (cnt=0; cnt<=length[0]; cnt++) fifo_data_data[cnt]=data[cnt];
				// console.log("Packet length:" + length[0] + "Seq: " + seq[0] + " " + enc.decode(fifo_data_data));	

				sock.emit('FIFO_DATA', fifo_data);
								
				current_idx++;
    			current_length-=262;
    			current_size = 262;	
    			break;
    		case 7:
    			var get_rssi = new ArrayBuffer(4);
				var get_rssi_cmd = new Uint16Array(get_rssi, 0, 1);
				var get_rssi_value = new Uint16Array(get_rssi, 2, 1);
				
				var val = new Uint16Array(buffer, 2+current_idx*4, 1);
		
				// console.log("Get RSSI value:" + val[0]);	
				
				get_rssi_cmd[0]=cmd[0];
				get_rssi_value[0]=val[0];
				sock.emit('RSSI_GET', get_rssi);
				
				current_idx++;
    			current_length-=4;
    			current_size = 4;	
    			break;
     		case 12:
    			var stop_tx = new ArrayBuffer(2);
				var stop_tx_cmd = new Uint16Array(stop_tx, 0, 1);
				
				
				stop_tx_cmd[0]=cmd[0];
				sock.emit('STOP_TX', stop_tx);
				
				current_idx++;
    			current_length-=2;
    			current_size = 2;	
    			break;
  		default:
    			console.log ("Unknown command");
    			break;
    	}
    	//current_idx++;
    	//current_length-=6;
    }
});

client.on('error', function(error){
  console.log('An ERROR occured');
  console.log('Error : ' + error);
});
	
//client.on('data', function(data) {
	//console.log("DADADADTTTTTTTT" + data);
//	});

http.listen(3000, function(){
	console.log('listenig on *:3000');
});
