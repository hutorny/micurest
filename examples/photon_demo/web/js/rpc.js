/**
 * main.js - JS script for micurest example application
 */

/**
 * Sugar for query selectors 
 */
function $one(sel) {
	return (typeof sel === 'string') ? document.querySelector(sel) : sel;
};

function $arr(sel) {
	if( sel === null ) return [];	
	return Array.apply(null, document.querySelectorAll(sel)) || [];
};


function LogMessage(id, clas, dlm, message) {
	var text = '<span class="' + clas + '">';
	var div = (id && $one('#_'+id));
	if( div ) text += dlm;
	text += message + '</span>';
	var log = ($one('#log') || document.body);
	if( ! div ){
		if( id ) div = document.createElement('div');
		else  div = $one('#log div.pending') || document.createElement('div');
		div.setAttribute('id','_'+(id||clas));		
		div.setAttribute('class',id?'pending':'');
	} else
		div.setAttribute('class',null);
	var fn = div.firstChild && div.firstChild.ondblclick; 
	div.innerHTML += text;
	log.insertBefore(div, log.firstChild);
	if( fn ) div.firstChild.ondblclick = fn;
	return div;
}

function Send(method, url, text, type, fn) {
	var xhr = new XMLHttpRequest();
	if( ! fn ) fn = function() {}
	xhr.onreadystatechange = function () {
		if( this.readyState == 4 ) {
			if( this.status >= 200  && this.status <= 204 )
	        	fn(null, this.responseText, this.getResponseHeader('Content-Type'));
	        else {		        	
	        	if ( this.status )
	        		fn(this.statusText, this.responseText, 
	        				this.getResponseHeader('Content-Type'));
	        }
		}
	};
    xhr.open(method, url);
   	xhr.timeout = 2000;
  	xhr.ontimeout = function () { 
  		fn('Network timeout');
  	};
    if( type !== null ) {
    	xhr.setRequestHeader(method === 'GET'?'Accept':'Content-Type', type);
    }
  	
	xhr.send(text);	
	return xhr;
};

function Get(url, fn) {
	return Send('GET', url, null, null, fn);
}

function GetObj(url, fn) {
	Send('GET', url, '', 'application/json', function(err, data, ctype){
		if( data && ctype ==  'application/json' ) 
		  try { 
			  data = JSON.parse(data);		
		  } catch(e) {
			  data = null;
			  if( ! err ) err = e;
		  }
		  fn(err, data);
	});					
}

function PutObj(url, obj, type, fn) {
	Send('PUT', url, JSON.stringify(obj), type||'application/json', function(err, data, ctype){
		if( data && ctype ==  'application/json' ) 
		  try { 
			  data = JSON.parse(data);		
		  } catch(e) {
			  data = null;
			  if( ! err ) err = e;
		  }
		  fn(err, data);
	});					
}

function makeID() {
	return (new Date().valueOf());
}

function Call(url, method, params, callback) {
	var id = makeID();
	PutObj(url, 
		{jsonrpc:"2.0", method:method, params: params, id: id},
		'application/json-rpc',	
		function(err, data) {
			var result = data ? data.result : undefined;
			var error = (data ? data.error : {message: err});   
			callback(data&&data.id, error, result);
		}
	);
	return id;
} 

function RemoteService(url) {
    var callback = function(cb) { console.log('!!!!'); return proxy; }
    callback.url = url;
    callback.callback = function(){};
	var proxy = new Proxy(callback,{ 
		apply:function(target, thiz, args){ 
			if( typeof args[0] == 'function' ) target.callback = args[0]; 
			return proxy; 
		},
		get:function(remote,method){
			return new Proxy(function(){}, { 
				apply:function(){
					var args = [].slice.call(arguments)[2];
					if( args.length == 1 && typeof args[0] === typeof {})
						// if only one argument and it is of object type
						args = args[0]; // assume it is parameter by names
					return Call(remote.url,method,args,remote.callback);
					}
				});
		}
	});
	return proxy; 
}

var RPC;
var Arduino;

var RPCErrors = {
  '-32700' : 'An error occurred on the server while parsing the JSON text.',
  '-32600' : 'The JSON sent is not a valid Request object.',
  '-32601' : 'The method does not exist.',
  '-32602' : 'Invalid method parameter(s).',
  '-32603' : 'Internal JSON-RPC server error.',
};

function RPCError(code) {
	return RPCErrors[''+code] || 'Server Error #' + code; 
}

function Callback( id, err, result) {
	var text;
	if( err )
		LogMessage(id, 'error', ':&nbsp;', (err.message || RPCError(err.code)));
	else
		LogMessage(id, 'result', ':&nbsp;', result);
}

function Execute(text) {
	var div = document.createElement('div');
	text = text.trim();
	if( text )
	try {
		var id = eval("RPC." + text);		
		
		if( typeof id == typeof 1 ) {
			var div = LogMessage(id, 'call', '', text);
			var input = $one('#main input');
			if( div && div.firstChild ) div.firstChild.ondblclick = function() {
				input.value = this.innerHTML;
			}
		} else {
			LogMessage(null, 'jserror', '', text + ':&nbsp;Not a function');
		}
	} catch(e) {
		div.innerHTML = LogMessage(undefined, 'jserror', null, text);
		console.log(e); 
	}
}

function BindControls() {
	var input = $one('#main input');
	$arr('.func').forEach(function(c) {
		c.ondblclick = function() {
			input.value += this.innerHTML + '()';
		}
	});
	$arr('.const').forEach(function(c) {
		c.ondblclick = function() {
			input.value += this.innerHTML;
		}
	});
}

function Init() {
	RPC = RemoteService('/rpc');
	RPC(Callback);
	Photon = RemoteService('/rpc');
	Photon(function(id, err,result){ console.log((id && (id + "->")) + 
		(typeof(result)!=typeof(undefined) ? result : err&&(err.message || RPCError(err.code))));});
	
	var copyr = document.createElement('div');
	copyr.setAttribute('id','copyright');
	copyr.innerHTML = 'Copyright &copy; 2017 Eugene Hutorny <span id="copyright-email">@</span>';
	($one('#main')||document.body).append(copyr);
	var input = $one('#main input');
	if( input )
		input.onkeypress = function(e) {
		if( e.code == 'Enter' ) {
			if( this.value )
				Execute(this.value);
			this.value = '';
		}
	}
	var origin = '#' + location.hostname + (location.port && ( ":" + location.port));  
	var right = $one('#right');
	if( right ) {
		right.innerHTML += '<label>About...</label>' +
			'<div><a target="_new" id="about" href="http://r.iot-ware.com/p/about.html'
			+ origin + '">This example</a></div>' +
			'<div><a target="_new" href="http://hutorny.in.ua/projects/micurpc">µcuRPC Library</a></div>' +
			'<div><a target="_new" href="http://www.jsonrpc.org/">JSON-RPC</a></div>' +
			'<label>References...</label>' +
			'<div><a target="_new" href="https://docs.particle.io/reference/firmware/photon/#input-output">Photon API Reference</a></div>'; 
	}
	var left = $one('#left');
	if( left ) 
		GetObj('/meta/rpc', function(err, data){
			if( err ) console.log(err);
			else
			if( typeof data === typeof []) {
				left.innerHTML += '<label>List of Functions</label><div class="func">'
					+ data.join('</div><div class="func">') + '</div>';
			}
			GetObj('/meta/const', function(err, data){
				if( err ) console.log(err);
				else
				if( typeof data === typeof {}) {
					left.innerHTML += '<label>List of Constants</label><div class="const">'
						+ Object.keys(data).join('</div><div class="const">') + '</div>';
					Object.keys(data).forEach(function(c) { window[c] = data[c]; });
				}
				setTimeout(BindControls, 0);
			});
		});
}

Init();