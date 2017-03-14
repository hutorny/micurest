/**
 * main.js - JS script for micurest example application
 */

/**
 * Sugar for query selectors 
 */
function $one(sel) {
	return (typeof sel === 'string') ? document.querySelector(sel) : sel;
};

function logerror(text) {
	var p = document.createElement('div');
	p.innerHTML = text;
	document.body.appendChild(p);
}

function send(method, url, text, type, fn) {
	var xhr = new XMLHttpRequest();
	if( ! fn ) fn = function() {}
	xhr.onreadystatechange = function () {
		if( this.readyState == 4 ) {
			if( this.status >= 200  && this.status <= 204 )
	        	fn(this.responseText, this.getResponseHeader('Content-Type'));
	        else {		        	
		       	logerror(this.statusText);
		       	fn(new Error(this.statusText));
	        }
		}
	};
    xhr.open(method, url);
   	xhr.timeout = 2000;
  	xhr.ontimeout = function () { 
  		logerror('Network timeout');
  		fn(new Error('Network timeout'));
  	};
    if( type !== null ) {
    	xhr.setRequestHeader(method === 'GET'?'Accept':'Content-Type', type);
    }
  	
	xhr.send(text);	
	return xhr;
};

function get(url, fn) {
	return send('GET', url, null, null, fn);
}

function convert(type, value) {
	switch(type) {
	case typeof(0): return Number(value);
	case typeof(false): return Boolean(value);
	}
	return value;
}

function put(url, field, fn) {
	var value = (field.type === 'checkbox') ? field.checked : convert(field.dataType, field.value);
	value = ( field.contentType === 'application/json' || value === null )
				? (JSON.stringify(value) + '\r\n')	:  value.toString();
				// extra CRLF is needed, see micurest.cpp:483 for details
	var type = field.contentType ? field.contentType : 'plain/text';
	send('PUT', url, value, type, fn);					
}

function queue(bind) {		
	if( ! bind  || ! bind.length ) return;
	var e = bind.shift();
	var url = e.id;
	get(url, function(data, type) {
		if( ! (data instanceof Error) ) {
			if( type === 'application/json' )
				try { data = JSON.parse(data) } catch(e) { console.log(e); }
			if( (e.type === 'checkbox') ) {
				e.checked = data;
				e.onchange = function(e) {
					put('/'+this.id, this);
				}
			} else if( e.type === 'select-one' ) {
				e.value = data;
				e.onchange = function() { 
					put('/'+this.id, this);
				}
			} else {
				e.value = data;
				e.onkeypress = function(e) {
					if( e.code == 'Enter' ) {
						put('/'+this.id, this);
					}
				}; 			
			}
			e.disabled = false;
			e.contentType = type;
			e.dataType = typeof(data);
		}
		queue(bind);	
	});
}

queue(Array.apply(null, document.querySelectorAll('.bind')));
